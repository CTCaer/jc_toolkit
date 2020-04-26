/**
 * 2020 Jonathan Mendez
 */
#pragma once
#include <functional>
#include <utility>
#include <string>
#include <stack>

#include "imgui.h"

/**
 * Return a maximum rect size for a rect_a that would fit in rect_b.
 */
template<typename RectA, typename RectB>
inline RectA resizeRectAToFitInRectB(const RectA& rect_a, const RectB& rect_b){
    RectB ratios{ rect_b.x / rect_a.x, rect_b.y / rect_a.y};

    float scale = -1.0;

    bool x_fit = true;
    if(ratios.y * rect_a.x > rect_b.x)
        x_fit = false;
    
    bool y_fit = true;
    if(ratios.x * rect_a.y > rect_b.y)
        y_fit = false;
    
    if(x_fit && y_fit)
        scale = 1.0;
    else if(x_fit && !y_fit)
        scale = ratios.y;
    else if(!x_fit && y_fit)
        scale = ratios.x;
    else
        ;//std::cerr << "No x_fit and no y_fit" << std::endl;

    return {
        static_cast<decltype(rect_a.x)>(scale*rect_a.x),
        static_cast<decltype(rect_a.y)>(scale*rect_a.y)
    };
}

/**
 * Return the position of the top-left corner of where rect_a should be.
 */
template<typename RectA, typename RectB>
inline RectB centerRectAInRectB(const RectA& rect_a, const RectB& rect_b){
    RectB center{
        rect_b.x / 2.0f,
        rect_b.y /2.0f
    };
    return {
        static_cast<decltype(rect_b.x)>(center.x - rect_a.x / 2.0f),static_cast<decltype(rect_b.y)>(center.y - rect_a.y / 2.0f)
    };
}

namespace ImGui {
    using Display = std::pair<std::string, std::function<void()>>;

    using NavStack = std::stack<Display>;

    void DisableItems();
    void EnableItems();

    void MakeSection(Display display_section, bool* collapsable = nullptr, int collapse_flags = 0);

    class ScopeDisableItems {
        bool disabled;
    public:
        inline ScopeDisableItems(bool disable_if_true) {
            if(disable_if_true) {
                DisableItems();
                disabled = true;
            } else 
                disabled = false;
        }
        inline ~ScopeDisableItems() {
            if(disabled)
                EnableItems();
        }
        inline void allowEnabled() {
            if(disabled) {
                EnableItems();
                disabled = false;
            }
        }
        /**
         * Make sure the items are disabled if enabled.
         */
        inline void ensureDisabled() {
            if(!disabled) {
                DisableItems();
                disabled = true;
            }
        }
    };

    void ImageAutoFit(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0,0), const ImVec2& uv1 = ImVec2(1,1), const ImVec4& tint_col = ImVec4(1,1,1,1), const ImVec4& border_col = ImVec4(0,0,0,0));
}
