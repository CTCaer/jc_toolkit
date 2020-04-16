#pragma once
#include <memory>
#include <cstdint>
#include "hidapi.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#pragma pack(push, 1)

struct brcm_hdr {
    u8 cmd;
    u8 timer;
    u8 rumble_l[4];
    u8 rumble_r[4];
};

struct brcm_cmd_01 {
    u8 subcmd;
    union {
        struct {
            u32 offset;
            u8 size;
        } spi_data;

        struct {
            u8 arg1;
            u8 arg2;
        } subcmd_arg;

        struct {
            u8  mcu_cmd;
            u8  mcu_subcmd;
            u8  mcu_mode;
        } subcmd_21_21;

        struct {
            u8  mcu_cmd;
            u8  mcu_subcmd;
            u8  no_of_reg;
            u16 reg1_addr;
            u8  reg1_val;
            u16 reg2_addr;
            u8  reg2_val;
            u16 reg3_addr;
            u8  reg3_val;
            u16 reg4_addr;
            u8  reg4_val;
            u16 reg5_addr;
            u8  reg5_val;
            u16 reg6_addr;
            u8  reg6_val;
            u16 reg7_addr;
            u8  reg7_val;
            u16 reg8_addr;
            u8  reg8_val;
            u16 reg9_addr;
            u8  reg9_val;
        } subcmd_21_23_04;

        struct {
            u8  mcu_cmd;
            u8  mcu_subcmd;
            u8  mcu_ir_mode;
            u8  no_of_frags;
            u16 mcu_major_v;
            u16 mcu_minor_v;
        } subcmd_21_23_01;
    };
};

struct ir_image_config {
    u8  ir_res_reg;
    u16 ir_exposure;
    u8  ir_leds; // Leds to enable, Strobe/Flashlight modes
    u16 ir_leds_intensity; // MSByte: Leds 1/2, LSB: Leds 3/4
    u8  ir_digital_gain;
    u8  ir_ex_light_filter;
    u32 ir_custom_register; // MSByte: Enable/Disable, Middle Byte: Edge smoothing, LSB: Color interpolation
    u16 ir_buffer_update_time;
    u8  ir_hand_analysis_mode;
    u8  ir_hand_analysis_threshold;
    u32 ir_denoise; // MSByte: Enable/Disable, Middle Byte: Edge smoothing, LSB: Color interpolation
    u8  ir_flip;
};

#pragma pack(pop)

enum ir_leds_Flags : u8 {
    ir_leds_FlashlightMode = 1,
    ir_leds_WideDisable = 1 << 4,
    ir_leds_NarrowDisable = 1 << 5,
    ir_leds_StrobeFlashMode = 1 << 7
};
enum flip_Flags : u8 {
    flip_dir_0 = 1 << 1
};
enum ir_ex_light_filter_Flags : u8 {
    ir_ex_light_fliter_0 = 0x3
};
enum ir_denoise_Flags : u32 {
    ir_denoise_Enable = 1 << 16
};

struct BatteryData {
    int percent;
    int report;
    float voltage;
};

struct TemperatureData {
    float celsius;
    float farenheight;
};


enum VIBType : unsigned char {
    VIBInvalid,
    VIBRaw,
    VIBBinary,
    VIBBinaryLoop,
    VIBBinaryLoopAndWait
};

struct VIBMetadata {
    VIBType vib_file_type;
    u16 sample_rate;
    u32 samples;
    u32 loop_start;
    u32 loop_end;
    u32 loop_wait;
    int loop_times;
};

#if __jctool_cpp_API__
using controller_hid_handle_t = hid_device*;

struct RumbleData {
    std::string from_file;
    VIBMetadata metadata;
    std::shared_ptr<u8> data;
};
#endif
