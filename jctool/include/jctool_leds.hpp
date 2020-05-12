#pragma once
#include "jctool_types.h"
#include "con_com.hpp"

namespace LEDs {
    enum LEDFlags : u8 {
        LED0_On    = 1,
        LED1_On    = 2,
        LED2_0n    = 4,
        LED3_On    = 8,
        LED0_Flash = 16,
        LED1_Flash = 32,
        LED2_Flash = 64,
        LED3_Flash = 128,
    };
    int set_player_leds(CT& ct, LEDFlags value, ConCom::Packet& pkt_buf);
    int set_led_busy(CT& ct, ConHID::ProdID con_type);
}