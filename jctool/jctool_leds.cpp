#include "jctool_leds.hpp"
#include "hidapi.h"

namespace LEDs {
    /**
     * The function zeros the buffer before modifying it.
     */
    int set_player_leds(CT& ct, LEDFlags value, ConCom::Packet& pkt_buf){
        pkt_buf.zero();
        auto& hdr = pkt_buf.header();
        auto& pkt = pkt_buf.command();
        hdr->cmd = ConCom::SUBC;
        hdr->timer = ct.timming_byte & 0xF;
        pkt->subcmd = ConCom::SLED;
        pkt->subcmd_arg.arg1 = value;
        return ConCom::send_pkt(ct, pkt_buf);
    }

    int set_led_busy(CT& ct, ConHID::ProdID con_type) {
        controller_hid_handle_t& handle = ct.handle;
        u8& timming_byte = ct.timming_byte;
        int res;
        ConCom::Packet p;

        auto& hdr = p.header();
        auto& pkt = p.command();
        set_player_leds(ct, LEDFlags(LED0_Flash | LED3_On), p);
        res = hid_read_timeout(handle, p.buf, 1, 64);

        //Set breathing HOME Led
        if(con_type != ConHID::JoyConLeft)
        {
            p.zero();
            hdr->cmd = ConCom::SUBC;
            hdr->timer = timming_byte & 0xF;
            pkt->subcmd = ConCom::HLED;
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