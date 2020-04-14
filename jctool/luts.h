#pragma once
#include <cstdint>
struct lut_amp {
	float amp_float[101];
	uint8_t ha[101];
	uint16_t la[101];
};

extern lut_amp lut_joy_amp;