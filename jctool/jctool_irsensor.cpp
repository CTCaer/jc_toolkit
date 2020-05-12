#include <cstring>
#include <iomanip>
#include <iostream>

#include "jctool.h"
#include "jctool_mcu.hpp"
#include "jctool_irsensor.hpp"
#include "jctool_helpers.hpp"
#include "ir_sensor.h"

#include "con_com.hpp"
#include "hidapi.h"


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
        ack.buf[47] = MCU::mcu_crc8_calc(ack.buf + 11, 36);
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
        else if (is_empty_report(packet_buf)) {
            pd.type = Empty;
        }
        return pd;
    }

        #ifndef __jctool_disable_legacy_ui__
    int ir_sensor_auto_exposure(int white_pixels_percent) {
    #else
    int ir_sensor_auto_exposure(CT& ct, int white_pixels_percent) {
        controller_hid_handle_t& handle = ct.handle;
        u8& timming_byte = ct.timming_byte;
    #endif
        int res;
        u8 buf[49];
        u16 new_exposure = 0;
    #ifndef __jctool_disable_legacy_ui__
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

        buf[48] = MCU::mcu_crc8_calc(buf + 12, 36);
        res = hid_write(handle, buf, sizeof(buf));

        return res;
    }

    #ifndef __jctool_disable_legacy_ui__
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
    #ifndef __jctool_disable_legacy_ui__
        System::Diagnostics::Stopwatch^ sw = System::Diagnostics::Stopwatch::StartNew();
    #else
        int frame_counter = 0;
        auto start = std::chrono::system_clock::now();
        auto _elapsedClockTimeMS = [&start](){
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
        };
    #endif

        int buf_size_img = get_img_buf_size(ir_max_frag_no);

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

        StreamCTX sctx {
            previous_frag_no,
            missed_packet_no,
            ir_max_frag_no,
            missed_packet
        };

        acknowledge ack {
            handle,
            timming_byte,
            hdr,
            p.buf,
            p.buf_size
        };

        // First ack
        ack_packet(ack, 0x0);

        // IR Read/ACK loop for fragmented data packets. 
        // It also avoids requesting missed data fragments, we just skip it to not complicate things.
    #ifndef __jctool_disable_legacy_ui__
        while (enable_IRVideoPhoto || initialization)
    #else
        while((capture_mode == IRCaptureMode::Video) || initialization)
    #endif
        {
            int packet_res = request_packet(handle, buf_reply, sizeof(buf_reply));
            /* Test */ {
                auto pd = get_packet_desc(sctx, buf_reply);
                got_frag_no = pd.frag_no;

                switch (pd.type)
                {
                case PacketType::Start:
                    ack_packet(ack, got_frag_no);
    #ifndef __jctool_disable_legacy_ui__
                    FormJoy::myform1->lbl_IRStatus->Text = (sw->ElapsedMilliseconds - elapsed_time).ToString() + "ms";
                    elapsed_time = sw->ElapsedMilliseconds;
                    Application::DoEvents();
    #else
                    elapsed_time = _elapsedClockTimeMS();
    #endif
                    previous_frag_no = 0;
                    break;
                // Final and Next flow result from the same logic, but Final is unique (Adressed after the switches.)
                case PacketType::Final:
                case PacketType::Next:{
                    capture_context.capture_status.last_frag_no = got_frag_no;
                    ack_packet(ack, got_frag_no);
                    previous_frag_no = got_frag_no;

                    // Auto exposure.
                    // TODO: Fix placement, so it doesn't drop next fragment.
    #ifndef __jctool_disable_legacy_ui__
                    if (enable_IRAutoExposure 
    #else
                    //if(use_ir_sensor.auto_exposure
    #endif
                    //&& initialization < 2 && got_frag_no == 0){
                    //    white_pixels_percent = (int)((*(u16*)&buf_reply[55] * 100) / buf_size_img);
    #ifndef __jctool_disable_legacy_ui__
                        ir_sensor_auto_exposure(white_pixels_percent);
    #else
                    //    ir_sensor_auto_exposure(handle, timming_byte, white_pixels_percent);
    #endif
                    //}
                }break;
                case PacketType::Repeat:
                    // Check if repeat ACK should be send. Avoid writing to image buffer.
                    // Repeat of fragment that is not max fragment.
                    ack_packet(ack, got_frag_no);
                    missed_packet = false;
                    break;
                case PacketType::HasJustMissed:
                    if ((pd.flags & PacketFlags::AckMissed) == PacketFlags::AckMissed) {
                        // Missed packet
                        ack_missed_packet(ack, previous_frag_no);
                        previous_frag_no = got_frag_no;
                        missed_packet_no = got_frag_no - 1;
                        missed_packet = true;
                    }
                    // Check if missed fragment and res is 30x40. Don't request it.
                    else {
                        ack_packet(ack, got_frag_no);
                        previous_frag_no = got_frag_no;
                    }
                    break;
                case PacketType::GotMissedFrag:
                    // Got the requested missed fragments.
                    //debug
                    //printf("%02X Frag: Got missed %02X\n", got_frag_no, missed_packet_no);
                    ack_packet(ack, got_frag_no);

                    previous_frag_no = got_frag_no;
                    missed_packet = false;
                    break;
                case PacketType::Empty:
                    // Empty IR report. Send Ack again. Otherwise, it fallbacks to high latency mode (30ms per data fragment)
                default:
                    // Send ACK again or request missed frag
                    if (buf_reply[49] == 0xFF) {
                        ack_packet(ack, previous_frag_no);
                    }
                    else if (buf_reply[49] == 0x00) {
                        ack_missed_packet(ack, previous_frag_no);
                    }
                    ack_buf_12_13_nullify(ack.buf);
                    break;
                }

                if((pd.flags & PacketFlags::UpdateIRStatus) == PacketFlags::UpdateIRStatus){
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
    #ifndef __jctool_disable_legacy_ui__
                    FormJoy::myform1->lbl_IRStatus->Text = gcnew String(ir_status.str().c_str()) + (sw->ElapsedMilliseconds - elapsed_time).ToString() + "ms";
                    elapsed_time = sw->ElapsedMilliseconds;
                    Application::DoEvents();
    #else
                    elapsed_time = _elapsedClockTimeMS();
    #endif
                }

                if((pd.flags & PacketFlags::WriteFrag) == PacketFlags::WriteFrag){
                    //assert(got_frag_no <= ir_max_frag_no);
                    if(!(got_frag_no <= ir_max_frag_no))
                        std::cout << "Got frag #" << got_frag_no << ", but the max frag # is " << (int)ir_max_frag_no << std::endl;
                    else
                        memcpy(buf_image.get() + (FRAG_SIZE * got_frag_no), buf_reply + FRAG_START_OFFSET, FRAG_SIZE);
                }

                // Check if final fragment. Draw the frame.
                if(pd.type == Final){
                    // Update Viewport
    #ifndef __jctool_disable_legacy_ui__
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
    #ifndef __jctool_disable_legacy_ui__
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

    #ifndef __jctool_disable_legacy_ui__
    int ir_sensor(ir_image_config &ir_cfg) {
    #else
    int ir_sensor(IRCaptureCTX& capture_context, StoreRawCaptureCB store_capture_cb) {
        controller_hid_handle_t handle = capture_context.handle;
        u8& timming_byte = capture_context.timming_byte;
        IRCaptureMode& capture_mode = capture_context.capture_mode;
        ir_image_config& ir_cfg = capture_context.ir_cfg;
        u8& ir_max_frag_no = capture_context.ir_max_frag_no;
        CT ct{handle, timming_byte};
        ConCom::Packet pout;
    #endif
        int error_reading = 0;
        int res;
        u8 buf[0x170];
        static int output_buffer_length = 49;
        int res_get = 0;
        // Set input report to x31
        res = ConCom::set_input_report_x31(ct, pout, buf, sizeof(buf));
        if(res == 0)
            goto step10;
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

            buf[48] = MCU::mcu_crc8_calc(buf + 12, 36);
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

            buf[48] = MCU::mcu_crc8_calc(buf + 12, 36);
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

            buf[47] = MCU::mcu_crc8_calc(buf + 11, 36);
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

            buf[48] = MCU::mcu_crc8_calc(buf + 12, 36);
            res = hid_write(handle, buf, output_buffer_length);

            // Request IR mode status, before waiting for the x21 ack
            memset(buf, 0, sizeof(buf));
            hdr->cmd = 0x11;
            hdr->timer = timming_byte & 0xF;
            timming_byte++;
            pkt->subcmd = 0x03;
            pkt->subcmd_arg.arg1 = 0x02;
            buf[47] = MCU::mcu_crc8_calc(buf + 11, 36);
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

            buf[48] = MCU::mcu_crc8_calc(buf + 12, 36);
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
    #ifndef __jctool_disable_legacy_ui__
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

    #ifndef __jctool_disable_legacy_ui__
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

            buf[47] = MCU::mcu_crc8_calc(buf + 11, 36);

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

    #ifndef __jctool_disable_legacy_ui__
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

        buf[48] = MCU::mcu_crc8_calc(buf + 12, 36);
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

        buf[48] = MCU::mcu_crc8_calc(buf + 12, 36);
        res = hid_write(handle, buf, sizeof(buf));

        // get_ir_registers(0,4); // Get all register pages
        // get_ir_registers((ir_cfg.ir_custom_register >> 8) & 0xFF, (ir_cfg.ir_custom_register >> 8) & 0xFF); // Get all registers based on changed register's page

        return res;
    }

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
}