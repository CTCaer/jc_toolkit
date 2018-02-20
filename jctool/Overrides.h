#pragma once
namespace Overrides {
    using namespace System::Windows::Forms;
    using namespace System::Drawing;

    public ref class TestColorTable : ProfessionalColorTable {
        public: property Color ToolStripDropDownBackground
        {
            virtual Color get() override {
                return Color::FromArgb(55, 55, 55);
            }
        }

        public: property Color MenuItemSelected
        {
            virtual Color get() override {
                return Color::FromArgb(85, 85, 85);
            }
        }

        public: property Color MenuBorder
        {
            virtual Color get() override {
                return Color::FromArgb(55, 55, 55);
            }
        }

        public: property Color MenuItemBorder
        {
            virtual Color get() override {
                return Color::FromArgb(70, 70, 70);
            }
        }

        public: property Color MenuItemPressedGradientBegin
        {
            virtual Color get() override {
                return Color::FromArgb(85, 85, 85);
            }
        }

        public: property Color MenuItemPressedGradientEnd
        {
            virtual Color get() override {
                return Color::FromArgb(85, 85, 85);
            }
        }

        public: property Color ImageMarginGradientBegin
        {
            virtual Color get() override {
                return Color::FromArgb(55, 55, 55);
            }
        }

        public: property Color ImageMarginGradientEnd
        {
            virtual Color get() override {
                return Color::FromArgb(55, 55, 55);
            }
        }

        public: property Color MenuItemSelectedGradientBegin
        {
            virtual Color get() override {
                return Color::FromArgb(85, 85, 85);
            }
        }

        public: property Color MenuItemSelectedGradientEnd
        {
            virtual Color get() override {
                return Color::FromArgb(85, 85, 85);
            }
        }

    };

    public ref class OverrideTSSR : ToolStripSystemRenderer {
        public: OverrideTSSR() {}

        protected: virtual void OnRenderToolStripBorder(ToolStripRenderEventArgs^ e) override {
            // Do nothing
        }
    };

}