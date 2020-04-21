#pragma once

#include <string>
#include <functional>

namespace ImGui {
    using Display = std::function<void()>;

    void DisableItems();
    void EnableItems();

    void MakeSection(const std::string headername, Display display_section, bool* collapsable = nullptr, int collapse_flags = 0);

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
}
