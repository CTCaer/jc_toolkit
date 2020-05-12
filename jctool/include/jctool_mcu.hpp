#pragma once
#include "jctool_types.h"

namespace MCU {
    // crc-8-ccitt / polynomial 0x07 look up table
    extern uint8_t mcu_crc8_table[];
    inline const size_t SPI_SIZE = 0x80000;

    u8 mcu_crc8_calc(u8 *buf, u8 size);
    std::string get_sn(CT& ct);
    int get_spi_data(CT& ct, u32 offset, const u16 read_len, u8 *test_buf);
    int write_spi_data(CT& ct, u32 offset, const u16 write_len, u8* test_buf);
    int write_spi_colors(CT& ct, const SPIColors& colors);
    int get_device_info(CT& ct, u8* test_buf);
    int get_battery(CT& ct, u8* test_buf);
    int get_temperature(CT& ct, u8* test_buf);
    int dump_spi(CT& ct, DumpSPICTX& dump_spi_ctx);
    SPIColors get_spi_colors(CT& ct);
}