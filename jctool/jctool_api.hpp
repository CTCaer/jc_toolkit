#pragma once
#include "jctool_types.h"

struct BatteryData {
    int percent;
    int report;
    float voltage;
};

BatteryData parseBatteryData(const unsigned char* batt_data);
	
float parseTemperatureCelsius(const unsigned char* temp_data);
