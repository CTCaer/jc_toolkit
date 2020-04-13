#pragma once
#include "jctool_types.h"

BatteryData parseBatteryData(const unsigned char* batt_data);
	
TemperatureData parseTemperatureData(const unsigned char* temp_data);

template<typename ByteArray>
VIBFileMetadata getVIBFileMetadata(ByteArray vib_loaded_file) {
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
