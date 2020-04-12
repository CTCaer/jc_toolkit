#include "Controller.hpp"

#include "jctool.h"
#include "jctool_api.hpp"

/**
 * Updates the battery information with the value read from the controller.
 */
void Controller::updateBatteryData(){
    unsigned char battery_data[3];
    memset(battery_data, 0, sizeof(battery_data));
    get_battery(this->handle, battery_data);

    this->battery = parseBatteryData(battery_data);
}

void Controller::updateTemperatureData(){
    unsigned char temperature_data[2];
    memset(temperature_data, 0, sizeof(temperature_data));
    get_temperature(this->handle, temperature_data);
    
    this->temperature = parseTemperatureData(temperature_data);
}

void Controller::connection(){
    int handle_ok = 0;
    if((handle_ok = device_connection(this->handle)) == 0) // TODO: , this->hid_serial_number)) == 0)
        memcpy_s(this->device_data, 10, "NONE", 5);
    else {
        get_device_info(this->handle, reinterpret_cast<unsigned char*>(this->device_data));
        this->updateBatteryData();
        this->updateTemperatureData();

        this->serial_number = get_sn(this->handle);
    }

    // Set the controller type based on the return value.
    switch (handle_ok)
    {
    case 1:
        this->controller_type = Controller::Type::JoyConLeft;
        break;
    case 2:
        this->controller_type = Controller::Type::JoyConRight;
        break;
    case 3:
        this->controller_type = Controller::Type::ProCon;
        break;
    
    case 0:
    default:
        this->controller_type = Controller::Type::None;
        break;
    }
}
