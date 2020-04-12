/**
 * Code extracted from the orginal UI framework source (CppWinForm.)
 * Goal: Eliminate dependency to the original UI framework so that useful code
 * that was once only accessible by the original UI is now accessible through
 * an API.
 */
#include <sstream>
#include <string>
#include <iterator>
#include <vector>

#include "jctool_api.hpp"
#include "jctool.h"
#include "hidapi.h"

#include "imgui.h"
#ifdef __jctool_cpp_API__
#include "ImageLoad/ImageLoad.hpp"
#include "Controller.hpp"
#endif

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

TemperatureData parseTemperatureData(const unsigned char* temp_data){
    // Convert reading to Celsius according to datasheet
    float celsius = 25.0f + uint16_to_int16(temp_data[1] << 8 | temp_data[0]) * 0.0625f;
    return {celsius, celsius*1.8f + 32};
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
    namespace UI {
        /**
         * UI Port from jctool/FormJoy.h
         */
        void showDeviceInfo(const unsigned char* device_info){
            ImGui::Text("Device Info");
            ImGui::Text("Firmware: %X.%2X", device_info[0], device_info[1]);
            ImGui::Text(
                "MAC: %2X:%2X:%2X:%2X:%2X:%2X",
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
                ImGui::Image(reinterpret_cast<ImTextureID>(battery_img.getRID()), ImVec2(battery_img.getWidth(), battery_img.getHeight()));
                ImGui::Text("%.2fV - %d", controller.batteryGetVoltage(), controller.batteryGetPercentage());
            }

            ImGui::Text("Temperature: %.2fF / %.2fC ", controller.temperatureGetFarenheight(), controller.temperatureGetCelsius());

            ImGui::Text("S/N: %s", controller.getSerialNumber().c_str());
        }

        void show(){
            static Controller default_controller;
            // Try to establish a connection with a controller.
            if(ImGui::Button("Try Connection Attempt"))
                default_controller.connection();
            showControllerInfo(default_controller);
        }
    }
}
#endif