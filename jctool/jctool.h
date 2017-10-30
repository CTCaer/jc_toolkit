#pragma once
#include <cstdint>
#include "hidapi.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Resources;
using namespace System::Xml;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::Globalization;

template <typename T> T CLAMP(const T& value, const T& low, const T& high)
{
	return value < low ? low : (value > high ? high : value);
}

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

extern std::string get_sn(u32 offset, const u16 read_len);
extern int16_t uint16_to_int16(uint16_t a);
extern int get_spi_data(u32 offset, const u16 read_len, u8 *test_buf);
extern int write_spi_data(u32 offset, const u16 write_len, u8* test_buf);
extern int get_device_info(u8* test_buf);
extern int get_battery(u8* test_buf);
extern int get_temperature(u8* test_buf);
extern int dump_spi(const char *dev_name);
extern int send_rumble();
extern int play_tune();
extern int play_hd_rumble_file(int file_type, u16 sample_rate, int samples, int loop_start, int loop_end, int loop_wait, int loop_times);
extern int send_custom_command(u8* arg);
extern int device_connection();
extern int set_led_busy();
extern int button_test();
int handle_ok;
bool enable_button_test;
bool allow_full_restore;
bool cancel_spi_dump;
hid_device *handle;
hid_device *handle_l;
int handler_close;
int option_is_on;

namespace CppWinFormJoy {
	class images
	{
		//For annoying designer..
		//Todo.
	};
}