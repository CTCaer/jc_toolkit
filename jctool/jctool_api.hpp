#pragma once
#include "jctool_types.h"
#include "jctool.h"
#include "jctool_helpers.hpp"
#include "luts.h"

BatteryData parseBatteryData(const unsigned char* batt_data);
	
TemperatureData parseTemperatureData(const unsigned char* temp_data);

template<typename ByteArray>
VIBMetadata getVIBMetadata(ByteArray vib_loaded_file) {
    u8 file_magic[] = { 0x52, 0x52, 0x41, 0x57, 0x4, 0xC, 0x3, 0x10 };

    VIBType vib_file_type = VIBInvalid;
    u16 vib_sample_rate = 0;
    u32 vib_samples = 0;
    u32 vib_loop_start = 0, vib_loop_end = 0;
    u32 vib_loop_wait = 0;

    //check for vib_file_type
    if (vib_loaded_file[0] == file_magic[0]) {
        for (int i = 1; i < 4; i++) {
            if (vib_loaded_file[i] == file_magic[i])
                vib_file_type = (VIBType)1; // Raw
            else
                break; // Early out; falls through to return VIB Unkown (invalid).
        }
        if (vib_file_type == 1) {
            vib_sample_rate = (vib_loaded_file[0x4] << 8) + vib_loaded_file[0x5];
            vib_samples = (vib_loaded_file[0x6] << 24) + (vib_loaded_file[0x7] << 16) + (vib_loaded_file[0x8] << 8) + vib_loaded_file[0x9];
            return { VIBRaw, vib_sample_rate, vib_samples };
        }
    }
    else if (vib_loaded_file[4] == file_magic[6]) {
        if (vib_loaded_file[0] == file_magic[4]) {
            vib_file_type = (VIBType)2; // Binary
            u32 vib_size = vib_loaded_file[0x8] + (vib_loaded_file[0x9] << 8) + (vib_loaded_file[0xA] << 16) + (vib_loaded_file[0xB] << 24);
            vib_sample_rate = 1000 / (vib_loaded_file[0x6] + (vib_loaded_file[0x7] << 8));
            vib_samples = vib_size / 4;
            return { VIBBinary, vib_sample_rate, vib_samples };
        }
        else if (vib_loaded_file[0] == file_magic[5]) {
            vib_file_type = (VIBType)3; // Loop Binary
            u32 vib_size = vib_loaded_file[0x10] + (vib_loaded_file[0x11] << 8) + (vib_loaded_file[0x12] << 16) + (vib_loaded_file[0x13] << 24);
            vib_sample_rate = 1000 / (vib_loaded_file[0x6] + (vib_loaded_file[0x7] << 8));
            vib_samples = vib_size / 4;
            vib_loop_start = vib_loaded_file[0x8] + (vib_loaded_file[0x9] << 8) + (vib_loaded_file[0xA] << 16) + (vib_loaded_file[0xB] << 24);
            vib_loop_end = vib_loaded_file[0xC] + (vib_loaded_file[0xD] << 8) + (vib_loaded_file[0xE] << 16) + (vib_loaded_file[0xF] << 24);
            return { VIBRaw, vib_sample_rate, vib_samples, vib_loop_start, vib_loop_end };
        }
        else if (vib_loaded_file[0] == file_magic[7]) {
            vib_file_type = (VIBType)4; // Loop and Wait Binary
            u32 vib_size = vib_loaded_file[0x14] + (vib_loaded_file[0x15] << 8) + (vib_loaded_file[0x16] << 16) + (vib_loaded_file[0x17] << 24);
            vib_sample_rate = 1000 / (vib_loaded_file[0x6] + (vib_loaded_file[0x7] << 8));
            vib_samples = vib_size / 4;
            vib_loop_start = vib_loaded_file[0x8] + (vib_loaded_file[0x9] << 8) + (vib_loaded_file[0xA] << 16) + (vib_loaded_file[0xB] << 24);
            vib_loop_end = vib_loaded_file[0xC] + (vib_loaded_file[0xD] << 8) + (vib_loaded_file[0xE] << 16) + (vib_loaded_file[0xF] << 24);
            vib_loop_wait = vib_loaded_file[0x10] + (vib_loaded_file[0x11] << 8) + (vib_loaded_file[0x12] << 16) + (vib_loaded_file[0x13] << 24);
            return { VIBRaw, vib_sample_rate, vib_samples, vib_loop_start, vib_loop_end, vib_loop_wait };
        }
    }

    //vib_file_type = (VIBType)0; // Unknown (Invalid)
    return {}; // Return a struct of all null values.
}

template<typename ByteArray>
void convertVIBBinaryToRaw(ByteArray vib_data, ByteArray vib_out, int lf_amp, int hf_amp, int lf_freq, int hf_freq, int lf_gain, int hf_gain, int lf_pitch, int hf_pitch) {
    VIBMetadata metadata = getVIBMetadata(vib_data);
    if (metadata.vib_file_type == VIBRaw)
        // No need to convert.
        return;

    u8 vib_off = 0;
    if (metadata.vib_file_type == 3)
        vib_off = 8;
    if (metadata.vib_file_type == 4)
        vib_off = 12;
    //Convert to RAW vibration, apply EQ and clamp inside safe values
    //vib_size = this->vib_loaded_file[0x8] + (this->vib_loaded_file[0x9] << 8) + (this->vib_loaded_file[0xA] << 16) + (this->vib_loaded_file[0xB] << 24);

    //Convert to raw
    for (u32 i = 0; i < (metadata.samples * 4); i = i + 4)
    {
        //Apply amp eq
        u8 tempLA = (lf_amp == 10 ? vib_data[0xC + vib_off + i] : (u8)CLAMP((float)vib_data[0xC + vib_off + i] * lf_gain, 0.0f, 255.0f));
        u8 tempHA = (hf_amp == 10 ? vib_data[0xE + vib_off + i] : (u8)CLAMP((float)vib_data[0xE + vib_off + i] * hf_gain, 0.0f, 255.0f));

        //Apply safe limit. The sum of LF and HF amplitudes should be lower than 1.0
        float apply_safe_limit = (float)tempLA / 255.0f + tempHA / 255.0f;
        //u8 tempLA = (apply_safe_limit > 1.0f ? (u8)((float)this->vib_file_converted[0xC + i] * (0.55559999f / apply_safe_limit)) : (u8)((float)this->vib_file_converted[0xC + i] * 0.55559999f));
        tempLA = (apply_safe_limit > 1.0f ? (u8)((float)tempLA * (1.0f / apply_safe_limit)) : tempLA);
        tempHA = (apply_safe_limit > 1.0f ? (u8)((float)tempHA * (1.0f / apply_safe_limit)) : tempHA);

        //Apply eq and convert frequencies to raw range
        u8 tempLF = (lf_freq == 10 ? vib_data[0xD + vib_off + i] : (u8)CLAMP((float)vib_data[0xD + vib_off + i] * lf_pitch, 0.0f, 191.0f)) - 0x40;
        u16 tempHF = ((hf_freq == 10 ? vib_data[0xF + vib_off + i] : (u8)CLAMP((float)vib_data[0xF + vib_off + i] * hf_pitch, 0.0f, 223.0f)) - 0x60) * 4;

        //Encode amplitudes with the look up table and direct encode frequencies
        int j;
        float temp = tempLA / 255.0f;
        for (j = 1; j < 101; j++) {
            if (temp < lut_joy_amp.amp_float[j]) {
                j--;
                break;
            }
        }
        vib_out[0xE + vib_off + i] = ((lut_joy_amp.la[j] >> 8) & 0xFF) + tempLF;
        vib_out[0xF + vib_off + i] = lut_joy_amp.la[j] & 0xFF;

        temp = tempHA / 255.0f;
        for (j = 1; j < 101; j++) {
            if (temp < lut_joy_amp.amp_float[j]) {
                j--;
                break;
            }
        }
        vib_out[0xC + vib_off + i] = tempHF & 0xFF;
        vib_out[0xD + vib_off + i] = ((tempHF >> 8) & 0xFF) + lut_joy_amp.ha[j];

    }
}
