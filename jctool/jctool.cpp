// Copyright (c) 2018 CTCaer. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// TODO: "hdr->timer = timming_byte & 0xF", should this be "timming_byte % 0xF"?
#include <functional>
#include <memory>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
// #define NOMINMAX
#include <chrono>
#ifdef WIN32
#include <cstdio>
#include <Windows.h>
#else
#include <stdio.h>
#include <unistd.h>
inline int Sleep(uint64_t ms){
    return usleep(ms*1000);
};
#include <math.h> // for sqrt
const auto min = [](auto a, auto b){
    return (a < b) ? a : b;
};
#endif

#include "jctool.h"
#include "ir_sensor.h"
#include "tune.h"
#include "jctool_helpers.hpp"
#include "luts.h"

#ifndef __jctool_cpp_API__
#include "FormJoy.h"
using namespace CppWinFormJoy;
#pragma comment(lib, "SetupAPI")
#else
#define EASY_HELPERS_UI
#include "easy-imgui/easy_imgui.h"
#include "jctool_ui.hpp"
#endif

#include "hidapi.h"

#ifndef __jctool_cpp_API__
bool enable_traffic_dump = false;

int  handle_ok;
bool enable_button_test;
bool enable_IRVideoPhoto;
bool enable_IRAutoExposure;
bool enable_NFCScanning;
bool cancel_spi_dump;
bool check_connection_ok;

u8 timming_byte;
u8 ir_max_frag_no;
hid_device *handle;
using namespace System;
hid_device *handle_l;
#endif


s16 uint16_to_int16(u16 a) {
    s16 b;
    char* aPointer = (char*)&a, *bPointer = (char*)&b;
    memcpy(bPointer, aPointer, sizeof(a));
    return b;
}


u16 int16_to_uint16(s16 a) {
    u16 b;
    char* aPointer = (char*)&a, *bPointer = (char*)&b;
    memcpy(bPointer, aPointer, sizeof(a));
    return b;
}


u8 mcu_crc8_calc(u8 *buf, u8 size) {
    u8 crc8 = 0x0;

    for (int i = 0; i < size; ++i) {
        crc8 = mcu_crc8_table[(u8)(crc8 ^ buf[i])];
    }
    return crc8;
}


void decode_stick_params(u16 *decoded_stick_params, u8 *encoded_stick_params) {
    decoded_stick_params[0] = (encoded_stick_params[1] << 8) & 0xF00 | encoded_stick_params[0];
    decoded_stick_params[1] = (encoded_stick_params[2] << 4) | (encoded_stick_params[1] >> 4);
}


void encode_stick_params(u8 *encoded_stick_params, u16 *decoded_stick_params) {
    encoded_stick_params[0] =  decoded_stick_params[0] & 0xFF;
    encoded_stick_params[1] = (decoded_stick_params[0] & 0xF00) >> 8 | (decoded_stick_params[1] & 0xF) << 4;
    encoded_stick_params[2] = (decoded_stick_params[1] & 0xFF0) >> 4;
}


// Credit to Hypersect (Ryan Juckett)
// http://blog.hypersect.com/interpreting-analog-sticks/
void AnalogStickCalc(
    float *pOutX,       // out: resulting stick X value
    float *pOutY,       // out: resulting stick Y value
    u16 x,              // in: initial stick X value
    u16 y,              // in: initial stick Y value
    u16 x_calc[3],      // calc -X, CenterX, +X
    u16 y_calc[3]       // calc -Y, CenterY, +Y
)
{
    float x_f, y_f;
    // Apply Joy-Con center deadzone. 0xAE translates approx to 15%. Pro controller has a 10% deadzone.
    float deadZoneCenter = 0.15f;
    // Add a small ammount of outer deadzone to avoid edge cases or machine variety.
    float deadZoneOuter = 0.10f;

    // convert to float based on calibration and valid ranges per +/-axis
    x = CLAMP(x, x_calc[0], x_calc[2]);
    y = CLAMP(y, y_calc[0], y_calc[2]);
    if (x >= x_calc[1])
        x_f = (float)(x - x_calc[1]) / (float)(x_calc[2] - x_calc[1]);
    else
        x_f = -((float)(x - x_calc[1]) / (float)(x_calc[0] - x_calc[1]));
    if (y >= y_calc[1])
        y_f = (float)(y - y_calc[1]) / (float)(y_calc[2] - y_calc[1]);
    else
        y_f = -((float)(y - y_calc[1]) / (float)(y_calc[0] - y_calc[1]));

    // Interpolate zone between deadzones
    float mag = sqrtf(x_f*x_f + y_f*y_f);
    if (mag > deadZoneCenter) {
        // scale such that output magnitude is in the range [0.0f, 1.0f]
        float legalRange = 1.0f - deadZoneOuter - deadZoneCenter;
        float normalizedMag = min(1.0f, (mag - deadZoneCenter) / legalRange);
        float scale = normalizedMag / mag;
        pOutX[0] = x_f * scale;
        pOutY[0] = y_f * scale;
    }
    else
    {
        // stick is in the inner dead zone
        pOutX[0] = 0.0f;
        pOutY[0] = 0.0f;
    }
}

/**
 * ===================
 * Controller Commands
 * ===================
 * https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/bluetooth_hid_notes.md
 */
namespace ConCom {
    /**
     * Command
     */
    enum Com {
        SUBC = 0x01, // Subcommand Type
        RUM0 = 0x10, // Rumble Type
        UNK0 = 0x12, // Unknown
        UNK1 = 0x28, 
    };
    /**
     * Output Reports Subcommand
     */
    enum OutSub {
        OMCU = 0x03, // Output Report NFC/IR MCU FW update packet
        RUM1 = 0x10, // Rumble
        RMCU = 0x11, // Request data from NFC/IR
    };

    /**
     * Input Reports Subcommand
     */
    enum InSub {
        CTRL = 0x3F, // Controller data packet
        STD0 = 0x21, // Standard input report, subcommand reply.
        IMCU = 0x23, // Input Report NFC/IR MCU FW update packet
        FULM = 0x30, // Full mode (60hz joycon)/(120hz procon)
        MCUM = 0x31, // A large packet with standard input report and NFC/IR MCU data
        STD1 = 0x32,
        STD2 = 0x33,
    };

    /**
     * [Send] Feature report
     */
    enum Feature {
        LAST = 0x02, // Get Last subcommand reply
        OFWU = 0x70, // Enable OTA FW Upgrade. Unlocks erase/write memory commands. (MUST SEND ONE BYTE)
        MEMR = 0x71, // Setup memory read.
    };

    namespace FeatureN {
        inline int enable_OTAFW_upgrade(controller_hid_handle_t handle){
            return -1;
            {
                static constexpr u8 send = OFWU;
                // Will not work.
                int res = hid_write(handle, &send, 1);
                return res;
            }
        }
        inline int setup_mem_read(controller_hid_handle_t handle){
            // TODO:
            return -1;
        }
        inline int get_last_reply(controller_hid_handle_t handle){
            return -1;
        }
    }

    class Packet {
    public:
        u8 buf[49];
        static constexpr int buf_size = sizeof(buf);

        inline brcm_hdr*& header() {return this->hdr; }
        inline brcm_cmd_01*& command() { return this->cmd; }
        inline void zero() { memset(this->buf, 0, sizeof(this->buf)); }

        inline Packet():
        hdr{(brcm_hdr*)this->buf},
        cmd{(brcm_cmd_01*)(this->buf + sizeof(brcm_hdr))},
        buf{}
        {}
    private:
        brcm_hdr* hdr;
        brcm_cmd_01* cmd;
    };

    /**
     * REMINDER: The timming byte gets incremented here!
     */
    int send_pkt(CT& ct, Packet& pkt){
        ct.timming_byte++;
        return hid_write(ct.handle, pkt.buf, sizeof(pkt.buf));
    }
};

#ifndef __jctool_cpp_API__
int set_led_busy() {
#else
int set_led_busy(CT& ct, ConHID::ProdID con_type) {
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
#endif
    int res;
    ConCom::Packet p;

    //p.zero();
    auto& hdr = p.header();
    auto& pkt = p.command();
    hdr->cmd = 0x01;
    hdr->timer = timming_byte & 0xF;
    pkt->subcmd = 0x30;
    pkt->subcmd_arg.arg1 = 0x81;
    res = ConCom::send_pkt(ct, p);
    res = hid_read_timeout(handle, p.buf, 1, 64);

    //Set breathing HOME Led
#ifndef __jctool_cpp_API__
    if (handle_ok != 1)
#else
    if(con_type != ConHID::JoyConLeft)
#endif
    {
        p.zero();
        hdr->cmd = 0x01;
        hdr->timer = timming_byte & 0xF;
        pkt->subcmd = 0x38;
        pkt->subcmd_arg.arg1 = 0x28;
        pkt->subcmd_arg.arg2 = 0x20;
        p.buf[13] = 0xF2;
        p.buf[14] = p.buf[15] = 0xF0;
        res = ConCom::send_pkt(ct, p);
        res = hid_read_timeout(handle, p.buf, 1, 64);
    }

    return 0;
}

#ifndef __jctool_cpp_API__
std::string get_sn(u32 offset, const u16 read_len) {
#else
std::string get_sn(CT& ct) {
    static const u32 offset = 0x6001;
    static const u16 read_len = 0xF;
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
#endif
    int res;
    int error_reading = 0;
    u8 buf[49];
    std::string test = "";
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 1;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x10;
        pkt->spi_data.offset = offset;
        pkt->spi_data.size = read_len;
        res = hid_write(handle, buf, sizeof(buf));

        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if ((*(u16*)&buf[0xD] == 0x1090) && (*(uint32_t*)&buf[0xF] == offset))
                goto check_result;

            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 20)
            return "Error!";
    }
    check_result:
    if (res >= 0x14 + read_len) {
        for (int i = 0; i < read_len; i++) {
            if (buf[0x14 + i] != 0x000) {
                test += buf[0x14 + i];
            }else
                test += "";
            }
    }
    else {
        return "Error!";
    }
    return test;
}

#ifndef __jctool_cpp_API__
int get_spi_data(u32 offset, const u16 read_len, u8 *test_buf) {
#else
int get_spi_data(CT& ct, u32 offset, const u16 read_len, u8 * test_buf) {
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
#endif
    int res;
    u8 buf[49];
    int error_reading = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 1;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x10;
        pkt->spi_data.offset = offset;
        pkt->spi_data.size = read_len;
        res = hid_write(handle, buf, sizeof(buf));

        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if ((*(u16*)&buf[0xD] == 0x1090) && (*(uint32_t*)&buf[0xF] == offset))
                goto check_result;

            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 20)
            return 1;
    }
    check_result:
    if (res >= 0x14 + read_len) {
            for (int i = 0; i < read_len; i++) {
                test_buf[i] = buf[0x14 + i];
            }
    }
    
    return 0;
}

#ifndef __jctool_cpp_API__
int write_spi_data(u32 offset, const u16 write_len, u8* test_buf) {
#else
int write_spi_data(CT& ct, u32 offset, const u16 write_len, u8* test_buf) {
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
#endif
    int res;
    u8 buf[49];
    int error_writing = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 1;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x11;
        pkt->spi_data.offset = offset;
        pkt->spi_data.size = write_len;
        for (int i = 0; i < write_len; i++)
            buf[0x10 + i] = test_buf[i];

        res = hid_write(handle, buf, sizeof(buf));
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (*(u16*)&buf[0xD] == 0x1180)
                goto check_result;

            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_writing++;
        if (error_writing == 20)
            return 1;
    }
    check_result:
    return 0;
}

#ifndef __jctool_cpp_API__
int get_device_info(u8* test_buf) {
#else
int get_device_info(CT& ct, u8* test_buf) {
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
#endif
    int res;
    u8 buf[49];
    int error_reading = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 1;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x02;
        res = hid_write(handle, buf, sizeof(buf));
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);        
            if (*(u16*)&buf[0xD] == 0x0282)
                goto check_result;

            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 20)
            break;
    }
    check_result:
    for (int i = 0; i < 0xA; i++) {
        test_buf[i] = buf[0xF + i];
    }

    return 0;
}

#ifndef __jctool_cpp_API__
int get_battery(u8* test_buf) {
#else
int get_battery(CT& ct, u8* test_buf) {
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
#endif
    int res;
    u8 buf[49];
    int error_reading = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 1;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x50;
        res = hid_write(handle, buf, sizeof(buf));
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (*(u16*)&buf[0xD] == 0x50D0)
                goto check_result;

            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 20)
            break;
    }
    check_result:
    test_buf[0] = buf[0x2];
    test_buf[1] = buf[0xF];
    test_buf[2] = buf[0x10];

    return 0;
}

#ifndef __jctool_cpp_API__
int get_temperature(u8* test_buf) {
#else
int get_temperature(CT& ct, u8* test_buf) {
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
#endif
    int res;
    u8 buf[49];
    int error_reading = 0;
    bool imu_changed = false;

    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 1;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x43;
        pkt->subcmd_arg.arg1 = 0x10;
        pkt->subcmd_arg.arg2 = 0x01;
        res = hid_write(handle, buf, sizeof(buf));
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (*(u16*)&buf[0xD] == 0x43C0)
                goto check_result;

            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 20)
            break;
    }
    check_result:
    if ((buf[0x11] >> 4) == 0x0) {

        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x01;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x40;
        pkt->subcmd_arg.arg1 = 0x01;
        res = hid_write(handle, buf, sizeof(buf));
        res = hid_read_timeout(handle, buf, 1, 64);

        imu_changed = true;

        // Let temperature sensor stabilize for a little bit.
        Sleep(64);
    }
    error_reading = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 1;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x43;
        pkt->subcmd_arg.arg1 = 0x20;
        pkt->subcmd_arg.arg2 = 0x02;
        res = hid_write(handle, buf, sizeof(buf));
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (*(u16*)&buf[0xD] == 0x43C0)
                goto check_result2;

            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 20)
            break;
    }
    check_result2:
    test_buf[0] = buf[0x11];
    test_buf[1] = buf[0x12];

    if (imu_changed) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x01;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x40;
        pkt->subcmd_arg.arg1 = 0x00;
        res = hid_write(handle, buf, sizeof(buf));
        res = hid_read_timeout(handle, buf, 1, 64);
    }

    return 0;
}

#ifndef __jctool_cpp_API__
int dump_spi(const char *dev_name) {
#else
int dump_spi(CT& ct, DumpSPICTX& dump_spi_ctx) {
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
    const char*& dev_name = dump_spi_ctx.file_name;
    bool& cancel_spi_dump = dump_spi_ctx.cancel_spi_dump;
#endif
    int error_reading = 0;
    std::string file_dev_name = dev_name;
#ifndef __jctool_cpp_API__
    String^ filename_sys = gcnew String(file_dev_name.c_str());
#endif
    file_dev_name = "./" + file_dev_name;

    FILE *f;

#ifdef WIN32
    errno_t err;
    if ((err = fopen_s(&f, file_dev_name.c_str(), "wb")) != 0) {
#elif defined(__linux__)
    if ((f = fopen(file_dev_name.c_str(), "wb")) == nullptr) {
#endif
#ifndef __jctool_cpp_API__
        MessageBox::Show(L"Cannot open file " + filename_sys + L" for writing!\n\nError: " + err, L"Error opening file!", MessageBoxButtons::OK ,MessageBoxIcon::Exclamation);
#endif
        
        return 1;
    }

    int res;
    u8 buf[49];
    
    u16 read_len = 0x1d;
    u32 offset = 0x0;
    while (offset < SPI_SIZE && !cancel_spi_dump) {
        error_reading = 0;
#ifndef __jctool_cpp_API__
        std::stringstream offset_label;
        offset_label << std::fixed << std::setprecision(2) << std::setfill(' ') << offset/1024.0f;
        offset_label << "KB of 512KB";
        FormJoy::myform1->label_progress->Text = gcnew String(offset_label.str().c_str());
        Application::DoEvents();
#endif

        while(1) {
            memset(buf, 0, sizeof(buf));
            auto hdr = (brcm_hdr *)buf;
            auto pkt = (brcm_cmd_01 *)(hdr + 1);
            hdr->cmd = 1;
            hdr->timer = timming_byte & 0xF;
            timming_byte++;
            pkt->subcmd = 0x10;
            pkt->spi_data.offset = offset;
            pkt->spi_data.size = read_len;
            res = hid_write(handle, buf, sizeof(buf));
            int retries = 0;
            while (1) {
                res = hid_read_timeout(handle, buf, sizeof(buf), 64);
                if ((*(u16*)&buf[0xD] == 0x1090) && (*(uint32_t*)&buf[0xF] == offset))
                    goto check_result;

                retries++;
                if (retries > 8 || res == 0)
                    break;
            }
            if (retries > 8)
                error_reading++;
            if (error_reading > 10) {
                fclose(f);
                return 1;
            }     
        }
        check_result:
        fwrite(buf + 0x14, read_len, 1, f);
        offset += read_len;
        if (offset == 0x7FFE6)
            read_len = 0x1A;
#ifdef __jctool_cpp_API__
        dump_spi_ctx.bytes_dumped = offset;
#endif
    }
    fclose(f);

    return 0;
}

namespace Rumble {
    constexpr int size_write = 40;
    constexpr int timeout_ms = 64;
    int enable_rumble(CT& ct, ConCom::Packet& packet_buf){
        controller_hid_handle_t& handle = ct.handle;
        u8& timming_byte = ct.timming_byte;
        packet_buf.zero();
        auto& hdr = packet_buf.header();
        auto& pkt = packet_buf.command();
        hdr->cmd = 0x01;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x48;
        pkt->subcmd_arg.arg1 = 0x01;
        int res = hid_write(handle, packet_buf.buf, size_write);
        if(res < 0)
            return res;
    }
}

#ifndef __jctool_cpp_API__
int send_rumble() {
#else
int send_rumble(CT& ct, ConHID::ProdID con_type) {
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
#endif
    int res;
    ConCom::Packet p;
    u8 buf2[49];
    
    //Enable Vibration
    Rumble::enable_rumble(ct, p);
    res = hid_read_timeout(handle, buf2, 1, 64);

    auto& hdr = p.header();
    auto& pkt = p.command();
    //New vibration like switch
    Sleep(16);
    //Send confirmation 
    p.zero();
    hdr->cmd = 0x01;
    hdr->timer = timming_byte & 0xF;
    hdr->rumble_l[0] = 0xc2;
    hdr->rumble_l[1] = 0xc8;
    hdr->rumble_l[2] = 0x03;
    hdr->rumble_l[3] = 0x72;
    memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
    res = ConCom::send_pkt(ct,p);
    res = hid_read_timeout(handle, buf2, 1, 64);

    Sleep(81);

    hdr->timer = timming_byte & 0xF;
    hdr->rumble_l[0] = 0x00;
    hdr->rumble_l[1] = 0x01;
    hdr->rumble_l[2] = 0x40;
    hdr->rumble_l[3] = 0x40;
    memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
    res = ConCom::send_pkt(ct,p);
    res = hid_read_timeout(handle, buf2, 1, 64);

    Sleep(5);

    hdr->timer = timming_byte & 0xF;
    hdr->rumble_l[0] = 0xc3;
    hdr->rumble_l[1] = 0xc8;
    hdr->rumble_l[2] = 0x60;
    hdr->rumble_l[3] = 0x64;
    memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
    res = ConCom::send_pkt(ct,p);
    res = hid_read_timeout(handle, buf2, 1, 64);

    Sleep(5);

    //Disable vibration
    p.zero();
    hdr->cmd = 0x01;
    hdr->timer = timming_byte & 0xF;
    hdr->rumble_l[0] = 0x00;
    hdr->rumble_l[1] = 0x01;
    hdr->rumble_l[2] = 0x40;
    hdr->rumble_l[3] = 0x40;
    memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
    pkt->subcmd = 0x48;
    pkt->subcmd_arg.arg1 = 0x00;
    res = ConCom::send_pkt(ct,p);
    res = hid_read_timeout(handle, p.buf, 1, 64);

    p.zero();
    hdr->cmd = 0x01;
    hdr->timer = timming_byte & 0xF;
    pkt->subcmd = 0x30;
    pkt->subcmd_arg.arg1 = 0x01;
    res = ConCom::send_pkt(ct,p);
    res = hid_read_timeout(handle, p.buf, 1, 64);

    // Set HOME Led
#ifndef __jctool_cpp_API__
    if (handle_ok != 1)
#else
    if(con_type != ConHID::JoyConLeft)
#endif
    {
        p.zero();
        hdr->cmd = 0x01;
        hdr->timer = timming_byte & 0xF;
        pkt->subcmd = 0x38;
        // Heartbeat style configuration
        p.buf[11] = 0xF1;
        p.buf[12] = 0x00;
        p.buf[13] = p.buf[14] = p.buf[15] = p.buf[16] = p.buf[17] = p.buf[18] = 0xF0;
        p.buf[19] = p.buf[22] = p.buf[25] = p.buf[28] = p.buf[31] = 0x00;
        p.buf[20] = p.buf[21] = p.buf[23] = p.buf[24] = p.buf[26] = p.buf[27] = p.buf[29] = p.buf[30] = p.buf[32] = p.buf[33] = 0xFF;
        res = ConCom::send_pkt(ct,p);
        res = hid_read_timeout(handle, p.buf, 1, 64);
    }

    return 0;
}

#ifndef __jctool_cpp_API__
int send_custom_command(u8* arg) {
#else
int send_custom_command(CT& ct, u8* arg){
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
#endif
    int res_write;
    int res;
    int byte_seperator = 1;
#ifndef __jctool_cpp_API__
    String^ input_report_cmd;
    String^ input_report_sys;
    String^ output_report_sys;
#else
    std::ostringstream input_report_cmd;
    std::ostringstream input_report_sys;
    std::ostringstream output_report_sys;
#endif
    u8 buf_cmd[49];
    u8 buf_reply[0x170];
    memset(buf_cmd, 0, sizeof(buf_cmd));
    memset(buf_reply, 0, sizeof(buf_reply));

    buf_cmd[0] = arg[0]; // cmd
    buf_cmd[1] = timming_byte & 0xF;
    timming_byte++;
    // Vibration pattern
    buf_cmd[2] = buf_cmd[6] = arg[1];
    buf_cmd[3] = buf_cmd[7] = arg[2];
    buf_cmd[4] = buf_cmd[8] = arg[3];
    buf_cmd[5] = buf_cmd[9] = arg[4];

    buf_cmd[10] = arg[5]; // subcmd

    // subcmd x21 crc byte
    if (arg[5] == 0x21)
        arg[43] = mcu_crc8_calc(arg + 7, 36);
#ifndef __jctool_cpp_API__
    output_report_sys = String::Format(L"Cmd:  {0:X2}   Subcmd: {1:X2}\r\n", buf_cmd[0], buf_cmd[10]);
#else
    // TODO: Implement else
    //output_report_sys << "Cmd: " << std::setbase(hex) << buf_cmd[0]
#endif
    if (buf_cmd[0] == 0x01 || buf_cmd[0] == 0x10 || buf_cmd[0] == 0x11) {
        for (int i = 6; i < 44; i++) {
            buf_cmd[5 + i] = arg[i];
#ifndef __jctool_cpp_API__
            output_report_sys += String::Format(L"{0:X2} ", buf_cmd[5 + i]);
            if (byte_seperator == 4)
                output_report_sys += L" ";
#else
            // TODO: Implement else
#endif
            if (byte_seperator == 8) {
                byte_seperator = 0;
#ifndef __jctool_cpp_API__
                output_report_sys += L"\r\n";
#else
                // TODO: Implement else
#endif
            }
            byte_seperator++;
        }
    }
    //Use subcmd after command
    else {
        for (int i = 6; i < 44; i++) {
            buf_cmd[i - 5] = arg[i];
#ifndef __jctool_cpp_API__
            output_report_sys += String::Format(L"{0:X2} ", buf_cmd[i - 5]);
            if (byte_seperator == 4)
                output_report_sys += L" ";
#else
            // TODO: Implement else
#endif
            if (byte_seperator == 8) {
                byte_seperator = 0;
#ifndef __jctool_cpp_API__
                output_report_sys += L"\r\n";
#else
                // TODO: Implement else
#endif
            }
            byte_seperator++;
        }
    }
#ifndef __jctool_cpp_API__
    FormJoy::myform1->textBoxDbg_sent->Text = output_report_sys;
#else
    // TODO: Implement else
#endif

    //Packet size header + subcommand and uint8 argument
    res_write = hid_write(handle, buf_cmd, sizeof(buf_cmd));
#ifndef __jctool_cpp_API__
    if (res_write < 0)
        input_report_sys += L"hid_write failed!\r\n\r\n";
#else
    // TODO: Implement else
#endif
    int retries = 0;
    while (1) {
        res = hid_read_timeout(handle, buf_reply, sizeof(buf_reply), 64);

        if (res > 0) {
            if (arg[0] == 0x01 && buf_reply[0] == 0x21)
                break;
            else if (arg[0] != 0x01)
                break;
        }

        retries++;
        if (retries == 20)
            break;
    }
    byte_seperator = 1;
    if (res > 12) {
        if (buf_reply[0] == 0x21 || buf_reply[0] == 0x30 || buf_reply[0] == 0x33 || buf_reply[0] == 0x31 || buf_reply[0] == 0x3F) {
#ifndef __jctool_cpp_API__
            input_report_cmd += String::Format(L"\r\nInput report: 0x{0:X2}\r\n", buf_reply[0]);
            input_report_sys += String::Format(L"Subcmd Reply:\r\n", buf_reply[0]);
#else
            // TODO: Implement else
#endif
            int len = 49;
            if (buf_reply[0] == 0x33 || buf_reply[0] == 0x31)
                len = 362;
            for (int i = 1; i < 13; i++) {
#ifndef __jctool_cpp_API__
                input_report_cmd += String::Format(L"{0:X2} ", buf_reply[i]);
                if (byte_seperator == 4)
                    input_report_cmd += L" ";
#else
                // TODO: Implement else
#endif
                if (byte_seperator == 8) {
                    byte_seperator = 0;
#ifndef __jctool_cpp_API__
                    input_report_cmd += L"\r\n";
#else
                    // TODO: Implement else
#endif
                }
                byte_seperator++;
            }
            byte_seperator = 1;
            for (int i = 13; i < len; i++) {
#ifndef __jctool_cpp_API__
                input_report_sys += String::Format(L"{0:X2} ", buf_reply[i]);
                if (byte_seperator == 4)
                    input_report_sys += L" ";
#else
                // TODO: Implement else
#endif
                if (byte_seperator == 8) {
                    byte_seperator = 0;
#ifndef __jctool_cpp_API__
                    input_report_sys += L"\r\n";
#else
                    // TODO: Implement else
#endif
                }
                byte_seperator++;
            }
            int crc_check_ok = 0;
            if (arg[5] == 0x21) {
                crc_check_ok = (buf_reply[48] == mcu_crc8_calc(buf_reply + 0xF, 33));
#ifndef __jctool_cpp_API__
                if (crc_check_ok)
                    input_report_sys += L"(CRC OK)";
                else
                    input_report_sys += L"(Wrong CRC)";
#else
                // TODO: Implement else
#endif
            }
        }
        else {
#ifndef __jctool_cpp_API__
            input_report_sys += String::Format(L"ID: {0:X2} Subcmd reply:\r\n", buf_reply[0]);
            for (int i = 13; i < res; i++) {
                input_report_sys += String::Format(L"{0:X2} ", buf_reply[i]);
                if (byte_seperator == 4)
                    input_report_sys += L" ";
                if (byte_seperator == 8) {
                    byte_seperator = 0;
                    input_report_sys += L"\r\n";
                }
                byte_seperator++;
            }
#else
            // TODO: Implement else
#endif
        }
    }
    else if (res > 0 && res <= 12) {
#ifndef __jctool_cpp_API__
        for (int i = 0; i < res; i++)
            input_report_sys += String::Format(L"{0:X2} ", buf_reply[i]);
#else
        // TODO: Implement else
#endif
    }
    else {
#ifndef __jctool_cpp_API__
        input_report_sys += L"No reply";
#else
        // TODO: Implement else
#endif
    }
#ifndef __jctool_cpp_API__
    FormJoy::myform1->textBoxDbg_reply->Text = input_report_sys;
    FormJoy::myform1->textBoxDbg_reply_cmd->Text = input_report_cmd;
#else
    // TODO: Implement else
#endif

    return 0;
}

#ifndef __jctool_cpp_API__
int button_test() {
    int res;
    int limit_output = 0;
    String^ input_report_cmd;
    String^ input_report_sys;
    u8 buf_cmd[49];
    u8 buf_reply[0x170];
    float acc_cal_coeff[3];
    float gyro_cal_coeff[3];
    float cal_x[1] = { 0.0f };
    float cal_y[1] = { 0.0f };

    bool has_user_cal_stick_l = false;
    bool has_user_cal_stick_r = false;
    bool has_user_cal_sensor = false;

    u8 factory_stick_cal[0x12];
    u8 user_stick_cal[0x16];
    u8 sensor_model[0x6];
    u8 stick_model[0x24];
    u8 factory_sensor_cal[0x18];
    u8 user_sensor_cal[0x1A];
    s16 sensor_cal[0x2][0x3];
    u16 stick_cal_x_l[0x3];
    u16 stick_cal_y_l[0x3];
    u16 stick_cal_x_r[0x3];
    u16 stick_cal_y_r[0x3];
    memset(factory_stick_cal, 0, 0x12);
    memset(user_stick_cal, 0, 0x16);
    memset(sensor_model, 0, 0x6);
    memset(stick_model, 0, 0x12);
    memset(factory_sensor_cal, 0, 0x18);
    memset(user_sensor_cal, 0, 0x1A);
    memset(sensor_cal, 0, sizeof(sensor_cal));
    memset(stick_cal_x_l, 0, sizeof(stick_cal_x_l));
    memset(stick_cal_y_l, 0, sizeof(stick_cal_y_l));
    memset(stick_cal_x_r, 0, sizeof(stick_cal_x_r));
    memset(stick_cal_y_r, 0, sizeof(stick_cal_y_r));
    get_spi_data(0x6020, 0x18, factory_sensor_cal);
    get_spi_data(0x603D, 0x12, factory_stick_cal);
    get_spi_data(0x6080, 0x6, sensor_model);
    get_spi_data(0x6086, 0x12, stick_model);
    get_spi_data(0x6098, 0x12, &stick_model[0x12]);
    get_spi_data(0x8010, 0x16, user_stick_cal);
    get_spi_data(0x8026, 0x1A, user_sensor_cal);

    // Analog Stick device parameters
    FormJoy::myform1->txtBox_devParameters->Text = String::Format(L"Flat surface ACC Offset:\r\n{0:X4} {1:X4} {2:X4}\r\n\r\n\r\nStick Parameters:\r\n{3:X3} {4:X3}\r\n{5:X2} (Deadzone)\r\n{6:X3} (Range ratio)",
        sensor_model[0] | sensor_model[1] << 8,
        sensor_model[2] | sensor_model[3] << 8,
        sensor_model[4] | sensor_model[5] << 8,
        (stick_model[1] << 8) & 0xF00 | stick_model[0], (stick_model[2] << 4) | (stick_model[1] >> 4),
        (stick_model[4] << 8) & 0xF00 | stick_model[3],
        ((stick_model[5] << 4) | (stick_model[4] >> 4)));

    for (int i = 0; i < 10; i = i + 3) {
        FormJoy::myform1->txtBox_devParameters->Text += String::Format(L"\r\n{0:X3} {1:X3}",
            (stick_model[7 + i] << 8) & 0xF00 | stick_model[6 + i],
            (stick_model[8 + i] << 4) | (stick_model[7 + i] >> 4));
    }

    FormJoy::myform1->txtBox_devParameters2->Text = String::Format(L"Stick Parameters 2:\r\n{0:X3} {1:X3}\r\n{2:X2} (Deadzone)\r\n{3:X3} (Range ratio)",
        (stick_model[19] << 8) & 0xF00 | stick_model[18], (stick_model[20] << 4) | (stick_model[19] >> 4),
        (stick_model[22] << 8) & 0xF00 | stick_model[21],
        ((stick_model[23] << 4) | (stick_model[22] >> 4)));

    for (int i = 0; i < 10; i = i + 3) {
        FormJoy::myform1->txtBox_devParameters2->Text += String::Format(L"\r\n{0:X3} {1:X3}",
            (stick_model[25 + i] << 8) & 0xF00 | stick_model[24 + i],
            (stick_model[26 + i] << 4) | (stick_model[25 + i] >> 4));
    }

    // Stick calibration
    if (handle_ok != 2) {
        stick_cal_x_l[1] = (factory_stick_cal[4] << 8) & 0xF00 | factory_stick_cal[3];
        stick_cal_y_l[1] = (factory_stick_cal[5] << 4) | (factory_stick_cal[4] >> 4);
        stick_cal_x_l[0] = stick_cal_x_l[1] - ((factory_stick_cal[7] << 8) & 0xF00 | factory_stick_cal[6]);
        stick_cal_y_l[0] = stick_cal_y_l[1] - ((factory_stick_cal[8] << 4) | (factory_stick_cal[7] >> 4));
        stick_cal_x_l[2] = stick_cal_x_l[1] + ((factory_stick_cal[1] << 8) & 0xF00 | factory_stick_cal[0]);
        stick_cal_y_l[2] = stick_cal_y_l[1] + ((factory_stick_cal[2] << 4) | (factory_stick_cal[2] >> 4));
        FormJoy::myform1->textBox_lstick_fcal->Text = String::Format(L"L Stick Factory:\r\nCenter X,Y: ({0:X3}, {1:X3})\r\nX: [{2:X3} - {4:X3}] Y: [{3:X3} - {5:X3}]",
            stick_cal_x_l[1], stick_cal_y_l[1], stick_cal_x_l[0], stick_cal_y_l[0], stick_cal_x_l[2], stick_cal_y_l[2]);
    }
    else {
        FormJoy::myform1->textBox_lstick_fcal->Text = L"L Stick Factory:\r\nNo calibration";
    }
    if (handle_ok != 1) {
        stick_cal_x_r[1] = (factory_stick_cal[10] << 8) & 0xF00 | factory_stick_cal[9];
        stick_cal_y_r[1] = (factory_stick_cal[11] << 4) | (factory_stick_cal[10] >> 4);
        stick_cal_x_r[0] = stick_cal_x_r[1] - ((factory_stick_cal[13] << 8) & 0xF00 | factory_stick_cal[12]);
        stick_cal_y_r[0] = stick_cal_y_r[1] - ((factory_stick_cal[14] << 4) | (factory_stick_cal[13] >> 4));
        stick_cal_x_r[2] = stick_cal_x_r[1] + ((factory_stick_cal[16] << 8) & 0xF00 | factory_stick_cal[15]);
        stick_cal_y_r[2] = stick_cal_y_r[1] + ((factory_stick_cal[17] << 4) | (factory_stick_cal[16] >> 4));
        FormJoy::myform1->textBox_rstick_fcal->Text = String::Format(L"R Stick Factory:\r\nCenter X,Y: ({0:X3}, {1:X3})\r\nX: [{2:X3} - {4:X3}] Y: [{3:X3} - {5:X3}]",
            stick_cal_x_r[1], stick_cal_y_r[1],    stick_cal_x_r[0], stick_cal_y_r[0],    stick_cal_x_r[2], stick_cal_y_r[2]);
    }
    else {
        FormJoy::myform1->textBox_rstick_fcal->Text = L"R Stick Factory:\r\nNo calibration";
    }

    if ((user_stick_cal[0] | user_stick_cal[1] << 8) == 0xA1B2) {
        stick_cal_x_l[1] = (user_stick_cal[6] << 8) & 0xF00 | user_stick_cal[5];
        stick_cal_y_l[1] = (user_stick_cal[7] << 4) | (user_stick_cal[6] >> 4);
        stick_cal_x_l[0] = stick_cal_x_l[1] - ((user_stick_cal[9] << 8) & 0xF00 | user_stick_cal[8]);
        stick_cal_y_l[0] = stick_cal_y_l[1] - ((user_stick_cal[10] << 4) | (user_stick_cal[9] >> 4));
        stick_cal_x_l[2] = stick_cal_x_l[1] + ((user_stick_cal[3] << 8) & 0xF00 | user_stick_cal[2]);
        stick_cal_y_l[2] = stick_cal_y_l[1] + ((user_stick_cal[4] << 4) | (user_stick_cal[3] >> 4));
        FormJoy::myform1->textBox_lstick_ucal->Text = String::Format(L"L Stick User:\r\nCenter X,Y: ({0:X3}, {1:X3})\r\nX: [{2:X3} - {4:X3}] Y: [{3:X3} - {5:X3}]",
            stick_cal_x_l[1], stick_cal_y_l[1], stick_cal_x_l[0], stick_cal_y_l[0], stick_cal_x_l[2], stick_cal_y_l[2]);
    }
    else {
        FormJoy::myform1->textBox_lstick_ucal->Text = L"L Stick User:\r\nNo calibration";
    }
    if ((user_stick_cal[0xB] | user_stick_cal[0xC] << 8) == 0xA1B2) {
        stick_cal_x_r[1] = (user_stick_cal[14] << 8) & 0xF00 | user_stick_cal[13];
        stick_cal_y_r[1] = (user_stick_cal[15] << 4) | (user_stick_cal[14] >> 4);
        stick_cal_x_r[0] = stick_cal_x_r[1] - ((user_stick_cal[17] << 8) & 0xF00 | user_stick_cal[16]);
        stick_cal_y_r[0] = stick_cal_y_r[1] - ((user_stick_cal[18] << 4) | (user_stick_cal[17] >> 4));
        stick_cal_x_r[2] = stick_cal_x_r[1] + ((user_stick_cal[20] << 8) & 0xF00 | user_stick_cal[19]);
        stick_cal_y_r[2] = stick_cal_y_r[1] + ((user_stick_cal[21] << 4) | (user_stick_cal[20] >> 4));
        FormJoy::myform1->textBox_rstick_ucal->Text = String::Format(L"R Stick User:\r\nCenter X,Y: ({0:X3}, {1:X3})\r\nX: [{2:X3} - {4:X3}] Y: [{3:X3} - {5:X3}]",
            stick_cal_x_r[1], stick_cal_y_r[1], stick_cal_x_r[0], stick_cal_y_r[0], stick_cal_x_r[2], stick_cal_y_r[2]);
    }
    else {
        FormJoy::myform1->textBox_rstick_ucal->Text = L"R Stick User:\r\nNo calibration";
    }

    // Sensor calibration
    FormJoy::myform1->textBox_6axis_cal->Text = L"6-Axis Factory (XYZ):\r\nAcc:  ";
    for (int i = 0; i < 0xC; i = i + 6) {
        FormJoy::myform1->textBox_6axis_cal->Text += String::Format(L"{0:X4} {1:X4} {2:X4}\r\n      ",
            factory_sensor_cal[i + 0] | factory_sensor_cal[i + 1] << 8,
            factory_sensor_cal[i + 2] | factory_sensor_cal[i + 3] << 8,
            factory_sensor_cal[i + 4] | factory_sensor_cal[i + 5] << 8);
    }
    // Acc cal origin position
    sensor_cal[0][0] = uint16_to_int16(factory_sensor_cal[0] | factory_sensor_cal[1] << 8);
    sensor_cal[0][1] = uint16_to_int16(factory_sensor_cal[2] | factory_sensor_cal[3] << 8);
    sensor_cal[0][2] = uint16_to_int16(factory_sensor_cal[4] | factory_sensor_cal[5] << 8);

    FormJoy::myform1->textBox_6axis_cal->Text += L"\r\nGyro: ";
    for (int i = 0xC; i < 0x18; i = i + 6) {
        FormJoy::myform1->textBox_6axis_cal->Text += String::Format(L"{0:X4} {1:X4} {2:X4}\r\n      ",
            factory_sensor_cal[i + 0] | factory_sensor_cal[i + 1] << 8,
            factory_sensor_cal[i + 2] | factory_sensor_cal[i + 3] << 8,
            factory_sensor_cal[i + 4] | factory_sensor_cal[i + 5] << 8);
    }
    // Gyro cal origin position
    sensor_cal[1][0] = uint16_to_int16(factory_sensor_cal[0xC] | factory_sensor_cal[0xD] << 8);
    sensor_cal[1][1] = uint16_to_int16(factory_sensor_cal[0xE] | factory_sensor_cal[0xF] << 8);
    sensor_cal[1][2] = uint16_to_int16(factory_sensor_cal[0x10] | factory_sensor_cal[0x11] << 8);

    if ((user_sensor_cal[0x0] | user_sensor_cal[0x1] << 8) == 0xA1B2) {
        FormJoy::myform1->textBox_6axis_ucal->Text = L"6-Axis User (XYZ):\r\nAcc:  ";
        for (int i = 0; i < 0xC; i = i + 6) {
            FormJoy::myform1->textBox_6axis_ucal->Text += String::Format(L"{0:X4} {1:X4} {2:X4}\r\n      ",
                user_sensor_cal[i + 2] | user_sensor_cal[i + 3] << 8,
                user_sensor_cal[i + 4] | user_sensor_cal[i + 5] << 8,
                user_sensor_cal[i + 6] | user_sensor_cal[i + 7] << 8);
        }
        // Acc cal origin position
        sensor_cal[0][0] = uint16_to_int16(user_sensor_cal[2] | user_sensor_cal[3] << 8);
        sensor_cal[0][1] = uint16_to_int16(user_sensor_cal[4] | user_sensor_cal[5] << 8);
        sensor_cal[0][2] = uint16_to_int16(user_sensor_cal[6] | user_sensor_cal[7] << 8);
        FormJoy::myform1->textBox_6axis_ucal->Text += L"\r\nGyro: ";
        for (int i = 0xC; i < 0x18; i = i + 6) {
            FormJoy::myform1->textBox_6axis_ucal->Text += String::Format(L"{0:X4} {1:X4} {2:X4}\r\n      ",
                user_sensor_cal[i + 2] | user_sensor_cal[i + 3] << 8,
                user_sensor_cal[i + 4] | user_sensor_cal[i + 5] << 8,
                user_sensor_cal[i + 6] | user_sensor_cal[i + 7] << 8);
        }
        // Gyro cal origin position
        sensor_cal[1][0] = uint16_to_int16(user_sensor_cal[0xE] | user_sensor_cal[0xF] << 8);
        sensor_cal[1][1] = uint16_to_int16(user_sensor_cal[0x10] | user_sensor_cal[0x11] << 8);
        sensor_cal[1][2] = uint16_to_int16(user_sensor_cal[0x12] | user_sensor_cal[0x13] << 8);
    }
    else {
        FormJoy::myform1->textBox_6axis_ucal->Text = L"\r\n\r\nUser:\r\nNo calibration";
    }


    // Enable nxpad standard input report
    memset(buf_cmd, 0, sizeof(buf_cmd));
    auto hdr = (brcm_hdr *)buf_cmd;
    auto pkt = (brcm_cmd_01 *)(hdr + 1);
    hdr->cmd = 0x01;
    hdr->timer = timming_byte & 0xF;
    timming_byte++;
    pkt->subcmd = 0x03;
    pkt->subcmd_arg.arg1 = 0x30;
    res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
    res = hid_read_timeout(handle, buf_cmd, 0, 120);

    // Enable IMU
    memset(buf_cmd, 0, sizeof(buf_cmd));
    hdr = (brcm_hdr *)buf_cmd;
    pkt = (brcm_cmd_01 *)(hdr + 1);
    hdr->cmd = 0x01;
    hdr->timer = timming_byte & 0xF;
    timming_byte++;
    pkt->subcmd = 0x40;
    pkt->subcmd_arg.arg1 = 0x01;
    res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
    res = hid_read_timeout(handle, buf_cmd, 0, 120);

    // Use SPI calibration and convert them to SI acc unit (m/s^2)
    acc_cal_coeff[0] = (float)(1.0 / (float)(16384 - uint16_to_int16(sensor_cal[0][0]))) * 4.0f  * 9.8f;
    acc_cal_coeff[1] = (float)(1.0 / (float)(16384 - uint16_to_int16(sensor_cal[0][1]))) * 4.0f  * 9.8f;
    acc_cal_coeff[2] = (float)(1.0 / (float)(16384 - uint16_to_int16(sensor_cal[0][2]))) * 4.0f  * 9.8f;

    // Use SPI calibration and convert them to SI gyro unit (rad/s)
    gyro_cal_coeff[0] = (float)(936.0 / (float)(13371 - uint16_to_int16(sensor_cal[1][0])) * 0.01745329251994);
    gyro_cal_coeff[1] = (float)(936.0 / (float)(13371 - uint16_to_int16(sensor_cal[1][1])) * 0.01745329251994);
    gyro_cal_coeff[2] = (float)(936.0 / (float)(13371 - uint16_to_int16(sensor_cal[1][2])) * 0.01745329251994);

    // Input report loop
    while (enable_button_test) {
        res = hid_read_timeout(handle, buf_reply, sizeof(buf_reply), 200);

        if (res > 12) {
            if (buf_reply[0] == 0x21 || buf_reply[0] == 0x30 || buf_reply[0] == 0x31 || buf_reply[0] == 0x32 || buf_reply[0] == 0x33) {
                if (((buf_reply[2] >> 1) & 0x3) == 3)
                    input_report_cmd = String::Format(L"Conn: BT");
                else if (((buf_reply[2] >> 1) & 0x3) == 0)
                    input_report_cmd = String::Format(L"Conn: USB");
                else
                    input_report_cmd = String::Format(L"Conn: {0:X}?", (buf_reply[2] >> 1) & 0x3);
                input_report_cmd += String::Format(L"\r\nBatt: {0:X}/4   ", buf_reply[2] >> 5);
                if ((buf_reply[2] >> 4) & 0x1)
                    input_report_cmd += L"Charging: Yes\r\n";
                else
                    input_report_cmd += L"Charging: No\r\n";

                input_report_cmd += String::Format(L"Vibration decision: ");
                input_report_cmd += String::Format(L"{0:X}, {1:X}\r\n", (buf_reply[12] >> 7) & 1, (buf_reply[12] >> 4) & 7);

                input_report_cmd += String::Format(L"\r\nButtons: ");
                for (int i = 3; i < 6; i++)
                    input_report_cmd += String::Format(L"{0:X2} ", buf_reply[i]);
            
                if (handle_ok != 2) {
                    input_report_cmd += String::Format(L"\r\n\r\nL Stick (Raw/Cal):\r\nX:   {0:X3}   Y:   {1:X3}\r\n",
                        buf_reply[6] | (u16)((buf_reply[7] & 0xF) << 8),
                        (buf_reply[7] >> 4) | (buf_reply[8] << 4));

                    AnalogStickCalc(
                        cal_x, cal_y,
                        buf_reply[6] | (u16)((buf_reply[7] & 0xF) << 8),
                        (buf_reply[7] >> 4) | (buf_reply[8] << 4),
                        stick_cal_x_l,
                        stick_cal_y_l);

                    input_report_cmd += String::Format(L"X: {0,5:f2}   Y: {1,5:f2}\r\n",
                        cal_x[0], cal_y[0]);
                }
                if (handle_ok != 1) {
                    input_report_cmd += String::Format(L"\r\n\r\nR Stick (Raw/Cal):\r\nX:   {0:X3}   Y:   {1:X3}\r\n",
                        buf_reply[9] | (u16)((buf_reply[10] & 0xF) << 8),
                        (buf_reply[10] >> 4) | (buf_reply[11] << 4));

                    AnalogStickCalc(
                        cal_x, cal_y,
                        buf_reply[9] | (u16)((buf_reply[10] & 0xF) << 8),
                        (buf_reply[10] >> 4) | (buf_reply[11] << 4),
                        stick_cal_x_r,
                        stick_cal_y_r);

                    input_report_cmd += String::Format(L"X: {0,5:f2}   Y: {1,5:f2}\r\n",
                        cal_x[0], cal_y[0]);
                }

                input_report_sys = String::Format(L"Acc/meter (Raw/Cal):\r\n");
                //The controller sends the sensor data 3 times with a little bit different values. Skip them
                input_report_sys += String::Format(L"X: {0:X4}  {1,7:F2} m/s\u00B2\r\n", buf_reply[13] | (buf_reply[14] << 8) & 0xFF00,
                    (float)(uint16_to_int16(buf_reply[13] | (buf_reply[14] << 8) & 0xFF00)) * acc_cal_coeff[0]);
                input_report_sys += String::Format(L"Y: {0:X4}  {1,7:F2} m/s\u00B2\r\n", buf_reply[15] | (buf_reply[16] << 8) & 0xFF00,
                    (float)(uint16_to_int16(buf_reply[15] | (buf_reply[16] << 8) & 0xFF00)) * acc_cal_coeff[1]);
                input_report_sys += String::Format(L"Z: {0:X4}  {1,7:F2} m/s\u00B2\r\n", buf_reply[17] | (buf_reply[18] << 8) & 0xFF00,
                    (float)(uint16_to_int16(buf_reply[17] | (buf_reply[18] << 8) & 0xFF00))  * acc_cal_coeff[2]);

                input_report_sys += String::Format(L"\r\nGyroscope (Raw/Cal):\r\n");
                input_report_sys += String::Format(L"X: {0:X4}  {1,7:F2} rad/s\r\n", buf_reply[19] | (buf_reply[20] << 8) & 0xFF00,
                    (float)(uint16_to_int16(buf_reply[19] | (buf_reply[20] << 8) & 0xFF00) - uint16_to_int16(sensor_cal[1][0])) * gyro_cal_coeff[0]);
                input_report_sys += String::Format(L"Y: {0:X4}  {1,7:F2} rad/s\r\n", buf_reply[21] | (buf_reply[22] << 8) & 0xFF00,
                    (float)(uint16_to_int16(buf_reply[21] | (buf_reply[22] << 8) & 0xFF00) - uint16_to_int16(sensor_cal[1][1])) * gyro_cal_coeff[1]);
                input_report_sys += String::Format(L"Z: {0:X4}  {1,7:F2} rad/s\r\n", buf_reply[23] | (buf_reply[24] << 8) & 0xFF00,
                    (float)(uint16_to_int16(buf_reply[23] | (buf_reply[24] << 8) & 0xFF00) - uint16_to_int16(sensor_cal[1][2])) * gyro_cal_coeff[2]);
            }
            else if (buf_reply[0] == 0x3F) {
                input_report_cmd = L"";
                for (int i = 0; i < 17; i++)
                    input_report_cmd += String::Format(L"{0:X2} ", buf_reply[i]);
            }

            if (limit_output == 1) {
                FormJoy::myform1->textBox_btn_test_reply->Text    = input_report_cmd;
                FormJoy::myform1->textBox_btn_test_subreply->Text = input_report_sys;
            }
            //Only update every 75ms for better readability. No need for real time parsing.
            else if (limit_output > 4) {
                limit_output = 0;
            }
            limit_output++;
        }
        Application::DoEvents();
    }

    memset(buf_cmd, 0, sizeof(buf_cmd));
    hdr = (brcm_hdr *)buf_cmd;
    pkt = (brcm_cmd_01 *)(hdr + 1);
    hdr->cmd = 0x01;
    hdr->timer = timming_byte & 0xF;
    timming_byte++;
    pkt->subcmd = 0x03;
    pkt->subcmd_arg.arg1 = 0x3F;
    res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
    res = hid_read_timeout(handle, buf_cmd, 0, 64);

    memset(buf_cmd, 0, sizeof(buf_cmd));
    hdr = (brcm_hdr *)buf_cmd;
    pkt = (brcm_cmd_01 *)(hdr + 1);
    hdr->cmd = 0x01;
    hdr->timer = timming_byte & 0xF;
    timming_byte++;
    pkt->subcmd = 0x40;
    pkt->subcmd_arg.arg1 = 0x00;
    res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
    res = hid_read_timeout(handle, buf_cmd, 0, 64);

    return 0;
}
#else
// TODO: Implement else
#endif

#ifndef __jctool_cpp_API__
int play_tune(int tune_no) {
#else
int play_tune(CT& ct, int tune_no) {
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
#endif
    int res;
    ConCom::Packet packet_buf;
    u8* buf = packet_buf.buf;
    constexpr int buf_size = sizeof(packet_buf.buf);
    u8 buf2[49];
    auto hdr = (brcm_hdr *)buf;
    auto pkt = (brcm_cmd_01 *)(hdr + 1);

    //Enable Vibration
    Rumble::enable_rumble(ct, packet_buf);
    res = hid_read_timeout(handle, buf2, 1, 120);
    // This needs to be changed for new bigger tunes.
    u32 *tune = new u32[6000];
    memset(tune, 0, sizeof(tune));
    int tune_size = 0;
    switch (tune_no) {
        case 0:
            memcpy(tune, &tune_SMB, sizeof(tune_SMB));
            tune_size = sizeof(tune_SMB) / sizeof(u32);
            break;
        case 1:
            memcpy(tune, &tune_SMO_OK, sizeof(tune_SMO_OK));
            tune_size = sizeof(tune_SMO_OK) / sizeof(u32);
            break;
    }

    for (int i = 0; i < tune_size; i++) {
        Sleep(15);
        memset(buf, 0, buf_size);
        hdr = (brcm_hdr *)buf;
        pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x10;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        hdr->rumble_l[0] = (tune[i] >> 24) & 0xFF;
        hdr->rumble_l[1] = (tune[i] >> 16) & 0xFF;
        hdr->rumble_l[2] = (tune[i] >> 8) & 0xFF;
        hdr->rumble_l[3] = tune[i] & 0xFF;
        memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
        res = hid_write(handle, buf, buf_size);
        // Joy-con does not reply when Output Report is 0x10
#ifndef __jctool_cpp_API__
        Application::DoEvents();
#else
    // TODO: Implement else
#endif
    }

    // Disable vibration
    Sleep(15);
    memset(buf, 0, buf_size);
    hdr = (brcm_hdr *)buf;
    pkt = (brcm_cmd_01 *)(hdr + 1);
    hdr->cmd = 0x01;
    hdr->timer = timming_byte & 0xF;
    timming_byte++;
    hdr->rumble_l[0] = 0x00;
    hdr->rumble_l[1] = 0x01;
    hdr->rumble_l[2] = 0x40;
    hdr->rumble_l[3] = 0x40;
    memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
    pkt->subcmd = 0x48;
    pkt->subcmd_arg.arg1 = 0x00;
    res = hid_write(handle, buf, buf_size);
    res = hid_read_timeout(handle, buf, 1, 64);

    memset(buf, 0, buf_size);
    hdr = (brcm_hdr *)buf;
    pkt = (brcm_cmd_01 *)(hdr + 1);
    hdr->cmd = 0x01;
    hdr->timer = timming_byte & 0xF;
    timming_byte++;
    pkt->subcmd = 0x30;
    pkt->subcmd_arg.arg1 = 0x01;
    res = hid_write(handle, buf, buf_size);
    res = hid_read_timeout(handle, buf, 1, 64);

    delete[] tune;

    return 0;
}

#ifndef __jctool_cpp_API__
int play_hd_rumble_file(int file_type, u16 sample_rate, int samples, int loop_start, int loop_end, int loop_wait, int loop_times) {
#else
int play_hd_rumble_file(CT& ct, const RumbleData& rumble_data) {
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
    VIBType file_type = rumble_data.metadata.vib_file_type;
    u16 sample_rate = rumble_data.metadata.sample_rate;
    int samples = rumble_data.metadata.samples;
    int loop_start = rumble_data.metadata.loop_start;
    int loop_end = rumble_data.metadata.loop_end;
    int loop_wait = rumble_data.metadata.loop_wait;
    int loop_times = rumble_data.metadata.loop_times;
#endif
    int res;
    u8 buf[49];
    u8 buf2[49];

    //Enable Vibration
    memset(buf, 0, sizeof(buf));
    auto hdr = (brcm_hdr *)buf;
    auto pkt = (brcm_cmd_01 *)(hdr + 1);
    hdr->cmd = 0x01;
    hdr->timer = timming_byte & 0xF;
    timming_byte++;
    pkt->subcmd = 0x48;
    pkt->subcmd_arg.arg1 = 0x01;
    res = hid_write(handle, buf, sizeof(buf));
    res = hid_read_timeout(handle, buf2, 1, 120);

    if (file_type == VIBRaw || file_type == VIBBinary) {
        for (int i = 0; i < samples * 4; i = i + 4) {
            Sleep(sample_rate);
            memset(buf, 0, sizeof(buf));
            hdr = (brcm_hdr *)buf;
            pkt = (brcm_cmd_01 *)(hdr + 1);
            hdr->cmd = 0x10;
            hdr->timer = timming_byte & 0xF;
            timming_byte++;

            auto use_rumble_data
#ifndef __jctool_cpp_API__
            =  (file_type == 1) ? FormJoy::myform1->vib_loaded_file : FormJoy::myform1->vib_file_converted;
#else
            = &*rumble_data.data;
#endif
            if (file_type == 1) {
                hdr->rumble_l[0] = use_rumble_data[0x0A + i];
                hdr->rumble_l[1] = use_rumble_data[0x0B + i];
                hdr->rumble_l[2] = use_rumble_data[0x0C + i];
                hdr->rumble_l[3] = use_rumble_data[0x0D + i];
            }
            //file_type is simple bnvib
            else {
                hdr->rumble_l[0] = use_rumble_data[0x0C + i];
                hdr->rumble_l[1] = use_rumble_data[0x0D + i];
                hdr->rumble_l[2] = use_rumble_data[0x0E + i];
                hdr->rumble_l[3] = use_rumble_data[0x0F + i];
            }
            memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));

            res = hid_write(handle, buf, sizeof(*hdr));
#ifndef __jctool_cpp_API__
            Application::DoEvents();
#else
            // TODO: Implement else
#endif
        }
    }
    else if (file_type == VIBBinaryLoop || file_type == VIBBinaryLoopAndWait) {
        u8 vib_off = 0;
        if (file_type == VIBBinaryLoop)
            vib_off = 8;
        else if (file_type == VIBBinaryLoopAndWait)
            vib_off = 12;

        for (int i = 0; i < loop_start * 4; i = i + 4) {
            Sleep(sample_rate);
            memset(buf, 0, sizeof(buf));
            hdr = (brcm_hdr *)buf;
            pkt = (brcm_cmd_01 *)(hdr + 1);
            hdr->cmd = 0x10;
            hdr->timer = timming_byte & 0xF;
            timming_byte++;

            auto use_rumble_data
#ifndef __jctool_cpp_API__
            = FormJoy::myform1->vib_loaded_file;
#else
            = &*rumble_data.data;
#endif
            hdr->rumble_l[0] = use_rumble_data[0x0C + vib_off + i];
            hdr->rumble_l[1] = use_rumble_data[0x0D + vib_off + i];
            hdr->rumble_l[2] = use_rumble_data[0x0E + vib_off + i];
            hdr->rumble_l[3] = use_rumble_data[0x0F + vib_off + i];

            memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
            
            res = hid_write(handle, buf, sizeof(*hdr));
#ifndef __jctool_cpp_API__
            Application::DoEvents();
#else
            // TODO: Implement else
#endif
        }
        for (int j = 0; j < 1 + loop_times; j++) {
            for (int i = loop_start * 4; i < loop_end * 4; i = i + 4) {
                Sleep(sample_rate);
                memset(buf, 0, sizeof(buf));
                hdr = (brcm_hdr *)buf;
                pkt = (brcm_cmd_01 *)(hdr + 1);
                hdr->cmd = 0x10;
                hdr->timer = timming_byte & 0xF;
                timming_byte++;

                auto use_rumble_data
#ifndef __jctool_cpp_API__
                = FormJoy::myform1->vib_loaded_file;
#else
                = &*rumble_data.data;
#endif
                hdr->rumble_l[0] = use_rumble_data[0x0C + vib_off + i];
                hdr->rumble_l[1] = use_rumble_data[0x0D + vib_off + i];
                hdr->rumble_l[2] = use_rumble_data[0x0E + vib_off + i];
                hdr->rumble_l[3] = use_rumble_data[0x0F + vib_off + i];

                memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));

                res = hid_write(handle, buf, sizeof(*hdr));
#ifndef __jctool_cpp_API__
                Application::DoEvents();
#else
                // TODO: Implement else
#endif
            }
            Sleep(sample_rate);
            // Disable vibration
            memset(buf, 0, sizeof(buf));
            hdr = (brcm_hdr *)buf;
            pkt = (brcm_cmd_01 *)(hdr + 1);
            hdr->cmd = 0x10;
            hdr->timer = timming_byte & 0xF;
            timming_byte++;
            hdr->rumble_l[0] = 0x00;
            hdr->rumble_l[1] = 0x01;
            hdr->rumble_l[2] = 0x40;
            hdr->rumble_l[3] = 0x40;
            memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
            res = hid_write(handle, buf, sizeof(*hdr));
            Sleep(loop_wait * sample_rate);
        }
        for (int i = loop_end * 4; i < samples * 4; i = i + 4) {
            Sleep(sample_rate);
            memset(buf, 0, sizeof(buf));
            hdr = (brcm_hdr *)buf;
            pkt = (brcm_cmd_01 *)(hdr + 1);
            hdr->cmd = 0x10;
            hdr->timer = timming_byte & 0xF;
            timming_byte++;
            auto use_rumble_data
#ifndef __jctool_cpp_API__
            = FormJoy::myform1->vib_loaded_file;
#else
            = &*rumble_data.data;
#endif
            hdr->rumble_l[0] = use_rumble_data[0x0C + vib_off + i];
            hdr->rumble_l[1] = use_rumble_data[0x0D + vib_off + i];
            hdr->rumble_l[2] = use_rumble_data[0x0E + vib_off + i];
            hdr->rumble_l[3] = use_rumble_data[0x0F + vib_off + i];

            memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));

            res = hid_write(handle, buf, sizeof(*hdr));
#ifndef __jctool_cpp_API__
            Application::DoEvents();
#else
            // TODO: Implement else
#endif
        }
    }

    Sleep(sample_rate);
    // Disable vibration
    memset(buf, 0, sizeof(buf));
    hdr = (brcm_hdr *)buf;
    pkt = (brcm_cmd_01 *)(hdr + 1);
    hdr->cmd = 0x10;
    hdr->timer = timming_byte & 0xF;
    timming_byte++;
    hdr->rumble_l[0] = 0x00;
    hdr->rumble_l[1] = 0x01;
    hdr->rumble_l[2] = 0x40;
    hdr->rumble_l[3] = 0x40;
    memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
    res = hid_write(handle, buf, sizeof(buf));

    Sleep(sample_rate + 120);
    memset(buf, 0, sizeof(buf));
    hdr = (brcm_hdr *)buf;
    pkt = (brcm_cmd_01 *)(hdr + 1);
    hdr->cmd = 0x01;
    hdr->timer = timming_byte & 0xF;
    timming_byte++;
    hdr->rumble_l[0] = 0x00;
    hdr->rumble_l[1] = 0x01;
    hdr->rumble_l[2] = 0x40;
    hdr->rumble_l[3] = 0x40;
    memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
    pkt->subcmd = 0x48;
    res = hid_write(handle, buf, sizeof(buf));
    res = hid_read_timeout(handle, buf, 1, 64);

    memset(buf, 0, sizeof(buf));
    hdr = (brcm_hdr *)buf;
    pkt = (brcm_cmd_01 *)(hdr + 1);
    hdr->cmd = 0x01;
    hdr->timer = timming_byte & 0xF;
    timming_byte++;
    pkt->subcmd = 0x30;
    pkt->subcmd_arg.arg1 = 0x01;
    res = hid_write(handle, buf, sizeof(buf));
    res = hid_read_timeout(handle, buf, 1, 64);

    return 0;
}

#ifndef __jctool_cpp_API__
int ir_sensor_auto_exposure(int white_pixels_percent) {
#else
int ir_sensor_auto_exposure(CT& ct, int white_pixels_percent) {
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
#endif
    int res;
    u8 buf[49];
    u16 new_exposure = 0;
#ifndef __jctool_cpp_API__
    int old_exposure = (u16)FormJoy::myform1->numeric_IRExposure->Value;

    // Calculate new exposure;
    if (white_pixels_percent == 0)
        old_exposure += 10;
    else if (white_pixels_percent > 5)
        old_exposure -= (white_pixels_percent / 4) * 20;

    old_exposure = CLAMP(old_exposure, 0, 600);
    FormJoy::myform1->numeric_IRExposure->Value = old_exposure;
    new_exposure = old_exposure * 31200 / 1000;
#else
    // TODO: Implement else
#endif

    memset(buf, 0, sizeof(buf));
    auto hdr = (brcm_hdr *)buf;
    auto pkt = (brcm_cmd_01 *)(hdr + 1);
    hdr->cmd = 0x01;
    hdr->timer = timming_byte & 0xF;
    timming_byte++;
    pkt->subcmd = 0x21;

    pkt->subcmd_21_23_04.mcu_cmd    = 0x23; // Write register cmd
    pkt->subcmd_21_23_04.mcu_subcmd = 0x04; // Write register to IR mode subcmd
    pkt->subcmd_21_23_04.no_of_reg  = 0x03; // Number of registers to write. Max 9.

    pkt->subcmd_21_23_04.reg1_addr = 0x3001; // R: 0x0130 - Set Exposure time LSByte
    pkt->subcmd_21_23_04.reg1_val  = new_exposure & 0xFF;
    pkt->subcmd_21_23_04.reg2_addr = 0x3101; // R: 0x0131 - Set Exposure time MSByte
    pkt->subcmd_21_23_04.reg2_val  = (new_exposure & 0xFF00) >> 8;
    pkt->subcmd_21_23_04.reg3_addr = 0x0700; // R: 0x0007 - Finalize config - Without this, the register changes do not have any effect.
    pkt->subcmd_21_23_04.reg3_val  = 0x01;

    buf[48] = mcu_crc8_calc(buf + 12, 36);
    res = hid_write(handle, buf, sizeof(buf));

    return res;
}

/**
 * ==================================================================================
 * A seperate namespace for things relating to IR logic.
 * Most of what is inside are helper functions wrapping the original code from CTCaer
 * so it is easier (well at least for me) to follow along and understand what
 * is going on.
 * ==================================================================================
 */
namespace IR {
    enum PacketType : u8 {
        Empty,
        Start,
        Next,
        Final,
        GotMissedFrag,
        Repeat,
        HasJustMissed
    };
    enum PacketFlags : u8 {
        WriteFrag = 1,
        UpdateIRStatus = 2,
        Ack = 4,
        AckMissed = 8
    };
    struct PacketDescription {
        int frag_no;
        PacketType type;
        PacketFlags flags;
    };

    struct StreamCTX {
        int& prev_frag_no;
        int& missed_frag_no;
        u8& max_frag_no;
        bool& missed_frag;
    };

    struct acknowledge {
        controller_hid_handle_t& handle;
        u8& timming_byte;
        brcm_hdr*& hdr;
        u8* buf;
        size_t buf_size;
    };
    /**
     * Request an IR sensor packet. 
     */
    int request_packet(controller_hid_handle_t handle, u8* buf_reply, size_t buf_size) {
        memset(buf_reply, 0, buf_size);
        return hid_read_timeout(handle, buf_reply, buf_size, 200);
    }

    /**
     * Acknowledge the packet.
     */
    int ack_packet(acknowledge& ack, u8 frag_no){
        ack.hdr->timer = ack.timming_byte & 0xF;
        ack.timming_byte++;
        ack.buf[14] = frag_no;
        ack.buf[47] = mcu_crc8_calc(ack.buf + 11, 36);
        return hid_write(ack.handle, ack.buf, ack.buf_size);
    }

    /**
     * Set the acknowledge buffer for a missed packet.
     * CTCaer's comment:
     * You send what the next fragment number will be,
     * instead of the actual missed packet.
     */
    inline void set_buf_missed_packet(u8 prev_frag_no, u8* ack_buf){
        ack_buf[12] = 0x1;
        ack_buf[13] = prev_frag_no + 1; // The next fragment number.
        ack_buf[14] = 0;
    }

    inline void ack_buf_12_13_nullify(u8* ack_buf){
        ack_buf[12] = 0x00;
        ack_buf[13] = 0x00;
    }

    /**
     * Send an acknowledge for a missed packet.
     */
    int ack_missed_packet(acknowledge& ack, u8 prev_frag_no){
        set_buf_missed_packet(prev_frag_no, ack.buf);

        int ack_res = ack_packet(ack, ack.buf[14]);

        ack_buf_12_13_nullify(ack.buf);

        return ack_res;
    }

    inline bool is_new_packet(u8* packet_buf){
        return packet_buf[0] == 0x31 && packet_buf[49] == 0x03;
    }

    inline bool is_next_frag(int frag_no, StreamCTX& sctx){
        return frag_no == (sctx.prev_frag_no + 1) % (sctx.max_frag_no + 1);
    }

    inline bool is_repeat_frag(int frag_no, StreamCTX& sctx){
        return frag_no == sctx.prev_frag_no;
    }

    inline bool has_missed_frag(int frag_no, StreamCTX& sctx){
        return sctx.missed_frag_no != frag_no && !sctx.missed_frag_no;
    }

    inline bool got_missed_frag(int frag_no, StreamCTX& sctx){
        return frag_no == sctx.missed_frag_no;
    }

    inline bool is_final_frag(int frag_no, int max_frag_no){
        return frag_no == max_frag_no;
    }

    inline bool should_request_missed(StreamCTX& sctx){
        return sctx.max_frag_no != 0x03; // 40x30 resolution
    }

    inline bool is_empty_report(u8* packet_buf){
        return packet_buf[0] == 0x31;
    }

    inline u8 frag_no(u8* packet_buf){
        return packet_buf[52];
    }
    
    const size_t FRAG_SIZE = 300;
    const size_t FRAG_START_OFFSET = 59;

    inline size_t get_img_buf_size(u8 max_frag_no){
        return FRAG_SIZE * (max_frag_no + 1);
    }

    /**
     * Original credit goes to CTCaer!
     * I (jon-dez) have only made it EASILY readable.
     * The logic (should) be the same.
     * The only difference is that the logic and the processing are done
     * seperate from each other.
     */
    inline PacketDescription get_packet_desc(StreamCTX& sctx, u8* packet_buf) {
        PacketDescription pd;
        memset(&pd, 0, sizeof(pd));
        //Check if new packet
        if (is_new_packet(packet_buf)) {
            pd.frag_no = frag_no(packet_buf);
            if (is_next_frag(pd.frag_no, sctx)) {
                pd.type = Next;
                pd.flags = (PacketFlags)(UpdateIRStatus | WriteFrag);
                // Check if final fragment. Draw the frame.
                if (is_final_frag(pd.frag_no, sctx.max_frag_no)) 
                    pd.type = Final; // Should draw the frame.
            }
            // Repeat/Missed fragment
            else if (pd.frag_no  || sctx.prev_frag_no) {
                pd.flags = UpdateIRStatus;
                // Check if repeat ACK should be send. Avoid writing to image buffer.
                if (is_repeat_frag(pd.frag_no, sctx)) {
                    pd.type = Repeat;
                }
                // Check if missed fragment and request it.
                else if(has_missed_frag(pd.frag_no, sctx)) {
                    pd.type = HasJustMissed;
                    pd.flags = (PacketFlags)(pd.flags | WriteFrag);

                    // Check if missed fragment and res is 30x40. Don't request it.
                    pd.flags = (should_request_missed(sctx))
                    ? ((PacketFlags)(pd.flags | AckMissed))
                    : (pd.flags);
                }
                // Got the requested missed fragments.
                else if (got_missed_frag(pd.frag_no, sctx)){
                    pd.type = GotMissedFrag;
                    pd.flags = (PacketFlags)(pd.flags | WriteFrag);
                }
                // Repeat of fragment that is not max fragment.
                else {
                    pd.type = Repeat;
                }
            }
            // Streaming start
            else {
                pd.type = Start;
                pd.flags = WriteFrag;
                pd.frag_no = 0;
            }
        }
        // Empty IR report. Send Ack again. Otherwise, it fallbacks to high latency mode (30ms per data fragment)
        else if (IR::is_empty_report(packet_buf)) {
            pd.type = Empty;
        }
        return pd;
    }
}

#ifndef __jctool_cpp_API__
int get_raw_ir_image(u8 capture_mode) {
    std::stringstream ir_status;
#else
int get_raw_ir_image(IRCaptureCTX& capture_context, StoreRawCaptureCB store_capture_cb) {
    controller_hid_handle_t handle = capture_context.handle;
    u8& timming_byte = capture_context.timming_byte;
    IRCaptureMode& capture_mode = capture_context.capture_mode;
    ir_image_config& ir_cfg = capture_context.ir_cfg;
    u8& ir_max_frag_no = capture_context.ir_max_frag_no;
    std::stringstream& ir_status = capture_context.capture_status.message_stream;
#endif

    int64_t elapsed_time = 0; // The time it took to get a fragment.
    int64_t elapsed_time2 = 0; // The time it took to get a frame.
#ifndef __jctool_cpp_API__
    System::Diagnostics::Stopwatch^ sw = System::Diagnostics::Stopwatch::StartNew();
#else
    int frame_counter = 0;
    auto start = std::chrono::system_clock::now();
    auto _elapsedClockTimeMS = [&start](){
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
    };
#endif

    int buf_size_img = IR::get_img_buf_size(ir_max_frag_no);

    ConCom::Packet p;
    u8 buf_reply[0x170];
    auto buf_image = std::make_unique<u8[]>(buf_size_img); // 8bpp greyscale image.
	uint16_t bad_signal = 0;
    int error_reading = 0;
    float noise_level = 0.0f;
    int avg_intensity_percent = 0.0f;
    int previous_frag_no = 0;
    int got_frag_no = 0;
    int missed_packet_no = 0;
    bool missed_packet = false;
    int initialization = 2; // Make 3, not 2?
    //int max_pixels = ((ir_max_frag_no < 218 ? ir_max_frag_no : 217) + 1) * 300;
    int white_pixels_percent = 0;

    memset(buf_image.get(), 0, buf_size_img);

    //p.zero();;
    memset(buf_reply, 0, sizeof(buf_reply));
    auto& hdr = p.header();
    auto& pkt = p.command();
    hdr->cmd = 0x11;
    pkt->subcmd = 0x03;
    p.buf[48] = 0xFF;

    IR::StreamCTX sctx {
        previous_frag_no,
        missed_packet_no,
        ir_max_frag_no,
        missed_packet
    };

    IR::acknowledge ack {
        handle,
        timming_byte,
        hdr,
        p.buf,
        p.buf_size
    };

    // First ack
    IR::ack_packet(ack, 0x0);

    // IR Read/ACK loop for fragmented data packets. 
    // It also avoids requesting missed data fragments, we just skip it to not complicate things.
#ifndef __jctool_cpp_API__
    while (enable_IRVideoPhoto || initialization)
#else
    while((capture_mode == IRCaptureMode::Video) || initialization)
#endif
    {
        int packet_res = IR::request_packet(handle, buf_reply, sizeof(buf_reply));
        /* Test */ {
            auto pd = IR::get_packet_desc(sctx, buf_reply);
            got_frag_no = pd.frag_no;

            switch (pd.type)
            {
            case IR::PacketType::Start:
                IR::ack_packet(ack, got_frag_no);
#ifndef __jctool_cpp_API__
                FormJoy::myform1->lbl_IRStatus->Text = (sw->ElapsedMilliseconds - elapsed_time).ToString() + "ms";
                elapsed_time = sw->ElapsedMilliseconds;
                Application::DoEvents();
#else
                elapsed_time = _elapsedClockTimeMS();
#endif
                previous_frag_no = 0;
                break;
            // Final and Next flow result from the same logic, but Final is unique (Adressed after the switches.)
            case IR::PacketType::Final:
            case IR::PacketType::Next:{
                capture_context.capture_status.last_frag_no = got_frag_no;
                IR::ack_packet(ack, got_frag_no);
                previous_frag_no = got_frag_no;

                // Auto exposure.
                // TODO: Fix placement, so it doesn't drop next fragment.
#ifndef __jctool_cpp_API__
                if (enable_IRAutoExposure 
#else
                //if(use_ir_sensor.auto_exposure
#endif
                //&& initialization < 2 && got_frag_no == 0){
                //    white_pixels_percent = (int)((*(u16*)&buf_reply[55] * 100) / buf_size_img);
#ifndef __jctool_cpp_API__
                    ir_sensor_auto_exposure(white_pixels_percent);
#else
                //    ir_sensor_auto_exposure(handle, timming_byte, white_pixels_percent);
#endif
                //}
            }break;
            case IR::PacketType::Repeat:
                // Check if repeat ACK should be send. Avoid writing to image buffer.
                // Repeat of fragment that is not max fragment.
                IR::ack_packet(ack, got_frag_no);
                missed_packet = false;
                break;
            case IR::PacketType::HasJustMissed:
                if ((pd.flags & IR::PacketFlags::AckMissed) == IR::PacketFlags::AckMissed) {
                    // Missed packet
                    IR::ack_missed_packet(ack, previous_frag_no);
                    previous_frag_no = got_frag_no;
                    missed_packet_no = got_frag_no - 1;
                    missed_packet = true;
                }
                // Check if missed fragment and res is 30x40. Don't request it.
                else {
                    IR::ack_packet(ack, got_frag_no);
                    previous_frag_no = got_frag_no;
                }
                break;
            case IR::PacketType::GotMissedFrag:
                // Got the requested missed fragments.
                //debug
                //printf("%02X Frag: Got missed %02X\n", got_frag_no, missed_packet_no);
                IR::ack_packet(ack, got_frag_no);

                previous_frag_no = got_frag_no;
                missed_packet = false;
                break;
            case IR::PacketType::Empty:
                // Empty IR report. Send Ack again. Otherwise, it fallbacks to high latency mode (30ms per data fragment)
            default:
                // Send ACK again or request missed frag
                if (buf_reply[49] == 0xFF) {
                    IR::ack_packet(ack, previous_frag_no);
                }
                else if (buf_reply[49] == 0x00) {
                    IR::ack_missed_packet(ack, previous_frag_no);
                }
                IR::ack_buf_12_13_nullify(ack.buf);
                break;
            }

            if((pd.flags & IR::PacketFlags::UpdateIRStatus) == IR::PacketFlags::UpdateIRStatus){
                // Status percentage
                ir_status.str("");
                ir_status.clear();
                if (initialization < 2) {
                    switch(capture_mode){
                        case IRCaptureMode::Image:
                        ir_status << "Status: Receiving.. ";
                        break;
                        case IRCaptureMode::Video:
                        ir_status << "Status: Streaming.. ";
                        break;
                        case IRCaptureMode::Off:
                        ir_status << "Status: Turning off.. ";
                        break;
                    }
                }
                else
                    ir_status << "Status: Initializing.. ";
                ir_status << std::setfill(' ') << std::setw(3);
                ir_status << std::fixed << std::setprecision(0) << (float)got_frag_no / (float)(ir_max_frag_no + 1) * 100.0f;
                ir_status << "% - ";

                //debug
               // printf("%02X Frag: Copy\n", got_frag_no);
#ifndef __jctool_cpp_API__
                FormJoy::myform1->lbl_IRStatus->Text = gcnew String(ir_status.str().c_str()) + (sw->ElapsedMilliseconds - elapsed_time).ToString() + "ms";
                elapsed_time = sw->ElapsedMilliseconds;
                Application::DoEvents();
#else
                elapsed_time = _elapsedClockTimeMS();
#endif
            }

            if((pd.flags & IR::PacketFlags::WriteFrag) == IR::PacketFlags::WriteFrag){
                //assert(got_frag_no <= ir_max_frag_no);
                if(!(got_frag_no <= ir_max_frag_no))
                    std::cout << "Got frag #" << got_frag_no << ", but the max frag # is " << (int)ir_max_frag_no << std::endl;
                else
                    memcpy(buf_image.get() + (IR::FRAG_SIZE * got_frag_no), buf_reply + IR::FRAG_START_OFFSET, IR::FRAG_SIZE);
            }

            // Check if final fragment. Draw the frame.
            if(pd.type == IR::Final){
                // Update Viewport
#ifndef __jctool_cpp_API__
                elapsed_time2 = sw->ElapsedMilliseconds - elapsed_time2;
                FormJoy::myform1->setIRPictureWindow(buf_image, true);
#endif

                //debug
                //printf("%02X Frag: Draw -------\n", got_frag_no);

                // Stats/IR header parsing
                // buf_reply[53]: Average Intensity. 0-255 scale.
                // buf_reply[54]: Unknown. Shows up only when EXFilter is enabled.
                // *(u16*)&buf_reply[55]: White pixels (pixels with 255 value). Max 65535. Uint16 constraints, even though max is 76800.
                // *(u16*)&buf_reply[57]: Pixels with ambient noise from external light sources (sun, lighter, IR remotes, etc). Cleaned by External Light Filter.
                noise_level = (float)(*(u16*)&buf_reply[57]) / ((float)(*(u16*)&buf_reply[55]) + 1.0f);
                white_pixels_percent = (int)((*(u16*)&buf_reply[55] * 100) / buf_size_img);
                avg_intensity_percent = (int)((buf_reply[53] * 100) / 255);
#ifndef __jctool_cpp_API__
                FormJoy::myform1->lbl_IRHelp->Text = String::Format("Amb Noise: {0:f2},  Int: {1:D}%,  FPS: {2:D} ({3:D}ms)\nEXFilter: {4:D},  White Px: {5:D}%,  EXF Int: {6:D}",
                    noise_level, avg_intensity_percent, (int)(1000 / elapsed_time2), elapsed_time2, *(u16*)&buf_reply[57], white_pixels_percent, buf_reply[54]);

                elapsed_time2 = sw->ElapsedMilliseconds;
#else
                int64_t curr_time = _elapsedClockTimeMS();
                elapsed_time2 = curr_time - elapsed_time2;
                float fps = 0.0f;
                if(elapsed_time2 > 0) {
                    fps = 1000 / elapsed_time2;
                }
                elapsed_time2 = _elapsedClockTimeMS();

                capture_context.capture_status.fps = fps;
                capture_context.capture_status.frame_counter = ++frame_counter;
                capture_context.capture_status.duration = (float) _elapsedClockTimeMS() / 1000;
                capture_context.capture_status.noise_level = noise_level;
                capture_context.capture_status.avg_intensity_percent = avg_intensity_percent;
                capture_context.capture_status.exfilter = *(u16*)&buf_reply[57];
                capture_context.capture_status.white_pixels_percent = white_pixels_percent;
                capture_context.capture_status.exf_int = buf_reply[54];
                
                store_capture_cb(buf_image.get(), buf_size_img);
#endif
                if (initialization)
                    initialization--;
            }
        }
    }
    return 0;
}

#ifndef __jctool_cpp_API__
int ir_sensor(ir_image_config &ir_cfg) {
#else
int ir_sensor(IRCaptureCTX& capture_context, StoreRawCaptureCB store_capture_cb) {
    controller_hid_handle_t handle = capture_context.handle;
    u8& timming_byte = capture_context.timming_byte;
    IRCaptureMode& capture_mode = capture_context.capture_mode;
    ir_image_config& ir_cfg = capture_context.ir_cfg;
    u8& ir_max_frag_no = capture_context.ir_max_frag_no;
#endif
    int res;
    u8 buf[0x170];
    static int output_buffer_length = 49;
    int error_reading = 0;
    int res_get = 0;
    // Set input report to x31
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 1;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x03;
        pkt->subcmd_arg.arg1 = 0x31;
        res = hid_write(handle, buf, output_buffer_length);
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (*(u16*)&buf[0xD] == 0x0380)
                goto step1;

            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 7) {
            res_get = 1;
            goto step10;
        }
    }

step1:
    // Enable MCU
    error_reading = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 1;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x22;
        pkt->subcmd_arg.arg1 = 0x1;
        res = hid_write(handle, buf, output_buffer_length);
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (*(u16*)&buf[0xD] == 0x2280)
                goto step2;

            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 7) {
            res_get = 2;
            goto step10;
        }
    }

step2:
    // Request MCU mode status
    error_reading = 0;
    while (1) { // Not necessary, but we keep to make sure the MCU is ready.
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x11;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x01;
        res = hid_write(handle, buf, output_buffer_length);
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (buf[0] == 0x31) {
                //if (buf[49] == 0x01 && buf[56] == 0x06) // MCU state is Initializing
                // *(u16*)buf[52]LE x04 in lower than 3.89fw, x05 in 3.89
                // *(u16*)buf[54]LE x12 in lower than 3.89fw, x18 in 3.89
                // buf[56]: mcu mode state
                if (buf[49] == 0x01 && buf[56] == 0x01) // MCU state is Standby
                    goto step3;
            }
            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 7) {
            res_get = 3;
            goto step10;
        }
    }

step3:
    // Set MCU mode
    error_reading = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x01;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x21;
        
        pkt->subcmd_21_21.mcu_cmd    = 0x21; // Set MCU mode cmd
        pkt->subcmd_21_21.mcu_subcmd = 0x00; // Set MCU mode cmd
        pkt->subcmd_21_21.mcu_mode   = 0x05; // MCU mode - 1: Standby, 4: NFC, 5: IR, 6: Initializing/FW Update?

        buf[48] = mcu_crc8_calc(buf + 12, 36);
        res = hid_write(handle, buf, output_buffer_length);
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (buf[0] == 0x21) {
                // *(u16*)buf[18]LE x04 in lower than 3.89fw, x05 in 3.89
                // *(u16*)buf[20]LE x12 in lower than 3.89fw, x18 in 3.89
                // buf[56]: mcu mode state
                if (buf[15] == 0x01 && *(u32*)&buf[22] == 0x01) // Mcu mode is Standby
                    goto step4;
            }
            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 7) {
            res_get = 4;
            goto step10;
        }
    }

step4:
    // Request MCU mode status
    error_reading = 0;
    while (1) { // Not necessary, but we keep to make sure the MCU mode changed.
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x11;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x01;
        res = hid_write(handle, buf, output_buffer_length);
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (buf[0] == 0x31) {
                // *(u16*)buf[52]LE x04 in lower than 3.89fw, x05 in 3.89
                // *(u16*)buf[54]LE x12 in lower than 3.89fw, x18 in 3.89
                if (buf[49] == 0x01 && buf[56] == 0x05) // Mcu mode is IR
                    goto step5;
            }
            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 7) {
            res_get = 5;
            goto step10;
        }
    }

step5:
    // Set IR mode and number of packets for each data blob. Blob size is packets * 300 bytes.
    error_reading = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x01;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;

        pkt->subcmd = 0x21;
        pkt->subcmd_21_23_01.mcu_cmd     = 0x23;
        pkt->subcmd_21_23_01.mcu_subcmd  = 0x01; // Set IR mode cmd
        pkt->subcmd_21_23_01.mcu_ir_mode = 0x07; // IR mode - 2: No mode/Disable?, 3: Moment, 4: Dpd (Wii-style pointing), 6: Clustering,
                                                 // 7: Image transfer, 8-10: Hand analysis (Silhouette, Image, Silhouette/Image), 0,1/5/10+: Unknown
        pkt->subcmd_21_23_01.no_of_frags = ir_max_frag_no; // Set number of packets to output per buffer
        pkt->subcmd_21_23_01.mcu_major_v = 0x0500; // Set required IR MCU FW v5.18. Major 0x0005.
        pkt->subcmd_21_23_01.mcu_minor_v = 0x1800; // Set required IR MCU FW v5.18. Minor 0x0018.

        buf[48] = mcu_crc8_calc(buf + 12, 36);
        res = hid_write(handle, buf, output_buffer_length);
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (buf[0] == 0x21) {
                // Mode set Ack
                if (buf[15] == 0x0b)
                    goto step6;
            }
            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 7) {
            res_get = 6;
            goto step10;
        }
    }

step6:
    // Request IR mode status
    error_reading = 0;
    while (0) { // Not necessary
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x11;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;

        pkt->subcmd = 0x03;
        pkt->subcmd_arg.arg1 = 0x02;

        buf[47] = mcu_crc8_calc(buf + 11, 36);
        buf[48] = 0xFF;
        res = hid_write(handle, buf, output_buffer_length);
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (buf[0] == 0x31) {
                // mode set to 7: Image transfer
                if (buf[49] == 0x13 && *(u16*)&buf[50] == 0x0700)
                    goto step7;
            }
            retries++;
            if (retries > 4 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 7) {
            res_get = 7;
            goto step10;
        }
    }

step7:
    // Write to registers for the selected IR mode
    error_reading = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x01;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x21;

        pkt->subcmd_21_23_04.mcu_cmd    = 0x23; // Write register cmd
        pkt->subcmd_21_23_04.mcu_subcmd = 0x04; // Write register to IR mode subcmd
        pkt->subcmd_21_23_04.no_of_reg  = 0x09; // Number of registers to write. Max 9.      

        pkt->subcmd_21_23_04.reg1_addr  = 0x2e00; // R: 0x002e - Set Resolution based on sensor binning and skipping
        pkt->subcmd_21_23_04.reg1_val   = ir_cfg.ir_res_reg;
        pkt->subcmd_21_23_04.reg2_addr  = 0x3001; // R: 0x0130 - Set Exposure time LSByte - (31200 * us /1000) & 0xFF - Max: 600us, Max encoded: 0x4920.
        pkt->subcmd_21_23_04.reg2_val   = ir_cfg.ir_exposure & 0xFF;
        pkt->subcmd_21_23_04.reg3_addr  = 0x3101; // R: 0x0131 - Set Exposure time MSByte - ((31200 * us /1000) & 0xFF00) >> 8
        pkt->subcmd_21_23_04.reg3_val   = (ir_cfg.ir_exposure & 0xFF00) >> 8;
        pkt->subcmd_21_23_04.reg4_addr  = 0x3201; // R: 0x0132 - Enable Max exposure Time - 0: Manual exposure, 1: Max exposure
        pkt->subcmd_21_23_04.reg4_val   = 0x00;
        pkt->subcmd_21_23_04.reg5_addr  = 0x1000; // R: 0x0010 - Set IR Leds groups state - Only 3 LSB usable
        pkt->subcmd_21_23_04.reg5_val   = ir_cfg.ir_leds;
        pkt->subcmd_21_23_04.reg6_addr  = 0x2e01; // R: 0x012e - Set digital gain LSB 4 bits of the value - 0-0xff
        pkt->subcmd_21_23_04.reg6_val   = (ir_cfg.ir_digital_gain & 0xF) << 4;
        pkt->subcmd_21_23_04.reg7_addr  = 0x2f01; // R: 0x012f - Set digital gain MSB 4 bits of the value - 0-0x7
        pkt->subcmd_21_23_04.reg7_val   = (ir_cfg.ir_digital_gain & 0xF0) >> 4;
        pkt->subcmd_21_23_04.reg8_addr  = 0x0e00; // R: 0x00e0 - External light filter - LS o bit0: Off/On, bit1: 0x/1x, bit2: ??, bit4,5: ??.
        pkt->subcmd_21_23_04.reg8_val   = ir_cfg.ir_ex_light_filter;
        pkt->subcmd_21_23_04.reg9_addr  = 0x4301; // R: 0x0143 - ExLF/White pixel stats threshold - 200: Default
        pkt->subcmd_21_23_04.reg9_val   = 0xc8;

        buf[48] = mcu_crc8_calc(buf + 12, 36);
        res = hid_write(handle, buf, output_buffer_length);

        // Request IR mode status, before waiting for the x21 ack
        memset(buf, 0, sizeof(buf));
        hdr->cmd = 0x11;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x03;
        pkt->subcmd_arg.arg1 = 0x02;
        buf[47] = mcu_crc8_calc(buf + 11, 36);
        buf[48] = 0xFF;
        res = hid_write(handle, buf, output_buffer_length);

        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (buf[0] == 0x21) {
                // Registers for mode 7: Image transfer set
                if (buf[15] == 0x13 && *(u16*)&buf[16] == 0x0700)
                    goto step8;
            }
            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 7) {
            res_get = 8;
            goto step10;
        }
    }

step8:
    // Write to registers for the selected IR mode
    error_reading = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x01;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x21;

        pkt->subcmd_21_23_04.mcu_cmd    = 0x23; // Write register cmd
        pkt->subcmd_21_23_04.mcu_subcmd = 0x04; // Write register to IR mode subcmd
        pkt->subcmd_21_23_04.no_of_reg  = 0x08; // Number of registers to write. Max 9.      

        pkt->subcmd_21_23_04.reg1_addr  = 0x1100; // R: 0x0011 - Leds 1/2 Intensity - Max 0x0F.
        pkt->subcmd_21_23_04.reg1_val   = (ir_cfg.ir_leds_intensity >> 8) & 0xFF;
        pkt->subcmd_21_23_04.reg2_addr  = 0x1200; // R: 0x0012 - Leds 3/4 Intensity - Max 0x10.
        pkt->subcmd_21_23_04.reg2_val   = ir_cfg.ir_leds_intensity & 0xFF;
        pkt->subcmd_21_23_04.reg3_addr  = 0x2d00; // R: 0x002d - Flip image - 0: Normal, 1: Vertically, 2: Horizontally, 3: Both 
        pkt->subcmd_21_23_04.reg3_val   = ir_cfg.ir_flip;
        pkt->subcmd_21_23_04.reg4_addr  = 0x6701; // R: 0x0167 - Enable De-noise smoothing algorithms - 0: Disable, 1: Enable.
        pkt->subcmd_21_23_04.reg4_val   = (ir_cfg.ir_denoise >> 16) & 0xFF;
        pkt->subcmd_21_23_04.reg5_addr  = 0x6801; // R: 0x0168 - Edge smoothing threshold - Max 0xFF, Default 0x23
        pkt->subcmd_21_23_04.reg5_val   = (ir_cfg.ir_denoise >> 8) & 0xFF;
        pkt->subcmd_21_23_04.reg6_addr  = 0x6901; // R: 0x0169 - Color Interpolation threshold - Max 0xFF, Default 0x44
        pkt->subcmd_21_23_04.reg6_val   = ir_cfg.ir_denoise & 0xFF;
        pkt->subcmd_21_23_04.reg7_addr  = 0x0400; // R: 0x0004 - LSB Buffer Update Time - Default 0x32
        if (ir_cfg.ir_res_reg == 0x69)
            pkt->subcmd_21_23_04.reg7_val = 0x2d; // A value of <= 0x2d is fast enough for 30 x 40, so the first fragment has the updated frame.  
        else
            pkt->subcmd_21_23_04.reg7_val = 0x32; // All the other resolutions the default is enough. Otherwise a lower value can break hand analysis.
        pkt->subcmd_21_23_04.reg8_addr  = 0x0700; // R: 0x0007 - Finalize config - Without this, the register changes do not have any effect.
        pkt->subcmd_21_23_04.reg8_val   = 0x01;

        buf[48] = mcu_crc8_calc(buf + 12, 36);
        res = hid_write(handle, buf, output_buffer_length);

        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (buf[0] == 0x21) {
                // Registers for mode 7: Image transfer set
                if (buf[15] == 0x13 && *(u16*)&buf[16] == 0x0700)
                    goto step9;
                // If the Joy-Con gets to reply to the previous x11 - x03 02 cmd before sending the above,
                // it will reply with the following if we do not send x11 - x03 02 again:
                else if (buf[15] == 0x23) // Got mcu mode config write.
                    goto step9;
            }
            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 7) {
            res_get = 9;
            goto step10;
        }
    }

step9:
    // Stream or Capture images from NIR Camera
#ifndef __jctool_cpp_API__
    if (enable_IRVideoPhoto)
        res_get = get_raw_ir_image(2);
    else
        res_get = get_raw_ir_image(1);
#else
    if(capture_mode > 0){
        res_get = get_raw_ir_image(capture_context, store_capture_cb);
    }
#endif

    //////
    // TODO: Should we send subcmd x21 with 'x230102' to disable IR mode before disabling MCU?
step10:
    // Disable MCU
    memset(buf, 0, sizeof(buf));
    auto hdr = (brcm_hdr *)buf;
    auto pkt = (brcm_cmd_01 *)(hdr + 1);
    hdr->cmd = 1;
    hdr->timer = timming_byte & 0xF;
    timming_byte++;
    pkt->subcmd = 0x22;
    pkt->subcmd_arg.arg1 = 0x00;
    res = hid_write(handle, buf, output_buffer_length);
    res = hid_read_timeout(handle, buf, sizeof(buf), 64);  


    // Set input report back to x3f
    error_reading = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 1;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x03;
        pkt->subcmd_arg.arg1 = 0x3f;
        res = hid_write(handle, buf, output_buffer_length);
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (*(u16*)&buf[0xD] == 0x0380)
                goto stepf;

            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 7) {
            goto stepf;
        }
    }

stepf:
    return res_get;
}

#ifndef __jctool_cpp_API__
int get_ir_registers(int start_reg, int reg_group) {
#else
int get_ir_registers(CT& ct, int start_reg, int reg_group) {
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
#endif
    int res;
    u8 buf[0x170];
    static int output_buffer_length = 49;
    int error_reading = 0;
    int res_get = 0;

    // Get the IR registers
    error_reading = 0;
    int pos_ir_registers = start_reg;
    while (1) {
    repeat_send:
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        memset(buf, 0, sizeof(buf));
        hdr->cmd = 0x11;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x03;
        pkt->subcmd_arg.arg1 = 0x03;

        buf[12] = 0x1; // seems to be always 0x01

        buf[13] = pos_ir_registers; // 0-4 registers page/group
        buf[14] = 0x00; // offset. this plus the number of registers, must be less than x7f
        buf[15] = 0x7f; // Number of registers to show + 1

        buf[47] = mcu_crc8_calc(buf + 11, 36);

        res = hid_write(handle, buf, output_buffer_length);

        int tries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (buf[49] == 0x1b && buf[51] == pos_ir_registers && buf[52] == 0x00) {
                error_reading = 0;
                printf("--->%02X, %02X : %02X:\n", buf[51], buf[52], buf[53]);
                for (int i = 0; i <= buf[52] + buf[53]; i++)
                    if ((i & 0xF) == 0xF)
                        printf("%02X | ", buf[54 + i]);
                    else
                        printf("%02X ", buf[54 + i]);
                printf("\n");
                break;
            }
            tries++;
            if (tries > 8) {
                error_reading++;
                if (error_reading > 5) {
                    return 1;
                }
                goto repeat_send;
            }

        }
        pos_ir_registers++;
        if (pos_ir_registers > reg_group) {
            break;
        }
        
    }
    printf("\n");

    return 0;
}

#ifndef __jctool_cpp_API__
int ir_sensor_config_live(ir_image_config &ir_cfg) {
#else
int ir_sensor_config_live(CT& ct, ir_image_config& ir_cfg) {
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
#endif
    int res;
    u8 buf[49];

    memset(buf, 0, sizeof(buf));
    auto hdr = (brcm_hdr *)buf;
    auto pkt = (brcm_cmd_01 *)(hdr + 1);
    hdr->cmd = 0x01;
    hdr->timer = timming_byte & 0xF;
    timming_byte++;
    pkt->subcmd = 0x21;

    pkt->subcmd_21_23_04.mcu_cmd    = 0x23; // Write register cmd
    pkt->subcmd_21_23_04.mcu_subcmd = 0x04; // Write register to IR mode subcmd
    pkt->subcmd_21_23_04.no_of_reg  = 0x09; // Number of registers to write. Max 9.

    pkt->subcmd_21_23_04.reg1_addr = 0x3001; // R: 0x0130 - Set Exposure time LSByte
    pkt->subcmd_21_23_04.reg1_val  = ir_cfg.ir_exposure & 0xFF;
    pkt->subcmd_21_23_04.reg2_addr = 0x3101; // R: 0x0131 - Set Exposure time MSByte
    pkt->subcmd_21_23_04.reg2_val  = (ir_cfg.ir_exposure & 0xFF00) >> 8;
    pkt->subcmd_21_23_04.reg3_addr = 0x1000; // R: 0x0010 - Set IR Leds groups state
    pkt->subcmd_21_23_04.reg3_val  = ir_cfg.ir_leds;
    pkt->subcmd_21_23_04.reg4_addr = 0x2e01; // R: 0x012e - Set digital gain LSB 4 bits
    pkt->subcmd_21_23_04.reg4_val  = (ir_cfg.ir_digital_gain & 0xF) << 4;
    pkt->subcmd_21_23_04.reg5_addr = 0x2f01; // R: 0x012f - Set digital gain MSB 4 bits
    pkt->subcmd_21_23_04.reg5_val  = (ir_cfg.ir_digital_gain & 0xF0) >> 4;
    pkt->subcmd_21_23_04.reg6_addr = 0x0e00; // R: 0x00e0 - External light filter
    pkt->subcmd_21_23_04.reg6_val  = ir_cfg.ir_ex_light_filter;
    pkt->subcmd_21_23_04.reg7_addr = (ir_cfg.ir_custom_register & 0xFF) << 8 | (ir_cfg.ir_custom_register >> 8) & 0xFF;
    pkt->subcmd_21_23_04.reg7_val  = (ir_cfg.ir_custom_register >> 16) & 0xFF;
    pkt->subcmd_21_23_04.reg8_addr = 0x1100; // R: 0x0011 - Leds 1/2 Intensity - Max 0x0F.
    pkt->subcmd_21_23_04.reg8_val  = (ir_cfg.ir_leds_intensity >> 8) & 0xFF;
    pkt->subcmd_21_23_04.reg9_addr = 0x1200; // R: 0x0012 - Leds 3/4 Intensity - Max 0x10.
    pkt->subcmd_21_23_04.reg9_val  = ir_cfg.ir_leds_intensity & 0xFF;

    buf[48] = mcu_crc8_calc(buf + 12, 36);
    res = hid_write(handle, buf, sizeof(buf));

    // Important. Otherwise we gonna have a dropped packet.
    Sleep(15);

    pkt->subcmd_21_23_04.no_of_reg = 0x06; // Number of registers to write. Max 9.

    pkt->subcmd_21_23_04.reg1_addr = 0x2d00; // R: 0x002d - Flip image - 0: Normal, 1: Vertically, 2: Horizontally, 3: Both 
    pkt->subcmd_21_23_04.reg1_val  = ir_cfg.ir_flip;
    pkt->subcmd_21_23_04.reg2_addr = 0x6701; // R: 0x0167 - Enable De-noise smoothing algorithms - 0: Disable, 1: Enable.
    pkt->subcmd_21_23_04.reg2_val  = (ir_cfg.ir_denoise >> 16) & 0xFF;
    pkt->subcmd_21_23_04.reg3_addr = 0x6801; // R: 0x0168 - Edge smoothing threshold - Max 0xFF, Default 0x23
    pkt->subcmd_21_23_04.reg3_val  = (ir_cfg.ir_denoise >> 8) & 0xFF;
    pkt->subcmd_21_23_04.reg4_addr = 0x6901; // R: 0x0169 - Color Interpolation threshold - Max 0xFF, Default 0x44
    pkt->subcmd_21_23_04.reg4_val  = ir_cfg.ir_denoise & 0xFF;
    pkt->subcmd_21_23_04.reg5_addr = 0x0400; // R: 0x0004 - LSB Buffer Update Time - Default 0x32
    if (ir_cfg.ir_res_reg == 0x69)
        pkt->subcmd_21_23_04.reg5_val = 0x2d; // A value of <= 0x2d is fast enough for 30 x 40, so the first fragment has the updated frame.  
    else
        pkt->subcmd_21_23_04.reg5_val = 0x32; // All the other resolutions the default is enough. Otherwise a lower value can break hand analysis.
    pkt->subcmd_21_23_04.reg6_addr = 0x0700; // R: 0x0007 - Finalize config - Without this, the register changes do not have any effect.
    pkt->subcmd_21_23_04.reg6_val  = 0x01;

    buf[48] = mcu_crc8_calc(buf + 12, 36);
    res = hid_write(handle, buf, sizeof(buf));

    // get_ir_registers(0,4); // Get all register pages
    // get_ir_registers((ir_cfg.ir_custom_register >> 8) & 0xFF, (ir_cfg.ir_custom_register >> 8) & 0xFF); // Get all registers based on changed register's page

    return res;
}

#ifndef __jctool_cpp_API__
int nfc_tag_info() {
#else
int nfc_tag_info(CT& ct, bool& enable_nfc_scanning) {
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
#endif
    /////////////////////////////////////////////////////
    // Kudos to Eric Betts (https://github.com/bettse) //
    // for nfc comm starters                           //
    /////////////////////////////////////////////////////
    int res;
    u8 buf[0x170];
    u8 buf2[0x170];
    static int output_buffer_length = 49;
    int error_reading = 0;
    int res_get = 0;
    u8  tag_uid_buf[10];
    u8  tag_uid_size = 0;
    u8  ntag_buffer[924];
    u16 ntag_buffer_pos = 0;
    u8  ntag_pages = 0; // Max 231
    u8  tag_type = 0;
    u16 payload_size = 0;
    bool ntag_init_done = false;

    memset(tag_uid_buf, 0, sizeof(tag_uid_buf));
    memset(ntag_buffer, 0, sizeof(ntag_buffer));

    // Set input report to x31
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 1;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x03;
        pkt->subcmd_arg.arg1 = 0x31;
        res = hid_write(handle, buf, output_buffer_length);
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (*(u16*)&buf[0xD] == 0x0380)
                goto step1;

            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 7) {
            res_get = 1;
            goto step9;
        }
    }

step1:
    // Enable MCU
    error_reading = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 1;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x22;
        pkt->subcmd_arg.arg1 = 0x1;
        res = hid_write(handle, buf, output_buffer_length);
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (*(u16*)&buf[0xD] == 0x2280)
                goto step2;

            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 7) {
            res_get = 2;
            goto step9;
        }
    }

step2:
    // Request MCU mode status
    error_reading = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x11;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x01;
        res = hid_write(handle, buf, output_buffer_length);
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (buf[0] == 0x31) {
                //if (buf[49] == 0x01 && buf[56] == 0x06) // MCU state is Initializing
                // *(u16*)buf[52]LE x04 in lower than 3.89fw, x05 in 3.89
                // *(u16*)buf[54]LE x12 in lower than 3.89fw, x18 in 3.89
                // buf[56]: mcu mode state
                if (buf[49] == 0x01 && buf[56] == 0x01) // Mcu mode is Standby
                    goto step3;
            }
            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 7) {
            res_get = 3;
            goto step9;
        }
    }

step3:
    // Set MCU mode
    error_reading = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x01;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x21;

        pkt->subcmd_21_21.mcu_cmd = 0x21; // Set MCU mode cmd
        pkt->subcmd_21_21.mcu_subcmd = 0x00; // Set MCU mode cmd
        pkt->subcmd_21_21.mcu_mode = 0x04; // MCU mode - 1: Standby, 4: NFC, 5: IR, 6: Initializing/FW Update?

        buf[48] = mcu_crc8_calc(buf + 12, 36);
        res = hid_write(handle, buf, output_buffer_length);
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (buf[0] == 0x21) {
                // *(u16*)buf[18]LE x04 in lower than 3.89fw, x05 in 3.89
                // *(u16*)buf[20]LE x12 in lower than 3.89fw, x18 in 3.89
                if (buf[15] == 0x01 && buf[22] == 0x01) // Mcu mode is standby
                    goto step4;
            }
            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 7) {
            res_get = 4;
            goto step9;
        }
    }

step4:
    // Request MCU mode status
    error_reading = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x11;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x01;
        res = hid_write(handle, buf, output_buffer_length);
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (buf[0] == 0x31) {
                // *(u16*)buf[52]LE x04 in lower than 3.89fw, x05 in 3.89
                // *(u16*)buf[54]LE x12 in lower than 3.89fw, x18 in 3.89
                if (buf[49] == 0x01 && buf[56] == 0x04) // Mcu mode is NFC
                    goto step5;
            }
            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 7) {
            res_get = 5;
            goto step9;
        }
    }

step5:
    // Request NFC mode status
    error_reading = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x11;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;

        pkt->subcmd = 0x02;
        pkt->subcmd_arg.arg1 = 0x04; // 0: Cancel all, 4: StartWaitingReceive
        pkt->subcmd_arg.arg2 = 0x00; // Count of the currecnt packet if the cmd is a series of packets.
        buf[13] = 0x00;
        buf[14] = 0x08; // 8: Last cmd packet, 0: More cmd packet should be  expected
        buf[15] = 0x00; // Length of data after cmd header

        buf[47] = mcu_crc8_calc(buf + 11, 36); //Without the last byte
        res = hid_write(handle, buf, output_buffer_length - 1);
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (buf[0] == 0x31) {
                if (buf[49] == 0x2a && *(u16*)&buf[50] == 0x0500 && buf[55] == 0x31 && buf[56] == 0x0b)// buf[56] == 0x0b: Initializing/Busy
                    break;
                if (buf[49] == 0x2a && *(u16*)&buf[50] == 0x0500 && buf[55] == 0x31 && buf[56] == 0x00) // buf[56] == 0x00: Awaiting cmd
                    goto step6;
            }
            retries++;
            if (retries > 4 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 9) {
            res_get = 6;
            goto step9;
        }
    }

step6:
    // Request NFC mode status
    error_reading = 0;
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x11;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;

        pkt->subcmd = 0x02;
        pkt->subcmd_arg.arg1 = 0x01; // 1: Start polling, 2: Stop polling, 
        pkt->subcmd_arg.arg2 = 0x00; // Count of the currecnt packet if the cmd is a series of packets.
        buf[13] = 0x00;
        buf[14] = 0x08; // 8: Last cmd packet, 0: More cmd packet should be expected
        buf[15] = 0x05; // Length of data after cmd header
        buf[16] = 0x01; // 1: Enable Mifare support
        buf[17] = 0x00; // Unknown.
        buf[18] = 0x00; // Unknown.
        buf[19] = 0x2c; // Unknown. Some values work (0x07) other don't.
        buf[20] = 0x01; // Unknown. This is not needed but Switch sends it.

        buf[47] = mcu_crc8_calc(buf + 11, 36); //Without the last byte
        res = hid_write(handle, buf, output_buffer_length - 1);
        int retries = 0;
        while (1) {
#ifndef __jctool_cpp_API__
            if (!enable_NFCScanning)
#else
            if (!enable_nfc_scanning)
#endif
                goto step7;
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (buf[0] == 0x31) {
                // buf[49] == 0x2a: NFC MCU input report
                // buf[50] shows when there's error?
                // buf[51] == 0x05: NFC
                // buf[54] always 9?
                // buf[55] always x31?
                // buf[56]: MCU/NFC state
                // buf[62]: nfc tag IC
                // buf[63]: nfc tag Type
                // buf[64]: size of following data and it's the last NFC header byte
                if (buf[49] == 0x2a && *(u16*)&buf[50] == 0x0500 && buf[56] == 0x09) { // buf[56] == 0x09: Tag detected
                    tag_uid_size = buf[64];
#ifndef __jctool_cpp_API__
                    FormJoy::myform1->txtBox_nfcUid->Text = "UID:  ";
#else
    // TODO: Implement else
#endif
                    for (int i = 0; i < tag_uid_size; i++) {
                        if (i < tag_uid_size - 1) {
                            tag_type = buf[62]; // Save tag type
                            tag_uid_buf[i] = buf[65 + i]; // Save UID
#ifndef __jctool_cpp_API__
                            FormJoy::myform1->txtBox_nfcUid->Text += String::Format("{0:X2}:", buf[65 + i]);
#else
    // TODO: Implement else
#endif
                        }
                        else {
#ifndef __jctool_cpp_API__
                            FormJoy::myform1->txtBox_nfcUid->Text += String::Format("{0:X2}", buf[65 + i]);
#else
    // TODO: Implement else
#endif
                        }
                    }
#ifndef __jctool_cpp_API__
                    FormJoy::myform1->txtBox_nfcUid->Text += String::Format("\r\nType: {0:s}", buf[62] == 0x2 ? "NTAG" : "MIFARE");
                    Application::DoEvents();
#else
    // TODO: Implement else
#endif
                    goto step7;
                }
                else if (buf[49] == 0x2a)
                    break;
            }
            retries++;
            if (retries > 4 || res == 0) {
#ifndef __jctool_cpp_API__
                Application::DoEvents();
#else
    // TODO: Implement else
#endif
                break;
            }
        }
        error_reading++;
        if (error_reading > 100) {
            res_get = 7;
#ifndef __jctool_cpp_API__
            if (ntag_init_done)
                FormJoy::myform1->txtBox_NFCTag->Text = String::Format("Tag lost!");
            else
                FormJoy::myform1->txtBox_NFCTag->Text = String::Format("No Tag detected!");
#else
    // TODO: Implement else
#endif
            goto step9;
        }
    }

step7:
    // Read NTAG contents
    error_reading = 0;
    while (1) {
        memset(buf2, 0, sizeof(buf2));
        auto hdr = (brcm_hdr *)buf2;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x11;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;

        pkt->subcmd = 0x02;
        pkt->subcmd_arg.arg1 = 0x06; // 6: Read Ntag data, 0xf: Read mifare data
        buf2[12] = 0x00;
        buf2[13] = 0x00;
        buf2[14] = 0x08;
        buf2[15] = 0x13; // Length of data after cmd header

        buf2[16] = 0xd0; // Unknown
        buf2[17] = 0x07; // Unknown or UID lentgh?
        buf2[18] = 0x00; // Only for Mifare cmds or should have a UID?

        buf2[19] = 0x00; //should have a UID?
        buf2[20] = 0x00; //should have a UID?
        buf2[21] = 0x00; //should have a UID?
        buf2[22] = 0x00; //should have a UID?
        buf2[23] = 0x00; //should have a UID?
        buf2[24] = 0x00; //should have a UID?

        buf2[25] = 0x00; // 1: Ntag215 only. 0: All tags, otherwise error x48 (Invalid format error)

        // https://www.tagnfc.com/en/info/11-nfc-tags-specs

        // If the following is selected wrongly, error x3e (Read error)
        switch (ntag_pages) {
            case 0:
                buf2[26] = 0x01;
                break;
                // Ntag213
            case 45:
                // The following 7 bytes should be decided with max the current ntag pages and what we want to read.
                buf2[26] = 0x01; // How many blocks to read. Each block should be <= 60 pages (240 bytes)? Min/Max values are 1/4, otherwise error x40 (Argument error)

                buf2[27] = 0x00; // Block 1 starting page
                buf2[28] = 0x2C; // Block 1 ending page
                buf2[29] = 0x00; // Block 2 starting page
                buf2[30] = 0x00; // Block 2 ending page
                buf2[31] = 0x00; // Block 3 starting page
                buf2[32] = 0x00; // Block 3 ending page
                buf2[33] = 0x00; // Block 4 starting page
                buf2[34] = 0x00; // Block 4 ending page
                break;
                // Ntag215
            case 135:
                // The following 7 bytes should be decided with max the current ntag pages and what we want to read.
                buf2[26] = 0x03; // How many page ranges to read. Each range should be <= 60 pages (240 bytes)? Max value is 4.

                buf2[27] = 0x00; // Block 1 starting page
                buf2[28] = 0x3b; // Block 1 ending page
                buf2[29] = 0x3c; // Block 2 starting page
                buf2[30] = 0x77; // Block 2 ending page
                buf2[31] = 0x78; // Block 3 starting page
                buf2[32] = 0x86; // Block 3 ending page
                buf2[33] = 0x00; // Block 4 starting page
                buf2[34] = 0x00; // Block 4 ending page
                break;
            case 231:
                // The following 7 bytes should be decided with max the current ntag pages and what we want to read.
                buf2[26] = 0x04; // How many page ranges to read. Each range should be <= 60 pages (240 bytes)? Max value is 4.

                buf2[27] = 0x00; // Block 1 starting page
                buf2[28] = 0x3b; // Block 1 ending page
                buf2[29] = 0x3c; // Block 2 starting page
                buf2[30] = 0x77; // Block 2 ending page
                buf2[31] = 0x78; // Block 3 starting page
                buf2[32] = 0xB3; // Block 3 ending page
                buf2[33] = 0xB4; // Block 4 starting page
                buf2[34] = 0xE6; // Block 4 ending page
                break;
            default:
                break;
        }

        buf2[47] = mcu_crc8_calc(buf2 + 11, 36);

        res = hid_write(handle, buf2, output_buffer_length - 1);

        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf2, sizeof(buf2), 64);
            if (buf2[0] == 0x31) {
                if ((buf2[49] == 0x3a || buf2[49] == 0x2a) && buf2[56] == 0x07) {
#ifndef __jctool_cpp_API__
                    FormJoy::myform1->txtBox_NFCTag->Text = String::Format("Error {0:X2}!", buf2[50]);
#else
    // TODO: Implement else
#endif
                    goto step9;///////////
                }
                else if (buf2[49] == 0x3a && buf2[51] == 0x07) {
                    if (ntag_init_done) {
                        payload_size = (buf2[54] << 8 | buf2[55]) & 0x7FF;
                        if (buf2[52] == 0x01) {
                            memcpy(ntag_buffer + ntag_buffer_pos, buf2 + 116, payload_size - 60);
                            ntag_buffer_pos += payload_size - 60;
                        }
                        else {
                            memcpy(ntag_buffer + ntag_buffer_pos, buf2 + 56, payload_size);
                        }
                    }
                    else if (buf2[52] == 0x01) {
                        if (tag_type == 2) {
                            switch (buf2[74]) {
                                case 0:
                                    ntag_pages = 135;
                                    break;
                                case 3:
                                    ntag_pages = 45;
                                    break;
                                case 4:
                                    ntag_pages = 231;
                                    break;
                                default:
                                    goto step9;///////////
                                    break;
                            }
                        }
                    }
                    break;
                }
                else if (buf2[49] == 0x2a && buf2[56] == 0x04) { // finished
                    if (ntag_init_done) {
#ifndef __jctool_cpp_API__
                        FormJoy::myform1->show_ntag_contents(ntag_buffer, ntag_pages);
                        Application::DoEvents();
#else
    // TODO: Implement else
#endif
                        goto step9;///////////
                    }
                    ntag_init_done = true;

                    memset(buf, 0, sizeof(buf));
                    auto hdr = (brcm_hdr *)buf;
                    auto pkt = (brcm_cmd_01 *)(hdr + 1);
                    hdr->cmd = 0x11;
                    hdr->timer = timming_byte & 0xF;
                    timming_byte++;

                    pkt->subcmd = 0x02;
                    pkt->subcmd_arg.arg1 = 0x02; // 0: Cancel all, 4: StartWaitingReceive
                    pkt->subcmd_arg.arg2 = 0x00; // Count of the currecnt packet if the cmd is a series of packets.
                    buf[13] = 0x00;
                    buf[14] = 0x08; // 8: Last cmd packet, 0: More cmd packet should be  expected
                    buf[15] = 0x00; // Length of data after cmd header

                    buf[47] = mcu_crc8_calc(buf + 11, 36); //Without the last byte
                    res = hid_write(handle, buf, output_buffer_length - 1);
                    Sleep(200);
                    goto step5;
                }
                else if (buf2[49] == 0x2a)
                    break;
            }
            retries++;
            if (retries > 4 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 9) {
            res_get = 8;
#ifndef __jctool_cpp_API__
            if (buf[62] == 0x4)
                FormJoy::myform1->txtBox_NFCTag->Text = String::Format("Mifare reading is not supported for now..");
#else
    // TODO: Implement else
#endif
            goto step9;
        }
    }

step9:
    // Disable MCU
    memset(buf, 0, sizeof(buf));
    auto hdr = (brcm_hdr *)buf;
    auto pkt = (brcm_cmd_01 *)(hdr + 1);
    hdr->cmd = 1;
    hdr->timer = timming_byte & 0xF;
    timming_byte++;
    pkt->subcmd = 0x22;
    pkt->subcmd_arg.arg1 = 0x00;
    res = hid_write(handle, buf, output_buffer_length);
    res = hid_read_timeout(handle, buf, sizeof(buf), 64);


    // Set input report to x3f
    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 1;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x03;
        pkt->subcmd_arg.arg1 = 0x3f;
        res = hid_write(handle, buf, output_buffer_length);
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (*(u16*)&buf[0xD] == 0x0380)
                goto stepf;

            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 7) {
            goto stepf;
        }
    }
stepf:
    if (res_get > 0)
        return res_get;

    return 0;
}

#ifndef __jctool_cpp_API__
int silence_input_report() {
#else
int silence_input_report(CT& ct) {
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
#endif
    int res;
    u8 buf[49];
    int error_reading = 0;

    while (1) {
        memset(buf, 0, sizeof(buf));
        auto hdr = (brcm_hdr *)buf;
        auto pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 1;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x03;
        pkt->subcmd_arg.arg1 = 0x3f;
        res = hid_write(handle, buf, sizeof(buf));
        int retries = 0;
        while (1) {
            res = hid_read_timeout(handle, buf, sizeof(buf), 64);
            if (*(u16*)&buf[0xD] == 0x0380)
                goto stepf;

            retries++;
            if (retries > 8 || res == 0)
                break;
        }
        error_reading++;
        if (error_reading > 4)
            break;
    }

stepf:
    return 0;
}


/*
//usb test
int usb_init(hid_device *handle) {
    int res;
    u8 buf_cmd[2];
    u8 buf_cmd2[2];
    memset(buf_cmd, 0, sizeof(buf_cmd));
    memset(buf_cmd2, 0, sizeof(buf_cmd2));

    Sleep(16);
    buf_cmd[0] = 1;
    buf_cmd[0] = 0x80;
    Sleep(16);
    buf_cmd[1] = 0x01;
    res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
    res = hid_read(handle, buf_cmd2, 0);
    Sleep(16);
    buf_cmd[1] = 0x02;
    res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
    res = hid_read(handle, buf_cmd2, 0);
    Sleep(16);
    buf_cmd[1] = 0x03;
    res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
    res = hid_read(handle, buf_cmd2, 0);
    Sleep(16);
    buf_cmd[1] = 0x02;
    res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
    res = hid_read(handle, buf_cmd2, 0);
    Sleep(16);
    buf_cmd[1] = 0x04;
    res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
    res = hid_read(handle, buf_cmd2, 0);

    return 0;
}

int usb_deinit(hid_device *handle) {
    int res;
    u8 buf_cmd[2];
    u8 buf_cmd2[2];
    memset(buf_cmd, 0, sizeof(buf_cmd));
    memset(buf_cmd2, 0, sizeof(buf_cmd2));

    Sleep(16);
    buf_cmd[0] = 0x80;
    buf_cmd[1] = 0x05;
    res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
    res = hid_read(handle, buf_cmd2, 0);

    return 0;
}

int usb_command(hid_device *handle) {
    int res;
    u8 buf_cmd[20];
    u8 buf_cmd2[1];
    memset(buf_cmd, 0, sizeof(buf_cmd));
    memset(buf_cmd2, 0, sizeof(buf_cmd2));

    buf_cmd[0] = 0x80;
    buf_cmd[1] = 0x92;
    buf_cmd[3] = 0x31;
    buf_cmd[8] = 0x01;

    Sleep(16);

    buf_cmd[9] = timming_byte & 0xF;
    timming_byte++;
    buf_cmd[18] = 0x40;
    buf_cmd[19] = 0x01;
    res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
    res = hid_read(handle, buf_cmd2, 0);
    Sleep(16);

    buf_cmd[9] = timming_byte & 0xF;
    timming_byte++;
    buf_cmd[18] = 0x30;
    buf_cmd[19] = 0x1;
    res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
    res = hid_read(handle, buf_cmd2, 0);

    Sleep(16);

    buf_cmd[9] = timming_byte & 0xF;
    timming_byte++;
    buf_cmd[18] = 0x38;
    buf_cmd[19] = 0x22;
    buf_cmd[9] = timming_byte & 0xF;
    timming_byte++;
    res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
    res = hid_read(handle, buf_cmd2, 0);

    return 0;
}

void usb_device_print(struct hid_device_info *dev)
{
    const wchar_t *interface_name = L"";

    if (wcscmp(dev->serial_number, L"000000000001"))
        interface_name = L"BT-HID";
    else
        interface_name = L"USB-HID";

    printf("USB device info:\n  VID:          0x%04hX\n" \
        "  PID:          0x%04hX\n  Dev Path:     %s\n" \
        "  MAC:          %ls\n  Interface:    %ls (%d)\n  Manufacturer: %ls\n" \
        "  Product:      %ls\n\n", dev->vendor_id, dev->product_id,
        dev->path, dev->serial_number, interface_name, dev->interface_number,
        dev->manufacturer_string, dev->product_string);
}
*/

int test_chamber() {

    //Add your testing code.

    return 0;
}

#ifndef __jctool_cpp_API__
int device_connection(){
    if (check_connection_ok) {
        handle_ok = 0;
#else
int device_connection(controller_hid_handle_t& handle){
        int handle_ok = 0;
#endif
        // Joy-Con (L)
        if (handle = hid_open(ConHID::VID, ConHID::JoyConLeft, nullptr)) {
            handle_ok = 1;
            return handle_ok;
        }
        // Joy-Con (R)
        if (handle = hid_open(ConHID::VID, ConHID::JoyConRight, nullptr)) {
            handle_ok = 2;
            return handle_ok;
        }
        // Pro Controller
        if (handle = hid_open(ConHID::VID, ConHID::ProCon, nullptr)) {
            handle_ok = 3;
            return handle_ok;
        }
        // Joy-Con Grip
        if(handle = hid_open(ConHID::VID, ConHID::JoyConGrip, nullptr)){
            handle_ok = 4;
            return handle_ok;
        }
        // Nothing found
        else {
            return 0;
        }
#ifndef __jctool_cpp_API__
    }
#endif
    /*
    //usb test
    if (!handle_ok) {
        hid_init();
        struct hid_device_info *devs = hid_enumerate(0x057E, 0x200e);
        if (devs){
            usb_device_print(devs);
            handle_l = hid_open_path(devs->path);
            devs = devs->next;
            handle = hid_open_path(devs->path);
            printf("\nlol\n");

            if (handle)
                handle_ok = 4;
        }
        hid_free_enumeration(devs);
    }
    */
    return handle_ok;
}

#ifndef __jctool_API_ONLY__
#ifndef __jctool_cpp_API__
[STAThread]
int Main(array<String^>^ args) {
#else
int main(int argc, char** args) {
#endif
    /*
    BOOL chk = AllocConsole();
    if (chk) {
        freopen("CONOUT$", "w", stdout);
    }
    */
#ifndef __jctool_cpp_API__
    check_connection_ok = true;
    while (!device_connection()) {
        if (MessageBox::Show(
            L"The device is not paired or the device was disconnected!\n\n" +
            L"To pair:\n  1. Press and hold the sync button until the leds are on\n" +
            L"  2. Pair the Bluetooth controller in Windows\n\nTo connect again:\n" +
            L"  1. Press a button on the controller\n  (If this doesn\'t work, re-pair.)\n\n" +
            L"To re-pair:\n  1. Go to 'Settings -> Devices' or Devices and Printers'\n" +
            L"  2. Remove the controller\n  3. Follow the pair instructions",
            L"CTCaer's Joy-Con Toolkit - Connection Error!",
            MessageBoxButtons::RetryCancel, MessageBoxIcon::Stop) == System::Windows::Forms::DialogResult::Cancel)
            return 1;
    }
    // Enable debugging
    if (args->Length > 0) {
        if (args[0] == "-d")
            enable_traffic_dump = true; // Enable hid_write/read logging to text file
        else if (args[0] == "-f")
            check_connection_ok = false;   // Don't check connection after the 1st successful one
    }
    timming_byte = 0x0;
#endif


    //test_chamber();
#ifndef __jctool_cpp_API__
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    CppWinFormJoy::FormJoy^  myform1 = gcnew FormJoy();
#else
    // TODO: Implement else
#endif

    /*
    //usb test
    usb_init(handle);
    usb_init(handle_l);
    usb_command(handle);
    usb_command(handle);
    Sleep(2000);
    if (handle_ok) {
        usb_deinit(handle);
        hid_close(handle);
        usb_deinit(handle_l);
        hid_close(handle_l);
    }
    hid_exit();
    */
#ifndef __jctool_cpp_API__
    Application::Run(myform1);
#else
    // Every UI frame call happens in the lambda function below.
    static const char* window_name = "Joy-Con Toolkit";
    auto imgui_calls = []() {
        static bool window_still_open = true;

        ImGui::SetNextWindowPos({}, ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

        if(!ImGui::Begin(window_name, &window_still_open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove)){
            ImGui::End();
        } else {
            JCToolkit::UI::show(window_name);
            ImGui::End();
        }

        if (!window_still_open)
            return -1;
        return 0;
    };

    ImGuiMain(
        {
            window_name, // The window title.
            640, // The window width.
            480 // The window height.
        },
        imgui_calls,
        JCToolkit::init,
        (ImGuiInitFlags)0
    );
    JCToolkit::exit();
#endif

    return 0;
}
#endif

/**
 * ===========================================================================
 * Below is some code extracted from the orginal UI framework source, as well
 * as other bits and pieces originating from the early Joy-Con Toolkit from
 * CTCaer, and is now conveniently placed in callable functions.
 * Goal: Eliminate dependency to the original UI framework so that useful code
 * that was once only accessible through that same framework is now accessible
 * through an API.
 * ===========================================================================
 */

BatteryData parseBatteryData(const unsigned char* batt_data) {
    int batt_percent = 0;
    int batt = ((u8)batt_data[0] & 0xF0) >> 4;

    // Calculate aproximate battery percent from regulated voltage
    u16 batt_volt = (u8)batt_data[1] + ((u8)batt_data[2] << 8);
    if (batt_volt < 0x560)
        batt_percent = 1;
    else if (batt_volt > 0x55F && batt_volt < 0x5A0) {
        batt_percent = static_cast<int>(((batt_volt - 0x60) & 0xFF) / 7.0f) + 1;
    }
    else if (batt_volt > 0x59F && batt_volt < 0x5E0) {
        batt_percent = static_cast<int>(((batt_volt - 0xA0) & 0xFF) / 2.625f) + 11;
    }
    else if (batt_volt > 0x5DF && batt_volt < 0x618) {
        batt_percent = static_cast<int>((batt_volt - 0x5E0) / 1.8965f) + 36;
    }
    else if (batt_volt > 0x617 && batt_volt < 0x658) {
        batt_percent = static_cast<int>(((batt_volt - 0x18) & 0xFF) / 1.8529f) + 66;
    }
    else if (batt_volt > 0x657)
        batt_percent = 100;

    return {batt_percent, batt, (batt_volt * 2.5f) / 1000};
}

TemperatureData parseTemperatureData(const unsigned char* temp_data){
    // Convert reading to Celsius according to datasheet
    float celsius = 25.0f + uint16_to_int16(temp_data[1] << 8 | temp_data[0]) * 0.0625f;
    return {celsius, celsius*1.8f + 32};
}

void colorizefrom8BitsPP(u8* pixel_data_in, u8* pixel_data_out, int ir_image_height, int ir_image_width, int bytes_pp_out, int col_fil, u8 color_order){
    int buf_pos = 0;

    u8 red_pos_idx = ColorOrder::getRedPosIdx(color_order);
    u8 green_pos_idx = ColorOrder::getGreenPosIdx(color_order);
    u8 blue_pos_idx = ColorOrder::getBluePosIdx(color_order);

    for (int y = 0; y < ir_image_height; y++) {
        u8* row = (u8 *)pixel_data_out + (y * bytes_pp_out * ir_image_width);
        for (int x = 0; x < ir_image_width; x++) {
            switch(col_fil){
                case IRGreyscale:
                    // Values are in BGR in memory. Here in RGB order.
                    row[x * bytes_pp_out + red_pos_idx]     = pixel_data_in[x + buf_pos];
                    row[x * bytes_pp_out + green_pos_idx]   = pixel_data_in[x + buf_pos];
                    row[x * bytes_pp_out + blue_pos_idx]    = pixel_data_in[x + buf_pos];
                    break;
                case IRNightVision:
                    // Values are in BGR in memory. Here in RGB order.
                    row[x * bytes_pp_out + red_pos_idx]     = 0;
                    row[x * bytes_pp_out + green_pos_idx]   = pixel_data_in[x + buf_pos];
                    row[x * bytes_pp_out + blue_pos_idx]    = 0;
                    break;
                case IRIronbow:
                    // Values are in BGR in memory. Here in RGB order.
                    row[x * bytes_pp_out + red_pos_idx]     = (iron_palette[pixel_data_in[x + buf_pos]] >> 16)&0xFF;
                    row[x * bytes_pp_out + green_pos_idx]   = (iron_palette[pixel_data_in[x + buf_pos]] >> 8) & 0xFF;
                    row[x * bytes_pp_out + blue_pos_idx]    =  iron_palette[pixel_data_in[x + buf_pos]] & 0xFF;
                    break;
                case IRInfrared:
                default:
                    // Values are in BGR in memory. Here in RGB order.
                    row[x * bytes_pp_out + red_pos_idx]     = pixel_data_in[x + buf_pos];
                    row[x * bytes_pp_out + green_pos_idx]   = 0;
                    row[x * bytes_pp_out + blue_pos_idx]    = 0;
                    break;
            }
        }
        buf_pos += ir_image_width;
    }
}

std::string ir_sensorErrorToString(int errno_ir_sensor){
    // Get error
    switch (errno_ir_sensor) {
    case 1:
        return "1ID31";
    case 2:
        return "2MCUON";
    case 3:
        return "3MCUONBUSY";
    case 4:
        return "4MCUMODESET";
    case 5:
        return "5MCUSETBUSY";
    case 6:
        return "6IRMODESET";
    case 7:
        return "7IRSETBUSY";
    case 8:
        return "8IRCFG";
    case 9:
        return "9IRFCFG";
    default:
        return "UNDEFINED_ERR";
    }
}

SPIColors get_spi_colors(CT& ct){
    unsigned char spi_colors[12];
    memset(spi_colors, 0, 12);

    int res = get_spi_data(ct, 0x6050, 12, spi_colors);

    SPIColors colors;
    memcpy(&colors, spi_colors, 12);
    return colors;
}

int write_spi_colors(CT& ct, const SPIColors& colors){
    unsigned char spi_colors[12];
    memcpy(spi_colors, &colors.body, sizeof(colors));

    return write_spi_data(ct, 0x6050, 12, spi_colors);
}

lut_amp lut_joy_amp{
	{ 0.00000f, 0.007843f, 0.011823f, 0.014061f, 0.01672f, 0.019885f, 0.023648f, 0.028123f,
    0.033442f, 0.039771f, 0.047296f, 0.056246f, 0.066886f, 0.079542f, 0.094592f, 0.112491f,
    0.117471f, 0.122671f, 0.128102f, 0.133774f, 0.139697f, 0.145882f, 0.152341f, 0.159085f,
    0.166129f, 0.173484f, 0.181166f, 0.189185f, 0.197561f, 0.206308f, 0.215442f, 0.224982f,
    0.229908f, 0.234943f, 0.240087f, 0.245345f, 0.250715f, 0.256206f, 0.261816f, 0.267549f,
    0.273407f, 0.279394f, 0.285514f, 0.291765f, 0.298154f, 0.304681f, 0.311353f, 0.318171f,
    0.325138f, 0.332258f, 0.339534f, 0.346969f, 0.354566f, 0.362331f, 0.370265f, 0.378372f,
    0.386657f, 0.395124f, 0.403777f, 0.412619f, 0.421652f, 0.430885f, 0.440321f, 0.449964f,
    0.459817f, 0.469885f, 0.480174f, 0.490689f, 0.501433f, 0.512413f, 0.523633f, 0.535100f,
    0.546816f, 0.558790f, 0.571027f, 0.583530f, 0.596307f, 0.609365f, 0.622708f, 0.636344f,
    0.650279f, 0.664518f, 0.679069f, 0.693939f, 0.709133f, 0.724662f, 0.740529f, 0.756745f,
    0.773316f, 0.790249f, 0.807554f, 0.825237f, 0.843307f, 0.861772f, 0.880643f, 0.899928f,
    0.919633f, 0.939771f, 0.960348f, 0.981378f, 1.002867f },

	{ 0x0, 0x2, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e,
    0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e,
    0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e,
    0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e,
    0x40, 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e,
    0x50, 0x52, 0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e,
    0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x6c, 0x6e,
    0x70, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x7e,
    0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c, 0x8e,
    0x90, 0x92, 0x94, 0x96, 0x98, 0x9a, 0x9c, 0x9e,
    0xa0, 0xa2, 0xa4, 0xa6, 0xa8, 0xaa, 0xac, 0xae,
    0xb0, 0xb2, 0xb4, 0xb6, 0xb8, 0xba, 0xbc, 0xbe,
    0xc0, 0xc2, 0xc4, 0xc6, 0xc8 },

	{ 0x0040, 0x8040, 0x0041, 0x8041, 0x0042, 0x8042, 0x0043, 0x8043,
    0x0044, 0x8044, 0x0045, 0x8045, 0x0046, 0x8046, 0x0047, 0x8047,
    0x0048, 0x8048, 0x0049, 0x8049, 0x004a, 0x804a, 0x004b, 0x804b,
    0x004c, 0x804c, 0x004d, 0x804d, 0x004e, 0x804e, 0x004f, 0x804f,
    0x0050, 0x8050, 0x0051, 0x8051, 0x0052, 0x8052, 0x0053, 0x8053,
    0x0054, 0x8054, 0x0055, 0x8055, 0x0056, 0x8056, 0x0057, 0x8057,
    0x0058, 0x8058, 0x0059, 0x8059, 0x005a, 0x805a, 0x005b, 0x805b,
    0x005c, 0x805c, 0x005d, 0x805d, 0x005e, 0x805e, 0x005f, 0x805f,
    0x0060, 0x8060, 0x0061, 0x8061, 0x0062, 0x8062, 0x0063, 0x8063,
    0x0064, 0x8064, 0x0065, 0x8065, 0x0066, 0x8066, 0x0067, 0x8067,
    0x0068, 0x8068, 0x0069, 0x8069, 0x006a, 0x806a, 0x006b, 0x806b,
    0x006c, 0x806c, 0x006d, 0x806d, 0x006e, 0x806e, 0x006f, 0x806f,
    0x0070, 0x8070, 0x0071, 0x8071, 0x0072 }
};

uint8_t mcu_crc8_table[] = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

uint32_t iron_palette[] = {
    0xff000014, 0xff000025, 0xff00002a, 0xff000032, 0xff000036, 0xff00003e, 0xff000042, 0xff00004f,
    0xff010055, 0xff010057, 0xff02005c, 0xff03005e, 0xff040063, 0xff050065, 0xff070069, 0xff0a0070,
    0xff0b0073, 0xff0d0075, 0xff0d0076, 0xff100078, 0xff120079, 0xff15007c, 0xff17007d, 0xff1c0081,
    0xff200084, 0xff220085, 0xff260087, 0xff280089, 0xff2c008a, 0xff2e008b, 0xff32008d, 0xff38008f,
    0xff390090, 0xff3c0092, 0xff3e0093, 0xff410094, 0xff420095, 0xff450096, 0xff470096, 0xff4c0097,
    0xff4f0097, 0xff510097, 0xff540098, 0xff560098, 0xff5a0099, 0xff5c0099, 0xff5f009a, 0xff64009b,
    0xff66009b, 0xff6a009b, 0xff6c009c, 0xff6f009c, 0xff70009c, 0xff73009d, 0xff75009d, 0xff7a009d,
    0xff7e009d, 0xff7f009d, 0xff83009d, 0xff84009d, 0xff87009d, 0xff89009d, 0xff8b009d, 0xff91009c, 
    0xff93009c, 0xff96009b, 0xff98009b, 0xff9b009b, 0xff9c009b, 0xff9f009b, 0xffa0009b, 0xffa4009b,
    0xffa7009a, 0xffa8009a, 0xffaa0099, 0xffab0099, 0xffae0198, 0xffaf0198, 0xffb00198, 0xffb30196,
    0xffb40296, 0xffb60295, 0xffb70395, 0xffb90495, 0xffba0495, 0xffbb0593, 0xffbc0593, 0xffbf0692,
    0xffc00791, 0xffc00791, 0xffc10990, 0xffc20a8f, 0xffc30b8e, 0xffc40c8d, 0xffc60d8b, 0xffc81088,
    0xffc91187, 0xffca1385, 0xffcb1385, 0xffcc1582, 0xffcd1681, 0xffce187e, 0xffcf187c, 0xffd11b78,
    0xffd21c75, 0xffd21d74, 0xffd32071, 0xffd4216f, 0xffd5236b, 0xffd52469, 0xffd72665, 0xffd92a60,
    0xffda2b5e, 0xffdb2e5a, 0xffdb2f57, 0xffdd3051, 0xffdd314e, 0xffde3347, 0xffdf3444, 0xffe0373a,
    0xffe03933, 0xffe13a30, 0xffe23c2a, 0xffe33d26, 0xffe43f20, 0xffe4411d, 0xffe5431b, 0xffe64616,
    0xffe74715, 0xffe74913, 0xffe84a12, 0xffe84c0f, 0xffe94d0e, 0xffea4e0c, 0xffea4f0c, 0xffeb520a,
    0xffec5409, 0xffec5608, 0xffec5808, 0xffed5907, 0xffed5b06, 0xffee5c06, 0xffee5d05, 0xffef6004,
    0xffef6104, 0xfff06303, 0xfff06403, 0xfff16603, 0xfff16603, 0xfff16803, 0xfff16902, 0xfff16b02,
    0xfff26d01, 0xfff26e01, 0xfff37001, 0xfff37101, 0xfff47300, 0xfff47400, 0xfff47600, 0xfff47a00,
    0xfff57b00, 0xfff57e00, 0xfff57f00, 0xfff68100, 0xfff68200, 0xfff78400, 0xfff78500, 0xfff88800,
    0xfff88900, 0xfff88a00, 0xfff88c00, 0xfff98d00, 0xfff98e00, 0xfff98f00, 0xfff99100, 0xfffa9400,
    0xfffa9500, 0xfffb9800, 0xfffb9900, 0xfffb9c00, 0xfffc9d00, 0xfffca000, 0xfffca100, 0xfffda400,
    0xfffda700, 0xfffda800, 0xfffdab00, 0xfffdac00, 0xfffdae00, 0xfffeaf00, 0xfffeb100, 0xfffeb400,
    0xfffeb500, 0xfffeb800, 0xfffeb900, 0xfffeba00, 0xfffebb00, 0xfffebd00, 0xfffebe00, 0xfffec200,
    0xfffec400, 0xfffec500, 0xfffec700, 0xfffec800, 0xfffeca01, 0xfffeca01, 0xfffecc02, 0xfffecf04,
    0xfffecf04, 0xfffed106, 0xfffed308, 0xfffed50a, 0xfffed60a, 0xfffed80c, 0xfffed90d, 0xffffdb10,
    0xffffdc14, 0xffffdd16, 0xffffde1b, 0xffffdf1e, 0xffffe122, 0xffffe224, 0xffffe328, 0xffffe531,
    0xffffe635, 0xffffe73c, 0xffffe83f, 0xffffea46, 0xffffeb49, 0xffffec50, 0xffffed54, 0xffffee5f,
    0xffffef67, 0xfffff06a, 0xfffff172, 0xfffff177, 0xfffff280, 0xfffff285, 0xfffff38e, 0xfffff49a,
    0xfffff59e, 0xfffff5a6, 0xfffff6aa, 0xfffff7b3, 0xfffff7b6, 0xfffff8bd, 0xfffff8c1, 0xfffff9ca,
    0xfffffad1, 0xfffffad4, 0xfffffcdb, 0xfffffcdf, 0xfffffde5, 0xfffffde8, 0xfffffeee, 0xfffffff6
};


