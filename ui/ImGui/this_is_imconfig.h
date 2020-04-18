#pragma once

namespace ImGui {
    void DisableItems();
    void EnableItems();

    class ScopedDisableItems {
        bool disabled;
    public:
        inline ScopedDisableItems(bool disable_if_true) {
            if(disable_if_true) {
                DisableItems();
                disabled = true;
            } else 
                disabled = false;
        }
        inline ~ScopedDisableItems() {
            if(disabled)
                EnableItems();
        }
        inline void allowEnable() {
            if(disabled)
                EnableItems();
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
