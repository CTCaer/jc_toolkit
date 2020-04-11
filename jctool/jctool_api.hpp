#pragma once
#include "jctool_types.h"

BatteryData parseBatteryData(const unsigned char* batt_data);
	
TemperatureData parseTemperatureData(const unsigned char* temp_data);
