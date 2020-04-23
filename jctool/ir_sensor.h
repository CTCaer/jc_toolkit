#pragma once
#include <tuple>
#include "jctool_types.h"

// crc-8-ccitt / polynomial 0x07 look up table
extern uint8_t mcu_crc8_table[];

// ARGB Ironbow palette
extern uint32_t iron_palette[];

inline constexpr std::tuple<const char*, IRResolution, Size2D> ir_resolutions[] = {
    std::tuple<const char*, IRResolution, Size2D>("320 x 240", IR_320x240, {320, 240}),
    std::tuple<const char*, IRResolution, Size2D>("160 x 120", IR_160x120, {160, 120}),
    std::tuple<const char*, IRResolution, Size2D>("80 x 60", IR_80x60, {80, 60}),
    std::tuple<const char*, IRResolution, Size2D>("40 x 30", IR_40x30, {40, 30})
};
