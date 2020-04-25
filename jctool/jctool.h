#pragma once
#include <string>
#include "jctool_types.h"

#ifdef __jctool_cpp_API__
#include <functional> // for storeCapture
#include "Controller.hpp"
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
std::string get_sn(controller_hid_handle_t handle, u8& timming_byte);
int get_spi_data(controller_hid_handle_t handle, u8& timming_byte, u32 offset, const u16 read_len, u8 *test_buf);
int write_spi_data(controller_hid_handle_t handle, u8& timming_byte, u32 offset, const u16 write_len, u8* test_buf);
int get_device_info(controller_hid_handle_t handle, u8& timming_byte, u8* test_buf);
int get_battery(controller_hid_handle_t handle, u8& timming_byte, u8* test_buf);
int get_temperature(controller_hid_handle_t handle, u8& timming_byte, u8* test_buf);
int dump_spi(controller_hid_handle_t handle, u8& timming_byte, bool& cancel_spi_dump, const char *dev_name);
int send_rumble(controller_hid_handle_t handle, u8& timming_byte);
int send_custom_command(controller_hid_handle_t handle, u8& timming_byte, u8* arg);
int play_tune(controller_hid_handle_t handle, u8& timming_byte, int tune_no);
int play_hd_rumble_file(controller_hid_handle_t handle, u8& timming_byte, const RumbleData& rumble_data);
int device_connection(controller_hid_handle_t& handle);
int button_test(controller_hid_handle_t handle, u8& timming_byte);
int set_led_busy(controller_hid_handle_t handle, u8& timming_byte);
int nfc_tag_info(controller_hid_handle_t handle, u8& timming_byte);
int silence_input_report(controller_hid_handle_t handle, u8& timming_byte);
int set_led_busy(controller_hid_handle_t handle, u8& timming_byte);
int button_test(controller_hid_handle_t handle, u8& timming_byte);
int nfc_tag_info(controller_hid_handle_t handle, u8& timming_byte);
int silence_input_report(controller_hid_handle_t handle, u8& timming_byte);
using StoreRawCaptureCB = std::function<void(std::shared_ptr<u8>)>;
int ir_sensor(IRCaptureCTX& capture_context, StoreRawCaptureCB store_capture_cb);
int ir_sensor_config_live(controller_hid_handle_t handle, u8& timming_byte, ir_image_config& ir_cfg);
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
