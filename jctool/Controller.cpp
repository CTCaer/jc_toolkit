#include <thread>

#include "Controller.hpp"

#include "jctool.h"
#include "jctool_api.hpp"
#include "ImageLoad/ImageLoad.hpp"

#ifdef __linux__
#include <cstring> // memset linux
#include <unistd.h>
#endif

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"

Controller::Controller() {
    // Set up an IR Sensor with a default configuration scheme.
    //memset(&ir_sensor, 0, sizeof(ir_sensor)); // Set all values to zero.
    ir_sensor.setHostController(*this);
    //memmove(&ir_sensor.message_stream, new std::stringstream(), sizeof(std::stringstream));
    ir_sensor.message_stream << "IR Message:" << std::endl;

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
    get_battery(this->hid_handle, this->timming_byte, battery_data);

    this->battery = parseBatteryData(battery_data);
}

void Controller::updateTemperatureData(){
    unsigned char temperature_data[2];
    memset(temperature_data, 0, sizeof(temperature_data));
    get_temperature(this->hid_handle, this->timming_byte, temperature_data);
    
    this->temperature = parseTemperatureData(temperature_data);
}

void Controller::connection(){
    int handle_ok = 0;
    if((handle_ok = device_connection(this->hid_handle)) == 0) {// TODO: , this->hid_serial_number)) == 0)
#ifdef __linux__
        memcpy(this->device_info, "NONE", 5);
#else
        memcpy_s(this->device_info, 10, "NONE", 5);
#endif
        this->controller_type = Controller::Type::None;
        this->hid_handle = nullptr;
        return;
    }
    else {
        get_device_info(this->hid_handle, this->timming_byte, reinterpret_cast<unsigned char*>(this->device_info));
        this->updateBatteryData();
        this->updateTemperatureData();

        this->serial_number = get_sn(this->hid_handle, this->timming_byte);
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
        default:
            this->controller_type = Controller::Type::Undefined;
            break;
    }
}

void Controller::IRSensor::capture(){
    if(this->host->hid_handle == nullptr)
        return; // There is no controller connected yet.
    
    this->capture_in_progress = true;
    // Set the max frag number.
    switch(this->config.ir_res_reg) {
        case IR_320x240:
            this->ir_max_frag_no = 0xff;
            break;
        case IR_160x120:
            this->ir_max_frag_no = 0x3f;
            break;
        case IR_80x60:
            this->ir_max_frag_no = 0x0f;
            break;
        case IR_40x30:
            this->ir_max_frag_no = 0x03;
            break;
    }

    std::thread ir_sensor_thread(irSensor<std::stringstream>, std::ref(*this), std::ref(this->message_stream)); // Dispatch the thread.
    ir_sensor_thread.detach(); // Detach the thread so it does not have to be explicitly joined.
}

void Controller::IRSensor::storeCapture(std::shared_ptr<u8> raw_capture){
    GPUTexture::SideLoader::addJob([this, raw_capture](){
        auto& resolution = std::get<2>(this->resolutions[this->res_idx_selected]);
        ImageResourceData ird;
        ird.width = resolution.width;
        ird.height = resolution.height;
        ird.num_channels = 3;
        ird.bytes = new u8[ird.width*ird.height*ird.num_channels];

        // Colorize the raw capture.
        colorizefrom8BitsPP(
            &*raw_capture,
            ird.bytes,
            ird.width,
            ird.height,
            ird.num_channels,
            this->colorize_with
        );

        std::lock_guard<std::mutex> lock(this->vstream_frame_dat.texture_mutex);
        this->vstream_frame_dat.updated = true;
        std::swap(this->vstream_frame_dat.idx_swap, this->vstream_frame_dat.idx_render);
        GPUTexture::openGLUpload(this->vstream_frame_dat.textures[this->vstream_frame_dat.idx_render], ird.width, ird.height, ird.num_channels, ird.bytes);
    });
}

uintptr_t Controller::IRSensor::getCaptureTexID() {
    auto& frame_dat = this->vstream_frame_dat;
    std::lock_guard<std::mutex> lock(frame_dat.texture_mutex);
    if(frame_dat.updated){
        std::swap(frame_dat.idx_swap, frame_dat.idx_display);
        frame_dat.updated = false;
        GPUTexture::SideLoader::addJob([this](){
            GPUTexture::openGLFree(this->vstream_frame_dat.textures[this->vstream_frame_dat.idx_swap]);
        });
    }
    return frame_dat.textures[frame_dat.idx_display];
}
