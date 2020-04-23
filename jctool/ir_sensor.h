#pragma once

// crc-8-ccitt / polynomial 0x07 look up table
extern uint8_t mcu_crc8_table[];

// ARGB Ironbow palette
extern uint32_t iron_palette[];


enum IRColor {
    IRGreyscale,
    IRNightVision,
    IRIronbow,
    IRInfrared,
    IRColorCount
};
/**
 * Enum values for the various resolutions of the IR Camera.
 * These are based on the normal orientation of the camera,
 * which is when the rail is facing upwards.
 */
enum IRResolution : u8 {
    IR_320x240 = 0b00000000,
    IR_160x120 = 0b1010000,
    IR_80x60 = 0b01100100,
    IR_40x30 = 0b01101001
};
