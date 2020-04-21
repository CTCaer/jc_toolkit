#pragma once
#include <string>

enum ChildAction {
    ChildAction_NoneSelected,
    ChildAction_None,
    ChildAction_OpenFile
};

namespace ImGui {
    namespace DirectoryExplorer {
        unsigned int NewDirExplorer(const std::string& context_name, const std::string& start_path);
        bool Begin(unsigned int dir_explorer_id);
        void ShowHistoryButtons();
        void ListDirectory();
        void ShowPathBar(float width = -1.0f);
        ChildAction SelectedChildShow(std::string& fill_in);
        void End();
        bool OpenFileDialog(uint32_t dir_explorer_ctx, std::string& selected_file_name, bool& show);
    }
}
