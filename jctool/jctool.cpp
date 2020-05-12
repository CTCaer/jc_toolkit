// Copyright (c) 2018 CTCaer. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// TODO: "hdr->timer = timming_byte & 0xF", should this be "timming_byte % 0xF"?
#include <string>
// #define NOMINMAX
#include <chrono>
#ifdef WIN32
#include <cstdio>
#include <Windows.h>
#else
#include <stdio.h>
#endif

#include "jctool.h"
#include "jctool_helpers.hpp"
#include "jctool_mcu.hpp"
#include "con_com.hpp"

#define EASY_HELPERS_UI
#include "easy-imgui/easy_imgui.h"
#include "jctool_ui.hpp"

#include "hidapi.h"

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

#ifndef __jctool_disable_legacy_ui__
int send_custom_command(u8* arg) {
#else
int send_custom_command(CT& ct, u8* arg){
    controller_hid_handle_t& handle = ct.handle;
    u8& timming_byte = ct.timming_byte;
#endif
    int res_write;
    int res;
    int byte_seperator = 1;
#ifndef __jctool_disable_legacy_ui__
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
        arg[43] = MCU::mcu_crc8_calc(arg + 7, 36);
#ifndef __jctool_disable_legacy_ui__
    output_report_sys = String::Format(L"Cmd:  {0:X2}   Subcmd: {1:X2}\r\n", buf_cmd[0], buf_cmd[10]);
#else
    // TODO: Implement else
    //output_report_sys << "Cmd: " << std::setbase(hex) << buf_cmd[0]
#endif
    if (buf_cmd[0] == 0x01 || buf_cmd[0] == 0x10 || buf_cmd[0] == 0x11) {
        for (int i = 6; i < 44; i++) {
            buf_cmd[5 + i] = arg[i];
#ifndef __jctool_disable_legacy_ui__
            output_report_sys += String::Format(L"{0:X2} ", buf_cmd[5 + i]);
            if (byte_seperator == 4)
                output_report_sys += L" ";
#else
            // TODO: Implement else
#endif
            if (byte_seperator == 8) {
                byte_seperator = 0;
#ifndef __jctool_disable_legacy_ui__
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
#ifndef __jctool_disable_legacy_ui__
            output_report_sys += String::Format(L"{0:X2} ", buf_cmd[i - 5]);
            if (byte_seperator == 4)
                output_report_sys += L" ";
#else
            // TODO: Implement else
#endif
            if (byte_seperator == 8) {
                byte_seperator = 0;
#ifndef __jctool_disable_legacy_ui__
                output_report_sys += L"\r\n";
#else
                // TODO: Implement else
#endif
            }
            byte_seperator++;
        }
    }
#ifndef __jctool_disable_legacy_ui__
    FormJoy::myform1->textBoxDbg_sent->Text = output_report_sys;
#else
    // TODO: Implement else
#endif

    //Packet size header + subcommand and uint8 argument
    res_write = hid_write(handle, buf_cmd, sizeof(buf_cmd));
#ifndef __jctool_disable_legacy_ui__
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
#ifndef __jctool_disable_legacy_ui__
            input_report_cmd += String::Format(L"\r\nInput report: 0x{0:X2}\r\n", buf_reply[0]);
            input_report_sys += String::Format(L"Subcmd Reply:\r\n", buf_reply[0]);
#else
            // TODO: Implement else
#endif
            int len = 49;
            if (buf_reply[0] == 0x33 || buf_reply[0] == 0x31)
                len = 362;
            for (int i = 1; i < 13; i++) {
#ifndef __jctool_disable_legacy_ui__
                input_report_cmd += String::Format(L"{0:X2} ", buf_reply[i]);
                if (byte_seperator == 4)
                    input_report_cmd += L" ";
#else
                // TODO: Implement else
#endif
                if (byte_seperator == 8) {
                    byte_seperator = 0;
#ifndef __jctool_disable_legacy_ui__
                    input_report_cmd += L"\r\n";
#else
                    // TODO: Implement else
#endif
                }
                byte_seperator++;
            }
            byte_seperator = 1;
            for (int i = 13; i < len; i++) {
#ifndef __jctool_disable_legacy_ui__
                input_report_sys += String::Format(L"{0:X2} ", buf_reply[i]);
                if (byte_seperator == 4)
                    input_report_sys += L" ";
#else
                // TODO: Implement else
#endif
                if (byte_seperator == 8) {
                    byte_seperator = 0;
#ifndef __jctool_disable_legacy_ui__
                    input_report_sys += L"\r\n";
#else
                    // TODO: Implement else
#endif
                }
                byte_seperator++;
            }
            int crc_check_ok = 0;
            if (arg[5] == 0x21) {
                crc_check_ok = (buf_reply[48] == MCU::mcu_crc8_calc(buf_reply + 0xF, 33));
#ifndef __jctool_disable_legacy_ui__
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
#ifndef __jctool_disable_legacy_ui__
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
#ifndef __jctool_disable_legacy_ui__
        for (int i = 0; i < res; i++)
            input_report_sys += String::Format(L"{0:X2} ", buf_reply[i]);
#else
        // TODO: Implement else
#endif
    }
    else {
#ifndef __jctool_disable_legacy_ui__
        input_report_sys += L"No reply";
#else
        // TODO: Implement else
#endif
    }
#ifndef __jctool_disable_legacy_ui__
    FormJoy::myform1->textBoxDbg_reply->Text = input_report_sys;
    FormJoy::myform1->textBoxDbg_reply_cmd->Text = input_report_cmd;
#else
    // TODO: Implement else
#endif

    return 0;
}

namespace ConCom {
    int set_input_report_x31(CT& ct, ConCom::Packet& p, u8* buf_read, size_t buf_read_size){
        controller_hid_handle_t& handle = ct.handle;
        u8& timming_byte = ct.timming_byte;
        auto& hdr = p.header();
        auto& pkt = p.command();
        int res;
        int error_reading = 0;
        while (1) {
            p.zero();
            hdr->cmd = 1;
            hdr->timer = timming_byte & 0xF;
            pkt->subcmd = 0x03;
            pkt->subcmd_arg.arg1 = 0x31;
            res = ConCom::send_pkt(ct, p);
            int retries = 0;
            while (1) {
                res = hid_read_timeout(handle, buf_read, buf_read_size, 64);
                if (*(u16*)&buf_read[0xD] == 0x0380)
                    return res;

                retries++;
                if (retries > 8 || res == 0)
                    break;
            }
            error_reading++;
            if (error_reading > 7)
                return 0;
        }
        return res; // TODO: Is this reachable?
    }
}

#ifndef __jctool_disable_legacy_ui__
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

#ifndef __jctool_disable_legacy_ui__
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
#ifndef __jctool_disable_legacy_ui__
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

#ifndef __jctool_no_UI__
int main(int argc, char** args) {
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
    return 0;
}
#endif
