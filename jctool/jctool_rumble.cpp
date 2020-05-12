#include <iomanip>
#include <iostream>

#include "jctool.h"
#include "luts.h"
#include "con_com.hpp"

#include "tune.h"

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

namespace Rumble {
    int enable_rumble(CT& ct, ConCom::Packet& packet_buf){
        packet_buf.zero();
        auto& hdr = packet_buf.header();
        auto& pkt = packet_buf.command();
        hdr->cmd = 0x01;
        hdr->timer = ct.timming_byte & 0xF;
        pkt->subcmd = 0x48;
        pkt->subcmd_arg.arg1 = 0x01;
        int res = ConCom::send_pkt(ct, packet_buf);
        return res;
    }

    #ifndef __jctool_disable_legacy_ui__
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
        res = Rumble::enable_rumble(ct, p);
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
    #ifndef __jctool_disable_legacy_ui__
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

        #ifndef __jctool_disable_legacy_ui__
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
        res = Rumble::enable_rumble(ct, packet_buf);
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
    #ifndef __jctool_disable_legacy_ui__
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

    #ifndef __jctool_disable_legacy_ui__
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
        ConCom::Packet p;
        auto& hdr = p.header();
        auto& pkt = p.command();
        u8 buf2[49];

        //Enable Vibration
        res = Rumble::enable_rumble(ct, p);
        res = hid_read_timeout(handle, buf2, 1, 120);

        if (file_type == VIBRaw || file_type == VIBBinary) {
            for (int i = 0; i < samples * 4; i = i + 4) {
                Sleep(sample_rate);
                p.zero();
                hdr->cmd = 0x10;
                hdr->timer = timming_byte & 0xF;
                timming_byte++;

                auto use_rumble_data
    #ifndef __jctool_disable_legacy_ui__
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

                res = hid_write(handle, p.buf, sizeof(*hdr));
    #ifndef __jctool_disable_legacy_ui__
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
                p.zero();
                hdr->cmd = 0x10;
                hdr->timer = timming_byte & 0xF;
                timming_byte++;

                auto use_rumble_data
    #ifndef __jctool_disable_legacy_ui__
                = FormJoy::myform1->vib_loaded_file;
    #else
                = &*rumble_data.data;
    #endif
                hdr->rumble_l[0] = use_rumble_data[0x0C + vib_off + i];
                hdr->rumble_l[1] = use_rumble_data[0x0D + vib_off + i];
                hdr->rumble_l[2] = use_rumble_data[0x0E + vib_off + i];
                hdr->rumble_l[3] = use_rumble_data[0x0F + vib_off + i];

                memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
                
                res = hid_write(handle, p.buf, sizeof(*hdr));
    #ifndef __jctool_disable_legacy_ui__
                Application::DoEvents();
    #else
                // TODO: Implement else
    #endif
            }
            for (int j = 0; j < 1 + loop_times; j++) {
                for (int i = loop_start * 4; i < loop_end * 4; i = i + 4) {
                    Sleep(sample_rate);
                    p.zero();
                    hdr->cmd = 0x10;
                    hdr->timer = timming_byte & 0xF;
                    timming_byte++;

                    auto use_rumble_data
    #ifndef __jctool_disable_legacy_ui__
                    = FormJoy::myform1->vib_loaded_file;
    #else
                    = &*rumble_data.data;
    #endif
                    hdr->rumble_l[0] = use_rumble_data[0x0C + vib_off + i];
                    hdr->rumble_l[1] = use_rumble_data[0x0D + vib_off + i];
                    hdr->rumble_l[2] = use_rumble_data[0x0E + vib_off + i];
                    hdr->rumble_l[3] = use_rumble_data[0x0F + vib_off + i];

                    memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));

                    res = hid_write(handle, p.buf, sizeof(*hdr));
    #ifndef __jctool_disable_legacy_ui__
                    Application::DoEvents();
    #else
                    // TODO: Implement else
    #endif
                }
                Sleep(sample_rate);
                // Disable vibration
                p.zero();
                hdr->cmd = 0x10;
                hdr->timer = timming_byte & 0xF;
                timming_byte++;
                hdr->rumble_l[0] = 0x00;
                hdr->rumble_l[1] = 0x01;
                hdr->rumble_l[2] = 0x40;
                hdr->rumble_l[3] = 0x40;
                memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
                res = hid_write(handle, p.buf, sizeof(*hdr));
                Sleep(loop_wait * sample_rate);
            }
            for (int i = loop_end * 4; i < samples * 4; i = i + 4) {
                Sleep(sample_rate);
                p.zero();
                hdr->cmd = 0x10;
                hdr->timer = timming_byte & 0xF;
                timming_byte++;
                auto use_rumble_data
    #ifndef __jctool_disable_legacy_ui__
                = FormJoy::myform1->vib_loaded_file;
    #else
                = &*rumble_data.data;
    #endif
                hdr->rumble_l[0] = use_rumble_data[0x0C + vib_off + i];
                hdr->rumble_l[1] = use_rumble_data[0x0D + vib_off + i];
                hdr->rumble_l[2] = use_rumble_data[0x0E + vib_off + i];
                hdr->rumble_l[3] = use_rumble_data[0x0F + vib_off + i];

                memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));

                res = hid_write(handle, p.buf, sizeof(*hdr));
    #ifndef __jctool_disable_legacy_ui__
                Application::DoEvents();
    #else
                // TODO: Implement else
    #endif
            }
        }

        Sleep(sample_rate);
        // Disable vibration
        p.zero();
        hdr->cmd = 0x10;
        hdr->timer = timming_byte & 0xF;
        hdr->rumble_l[0] = 0x00;
        hdr->rumble_l[1] = 0x01;
        hdr->rumble_l[2] = 0x40;
        hdr->rumble_l[3] = 0x40;
        memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
        res = ConCom::send_pkt(ct, p);

        Sleep(sample_rate + 120);
        p.zero();
        hdr->cmd = 0x01;
        hdr->timer = timming_byte & 0xF;
        hdr->rumble_l[0] = 0x00;
        hdr->rumble_l[1] = 0x01;
        hdr->rumble_l[2] = 0x40;
        hdr->rumble_l[3] = 0x40;
        memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
        pkt->subcmd = 0x48;
        res = ConCom::send_pkt(ct, p);
        res = hid_read_timeout(handle, p.buf, 1, 64);

        p.zero();
        hdr->cmd = 0x01;
        hdr->timer = timming_byte & 0xF;
        pkt->subcmd = 0x30;
        pkt->subcmd_arg.arg1 = 0x01;
        res = ConCom::send_pkt(ct, p);
        res = hid_read_timeout(handle, p.buf, 1, 64);

        return 0;
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
}
