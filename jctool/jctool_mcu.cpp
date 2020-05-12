#include <cstring>

#include "jctool.h"
#include "jctool_mcu.hpp"
#include "jctool_helpers.hpp"

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

namespace MCU {
    u8 mcu_crc8_calc(u8 *buf, u8 size) {
        u8 crc8 = 0x0;

        for (int i = 0; i < size; ++i) {
            crc8 = mcu_crc8_table[(u8)(crc8 ^ buf[i])];
        }
        return crc8;
    }

    #ifndef __jctool_disable_legacy_ui__
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
    #ifndef __jctool_disable_legacy_ui__
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

    #ifndef __jctool_disable_legacy_ui__
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

    #ifndef __jctool_disable_legacy_ui__
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

    #ifndef __jctool_disable_legacy_ui__
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
    #ifndef __jctool_disable_legacy_ui__
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

    #ifndef __jctool_disable_legacy_ui__
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
    #ifndef __jctool_disable_legacy_ui__
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
    #ifndef __jctool_disable_legacy_ui__
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
    #ifndef __jctool_disable_legacy_ui__
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
    #ifdef __jctool_disable_legacy_ui__
            dump_spi_ctx.bytes_dumped = offset;
    #endif
        }
        fclose(f);

        return 0;
}

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


}