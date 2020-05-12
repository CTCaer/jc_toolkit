#include <cstring>

#include "jctool.h"
#include "jctool_mcu.hpp"
#include "jctool_nfc.hpp"

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

namespace NFC {
    #ifndef __jctool_disable_legacy_ui__
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

            buf[48] = MCU::mcu_crc8_calc(buf + 12, 36);
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

            buf[47] = MCU::mcu_crc8_calc(buf + 11, 36); //Without the last byte
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

            buf[47] = MCU::mcu_crc8_calc(buf + 11, 36); //Without the last byte
            res = hid_write(handle, buf, output_buffer_length - 1);
            int retries = 0;
            while (1) {
    #ifndef __jctool_disable_legacy_ui__
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
    #ifndef __jctool_disable_legacy_ui__
                        FormJoy::myform1->txtBox_nfcUid->Text = "UID:  ";
    #else
        // TODO: Implement else
    #endif
                        for (int i = 0; i < tag_uid_size; i++) {
                            if (i < tag_uid_size - 1) {
                                tag_type = buf[62]; // Save tag type
                                tag_uid_buf[i] = buf[65 + i]; // Save UID
    #ifndef __jctool_disable_legacy_ui__
                                FormJoy::myform1->txtBox_nfcUid->Text += String::Format("{0:X2}:", buf[65 + i]);
    #else
        // TODO: Implement else
    #endif
                            }
                            else {
    #ifndef __jctool_disable_legacy_ui__
                                FormJoy::myform1->txtBox_nfcUid->Text += String::Format("{0:X2}", buf[65 + i]);
    #else
        // TODO: Implement else
    #endif
                            }
                        }
    #ifndef __jctool_disable_legacy_ui__
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
    #ifndef __jctool_disable_legacy_ui__
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
    #ifndef __jctool_disable_legacy_ui__
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

            buf2[47] = MCU::mcu_crc8_calc(buf2 + 11, 36);

            res = hid_write(handle, buf2, output_buffer_length - 1);

            int retries = 0;
            while (1) {
                res = hid_read_timeout(handle, buf2, sizeof(buf2), 64);
                if (buf2[0] == 0x31) {
                    if ((buf2[49] == 0x3a || buf2[49] == 0x2a) && buf2[56] == 0x07) {
    #ifndef __jctool_disable_legacy_ui__
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
    #ifndef __jctool_disable_legacy_ui__
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

                        buf[47] = MCU::mcu_crc8_calc(buf + 11, 36); //Without the last byte
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
    #ifndef __jctool_disable_legacy_ui__
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
}