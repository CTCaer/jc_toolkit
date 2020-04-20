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

#ifdef __jctool_cpp_API__
#include "imgui.h"
#include "ImGui/this_is_imconfig.h"
#include "imgui_internal.h" // PushItemFlags
#include "ui_helpers.hpp"
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

void colorizefrom8BitsPP(u8* pixel_data_in, u8* pixel_data_out, int ir_image_height, int ir_image_width, int bytes_pp_out, int col_fil, u8 color_order){
    int buf_pos = 0;

    u8 red_pos_idx = ColorOrder::getRedPosIdx(color_order);
    u8 green_pos_idx = ColorOrder::getGreenPosIdx(color_order);
    u8 blue_pos_idx = ColorOrder::getBluePosIdx(color_order);

    for (int y = 0; y < ir_image_height; y++) {
        u8* row = (u8 *)pixel_data_out + (y * bytes_pp_out * ir_image_width);
        for (int x = 0; x < ir_image_width; x++) {
            switch(col_fil){
                case IRGreyscale:
                    // Values are in BGR in memory. Here in RGB order.
                    row[x * bytes_pp_out + red_pos_idx]     = pixel_data_in[x + buf_pos];
                    row[x * bytes_pp_out + green_pos_idx]   = pixel_data_in[x + buf_pos];
                    row[x * bytes_pp_out + blue_pos_idx]    = pixel_data_in[x + buf_pos];
                    break;
                case IRNightVision:
                    // Values are in BGR in memory. Here in RGB order.
                    row[x * bytes_pp_out + red_pos_idx]     = 0;
                    row[x * bytes_pp_out + green_pos_idx]   = pixel_data_in[x + buf_pos];
                    row[x * bytes_pp_out + blue_pos_idx]    = 0;
                    break;
                case IRIronbow:
                    // Values are in BGR in memory. Here in RGB order.
                    row[x * bytes_pp_out + red_pos_idx]     = (iron_palette[pixel_data_in[x + buf_pos]] >> 16)&0xFF;
                    row[x * bytes_pp_out + green_pos_idx]   = (iron_palette[pixel_data_in[x + buf_pos]] >> 8) & 0xFF;
                    row[x * bytes_pp_out + blue_pos_idx]    =  iron_palette[pixel_data_in[x + buf_pos]] & 0xFF;
                    break;
                case IRInfrared:
                default:
                    // Values are in BGR in memory. Here in RGB order.
                    row[x * bytes_pp_out + red_pos_idx]     = pixel_data_in[x + buf_pos];
                    row[x * bytes_pp_out + green_pos_idx]   = 0;
                    row[x * bytes_pp_out + blue_pos_idx]    = 0;
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
            auto device_info = controller.deviceInfo();
            auto controller_type = controller.type();

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
            int battery_report = controller.batteryReport();
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
                ImGui::Text("%.2fV - %d", controller.batteryVoltage(), controller.batteryPercentage());
            }

            ImGui::Text("Temperature: %.2fF / %.2fC ", controller.temperatureF(), controller.temperatureC());

            ImGui::Text("S/N: %s", controller.serialNumber().c_str());
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
#elif defined(__linux__)
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
                if(default_controller.handle() && ImGui::Button("Play")){
                    play_hd_rumble_file(default_controller.handle(), default_controller.timming_byte, rumble_data);
                }
            }
        }
        
        void showIRCamera(Controller& controller) {
            static const char* col_fils[] = {
                "Greyscale",
                "Night Vision",
                "Ironbow",
                "Infrared"
            };
            static auto& resolutions = Controller::IRSensor::resolutions;
            
            static u16 exposure_amt = 300;
            static int denoise_edge_smooth = 35;
            static int denoise_color_intrpl = 68; // Color Interpolation

            // The controller's ir config
            ir_image_config& ir_config_0 = controller.ir_sensor.config;

            ImGui::Columns(3);
            /* Stream/Capture */ {
                ImGui::ScopedDisableItems disable(controller.ir_sensor.capture_in_progress);

                ImGui::BeginGroup();
                ImGui::CheckboxFlags("Flip Capture", (uint32_t*)&ir_config_0.ir_flip, flip_dir_0);
                if(ImGui::Button("Capture Image")) {
                    // Initialize the IR Sensor AND Take a photo with the ir sensor configs store in Controller::ir_sensor.
                    controller.ir_sensor.capture();
                    disable.ensureDisabled();
                }
                ImGui::SameLine();
                bool video_in_progress = controller.ir_sensor.enable_ir_video_photo && controller.ir_sensor.capture_in_progress;
                if(video_in_progress)
                    disable.allowEnable();
                if(ImGui::Button(video_in_progress ? "Stop" : "Stream Video")){
                    if(!video_in_progress) {
                        controller.ir_sensor.enable_ir_video_photo = true;
                        controller.ir_sensor.capture();
                    } else
                        controller.ir_sensor.enable_ir_video_photo = false; // For stopping the video stream
                }
                disable.ensureDisabled();
                auto& size = std::get<2>(resolutions[controller.ir_sensor.res_idx_selected]);
                auto avail_size = ImGui::GetContentRegionAvail();
                auto resize = resizeRectAToFitInRectB(size, avail_size);
                ImGui::Image((ImTextureID)controller.ir_sensor.getCaptureTexID(), {(float)resize.width, (float)resize.height});
                ImGui::EndGroup();
            }

            ImGui::NextColumn();
            /* IR Message Stream / Capture Information */ {
                ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvailWidth());
                ImGui::Text("%s", controller.ir_sensor.message_stream.rdbuf()->str().c_str());
                ImGui::PopTextWrapPos();

                auto capture_info = controller.ir_sensor.capture_info;
                ImGui::Text("FPS: %.2f (%d ms)", capture_info.fps, (int) ((capture_info.fps > 0.0f) ? (1/capture_info.fps) : NAN));
                ImGui::Text("Frame counter: %d", capture_info.frame_counter);
                ImGui::Text("Duration: %f (seconds)", capture_info.duration);
                ImGui::Text("Ambient Noise: %.2f", capture_info.noise_level);
                ImGui::Text("Intensity: %d%%", capture_info.avg_intensity_percent);
                ImGui::Text("EXFilter: %d", capture_info.exfilter);
                ImGui::Text("EXFilter Int: %d", capture_info.exf_int);
                ImGui::Text("White Px: %d%%", capture_info.white_pixels_percent);
            }

            ImGui::NextColumn();
            
            /* IR Camera Settings */ {
                ImGui::BeginGroup();
                ImGui::Text("IR Camera Settings");
                /* Output Frame Settings */ {
                    ImGui::BeginGroup();
                    if(ImGui::CollapsingHeader("Frame Display Settings")){
                        ImGui::ScopedDisableItems disable(controller.ir_sensor.capture_in_progress);

                        // Resolution settings
                        ImGui::BeginGroup();
                        ImGui::Text("Resolution");

                        constexpr int res_count = IM_ARRAYSIZE(resolutions);
                        for(int i=0; i < res_count; i++){
                            auto& res_tuple = resolutions[i];
                            bool selected = std::get<1>(res_tuple) == std::get<1>(resolutions[controller.ir_sensor.res_idx_selected]);
                            if(ImGui::RadioButton(std::get<0>(res_tuple), selected)){
                                controller.ir_sensor.res_idx_selected = i;
                                // Set the resolution value on the config.
                                ir_config_0.ir_res_reg = std::get<1>(res_tuple);
                            }
                        }
                        ImGui::EndGroup();

                        ImGui::SameLine();
                        // Color filter settings
                        ImGui::BeginGroup();
                        ImGui::Text("Color Filter");

                        for(int i = 0; i < IRColorCount; i++){
                            if(ImGui::RadioButton(col_fils[i], i == controller.ir_sensor.colorize_with))
                                controller.ir_sensor.colorize_with = (IRColor)i;
                        }
                        ImGui::EndGroup();
                    }
                    ImGui::EndGroup();

                    ImGui::BeginGroup();
                    if(ImGui::CollapsingHeader("Advanced Settings")){
                        ImGui::ScopedDisableItems disable(controller.ir_sensor.capture_in_progress);

                        static const u8 min_x = 1;
                        static const u8 max_x = 20;
                        ImGui::SliderScalar("Digital Gain (lossy)", ImGuiDataType_U8, &ir_config_0.ir_digital_gain, &min_x, &max_x, "x%d");
                        /* Exposure */ {
                            ImGui::BeginGroup();
                            if(ImGui::InputScalar("Exposure (us | micro-seconds)", ImGuiDataType_U16, &exposure_amt))
                                ir_image_config_Sets::exposure(ir_config_0.ir_exposure, exposure_amt);
                            if(ImGui::Checkbox("Auto Exposure (experimental)", &controller.ir_sensor.auto_exposure)){
                                if(!controller.ir_sensor.auto_exposure && controller.ir_sensor.enable_ir_video_photo) {
                                    controller.ir_sensor.auto_exposure = false;
                                    ir_image_config_Sets::exposure(ir_config_0.ir_exposure, exposure_amt);
                                } else {
                                    ir_config_0.ir_digital_gain = 1;
                                }
                            }
                            ImGui::EndGroup();
                        }
                        /* Denoise */ {
                            ImGui::BeginGroup();
                            ImGui::CheckboxFlags("Enable", &ir_config_0.ir_denoise, ir_denoise_Enable);
                            if(ImGui::InputInt("Edge Smoothing", &denoise_edge_smooth))
                                ir_image_config_Sets::denoise_edge_smooth(ir_config_0.ir_denoise, denoise_edge_smooth); // TODO: Clamp to 0-256
                            if(ImGui::InputInt("Color Interpolation", &denoise_color_intrpl))
                                ir_image_config_Sets::denoise_color_intrpl(ir_config_0.ir_denoise, denoise_color_intrpl); // TODO: Clamp to 0-256
                            ImGui::EndGroup();
                        }
                    }
                    ImGui::EndGroup();
                }
                /* Near-Infrared Light Settings */ {
                    ImGui::BeginGroup();
                    if(ImGui::CollapsingHeader("Near-Infrared Light")){
                        ImGui::ScopedDisableItems disable(controller.ir_sensor.capture_in_progress);

                        static constexpr u8 max_intensity = ~u8(0);
                        static constexpr u8 min_intensity = 0;
                        ImGui::CheckboxFlags("(Disable) Far/Narrow (75*) Leds 1-2", (uint32_t*)&ir_config_0.ir_leds, ir_leds_NarrowDisable);
                        ImGui::SliderScalar("Intensity##narrow", ImGuiDataType_U8, &ir_config_0.ir_leds_intensity, &min_intensity, &max_intensity, "%u");
                        ImGui::CheckboxFlags("(Disable) Near/Wide (130*) Leds 3-4", (uint32_t*)&ir_config_0.ir_leds, ir_leds_WideDisable);
                        ImGui::SliderScalar("Intensity##wide", ImGuiDataType_U8, (u8*)&ir_config_0.ir_leds_intensity + 1, &min_intensity, &max_intensity, "%u");
                        ImGui::CheckboxFlags("Flashlight Mode", (uint32_t*)&ir_config_0.ir_leds, ir_leds_FlashlightMode);
                        ImGui::CheckboxFlags("Strobe Flash Mode", (uint32_t*)&ir_config_0.ir_leds, ir_leds_StrobeFlashMode);
                        ImGui::CheckboxFlags("External IR Filter", (uint32_t*)&ir_config_0.ir_ex_light_filter, ir_ex_light_fliter_0);
                    }
                    ImGui::EndGroup();

                }
                /* IR Sensor Register/Value */ {
                    ImGui::BeginGroup();
                    if(ImGui::CollapsingHeader("Custom IR Sensor Register/Value")){
                        ImGui::ScopedDisableItems disable(controller.ir_sensor.capture_in_progress);

                        ImGui::InputInt("Register", (int*)&ir_config_0.ir_custom_register); // TODO: clamp to 0x5FF
                        ImGui::InputInt("Value", (int *)((uint16_t*)&ir_config_0.ir_custom_register + 1)); // TODO: clamp to 0xFF
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndGroup();
            }
            ImGui::Columns(1);
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
    void init() {
        GPUTexture::SideLoader::start();
        Helpers::loadBatteryImages();
    }
}
#endif
