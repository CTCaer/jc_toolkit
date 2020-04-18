#include <thread>

#include "Controller.hpp"

#include "jctool.h"
#include "jctool_api.hpp"
#include "ImageLoad/ImageLoad.hpp"

#ifdef linux
#include <cstring> // memset linux
#include <unistd.h>
#endif

Controller::Controller() {
    // Set up an IR Sensor with a default configuration scheme.
    memset(&ir_sensor, 0, sizeof(ir_sensor)); // Set all values to zero.
    ir_sensor.host = &hid_handle;

    ir_image_config& ir_config = ir_sensor.config;
    ir_config.ir_leds_intensity = ~ir_config.ir_leds_intensity; // Max intensity.
    ir_config.ir_ex_light_filter = ir_ex_light_fliter_0;
    ir_config.ir_denoise = ir_denoise_Enable;
    ir_config.ir_digital_gain = 1;

    ir_image_config_Sets::exposure(ir_config.ir_exposure, 300);
    ir_image_config_Sets::denoise_edge_smooth(ir_config.ir_denoise, 35);
    ir_image_config_Sets::denoise_color_intrpl(ir_config.ir_denoise, 68);
}

/**
 * Updates the battery information with the value read from the controller.
 */
void Controller::updateBatteryData(){
    unsigned char battery_data[3];
    memset(battery_data, 0, sizeof(battery_data));
    get_battery(this->hid_handle, battery_data);

    this->battery = parseBatteryData(battery_data);
}

void Controller::updateTemperatureData(){
    unsigned char temperature_data[2];
    memset(temperature_data, 0, sizeof(temperature_data));
    get_temperature(this->hid_handle, temperature_data);
    
    this->temperature = parseTemperatureData(temperature_data);
}

void Controller::connection(){
    int handle_ok = 0;
    if((handle_ok = device_connection(this->hid_handle)) == 0) // TODO: , this->hid_serial_number)) == 0)
#ifdef linux
        memcpy(this->device_info, "NONE", 5);
#else
        memcpy_s(this->device_info, 10, "NONE", 5);
#endif
    else {
        get_device_info(this->hid_handle, reinterpret_cast<unsigned char*>(this->device_info));
        this->updateBatteryData();
        this->updateTemperatureData();

        this->serial_number = get_sn(this->hid_handle);
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

void Controller::IRSensorCapture(){
    if(*ir_sensor.host == nullptr)
        return; // There is no controller connected yet.
    
    ir_sensor.capture_in_progress = true;
    // Set the max frag number.
    switch(ir_sensor.config.ir_res_reg) {
        case IR_320x240:
            ir_sensor.ir_max_frag_no = 0xff;
            break;
        case IR_160x120:
            ir_sensor.ir_max_frag_no = 0x3f;
            break;
        case IR_80x60:
            ir_sensor.ir_max_frag_no = 0x0f;
            break;
        case IR_40x30:
            ir_sensor.ir_max_frag_no = 0x03;
            break;
    }

    std::string err_msg;
    std::thread ir_sensor_thread(irSensor<std::string>, std::ref(this->ir_sensor), std::ref(err_msg)); // Dispatch the thread.
    ir_sensor_thread.detach(); // Detach the thread so it does not have to be explicitly joined.
}

void Controller::IRSensor::storeCapture(u8* raw_capture){
    auto& resolution = std::get<2>(this->resolutions[this->res_idx_selected]);
    ImageResourceData ird;
    ird.width = resolution.width;
    ird.height = resolution.height;
    ird.num_channels = 3;
    ird.bytes = new u8[ird.width*ird.height*ird.num_channels];

    // Colorize the raw capture.
    colorizefrom8BitsPP(
        raw_capture,
        ird.bytes,
        ird.width,
        ird.height,
        ird.num_channels,
        this->colorize_with
    );
    
    /*
    stbi_write_bmp("jctoolapi_test_ir_raw.bmp", resolution.width, resolution.height, 1, raw_capture);
    for(int i=0; i < 4; i++){ // For all 4 colorized options.
        colorizefrom8BitsPP(raw_capture, ird.bytes, resolution.width, resolution.height, 3, i);
        stbi_write_png(
            std::string("jctoolapi_test_ir_colorized" + std::to_string(this->colorize_with) + ".png").c_str(),
            resolution.width,
            resolution.height,
            3,
            ird.bytes,
            resolution.width*3
        );
    }
    */
    GPUTexture::SideLoader::uploadTexture(this->last_capture_tex_id, ird);
}
