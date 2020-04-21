#include "imgui.h"
#include "imgui_internal.h"
#include "this_is_imconfig.h"

namespace ImGui {
    /**
     * Credit: Ocornut
     * https://github.com/ocornut/imgui/issues/211#issuecomment-339241929
     */
    void DisableItems() {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }

    /**
     * Credit: Ocornut
     * https://github.com/ocornut/imgui/issues/211#issuecomment-339241929
     */
    void EnableItems() {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }

    void MakeSection(const std::string headername, Display display_section, bool* collapsable, int collapse_flags){
        if((!collapsable) |
            (collapsable && ImGui::CollapsingHeader(headername.c_str(), collapsable, collapse_flags))
        ){
            if(!collapsable)
                ImGui::Text("%s", headername.c_str());
            std::invoke(display_section);
        }
    }
}
