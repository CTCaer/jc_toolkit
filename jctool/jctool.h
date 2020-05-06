#pragma once
#include <string>
#include "jctool_types.h"

#ifdef __jctool_cpp_API__
#include <functional> // for storeCapture
#endif

s16 uint16_to_int16(u16 a);
void decode_stick_params(u16 *decoded_stick_params, u8 *encoded_stick_params);
void encode_stick_params(u8 *encoded_stick_params, u16 *decoded_stick_params);
#ifndef __jctool_cpp_API__
std::string get_sn(u32 offset, const u16 read_len);
int get_spi_data(u32 offset, const u16 read_len, u8 *test_buf);
int write_spi_data(u32 offset, const u16 write_len, u8* test_buf);
int get_device_info(u8* test_buf);
int get_battery(u8* test_buf);
int get_temperature(u8* test_buf);
int dump_spi(const char *dev_name);
int send_rumble();
int send_custom_command(u8* arg);
int play_tune(int tune_no);
int play_hd_rumble_file(int file_type, u16 sample_rate, int samples, int loop_start, int loop_end, int loop_wait, int loop_times);
int device_connection();
int button_test();
int set_led_busy();
int nfc_tag_info();
int silence_input_report();
int ir_sensor(ir_image_config &ir_cfg);
int ir_sensor_config_live(ir_image_config &ir_cfg);
#else
std::string get_sn(CT& ct);
int get_spi_data(CT& ct, u32 offset, const u16 read_len, u8 *test_buf);
int write_spi_data(CT& ct, u32 offset, const u16 write_len, u8* test_buf);
int get_device_info(CT& ct, u8* test_buf);
int get_battery(CT& ct, u8* test_buf);
int get_temperature(CT& ct, u8* test_buf);
int dump_spi(CT& ct, DumpSPICTX& dump_spi_ctx);
int send_rumble(CT& ct, ConHID::ProdID con_type);
int send_custom_command(CT& ct, u8* arg);
int play_tune(CT& ct, int tune_no);
int play_hd_rumble_file(CT& ct, const RumbleData& rumble_data);
int device_connection(controller_hid_handle_t& handle);
int button_test(CT& ct);
int set_led_busy(CT& ct, ConHID::ProdID con_type);
int nfc_tag_info(CT& ct);
int silence_input_report(CT& ct);
int button_test(CT& ct);
int nfc_tag_info(CT& ct);
int silence_input_report(CT& ct);
using StoreRawCaptureCB = std::function<void(const u8*, size_t)>;
int ir_sensor(IRCaptureCTX& capture_context, StoreRawCaptureCB store_capture_cb);
int ir_sensor_config_live(CT& ct, ir_image_config& ir_cfg);
#endif

#ifndef __jctool_cpp_API__
extern int  handle_ok;
extern bool enable_button_test;
extern bool enable_IRVideoPhoto;
extern bool enable_IRAutoExposure;
extern bool enable_NFCScanning;
extern bool cancel_spi_dump;
extern bool check_connection_ok;

extern u8 timming_byte;
extern u8 ir_max_frag_no;

namespace CppWinFormJoy {
    class images
    {
        //For annoying designer..
        //Todo.
    };
}
#endif

const size_t SPI_SIZE = 0x80000;
