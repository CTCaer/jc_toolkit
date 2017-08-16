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

extern std::string get_sn(hid_device *handle, u32 offset, const u16 read_len);
extern int get_spi_data(hid_device *handle, u32 offset, const u16 read_len, u8 *test_buf);
extern int write_spi_data(hid_device *handle, u32 offset, const u16 write_len, u8* test_buf);
extern int get_device_info(hid_device *handle, u8* test_buf);
extern int get_battery(hid_device *handle, u8* test_buf);
extern int dump_spi(hid_device *handle, const char *dev_name);
extern int send_rumble(hid_device *handle);
extern int play_tune(hid_device *handle);
extern int send_custom_command(hid_device *handle, u8* arg);
extern int device_connection();
extern int set_led_busy();
int handle_ok;
bool allow_full_restore;
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