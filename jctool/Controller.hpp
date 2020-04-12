#pragma once
#include <string>
#include "hidapi.h"
#include "jctool_types.h"

class Controller {
public:
    enum class Type : unsigned char {
        None,
        JoyConLeft,
        JoyConRight,
        ProCon
    };

    void connection();
    void updateBatteryData();
    void updateTemperatureData();
    
    /**
     * Return -1 on invalid battery report.
     */
    inline int batteryGetReport() { return ((this->battery.report >= 0) && (this->battery.report < 10)) ? this->battery.report : -1;}
    inline int batteryGetPercentage() { return this->battery.percent; }
    inline float batteryGetVoltage() { return this->battery.voltage; }

    inline float temperatureGetFarenheight() { return this->temperature.farenheight; }
    inline float temperatureGetCelsius() { return this->temperature.celsius; }

    inline Type getType() { return this->controller_type; }
    inline const unsigned char* getDeviceInfo() { return this->device_data; }
    inline const std::string getSerialNumber() { return this->serial_number; }
private:
    unsigned char device_data[10] = {}; // Initialize to nulls.
    // The serial number of the controller.
    std::string serial_number; // Initialize to nulls.
    // TODO: (need to implement) The serial number of the HID device.
    wchar_t* hid_serial_number = nullptr;
    TemperatureData temperature;
    BatteryData battery;
    Type controller_type = Type::None;
    controller_hid_handle_t handle = nullptr;
};
