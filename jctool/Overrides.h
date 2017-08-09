#pragma once
namespace Overrides {
	public ref class TestColorTable : ProfessionalColorTable
	{
		public: property Color ToolStripDropDownBackground
		{
			virtual Color get() override
			{
				return Color::FromArgb(85, 85, 85);
			}
		}
		public: property Color MenuItemSelected
		{
			virtual Color get() override
			{
				return Color::FromArgb(105, 105, 105);
			}
		}
		public: property Color MenuBorder
		{
			virtual Color get() override
			{
				return Color::FromArgb(85, 85, 85);
			}
		}
		public: property Color MenuItemBorder
		{
			virtual Color get() override
			{
				return Color::FromArgb(105, 105, 105);
			}
		}

		public: property Color MenuItemPressedGradientBegin
		{
			virtual Color get() override
			{
				return Color::FromArgb(125, 125, 125);
			}
		}

		public: property Color MenuItemPressedGradientEnd
		{
			virtual Color get() override
			{
				return Color::FromArgb(125, 125, 125);
			}
		}
		public: property Color ImageMarginGradientBegin
		{
			virtual Color get() override
			{
				return Color::FromArgb(75, 75, 75);
			}
		}

		public: property Color ImageMarginGradientEnd
		{
			virtual Color get() override
			{
				return Color::FromArgb(75, 75, 75);
			}
		}

		public: property Color MenuItemSelectedGradientBegin
		{
			virtual Color get() override
			{
				return Color::FromArgb(105, 105, 105);
			}
		}
		public: property Color MenuItemSelectedGradientEnd
		{
			virtual Color get() override
			{
				return Color::FromArgb(105, 105, 105);
			}
		}
			
};


}