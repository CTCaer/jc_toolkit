#include "jctool_leds.hpp"

#include "con_com.hpp"
#include "hidapi.h"

namespace LEDS {
    #ifndef __jctool_disable_legacy_ui__
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
            pkt->subcmd_arg.arg1 = 0x28;
            pkt->subcmd_arg.arg2 = 0x20;
            p.buf[13] = 0xF2;
            p.buf[14] = p.buf[15] = 0xF0;
            res = ConCom::send_pkt(ct, p);
            res = hid_read_timeout(handle, p.buf, 1, 64);
        }

        return 0;
    }
}