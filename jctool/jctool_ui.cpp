/*

MIT License

Copyright (c) 2020 Jonathan Mendez

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

/**
 * =============================================================================
 * The code below attempts to provide a crossplatform user-interface which
 * unfortunately was not included in the original Joy-Con Toolkit.
 * =============================================================================
 */

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <vector>
#include <map>
#include <memory>

// Open a file dialog window
#ifdef WIN32
#include "Windows.h"
#include "commdlg.h"
#endif

#include "jctool.h"
#include "jctool_helpers.hpp"
#include "ir_sensor.h"

#include "hidapi.h"

#define EASY_HELPERS_UI
#define EASY_IMAGELOAD
#define EASY_DIREXPLORER_UI
#include "easy-imgui/easy_imgui.h"
#include "imgui_internal.h" // PushItemFlags

#include "IRSensor.hpp"
#define IMAGE_RES_PATH "jctool/original_res/"
#define IMAGE_RES_EXT ".png"

#include "session/controller_session.hpp"
#include "TP/TP.hpp"

//#include "commands/com_builder_ui.hpp"

namespace JCToolkit {
    namespace Assets {
        static ImageResource battery_indicators[] = {
            {IMAGE_RES_PATH "batt_0" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "batt_0_chr" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "batt_25" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "batt_25_chr" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "batt_50" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "batt_50_chr" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "batt_75" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "batt_75_chr" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "batt_100" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "batt_100_chr" IMAGE_RES_EXT, false}
        };
        static ImageResource left_joycon[] = {
            {IMAGE_RES_PATH "l_joy_body" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "l_joy_buttons" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "l_joy_lines" IMAGE_RES_EXT, false},
        };
        static ImageResource right_joycon[] = {
            {IMAGE_RES_PATH "r_joy_body" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "r_joy_buttons" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "r_joy_lines" IMAGE_RES_EXT, false},
        };
        static ImageResource pro_controller[] = {
            {IMAGE_RES_PATH "pro_body" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "pro_buttons" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "pro_grips_l" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "pro_grips_r" IMAGE_RES_EXT, false},
            {IMAGE_RES_PATH "pro_lines" IMAGE_RES_EXT, false},
        };

        enum ControllerPartIdx {
            Body,
            Buttons,
            Lines,
            SharedPartCount,
            LeftGrip = Lines,
            RightGrip,
        };

        constexpr ImageResource* get_con_lines(ConHID::ProdID con_prod_id){
            #define ARRAY_LAST(x) x[IM_ARRAYSIZE(x) - 1]
            switch(con_prod_id){
                case ConHID::JoyConLeft:
                    return &ARRAY_LAST(left_joycon);
                case ConHID::JoyConRight:
                    return &ARRAY_LAST(right_joycon);
                case ConHID::ProCon:
                    return &ARRAY_LAST(pro_controller);
                default:
                    return nullptr;
            }
        }

        static const char* default_rumbles[] = {
            "Super Mario Bros - Main theme",
            "Super Mario Odyssey - Jump Up, Super Star!"
        };

        enum RetailColorTuple : u8 {
            RetailColLabel,
            RetailColBody,
            RetailColButton
        };
        using RGBColor = u32;
        using RetailColor = std::tuple<const char*, const SPIColors::color_t, const SPIColors::color_t>;
        // Extracted from original_res/retail_colors.xml
        constexpr RetailColor retail_color_presets[] = {
            {"Grey",        {0x82,0x82,0x82}, {0x0F,0x0F,0x0F}},
            {"Neon Blue",   {0x0A,0xB9,0xE6}, {0x00,0x1E,0x1E}},
            {"Neon Red",    {0xFF,0x3C,0x28}, {0x1E,0x0A,0x0A}},
            {"Neon Yellow", {0xE6,0xFF,0x00}, {0x14,0x28,0x00}},
            {"Neon Pink",   {0xFF,0x32,0x78}, {0x28,0x00,0x1E}},
            {"Neon Green",  {0x1E,0xDC,0x00}, {0x00,0x28,0x00}},
            {"Red",         {0xE1,0x0F,0x00}, {0x28,0x0A,0x0A}},
            {"Pro Black",   {0x32,0x32,0x32}, {0xFF,0xFF,0xFF}}
        };
    }
    namespace Helpers {
        void load_image_res_gpu() {
            for(int i = 0; i < IM_ARRAYSIZE(Assets::battery_indicators); i++)
                Assets::battery_indicators[i].sendToGPU();
            for(int i = 0; i < IM_ARRAYSIZE(Assets::left_joycon); i++)
                Assets::left_joycon[i].sendToGPU();
            for(int i = 0; i < IM_ARRAYSIZE(Assets::right_joycon); i++)
                Assets::right_joycon[i].sendToGPU();
            for(int i = 0; i < IM_ARRAYSIZE(Assets::pro_controller); i++)
                Assets::pro_controller[i].sendToGPU();
        }
    }

    struct ConSessData {
        struct User {
            bool spi_dumping = false;
            bool spi_cancel_dump = false;
            size_t spi_bytes_dumped = 0;
            bool is_rumble_active = false;
            SPIColors preview_col;
            RumbleData rd;
            std::unique_ptr<IRSensor> ir{new IRSensor()};
        };
        struct State {
            u8 device_info[10];
            TemperatureData temp_data;
            BatteryData batt_data;
            SPIColors saved_col;
            SessionStatus status;
            std::string sn;
        };
        State state;
        User user;
    };
    namespace UI {
        void draw_controller(ConHID::ProdID con_type, const SPIColors& use_colors){
            // Detect which controller type label to use.
            const char* controller_type_label = "NONE";
            ImageResource* con_images = nullptr;
            switch (con_type)
            {
            case ConHID::JoyConLeft:
                controller_type_label = "Joy-Con (L)";
                con_images = Assets::left_joycon;
                break;
            case ConHID::JoyConRight:
                controller_type_label = "Joy-Con (R)";
                con_images = Assets::right_joycon;
                break;
            case ConHID::ProCon:
                controller_type_label = "Pro Controller";
                con_images = Assets::pro_controller;
                break;
            default:
                controller_type_label = "Unrecognized Controller";
                return;
            }

            auto con_start = ImGui::GetCursorPos();
  
            for(int i = 0,
                extra = ((con_type == ConHID::ProCon) ? 2 : 0);
                i < (Assets::SharedPartCount + extra);
                i++
            ){
                SPIColors::color_t col;
                switch(i){
                    case Assets::Body:{
                        col = use_colors.body;
                    }break;
                    case Assets::Buttons:{
                        col = use_colors.buttons;
                    }break;
                    case Assets::LeftGrip:{
                        col = use_colors.left_grip;
                    }break;
                    case Assets::RightGrip:{
                        col = use_colors.right_grip;
                    }break;
                    default:
                        col.r = ~u8(0);
                        col.g = ~u8(0);
                        col.b = ~u8(0);
                    break;
                }
                ImGui::SetCursorPos(con_start);
                ImGui::ImageAutoFit(
                    (ImTextureID)con_images[i].getRID(),
                    {(float)con_images[i].getWidth(), (float)con_images[i].getHeight()},
                    {0,0}, {1,1},
                    ImGui::ColorConvertU32ToFloat4(IM_COL32(col.r,col.g,col.b,255))
                );
            }
            if(ImGui::IsItemHovered()){
                ImGui::SetTooltip("%s", controller_type_label);
            }
        }

        namespace ModifyController {
            enum class SPIDumpAction {
                None,
                DoDump,
                CancelDump
            };
            SPIDumpAction show_dump_spi(bool dumping, size_t bytes_dumped){
                SPIDumpAction res = SPIDumpAction::None;

                ImGui::TextWrapped("ATTENTION: It is recommended that you backup your SPI before uploading any modifications to your controller! The only person to blame will be yourself if you choose to ignore this warning!");
                
                ImGui::ScopeDisableItems disable(dumping);
                if(ImGui::Button("Dump SPI"))
                    res = SPIDumpAction::DoDump;

                if(dumping)
                    disable.allowEnabled();
                else
                    disable.ensureDisabled();
                
                if(ImGui::Button("Cancel Dump"))
                    res = SPIDumpAction::CancelDump;
                ImGui::SameLine();
                ImGui::ProgressBar((float) bytes_dumped / SPI_SIZE, {-1, 0},
                    std::string(std::to_string(int((float) bytes_dumped / SPI_SIZE * 100)) + "%").c_str()
                );
                ImGui::Text("%.2f KB / %.2f KB", (float) bytes_dumped / 1024, float(SPI_SIZE / 1024));

                return res;
            }

            void show_retail_colors(std::function<void(SPIColors::color_t, float)> show_color){
                if(ImGui::BeginTable("Retail Colors",
                    3,
                    ImGuiTableFlags_ScrollX | ImGuiTableFlags_NoHostExtendY | ImGuiTableFlags_ScrollFreezeLeftColumn | ImGuiTableFlags_ScrollFreezeTopRow | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInner)
                ){
                    ImGui::TableNextCell();
                    ImGui::Text("Color Name");
                    ImGui::TableNextCell();
                    ImGui::Text("Body");
                    ImGui::TableNextCell();
                    ImGui::Text("Buttons");
                    int id = 0;

                    const float radius = 10.0f;
                    // Labels
                    for(auto& retail_color: Assets::retail_color_presets){
                        ImGui::TableNextCell();
                        ImGui::Text("%s", std::get<Assets::RetailColLabel>(retail_color));

                        ImGui::TableNextCell();
                        ImGui::PushID(id++);
                        show_color(std::get<Assets::RetailColBody>(retail_color), radius);
                        ImGui::PopID();

                        ImGui::TableNextCell();
                        ImGui::PushID(id++);
                        show_color(std::get<Assets::RetailColButton>(retail_color), radius);
                        ImGui::PopID();
                    }
                    ImGui::EndTable();
                }
            }

            bool show_color_editor(ImVec4& primary_color, ImVec4& secondary_color){
                ImGui::BeginGroup();
                // Generate a dummy default palette. The palette will persist and can be edited.
                static bool saved_palette_init = true;
                static ImVec4 saved_palette[32] = {
                    ImGui::ColorConvertU32ToFloat4(0x0AB9E6 + 0xff000000),
                    ImGui::ColorConvertU32ToFloat4(0x828282 + 0xff000000),
                    ImGui::ColorConvertU32ToFloat4(0x828282),
                    ImGui::ColorConvertU32ToFloat4(0x828282),
                };

                bool color_changed = false;

                color_changed |= ImGui::ColorPicker3("##picker", (float*)&primary_color, ImGuiColorEditFlags_NoSidePreview);
                ImGui::SameLine();

                ImGui::BeginGroup(); // Lock X position

                ImGui::Text("Current");
                ImGui::ColorButton("##current", primary_color, ImGuiColorEditFlags_NoPicker, ImVec2(60,40));

                ImGui::Text("Previous");
                if (ImGui::ColorButton("##previous", secondary_color, ImGuiColorEditFlags_NoPicker, ImVec2(60,40)))
                    std::swap(primary_color, secondary_color);

                for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
                {
                    ImGui::PushID(n);
                    if ((n % 8) != 0)
                        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);
                    if (ImGui::ColorButton("##palette", saved_palette[n], ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip, ImVec2(20,20)))
                        primary_color = ImVec4(saved_palette[n].x, saved_palette[n].y, saved_palette[n].z, primary_color.w); // Preserve alpha!

                    // Allow user to drop colors into each palette entry
                    // (Note that ColorButton is already a drag source by default, unless using ImGuiColorEditFlags_NoDragDrop)
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
                            memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 3);
                        ImGui::EndDragDropTarget();
                    }

                    ImGui::PopID();
                }

                ImGui::EndGroup();
                
                
                ImGui::EndGroup();

                return color_changed;
            }

            void color_editor_page(ConHID::ProdID con_type, SPIColors& preview_colors) {
                static int selected_part = 0;
                static const char* part_labels[] = {
                    "Body",
                    "Buttons",
                    "Left Grip",
                    "Right Grip"
                };

                ImGui::Columns(2);

                int j;
                if((con_type != ConHID::ProCon)){
                    j = Assets::ControllerPartIdx::Lines;
                    if(!(selected_part < Assets::ControllerPartIdx::Lines))
                        selected_part = 0;
                } else {
                    j = IM_ARRAYSIZE(part_labels);
                }
                if(ImGui::BeginCombo("Part to color", part_labels[selected_part])){
                    for(int i = 0; i < j; i++){
                        if(ImGui::Selectable(part_labels[i]))
                            selected_part = i;
                    }
                    ImGui::EndCombo();
                }

                draw_controller(con_type, preview_colors); // Draw the controller color preview.

                ImGui::NextColumn();

                ImGui::MakeSection({"Color Picker",
                    [&](){
                        static ImVec4 primary_color = ImGui::ColorConvertU32ToFloat4(IM_COL32_WHITE);
                        static ImVec4 secondary_color = ImGui::ColorConvertU32ToFloat4(IM_COL32_BLACK);
                        bool color_changed = false;

                        if(ImGui::BeginTabBar("palletes")) {
                            if(ImGui::BeginTabItem("Retail Colors")){
                                auto _retailColorButton = [&color_changed](SPIColors::color_t spi_color, float radius){
                                    auto conv_col = ImGui::ColorConvertU32ToFloat4(IM_COL32(spi_color.r, spi_color.g, spi_color.b, 255));
                                    ImGui::PushStyleColor(ImGuiCol_Button, conv_col);
                                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, radius);
                                    if(ImGui::Button("##retail_palette",
                                            {radius*2, radius*2}
                                    )){
                                        std::swap(primary_color, secondary_color);
                                        primary_color = conv_col;
                                        color_changed |= true;
                                    }
                                    ImGui::PopStyleVar();
                                    ImGui::PopStyleColor();
                                };

                                if(!ImGui::BeginChild("retail colors body")) {
                                    ImGui::EndChild();
                                } else {
                                    ModifyController::show_retail_colors(_retailColorButton);   
                                    ImGui::EndChild();
                                }

                                ImGui::EndTabItem();
                            }

                            if(ImGui::BeginTabItem("Custom Colors")){
                                color_changed |= ModifyController::show_color_editor(primary_color, secondary_color);
                                ImGui::EndTabItem();
                            }

                            ImGui::EndTabBar();
                        }

                        if(color_changed){
                            static auto _set_preview_color = [](SPIColors::color_t& prev_col, ImU32& col){
                                memcpy(&prev_col, &col, 3);
                            };
                            auto x = ImGui::GetColorU32(primary_color);
                            switch(selected_part){
                                case Assets::Body:
                                _set_preview_color(preview_colors.body, x);
                                break;
                                case Assets::Buttons:
                                _set_preview_color(preview_colors.buttons, x);
                                break;
                                case Assets::LeftGrip:
                                _set_preview_color(preview_colors.left_grip, x);
                                break;
                                case Assets::RightGrip:
                                _set_preview_color(preview_colors.right_grip, x);
                                break;
                            }
                        }

                        ImGui::Columns(1);
                    }
                });
            }
        }
        /**
         * Firmware, MAC, S/N
         * =============================
         * UI Port from jctool/FormJoy.h
        void show_controller_info(Controller& controller){
            auto device_info = controller.deviceInfo();
            ImGui::BeginGroup();

            ImGui::Text("Firmware: %X.%02X", device_info[0], device_info[1]);
            ImGui::Text(
                "MAC: %02X:%02X:%02X:%02X:%02X:%02X",
                device_info[4], device_info[5], device_info[6],
                device_info[7], device_info[8], device_info[9]
            );
            ImGui::Text("S/N: %s", controller.serialNumber().c_str());

            ImGui::EndGroup();
        }
        */

        void showBattery(BatteryData& batt){
            auto report = ((batt.report >= 0) && (batt.report < 10)) ? batt.report : -1;
            if(report < 0)
                ImGui::Text("Battery: Invalid reading.");
            else{
                ImageResource& battery_img = Assets::battery_indicators[report];
                ImGui::Image(
                    reinterpret_cast<ImTextureID>(battery_img.getRID()),
                    ImVec2(
                        static_cast<float>(battery_img.getWidth()),
                        static_cast<float>(battery_img.getHeight())
                    )
                );
                ImGui::SameLine();
                ImGui::Text("%d%% [%.2fV]", batt.percent, batt.voltage);
            }
        }

        void showTemperature(TemperatureData& temp){
            ImGui::Text("Temperature: %.2fF / %.2fC ", temp.fahrenheit, temp.celsius);
        }

        int showSelectDefaultTune(){
            static int selected_song = 0;
            if(ImGui::BeginCombo("Default songs", Assets::default_rumbles[selected_song])){
                for(int i = 0; i < IM_ARRAYSIZE(Assets::default_rumbles); i++){
                    if(ImGui::Selectable(Assets::default_rumbles[i], selected_song == i))
                        selected_song = i;
                }
                ImGui::EndCombo();
            }
            if(ImGui::Button("Play selected song"))
                return selected_song;
            return -1;
        }

        void showRumbleData(RumbleData& rumble_data){
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
                    "Samples: %d\n"
                    "Play time: %.2f",
                    std::filesystem::path(rumble_data.from_file).filename().generic_string().c_str(),
                    type_label,
                    rumble_data.metadata.sample_rate,
                    rumble_data.metadata.samples,
                    ((float) rumble_data.metadata.sample_rate) / 1000 * rumble_data.metadata.samples
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
            }
        }

        void showSelectRumbleFile(RumbleData& rumble_data){
            static bool show_explorer = false;
            static const char* explorer_name = "Choose HD Rumble File";
            static ImGuiID dir_exp = ImGui::DirectoryExplorer::NewDirExplorer(explorer_name, std::filesystem::current_path().string());
            static std::string selected;

            if(ImGui::DirectoryExplorer::OpenFileDialog(dir_exp, selected, show_explorer, ".jcvib,.bnvib")) {
                const char* file_name = selected.c_str();
                if(file_name) {
                    std::ifstream rumble_fstream = std::ifstream(file_name, std::ios_base::binary);
                    if(!rumble_fstream.bad()){
                        RumbleData new_rumble_data;
                        // Calculate the size of the file.
                        size_t rumble_data_size = std::filesystem::file_size(file_name);
                        u8* read_buf = new u8[rumble_data_size];
                        rumble_fstream.read((char*)read_buf, rumble_data_size);

                        new_rumble_data.from_file = file_name;
                        new_rumble_data.metadata = getVIBMetadata(read_buf);
                        new_rumble_data.data.reset(read_buf, [](u8* p){
                            delete[] p;
                        }); // Allocate enough space for the rumble data.

                        rumble_data = new_rumble_data;
                    }
                }
            }

            if(ImGui::Button("Load Rumble Data"))
                show_explorer = true;
        }

        static const char* col_fils[] = {
            "Greyscale",
            "Night Vision",
            "Ironbow",
            "Infrared"
        };

        void showIRCameraFeed(IRSensor& ir_sensor){
            auto& size = std::get<2>(ir_resolutions[ir_sensor.res_idx_selected]);
            ImGui::ImageAutoFit((ImTextureID)ir_sensor.getCaptureTexID(), {(float)size.x, (float)size.y});
        }

        enum class IRFeedControl {
            None,
            Capture
        };
        IRFeedControl showIRCameraFeedControl(IRSensor& ir){
            ir_image_config& ir_config = ir.config;
            IRFeedControl res = IRFeedControl::None;
            ImGui::ScopeDisableItems disable(ir.capture_in_progress);

            ImGui::BeginGroup();
            ImGui::CheckboxFlags("Flip Capture", (uint32_t*)&ir_config.ir_flip, flip_dir_0);
            if(ImGui::Button("Capture Image")){
                ir.capture_mode = IRCaptureMode::Image;
                res = IRFeedControl::Capture;
            }
            ImGui::SameLine();
            bool video_in_progress = ir.capture_mode == IRCaptureMode::Video;
            if(video_in_progress)
                disable.allowEnabled();
            if(ImGui::Button(video_in_progress ? "Stop" : "Stream Video")){
                if(!video_in_progress) {
                    ir.capture_mode = IRCaptureMode::Video;
                    res = IRFeedControl::Capture;
                } else {
                    ir.capture_mode = IRCaptureMode::Off; // For stopping the video stream
                }
            }
            ImGui::EndGroup();
            return res;
        }

        void showIRCaptureSettings(IRSensor& ir){
            static u16 exposure_amt = 300;
            static int denoise_edge_smooth = 35;
            static int denoise_color_intrpl = 68; // Color Interpolation

            ir_image_config& ir_config = ir.config;
            ImGui::BeginGroup();
            ImGui::Text("IR Camera Settings");
            /* Output Frame Settings */ {
                ImGui::BeginGroup();
                if(ImGui::CollapsingHeader("Frame Display Settings")){
                    ImGui::ScopeDisableItems disable(ir.capture_in_progress);

                    // Resolution settings
                    ImGui::BeginGroup();
                    ImGui::Text("Resolution");

                    const int res_count = IM_ARRAYSIZE(ir_resolutions);
                    for(int i=0; i < res_count; i++){
                        auto& res_tuple = ir_resolutions[i];
                        bool selected = std::get<1>(res_tuple) == std::get<1>(ir_resolutions[ir.res_idx_selected]);
                        if(ImGui::RadioButton(std::get<0>(res_tuple), selected)){
                            ir.res_idx_selected = i;
                            // Set the resolution value on the config.
                            ir_config.ir_res_reg = std::get<1>(res_tuple);
                        }
                    }
                    ImGui::EndGroup();

                    ImGui::SameLine();
                    // Color filter settings
                    ImGui::BeginGroup();
                    ImGui::Text("Color Filter");

                    for(int i = 0; i < IRColorCount; i++){
                        if(ImGui::RadioButton(col_fils[i], i == ir.colorize_with))
                            ir.colorize_with = (IRColor)i;
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndGroup();

                ImGui::BeginGroup();
                if(ImGui::CollapsingHeader("Advanced Settings")){
                    ImGui::ScopeDisableItems disable(ir.capture_in_progress);

                    static const u8 min_x = 1;
                    static const u8 max_x = 20;
                    ImGui::SliderScalar("Digital Gain (lossy)", ImGuiDataType_U8, &ir_config.ir_digital_gain, &min_x, &max_x, "x%d");
                    /* Exposure */ {
                        ImGui::BeginGroup();
                        if(ImGui::InputScalar("Exposure (us | micro-seconds)", ImGuiDataType_U16, &exposure_amt))
                            ir_image_config_Sets::exposure(ir_config.ir_exposure, exposure_amt);
                        if(ImGui::Checkbox("Auto Exposure (experimental)", &ir.auto_exposure)){
                            if(!ir.auto_exposure && (ir.capture_mode == IRCaptureMode::Video)) {
                                ir.auto_exposure = false;
                                ir_image_config_Sets::exposure(ir_config.ir_exposure, exposure_amt);
                            } else {
                                ir_config.ir_digital_gain = 1;
                            }
                        }
                        ImGui::EndGroup();
                    }
                    /* Denoise */ {
                        ImGui::BeginGroup();
                        ImGui::CheckboxFlags("Enable", &ir_config.ir_denoise, ir_denoise_Enable);
                        if(ImGui::InputInt("Edge Smoothing", &denoise_edge_smooth))
                            ir_image_config_Sets::denoise_edge_smooth(ir_config.ir_denoise, denoise_edge_smooth); // TODO: Clamp to 0-256
                        if(ImGui::InputInt("Color Interpolation", &denoise_color_intrpl))
                            ir_image_config_Sets::denoise_color_intrpl(ir_config.ir_denoise, denoise_color_intrpl); // TODO: Clamp to 0-256
                        ImGui::EndGroup();
                    }
                }
                ImGui::EndGroup();
            }
            /* Near-Infrared Light Settings */ {
                ImGui::BeginGroup();
                if(ImGui::CollapsingHeader("Near-Infrared Light")){
                    ImGui::ScopeDisableItems disable(ir.capture_in_progress);

                    static constexpr u8 max_intensity = ~u8(0);
                    static constexpr u8 min_intensity = 0;
                    ImGui::CheckboxFlags("(Disable) Far/Narrow (75*) Leds 1-2", (uint32_t*)&ir_config.ir_leds, ir_leds_NarrowDisable);
                    ImGui::SliderScalar("Intensity##narrow", ImGuiDataType_U8, &ir_config.ir_leds_intensity, &min_intensity, &max_intensity, "%u");
                    ImGui::CheckboxFlags("(Disable) Near/Wide (130*) Leds 3-4", (uint32_t*)&ir_config.ir_leds, ir_leds_WideDisable);
                    ImGui::SliderScalar("Intensity##wide", ImGuiDataType_U8, (u8*)&ir_config.ir_leds_intensity + 1, &min_intensity, &max_intensity, "%u");
                    ImGui::CheckboxFlags("Flashlight Mode", (uint32_t*)&ir_config.ir_leds, ir_leds_FlashlightMode);
                    ImGui::CheckboxFlags("Strobe Flash Mode", (uint32_t*)&ir_config.ir_leds, ir_leds_StrobeFlashMode);
                    ImGui::CheckboxFlags("External IR Filter", (uint32_t*)&ir_config.ir_ex_light_filter, ir_ex_light_fliter_0);
                }
                ImGui::EndGroup();

            }
            /* IR Sensor Register/Value */ {
                ImGui::BeginGroup();
                if(ImGui::CollapsingHeader("Custom IR Sensor Register/Value")){
                    ImGui::ScopeDisableItems disable(ir.capture_in_progress);

                    ImGui::InputInt("Register", (int*)&ir_config.ir_custom_register); // TODO: clamp to 0x5FF
                    ImGui::InputInt("Value", (int *)((uint16_t*)&ir_config.ir_custom_register + 1)); // TODO: clamp to 0xFF
                }
                ImGui::EndGroup();
            }
            ImGui::EndGroup();
        }

        void showIRCaptureInfo(IRSensor& ir){
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvailWidth());
            ImGui::Text("%s", ir.capture_status.message_stream.rdbuf()->str().c_str());
            ImGui::PopTextWrapPos();

            auto& capture_status = ir.capture_status;
            ImGui::Text("FPS: %.2f (%d ms)", capture_status.fps, (int) ((capture_status.fps > 0.0f) ? (1/capture_status.fps) : NAN));
            ImGui::Text("Frame counter: %d", capture_status.frame_counter);
            ImGui::Text("Last frag no: %d", capture_status.last_frag_no);
            ImGui::Text("Duration: %f (seconds)", capture_status.duration);
            ImGui::Text("Ambient Noise: %.2f", capture_status.noise_level);
            ImGui::Text("Intensity: %d%%", capture_status.avg_intensity_percent);
            ImGui::Text("EXFilter: %d", capture_status.exfilter);
            ImGui::Text("EXFilter Int: %d", capture_status.exf_int);
            ImGui::Text("White Px: %d%%", capture_status.white_pixels_percent);
        }

        void show_as_sections_page(ImGui::Display* sections, int section_count, ImGui::Display*& selected_section, ImGui::NavStack& nav_stack){

            static const float btn_padding = 5.0f;
            static const float btn_line_thickness = btn_padding / 2;
            static const auto btn_button_start_offset = ImVec2{ // The start offset of the rect containing the label.
                btn_padding + btn_line_thickness*2, // (1)Border and (2)selection indicator.
                btn_padding + btn_line_thickness
            };
            static auto _display_section_button = [&](ImGui::Display& section, bool selected, const ImVec2& use_region_size){
                auto draw_list = ImGui::GetWindowDrawList();

                /* Setup button style stack */ {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0,0,0,0));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0,0,0,0));
                    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, {0.0f, 0.5f});
                }

                auto cursor_pos = ImGui::GetCursorScreenPos();
                auto button_start_pos = ImVec2{
                    cursor_pos.x + btn_button_start_offset.x,
                    cursor_pos.y + btn_button_start_offset.y
                };
                auto size = ImVec2{
                    use_region_size.x,
                    use_region_size.y - btn_button_start_offset.y*2
                };
                ImGui::SetCursorScreenPos(button_start_pos);
                if(ImGui::Button(section.first.c_str(), size)){
                    selected_section = &section;
                }

                /* Remove Button style stack */{
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                }

                /* Draw the button decoration (Hovering outline and is-selected indicator) */ {
                    if(selected){
                        auto pos = ImVec2{
                            cursor_pos.x + btn_padding + btn_line_thickness,
                            cursor_pos.y + btn_padding + btn_line_thickness
                        };
                        draw_list->AddRectFilled(
                            pos,
                            {
                                pos.x + btn_line_thickness,
                                pos.y + (use_region_size.y - btn_button_start_offset.y*2)
                            },
                            ImGui::GetColorU32(ImGuiCol_Button)
                        );
                    }

                    auto button_max = ImGui::GetItemRectMax();
                    button_max.y += btn_padding + btn_line_thickness;
                    if(ImGui::IsItemHovered()){
                        draw_list->AddRect(
                            cursor_pos,
                            button_max,
                            ImGui::GetColorU32(ImGuiCol_ButtonHovered),
                            btn_line_thickness/2,
                            ImDrawCornerFlags_All,
                            btn_line_thickness
                        );
                    }
                }
            };

            auto avail_size = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {16.0f,16.0f});
            ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
            // We use child frames so that we could have some padding,
            // but we also disable the frame bg because we do not want it.
            if(!ImGui::BeginChildFrame(ImGui::GetID("Section List"), {avail_size.x * (1/3.0f), avail_size.y})){
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
                ImGui::EndChildFrame();
            } else {
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
                
                for(int i=0; i < section_count; i++){
                    auto& section = sections[i];
                    _display_section_button(
                        section,
                        selected_section == &section,
                        {-1,40} // {extend to end, 40}
                    );
                }

                ImGui::EndChildFrame();
            }

            ImGui::SameLine(0.0f, 0.0f);

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {16.0f,16.0f});
            ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
            if(!ImGui::BeginChildFrame(ImGui::GetID("Section Content"), {avail_size.x * (2/3.0f), avail_size.y})){
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
                ImGui::EndChildFrame();
            } else{
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
                
                // Align to the first section label.
                ImGui::Dummy({
                    0,
                    btn_button_start_offset.y
                });

                auto& section = *selected_section;
                section.second();
                ImGui::EndChildFrame();
            }
        }

        void show_session_page(std::pair<ConSess, ConSessData>& session, ImGui::NavStack& nav_stack){
            static ConSess* con_p{nullptr};
            static ConSessData* sess_data_p{nullptr};
            static SessionStatus status = NO_SESS;
            static auto _sess_ok = [&](){
                switch(status){
                    case SESS_OK:
                        return true;
                    default:
                    ImGui::Text("Waiting for \"SESS_OK.\"");
                        break;
                }
                return false;
            };
            static ImGui::Display sections[] = {
                {"Controller", [&](){
                    if(!_sess_ok())
                        return;
                    draw_controller(con_p->getProdID(), sess_data_p->state.saved_col);
                    if(ImGui::Button("Get Colors")){
                        con_p->getColors(sess_data_p->state.saved_col);
                    }
                }},
                {"Basic Stuff", [&](){
                    if(!_sess_ok())
                        return;
                    if(ImGui::Button("Set Led Busy")){
                        con_p->testSetLedBusy();
                    }
                    if(ImGui::Button("Send Rumble")){
                        con_p->testSendRumble();
                    }
                    showTemperature(sess_data_p->state.temp_data);
                    if(ImGui::Button("Update Temperature")){
                        con_p->getTemperature(sess_data_p->state.temp_data);
                    }
                    showBattery(sess_data_p->state.batt_data);
                    if(ImGui::Button("Update Battery")){
                        con_p->getBattery(sess_data_p->state.batt_data);
                    }
                }},
                {"HD Rumble Player", [&](){
                    if(!_sess_ok())
                        return;
                    auto& rd = sess_data_p->user.rd;
                    ImGui::ScopeDisableItems disable(sess_data_p->user.is_rumble_active);
                    showSelectRumbleFile(rd);
                    showRumbleData(rd);
                    if((rd.data != nullptr) && ImGui::Button("Play")){
                        con_p->testHDRumble(rd, sess_data_p->user.is_rumble_active);
                    }
                }},
                {"IR Camera", [&](){
                    if(!_sess_ok())
                        return;
                    auto& ir = sess_data_p->user.ir;
                    if(showIRCameraFeedControl(*ir) == IRFeedControl::Capture)
                        con_p->testIRCapture(*ir);
                    showIRCameraFeed(*ir);
                    showIRCaptureInfo(*ir);
                    showIRCaptureSettings(*ir);
                }},
                {"Modify Controller", [&](){
                    if(!_sess_ok())
                        return;
                    auto that_res = ModifyController::show_dump_spi(sess_data_p->user.spi_dumping, sess_data_p->user.spi_bytes_dumped);
                    
                    switch(that_res){
                        case ModifyController::SPIDumpAction::DoDump:
                            con_p->dumpSPI("spi_dump.bin", sess_data_p->user.spi_dumping, sess_data_p->user.spi_bytes_dumped, sess_data_p->user.spi_cancel_dump);
                        break;
                        case ModifyController::SPIDumpAction::CancelDump:
                            sess_data_p->user.spi_cancel_dump = true;
                        break;
                    }

                    if(ImGui::Button("Edit Colors")){
                        nav_stack.push({"Color Editor",
                            [&](){
                                ModifyController::color_editor_page(con_p->getProdID(), sess_data_p->user.preview_col);
                            }
                        });
                    }

                    draw_controller(con_p->getProdID(), sess_data_p->user.preview_col);
                    if(ImGui::Button("Write colors to SPI")){
                        con_p->writeColorsToSPI(sess_data_p->user.preview_col);
                    }
                }},
                /*
                {"Custom Commands", [&](){
                    if(ImGui::Button("New Command")){
                        nav_stack.push({"Command Builder",
                            ConCom::UI::page
                        });
                    }
                }}*/
                {"Session Messages", [&](){
                    if(!_sess_ok())
                        return;
                    auto avail_size = ImGui::GetContentRegionAvail();
                    ImGui::MakeSection({"Message stream",
                        [&](){
                            if(con_p->messageStream())
                                ImGui::Text("%s", con_p->messageStream()->str().c_str());
                        }
                    }, {avail_size.x, avail_size.y/2});
                    
                    ImGui::MakeSection({"Error stream",
                        [&](){
                            if(con_p->errorStream())
                                ImGui::Text("%s", con_p->errorStream()->str().c_str());
                        }
                    }, {avail_size.x, avail_size.y/2});
                }}
            };
            static ImGui::Display* selected_section{sections};
            con_p = &session.first;
            sess_data_p = &session.second;
            status = sess_data_p->state.status;
            show_as_sections_page(sections, IM_ARRAYSIZE(sections), selected_section, nav_stack);
        }

        void show_default_page_sessions(ImGui::NavStack& nav_stack){
            static std::unordered_map<Con, std::pair<ConSess, ConSessData>, ConHash, ConEqual> sessions;
            static std::vector<Con> cons_found;

            ImGui::BeginGroup();

            if(ImGui::Button("Scan for Controllers")){
                hid_device_info *devs, *curr;
                devs = hid_enumerate(ConHID::VID, 0);

                curr = devs;
                cons_found.clear();
                while(curr){
                    cons_found.push_back(Con(curr));
                    curr = curr->next;
                }

                hid_free_enumeration(devs);
            }
            
            for(auto& con: cons_found){
                ImGui::PushID(&con);

                ImTextureID icon_rid = 0;
                auto icon_size = ImVec2();
                ImageResource* icon;
                if(icon = Assets::get_con_lines(con.prod_id)){
                    icon_rid = (ImTextureID)icon->getRID();
                    icon_size.x = icon->getWidth();
                    icon_size.y = icon->getHeight();
                }
                ImGui::MakeSection({con.prod_string,
                    [&](){
                        ImGui::ImageAutoFit(icon_rid, icon_size);
                    }
                }, {100, 100});
                ImGui::PopID();

                if(ImGui::IsItemHovered()){
                    ImGui::BeginTooltip();
                    ImGui::Text("ProdID: %X", con.prod_id);
                    ImGui::Text("HID S/N: %s", con.hid_sn.c_str());
                    ImGui::Text("Man. String: %s", con.manu_string.c_str());
                    ImGui::Text("Prod. String: %s", con.prod_string.c_str());
                    ImGui::EndTooltip();
                }

                if(ImGui::IsItemClicked()){
                    // Construct from a const ref con.
                    auto sess_insert = sessions.insert(std::make_pair(con, std::make_pair(ConSess(con), ConSessData())));
                    auto& session = sess_insert.first->second;
                    session.first.startSession([&](SessionStatus s){
                        session.second.state.status = s;
                    });
                    nav_stack.push({session.first.getProdStr() + " " + session.first.getHIDSN(),
                        [&](){
                            show_session_page(session, nav_stack);
                        }
                    });
                }
            }
            
            ImGui::EndGroup();

            ImGui::SameLine();

            static const ImVec2 feed_size{160,120};
            // Show all IR Feeds
            for(auto& session: sessions){
                ImGui::MakeSection({session.first.hid_sn,
                    [&](){
                        showIRCameraFeed(*session.second.second.user.ir);
                    }
                }, feed_size);
                ImGui::SameLine();
            }
        }

        void show(const char* window_name) {
            //static Controller controller;
            //static RumbleData rumble_data;
            static ImGui::NavStack nav_stack;
            static ImGui::Display default_page{window_name,
                [](){
                    JCToolkit::UI::show_default_page_sessions(nav_stack);
                }
            };

            auto avail_size = ImGui::GetContentRegionAvail();
            auto bottom_bar_height = 32.0f; 
            auto avail_size_page = ImVec2{avail_size.x, avail_size.y - bottom_bar_height};

            if(!ImGui::BeginChild("current page", avail_size_page)){
                ImGui::EndChild();
            } else {
                if(nav_stack.size() > 0){
                    ImGui::MakeSection(nav_stack.top());
                } else {
                    ImGui::MakeSection(default_page);
                }
                ImGui::EndChild();
            }
            
            ImGui::Separator();

            ImGui::SameLine();

            if(nav_stack.size() > 0){
                auto pos = ImGui::GetCursorPos();

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0,0});
                if(ImGui::ArrowButton("back navigation", ImGuiDir_Left))
                    nav_stack.pop();
                ImGui::PopStyleVar();
                ImGui::SameLine();
                ImGui::Text("Back");
            }
        }
    }
    void init() {
        GPUTexture::SideLoader::create_context();
        TP::prepare_pool();
        Helpers::load_image_res_gpu();

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(IM_COL32(0x46,0x46,0x46,255)));
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::ColorConvertU32ToFloat4(IM_COL32(0x5D,0xFD,0xD9,155)));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::ColorConvertU32ToFloat4(IM_COL32(0x39,0x97,0xBD,155)));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, {}); // No rounding
    }

    void exit(){
        TP::join_pool();
    }
}
