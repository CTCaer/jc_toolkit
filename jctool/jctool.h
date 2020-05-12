#pragma once
#include <string>
#include "jctool_types.h"

s16 uint16_to_int16(u16 a);
int device_connection(controller_hid_handle_t& handle);
int silence_input_report(CT& ct);
