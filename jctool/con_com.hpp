#pragma once
#include "jctool_types.h"
#include <cstring>
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
     * CT is a controller handle and timming byte reference pair.
     */
    inline int send_pkt(CT& ct, Packet& pkt){
        ct.timming_byte++;
        return hid_write(ct.handle, pkt.buf, sizeof(pkt.buf));
    }
    
    int set_input_report_x31(CT& ct, ConCom::Packet& p, u8* buf_read, size_t buf_read_size);
};