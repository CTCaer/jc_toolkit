#pragma once
#include "jctool_types.h"

using ControllerHIDHandle= void*;

struct BatteryData {
    int percent;
    int report;
    float voltage;
};

struct TemperatureData {
    float celsius;
    float farenheight;
};

BatteryData parseBatteryData(const unsigned char* batt_data);
	
TemperatureData parseTemperatureData(const unsigned char* temp_data);
