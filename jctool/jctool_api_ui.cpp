/**
 * Code extracted from the orginal UI framework source (CppWinForm.)
 * Goal: Eliminate dependency to the original UI framework so that useful code
 * that was once only accessible by the original UI is now accessible through
 * an API.
 */
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <vector>
#include <memory>

// Open a file dialog window
#ifdef WIN32
#include "Windows.h"
#include "commdlg.h"
#endif

#include "jctool_api.hpp"
#include "jctool.h"
#include "hidapi.h"

#include "imgui.h"
#ifdef __jctool_cpp_API__
#include "ImageLoad/ImageLoad.hpp"
#include "Controller.hpp"
#define BATTERY_INDICATORS_PATH "jctool/original_res/batt_"
#define BATTERY_INDICATORS_COUNT 10
#define BATTERY_INDICATOR_NAMES "0 0_chr 25 25_chr 50 50_chr 75 75_chr 100 100_chr"
#define BATTERY_INDICATORS_EXT ".png"
#endif

#include "ir_sensor.h"

BatteryData parseBatteryData(const unsigned char* batt_data) {
    int batt_percent = 0;
    int batt = ((u8)batt_data[0] & 0xF0) >> 4;

    // Calculate aproximate battery percent from regulated voltage
    u16 batt_volt = (u8)batt_data[1] + ((u8)batt_data[2] << 8);
    if (batt_volt < 0x560)
        batt_percent = 1;
    else if (batt_volt > 0x55F && batt_volt < 0x5A0) {
        batt_percent = static_cast<int>(((batt_volt - 0x60) & 0xFF) / 7.0f) + 1;
    }
    else if (batt_volt > 0x59F && batt_volt < 0x5E0) {
        batt_percent = static_cast<int>(((batt_volt - 0xA0) & 0xFF) / 2.625f) + 11;
    }
    else if (batt_volt > 0x5DF && batt_volt < 0x618) {
        batt_percent = static_cast<int>((batt_volt - 0x5E0) / 1.8965f) + 36;
    }
    else if (batt_volt > 0x617 && batt_volt < 0x658) {
        batt_percent = static_cast<int>(((batt_volt - 0x18) & 0xFF) / 1.8529f) + 66;
    }
    else if (batt_volt > 0x657)
        batt_percent = 100;

    return {batt_percent, batt, (batt_volt * 2.5f) / 1000};
}

TemperatureData parseTemperatureData(const unsigned char* temp_data){
    // Convert reading to Celsius according to datasheet
    float celsius = 25.0f + uint16_to_int16(temp_data[1] << 8 | temp_data[0]) * 0.0625f;
    return {celsius, celsius*1.8f + 32};
}

enum IRColor {
    IRGreyscale,
    IRNightVision,
    IRIronbow,
    IRInfrared
};

void colorizefrom8BitsPP(u8* pixel_data_in, u8* pixel_data_out, int ir_image_height, int ir_image_width, int bytes_pp_out, int col_fil){
    int buf_pos = 0;
    
    for (int y = 0; y < ir_image_height; y++) {
        byte* row = (byte *)pixel_data_out + (y * bytes_pp_out * ir_image_width);
        for (int x = 0; x < ir_image_width; x++) {
            switch(col_fil){
                case IRGreyscale:
                    // Values are in BGR in memory. Here in RGB order.
                    row[x * bytes_pp_out + 2] = pixel_data_in[x + buf_pos];
                    row[x * bytes_pp_out + 1] = pixel_data_in[x + buf_pos];
                    row[x * bytes_pp_out]     = pixel_data_in[x + buf_pos];
                    break;
                case IRNightVision:
                    // Values are in BGR in memory. Here in RGB order.
                    row[x * bytes_pp_out + 2] = 0;
                    row[x * bytes_pp_out + 1] = pixel_data_in[x + buf_pos];
                    row[x * bytes_pp_out]     = 0;
                    break;
                case IRIronbow:
                    // Values are in BGR in memory. Here in RGB order.
                    row[x * bytes_pp_out + 2] = (iron_palette[pixel_data_in[x + buf_pos]] >> 16)&0xFF;
                    row[x * bytes_pp_out + 1] = (iron_palette[pixel_data_in[x + buf_pos]] >> 8) & 0xFF;
                    row[x * bytes_pp_out]     =  iron_palette[pixel_data_in[x + buf_pos]] & 0xFF;
                    break;
                case IRInfrared:
                default:
                    // Values are in BGR in memory. Here in RGB order.
                    row[x * bytes_pp_out + 2] = pixel_data_in[x + buf_pos];
                    row[x * bytes_pp_out + 1] = 0;
                    row[x * bytes_pp_out]     = 0;
                    break;
            }
        }
        buf_pos += ir_image_width;
    }
}

#ifdef __jctool_cpp_API__
namespace JCToolkit {
    namespace Assets {
        ImageResource battery_indicators[BATTERY_INDICATORS_COUNT];
    }
    namespace Helpers {
        void loadBatteryImages(){
            std::istringstream in(BATTERY_INDICATOR_NAMES);
            std::vector<std::string> vec = std::vector<std::string>(std::istream_iterator<std::string>(in), std::istream_iterator<std::string>());

            if(vec.size() != BATTERY_INDICATORS_COUNT)
                return;

            for(
                int i=0, tokenize = NULL;
                i<BATTERY_INDICATORS_COUNT;
                i++
            ){
                Assets::battery_indicators[i].load(std::string(BATTERY_INDICATORS_PATH) + vec[i] + BATTERY_INDICATORS_EXT);
            }
        }
    }
    Controller default_controller;
    RumbleData default_rumble_data;
    namespace UI {
        /**
         * UI Port from jctool/FormJoy.h
         */
        void showDeviceInfo(const unsigned char* device_info){
            ImGui::Text("Device Info");
            ImGui::Text("Firmware: %X.%02X", device_info[0], device_info[1]);
            ImGui::Text(
                "MAC: %02X:%02X:%02X:%02X:%02X:%02X",
                device_info[4], device_info[5], device_info[6],
                device_info[7], device_info[8], device_info[9]
            );
            ImGui::Text("Bytes [2,3]: %X:%X", device_info[2], device_info[3]);
        }

        void showControllerInfo(Controller& controller){
            auto device_info = controller.getDeviceInfo();
            auto controller_type = controller.getType();

            if(controller_type == Controller::Type::None) {
                ImGui::Text("No controller is connected.");
                return;
            }

            // Show the controller's device information.
            showDeviceInfo(device_info);

            // Detect which controller type label to use.
            const char* controller_type_label = "NONE";
            switch (controller_type)
            {
            case Controller::Type::JoyConLeft:
                controller_type_label = "Joy-Con (L)";
                break;
            case Controller::Type::JoyConRight:
                    controller_type_label = "Joy-Con (R)";
                break;
            case Controller::Type::ProCon:
                controller_type_label = "Pro Controller";
                break;
            default:
                controller_type_label = "Unrecognized Controller";
                break;
            }

            ImGui::Text("Controller Type: %s", controller_type_label);
            int battery_report = controller.batteryGetReport();
            ImGui::Text("Battery");
            if(battery_report < 0)
                ImGui::Text("Invalid reading.");
            else{
                ImageResource& battery_img = Assets::battery_indicators[battery_report];
                ImGui::Image(
                    reinterpret_cast<ImTextureID>(battery_img.getRID()),
                    ImVec2(
                        static_cast<float>(battery_img.getWidth()),
                        static_cast<float>(battery_img.getHeight())
                    )
                );
                ImGui::Text("%.2fV - %d", controller.batteryGetVoltage(), controller.batteryGetPercentage());
            }

            ImGui::Text("Temperature: %.2fF / %.2fC ", controller.temperatureGetFarenheight(), controller.temperatureGetCelsius());

            ImGui::Text("S/N: %s", controller.getSerialNumber().c_str());
        }

        void showRumblePlayer(RumbleData& rumble_data) {
            if(ImGui::Button("Load Rumble Data")){
                char file_name_buf[FILENAME_MAX];
#ifdef WIN32
                OPENFILENAME ofn;
                ZeroMemory(&ofn, sizeof(ofn));
                HWND hwnd = GetActiveWindow();
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = file_name_buf;
                // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
                // use the contents of szFile to initialize itself.
                ofn.lpstrFile[0] = '\0';
                ofn.nMaxFile = sizeof(file_name_buf);
                ofn.lpstrFilter = "HD Rumble\0*.bnvib;*.jcvib\0\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrFileTitle = NULL;
                ofn.nMaxFileTitle = 0;
                ofn.lpstrInitialDir = NULL;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                if(GetOpenFileName(&ofn)) {
#elif defined(linux)
                if(false) { // TODO: linux file open.
#endif
                    std::ifstream rumble_fstream = std::ifstream(file_name_buf, std::ios_base::binary);
                    if(!rumble_fstream.bad()){
                        RumbleData new_rumble_data;
                        // Calculate the size of the file.
                        size_t rumble_data_size = std::filesystem::file_size(file_name_buf);
                        char* read_buf = new char[rumble_data_size];
                        rumble_fstream.read(read_buf, rumble_data_size);

                        new_rumble_data.from_file = file_name_buf;
                        new_rumble_data.metadata = getVIBMetadata(read_buf);
                        new_rumble_data.data.reset(reinterpret_cast<u8*>(read_buf), [](u8* p){
                            delete[] p;
                        }); // Allocate enough space for the rumble data.

                        rumble_data = new_rumble_data;
                    }
                }
            }
            if(rumble_data.data == nullptr)
                ImGui::Text("There is no rumble data loaded.");
            else {
                const char* type_label;
                bool loop = false;
                switch(rumble_data.metadata.vib_file_type){
                    case VIBRaw:
                    type_label = "Raw";
                    break;
                    case VIBBinary:
                    type_label = "Binary";
                    break;
                    case VIBBinaryLoop:
                    type_label = "Binary loop";
                    loop = true;
                    break;
                    case VIBBinaryLoopAndWait:
                    type_label = "Binary loop and wait";
                    loop = true;
                    break;
                    case VIBInvalid:
                    type_label = "Invalid";
                    break;
                }
                ImGui::Text(
                    "File: %s\n"
                    "Rumble data type: %s\n"
                    "Sample rate: %u\n"
                    "Samples: %d",
                    std::filesystem::path(rumble_data.from_file).filename().generic_string().c_str(),
                    type_label,
                    rumble_data.metadata.sample_rate,
                    rumble_data.metadata.samples
                );
                if(loop){
                    ImGui::SameLine();
                    ImGui::Text(
                        "Start: %u\n"
                        "End: %u\n"
                        "Wait: %u\n"
                        "Repeat: %u",
                        rumble_data.metadata.loop_start,
                        rumble_data.metadata.loop_end,
                        rumble_data.metadata.loop_wait,
                        rumble_data.metadata.loop_times
                    );
                }
                if(default_controller.getHandle() && ImGui::Button("Play")){
                    play_hd_rumble_file(default_controller.getHandle(), rumble_data);
                }
            }
        }
        
        void showIRCamera(Controller& controller) {
            // Resolution and Color Filter
            static int res_selected = 0;
            static std::pair<ImVec2, const char*> resolutions[] = {
                {{240, 320}, "240 x 320"},
                {{120, 160}, "120 x 160"},
                {{60, 80}, "60 x 80"},
                {{30, 40}, "30 x 40"}
            };
            static int col_fil_selected = 0;
            static const char* col_fils[] = {
                "Greyscale",
                "Night Vision",
                "Ironbow",
                "Infrared"
            };

            // Near IR Light Settings
            static u8 narrow_ir_intensity = ~u8(0);
            static u8 wide_ir_intensity = ~u8(0);
            static bool use_narrow_ir = true;
            static bool use_wide_ir = true;
            static bool use_flashlight = false;
            static bool use_strobe_flash = false;
            static bool use_ext_ir_filter = true;

            // Exposure Settings
            static u16 exposure_amt = 300;
            static bool auto_exposure = false;

            // Denoise Settings
            static bool use_denoise = true;
            static int denoise_edge_smooth = 35;
            static int denoise_color_intrpl = 68; // Color Interpolation

            // More
            static bool flip_capture = false;
            static int ir_reg = 0; // IR Register
            static int ir_reg_val = 0; // IR Register Value
            static u8 digital_gain = 1;

            /* Stream/Capture */ {
                ImGui::BeginGroup();
                ImGui::Image(0, resolutions[res_selected].first);
                if(ImGui::Button("Capture")) {
                    ir_image_config ir_config = {};
                    /* IR Configuration */ {
                        // Resolution config
                        switch(res_selected){
                            case 0: // 320 x 240
                                ir_config.ir_res_reg = 0b00000000;
                                ir_max_frag_no = 0xff;
                                break;
                            case 1: // 160 x 120
                                ir_config.ir_res_reg = 0b1010000;
                                ir_max_frag_no = 0x3f;
                                break;
                            case 2: // 80 x 60
                                ir_config.ir_res_reg = 0b01100100;
                                ir_max_frag_no = 0x0f;
                                break;
                            case 3: // 40 x 30
                                ir_config.ir_res_reg = 0b01101001;
                                ir_max_frag_no = 0x03;
                                break;
                        }

                        // IR led config
                        ir_config.ir_leds = ((!use_wide_ir << 1) | (!use_narrow_ir)) << 4;
                        // IR led effects
                        if(use_flashlight)
                            ir_config.ir_leds |= 0b01;
                        if(use_strobe_flash)
                            ir_config.ir_leds |= 0b10000000;

                        // IR leds intensity
                        ir_config.ir_leds_intensity = ((u8)wide_ir_intensity << 8) | (u8)narrow_ir_intensity;

                        // External light filter
                        if (use_ext_ir_filter && !use_strobe_flash)
                            ir_config.ir_ex_light_filter = 0x03;
                        else
                            ir_config.ir_ex_light_filter = 0x00;
                        
                        // Flip the image
                        if(flip_capture)
                            ir_config.ir_flip = 0x02;
                        else
                            ir_config.ir_flip = 0x00;

                        // Exposure time.
                        ir_config.ir_exposure = (u16)(exposure_amt * 31200 / 1000);
                        if(!auto_exposure && enable_IRVideoPhoto){
                            enable_IRAutoExposure = false;
                            ir_config.ir_digital_gain = (u8)digital_gain;
                        } else {
                            enable_IRAutoExposure = true;
                            ir_config.ir_digital_gain = 1;
                        }

                        // De-noise
                        if(use_denoise)
                            ir_config.ir_denoise = 0x01 << 16;
                        else
                            ir_config.ir_denoise = 0x00 << 16;
                        ir_config.ir_denoise |= ((u8) denoise_edge_smooth & 0xff) << 8;
                        ir_config.ir_denoise |= (u8) denoise_color_intrpl & 0xff;
                    }
                    // Initialize the IR Sensor AND Take a photo.
                    std::string err_msg;
                    int res = irSensor(ir_config, err_msg, controller.getHandle());
                    if(res > 0)
                        ; // Error.
                }
                ImGui::EndGroup();
            }
            ImGui::SameLine();
            /* IR Camera Settings */ {
                ImGui::BeginGroup();
                ImGui::Text("IR Camera Settings");
                /* Output Frame Settings */ {
                    ImGui::BeginGroup();
                    if(ImGui::CollapsingHeader("Frame Display Settings")){
                        // Resolution settings
                        ImGui::BeginGroup();
                        ImGui::Text("Resolution");

                        constexpr int res_count = _ARRAYSIZE(resolutions);
                        for(int i = 0; i < res_count; i++){
                            auto& res = resolutions[i];
                            if(ImGui::RadioButton(res.second, i == res_selected))
                                res_selected = i;
                        }
                        ImGui::EndGroup();

                        ImGui::SameLine();
                        // Color filter settings
                        ImGui::BeginGroup();
                        ImGui::Text("Color Filter");

                        constexpr int col_fils_count = _ARRAYSIZE(col_fils);
                        for(int i = 0; i < col_fils_count; i++){
                            if(ImGui::RadioButton(col_fils[i], i == col_fil_selected))
                                col_fil_selected = i;
                        }
                        ImGui::EndGroup();
                    }
                    ImGui::EndGroup();

                    ImGui::BeginGroup();
                    if(ImGui::CollapsingHeader("Advanced Settings")){
                        static const u8 min_x = 1;
                        static const u8 max_x = 20;
                        ImGui::SliderScalar("Digital Gain (lossy)", ImGuiDataType_U8, &digital_gain, &min_x, &max_x, "x%d");
                        /* Exposure */ {
                            ImGui::BeginGroup();
                            ImGui::InputScalar("Exposure (us | micro-seconds)", ImGuiDataType_U16, &exposure_amt);
                            ImGui::Checkbox("Auto Exposure (experimental)", &auto_exposure);
                            ImGui::EndGroup();
                        }
                        /* Denoise */ {
                            ImGui::BeginGroup();
                            ImGui::Checkbox("Enable", &use_denoise);
                            ImGui::InputInt("Edge Smoothing", &denoise_edge_smooth); // TODO: Clamp to 0-256
                            ImGui::InputInt("Color Interpolation", &denoise_color_intrpl); // TODO: Clamp to 0-256
                            ImGui::EndGroup();
                        }
                    }
                    ImGui::EndGroup();
                }
                /* Near-Infrared Light Settings */ {
                    ImGui::BeginGroup();
                    if(ImGui::CollapsingHeader("Near-Infrared Light")){
                        static constexpr u8 max_intensity = ~u8(0);
                        static constexpr u8 min_intensity = 0;
                        ImGui::Checkbox("Far/Narrow (75*) Leds 1-2", &use_narrow_ir);
                        ImGui::SliderScalar("Intensity##narrow", ImGuiDataType_U8, &narrow_ir_intensity, &min_intensity, &max_intensity, "%u");
                        ImGui::Checkbox("Near/Wide (130*) Leds 3-4", &use_wide_ir);
                        ImGui::SliderScalar("Intensity##wide", ImGuiDataType_U8, &wide_ir_intensity, &min_intensity, &max_intensity, "%u");
                        ImGui::Checkbox("Flashlight Mode", &use_flashlight);
                        ImGui::Checkbox("Strobe Flash Mode", &use_strobe_flash);
                        ImGui::Checkbox("External IR Filter", &use_ext_ir_filter);
                    }
                    ImGui::EndGroup();

                }
                /* IR Sensor Register/Value */ {
                    ImGui::BeginGroup();
                    if(ImGui::CollapsingHeader("IR Sensor Register/Value")){
                        ImGui::InputInt("Register", &ir_reg); // TODO: clamp to 0x5FF
                        ImGui::InputInt("Value", &ir_reg_val); // TODO: clamp to 0xFF
                    }
                    ImGui::EndGroup();
                }
                ImGui::Checkbox("Flip Capture", &flip_capture);
                ImGui::EndGroup();
            }

        }

        void show(){
            // Try to establish a connection with a controller.
            if(ImGui::Button("Try Connection Attempt"))
                default_controller.connection();
            showControllerInfo(default_controller);
            if(ImGui::CollapsingHeader("HD Rumble Player"))
                showRumblePlayer(default_rumble_data);
            if(ImGui::CollapsingHeader("IR Camera"))
                showIRCamera(default_controller);
        }
    }
}
#endif
