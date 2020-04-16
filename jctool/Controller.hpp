#pragma once
#include <string>
#include "hidapi.h"
#include "jctool_types.h"
#include "ir_sensor.h"

#include <tuple>

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
    void IRSensorCapture();
    
    /**
     * Return -1 on invalid battery report.
     */
    inline int batteryReport() const { return ((this->battery.report >= 0) && (this->battery.report < 10)) ? this->battery.report : -1;}
    inline int batteryPercentage() const { return this->battery.percent; }
    inline float batteryVoltage() const { return this->battery.voltage; }

    inline float temperatureF() const { return this->temperature.farenheight; }
    inline float temperatureC() const { return this->temperature.celsius; }

    inline Type type() const { return this->controller_type; }
    inline const u8* deviceInfo() const{ return this->device_info; }
    inline const std::string serialNumber() const { return this->serial_number; }
    inline const controller_hid_handle_t handle() const { return this->hid_handle; }
private:
    u8 device_info[10] = {}; // Initialize to nulls.
    // The serial number of the controller.
    std::string serial_number;
    // TODO: (need to implement) The serial number of the HID device.
    // wchar_t* hid_serial_number = nullptr;
    TemperatureData temperature;
    BatteryData battery;
    Type controller_type = Type::None;
    controller_hid_handle_t hid_handle = nullptr;
public:
    class IRSensor {
        friend class Controller;
    public:
        struct Res {
            u16 width;
            u16 height;
        };
        static constexpr std::tuple<const char*, IRResolution, const Res> resolutions[] = {
            std::tuple<const char*, IRResolution, Res>("320 x 240", IR_320x240, {320, 240}),
            std::tuple<const char*, IRResolution, Res>("160 x 120", IR_160x120, {160, 120}),
            std::tuple<const char*, IRResolution, Res>("80 x 60", IR_80x60, {80, 60}),
            std::tuple<const char*, IRResolution, Res>("40 x 30", IR_40x30, {40, 30})
        };

        int res_idx_selected; /** The index number of the resolution selected.
        * See the static member IRSensor::resolution.
        */
        ir_image_config config;
        IRColor ir_color;
        bool auto_exposure;

        inline const controller_hid_handle_t* hostController() const { return this->host; }
        inline u8 maxFragNo() const { return this->ir_max_frag_no; }
    private:
        u8 ir_max_frag_no;
        controller_hid_handle_t* host;
    } ir_sensor;

    Controller();
};
