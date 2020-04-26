#pragma once
#include <string>
#include "imgui.h"

enum ChildAction {
    ChildAction_None,
    ChildAction_OpenFile
};

namespace ImGui {
    namespace DirectoryExplorer {
        ImGuiID NewDirExplorer(const std::string& context_name, const std::string& start_path);
        bool Begin(ImGuiID dir_explorer_id);
        void ShowHistoryButtons();
        void ListDirectory(const char * filter_list = "");
        void ShowPathBar(float width = -1.0f);
        ChildAction SelectedChildShow(std::string& fill_in);
        void End();
        bool OpenFileDialog(ImGuiID dir_explorer_ctx, std::string& selected_file_name, bool& show, const char* filter_list = "", const ImVec2& init_size = {300.0f,200.0f});
    }
}
