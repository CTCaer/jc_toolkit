#pragma once
#include <string>
#include <mutex>
#include <tuple>

#include "jctool_types.h"

class Controller {
public:
    enum class Type : unsigned char {
        None,
        JoyConLeft,
        JoyConRight,
        ProCon,
        Undefined
    };

    void connection();
    void updateBatteryData();
    void updateTemperatureData();
    void rumble(RumbleData& rumble_data);
    
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
    public:
        ir_image_config config;


        bool capture_in_progress;
        IRCaptureMode capture_mode;
        IRCaptureStatus capture_status;

        int res_idx_selected; /** The index number of the resolution selected.
        * See ir_sensor.h.
        */
        IRColor colorize_with;
        bool auto_exposure;

        void capture(controller_hid_handle_t host_controller, u8& timming_byte);
        uintptr_t getCaptureTexID();
    private:
        u8 ir_max_frag_no;
        struct VideoStreamFrameData {
            uintptr_t textures[3] = {};
            int idx_render = 0;
            int idx_swap = 1;
            int idx_display = 2;
            bool updated = false;
            std::mutex texture_mutex;
        } vstream_frame_dat;
    } ir;

    bool cancel_spi_dump = false;
    bool enable_nfc_scanning = false;
    u8 timming_byte = 0;

    bool rumble_active = false;

    Controller();
};
