#include "jctool_api.hpp"
#include "jctool.h"
#include "hidapi.h"

BatteryData parseBatteryData(const unsigned char* batt_data) {
    int batt_percent = 0;
    int batt = ((u8)batt_data[0] & 0xF0) >> 4;

    // Calculate aproximate battery percent from regulated voltage
    u16 batt_volt = (u8)batt_data[1] + ((u8)batt_data[2] << 8);
    if (batt_volt < 0x560)
        batt_percent = 1;
    else if (batt_volt > 0x55F && batt_volt < 0x5A0) {
        batt_percent = ((batt_volt - 0x60) & 0xFF) / 7.0f + 1;
    }
    else if (batt_volt > 0x59F && batt_volt < 0x5E0) {
        batt_percent = ((batt_volt - 0xA0) & 0xFF) / 2.625f + 11;
    }
    else if (batt_volt > 0x5DF && batt_volt < 0x618) {
        batt_percent = (batt_volt - 0x5E0) / 1.8965f + 36;
    }
    else if (batt_volt > 0x617 && batt_volt < 0x658) {
        batt_percent = ((batt_volt - 0x18) & 0xFF) / 1.8529f + 66;
    }
    else if (batt_volt > 0x657)
        batt_percent = 100;

    return {batt_percent, batt, (batt_volt * 2.5f) / 1000};
}

float getTemperatureCelsius(const unsigned char* temp_data){
    // Convert reading to Celsius according to datasheet
    return 25.0f + uint16_to_int16(temp_data[1] << 8 | temp_data[0]) * 0.0625f;
}
