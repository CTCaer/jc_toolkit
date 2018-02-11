/******************************************************************/
/*****                                                        *****/
/*****     Project:           Joy-Con Color Picker            *****/
/*****     Filename:          AdobeColors.cs                  *****/
/*****     Original Project:  Adobe Color Picker Clone 1      *****/
/*****     Original Author:   Danny Blanchard                 *****/
/*****                        - scrabcakes@gmail.com          *****/
/*****     Updates:	                                          *****/
/*****      3/28/2005 - Initial Version : Danny Blanchard     *****/
/*****       Feb 2018 - JoyCon Version  : CTCaer              *****/
/*****                                                        *****/
/******************************************************************/

/******************************************************************/
/*****                                                        *****/
/*****     This version is heavily optimised for use in       *****/
/*****     CTCaer's Joy-Con Toolkit and other projects.       *****/
/*****                                                        *****/
/******************************************************************/

using System.Drawing;

namespace jcColor {
	/// <summary>
	/// Summary description for AdobeColors.
	/// </summary>
	public class AdobeColors
	{
		#region Constructors / Destructors

		public AdobeColors() 
		{ 
		} 


		#endregion

		#region Public Methods

		/// <summary> 
		/// Sets the absolute brightness of a colour 
		/// </summary> 
		/// <param name="c">Original colour</param> 
		/// <param name="brightness">The luminance level to impose</param> 
		/// <returns>an adjusted colour</returns> 
		public static  Color SetBrightness(Color c, double brightness) 
		{ 
			HSL hsl = RGB_to_HSL(c); 
			hsl.L=brightness; 
			return HSL_to_RGB(hsl); 
		} 


		/// <summary> 
		/// Modifies an existing brightness level 
		/// </summary> 
		/// <remarks> 
		/// To reduce brightness use a number smaller than 1. To increase brightness use a number larger tnan 1 
		/// </remarks> 
		/// <param name="c">The original colour</param> 
		/// <param name="brightness">The luminance delta</param> 
		/// <returns>An adjusted colour</returns> 
		public static  Color ModifyBrightness(Color c, double brightness) 
		{ 
			HSL hsl = RGB_to_HSL(c); 
			hsl.L*=brightness; 
			return HSL_to_RGB(hsl); 
		} 


		/// <summary> 
		/// Sets the absolute saturation level 
		/// </summary> 
		/// <remarks>Accepted values 0-1</remarks> 
		/// <param name="c">An original colour</param> 
		/// <param name="Saturation">The saturation value to impose</param> 
		/// <returns>An adjusted colour</returns> 
		public static  Color SetSaturation(Color c, double Saturation) 
		{ 
			HSL hsl = RGB_to_HSL(c); 
			hsl.S=Saturation; 
			return HSL_to_RGB(hsl); 
		} 


		/// <summary> 
		/// Modifies an existing Saturation level 
		/// </summary> 
		/// <remarks> 
		/// To reduce Saturation use a number smaller than 1. To increase Saturation use a number larger tnan 1 
		/// </remarks> 
		/// <param name="c">The original colour</param> 
		/// <param name="Saturation">The saturation delta</param> 
		/// <returns>An adjusted colour</returns> 
		public static  Color ModifySaturation(Color c, double Saturation) 
		{ 
			HSL hsl = RGB_to_HSL(c); 
			hsl.S*=Saturation; 
			return HSL_to_RGB(hsl); 
		} 


		/// <summary> 
		/// Sets the absolute Hue level 
		/// </summary> 
		/// <remarks>Accepted values 0-1</remarks> 
		/// <param name="c">An original colour</param> 
		/// <param name="Hue">The Hue value to impose</param> 
		/// <returns>An adjusted colour</returns> 
		public static  Color SetHue(Color c, double Hue) 
		{ 
			HSL hsl = RGB_to_HSL(c); 
			hsl.H=Hue; 
			return HSL_to_RGB(hsl); 
		} 


		/// <summary> 
		/// Modifies an existing Hue level 
		/// </summary> 
		/// <remarks> 
		/// To reduce Hue use a number smaller than 1. To increase Hue use a number larger tnan 1 
		/// </remarks> 
		/// <param name="c">The original colour</param> 
		/// <param name="Hue">The Hue delta</param> 
		/// <returns>An adjusted colour</returns> 
		public static  Color ModifyHue(Color c, double Hue) 
		{ 
			HSL hsl = RGB_to_HSL(c); 
			hsl.H*=Hue; 
			return HSL_to_RGB(hsl); 
		} 


		/// <summary> 
		/// Converts a colour from HSL to RGB 
		/// </summary> 
		/// <remarks>Adapted from the algoritm in Foley and Van-Dam</remarks> 
		/// <param name="hsl">The HSL value</param> 
		/// <returns>A Color structure containing the equivalent RGB values</returns> 
		public static Color HSL_to_RGB(HSL hsl) 
		{ 
			int Max, Mid, Min;
			double q;

			Max = Round(hsl.L * 255);
			Min = Round((1.0 - hsl.S)*(hsl.L/1.0)*255);
			q   = (double)(Max - Min)/255;

			if ( hsl.H >= 0 && hsl.H <= (double)1/6 )
			{
				Mid = Round(((hsl.H - 0) * q) * 1530 + Min);
				return Color.FromArgb(Max,Mid,Min);
			}
			else if ( hsl.H <= (double)1/3 )
			{
				Mid = Round(-((hsl.H - (double)1/6) * q) * 1530 + Max);
				return Color.FromArgb(Mid,Max,Min);
			}
			else if ( hsl.H <= 0.5 )
			{
				Mid = Round(((hsl.H - (double)1/3) * q) * 1530 + Min);
				return Color.FromArgb(Min,Max,Mid);
			}
			else if ( hsl.H <= (double)2/3 )
			{
				Mid = Round(-((hsl.H - 0.5) * q) * 1530 + Max);
				return Color.FromArgb(Min,Mid,Max);
			}
			else if ( hsl.H <= (double)5/6 )
			{
				Mid = Round(((hsl.H - (double)2/3) * q) * 1530 + Min);
				return Color.FromArgb(Mid,Min,Max);
			}
			else if ( hsl.H <= 1.0 )
			{
				Mid = Round(-((hsl.H - (double)5/6) * q) * 1530 + Max);
				return Color.FromArgb(Max,Min,Mid);
			}
			else	return Color.FromArgb(0,0,0);
		} 
  

		/// <summary> 
		/// Converts RGB to HSL 
		/// </summary> 
		/// <remarks>Takes advantage of whats already built in to .NET by using the Color.GetHue, Color.GetSaturation and Color.GetBrightness methods</remarks> 
		/// <param name="c">A Color to convert</param> 
		/// <returns>An HSL value</returns> 
		public static HSL RGB_to_HSL (Color c) 
		{ 
			HSL hsl =  new HSL(); 
          
			int Max, Min, Diff, Sum;

			//	Of our RGB values, assign the highest value to Max, and the Smallest to Min
			if ( c.R > c.G )	{ Max = c.R; Min = c.G; }
			else				{ Max = c.G; Min = c.R; }
			if ( c.B > Max )	  Max = c.B;
			else if ( c.B < Min ) Min = c.B;

			Diff = Max - Min;
			Sum = Max + Min;

			//	Luminance - a.k.a. Brightness - Adobe photoshop uses the logic that the
			//	site VBspeed regards (regarded) as too primitive = superior decides the 
			//	level of brightness.
			hsl.L = (double)Max/255;

			//	Saturation
			if ( Max == 0 ) hsl.S = 0;	//	Protecting from the impossible operation of division by zero.
			else hsl.S = (double)Diff/Max;	//	The logic of Adobe Photoshops is this simple.

			//	Hue		R is situated at the angel of 360 eller noll degrees; 
			//			G vid 120 degrees
			//			B vid 240 degrees
			double q;
			if ( Diff == 0 ) q = 0; // Protecting from the impossible operation of division by zero.
			else q = (double)60/Diff;
			
			if ( Max == c.R )
			{
					if ( c.G < c.B )	hsl.H = (double)(360 + q * (c.G - c.B))/360;
				else				hsl.H = (double)(q * (c.G - c.B))/360;
			}
			else if ( Max == c.G )	hsl.H = (double)(120 + q * (c.B - c.R))/360;
			else if ( Max == c.B )	hsl.H = (double)(240 + q * (c.R - c.G))/360;
			else					hsl.H = 0.0;

			return hsl; 
		}

		/// <summary>
		/// Custom rounding function.
		/// </summary>
		/// <param name="val">Value to round</param>
		/// <returns>Rounded value</returns>
		private static int Round(double val)
		{
			int ret_val = (int)val;
			
			int temp = (int)(val * 100);

			if ( (temp % 100) >= 50 )
				ret_val += 1;

			return ret_val;
		}


		#endregion

		#region Public Classes

		public class HSL 
		{ 
			#region Class Variables

			public HSL() 
			{ 
				_h=0; 
				_s=0; 
				_l=0; 
			} 

			double _h; 
			double _s; 
			double _l; 

			#endregion

			#region Public Methods

			public double H 
			{ 
				get{return _h;} 
				set 
				{ 
					_h=value; 
					_h=_h>1 ? 1 : _h<0 ? 0 : _h; 
				} 
			} 


			public double S 
			{ 
				get{return _s;} 
				set 
				{ 
					_s=value; 
					_s=_s>1 ? 1 : _s<0 ? 0 : _s; 
				} 
			} 


			public double L 
			{ 
				get{return _l;} 
				set 
				{ 
					_l=value; 
					_l=_l>1 ? 1 : _l<0 ? 0 : _l; 
				} 
			} 


			#endregion
		} 

		#endregion
	}
}
