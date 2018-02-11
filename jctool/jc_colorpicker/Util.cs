// Original code from
// Color Picker with Color Wheel and Eye Dropper
// by jkristia
//
// Modified by:
// CTCaer  |  Feb 2018.

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Linq;


namespace jcColor {
	static class Util
	{
		public static Rectangle Rect(RectangleF rf)
		{
			Rectangle r = new Rectangle();
			r.X = (int)rf.X;
			r.Y = (int)rf.Y;
			r.Width = (int)rf.Width;
			r.Height = (int)rf.Height;
			return r;
		}
		public static RectangleF Rect(Rectangle r)
		{
			RectangleF rf = new RectangleF();
			rf.X = (float)r.X;
			rf.Y = (float)r.Y;
			rf.Width = (float)r.Width;
			rf.Height = (float)r.Height;
			return rf;
		}
		public static Point Point(PointF pf)
		{
			return new Point((int)pf.X, (int)pf.Y);
		}
		public static PointF Center(RectangleF r)
		{
			PointF center = r.Location;
			center.X += r.Width / 2;
			center.Y += r.Height / 2;
			return center;
		}

		public static void DrawFrame(Graphics dc, RectangleF r, float cornerRadius, Color color)
		{
			Pen pen = new Pen(color);
			if (cornerRadius <= 0)
			{
				dc.DrawRectangle(pen, Rect(r));
				return;
			}
			cornerRadius = (float)Math.Min(cornerRadius, Math.Floor(r.Width) - 2);
			cornerRadius = (float)Math.Min(cornerRadius, Math.Floor(r.Height) - 2);

			GraphicsPath path = new GraphicsPath();
			path.AddArc(r.X,r.Y, cornerRadius, cornerRadius, 180, 90);
			path.AddArc(r.Right-cornerRadius,r.Y, cornerRadius, cornerRadius, 270, 90);
			path.AddArc(r.Right-cornerRadius,r.Bottom-cornerRadius, cornerRadius, cornerRadius, 0, 90);
			path.AddArc(r.X,r.Bottom-cornerRadius, cornerRadius, cornerRadius, 90, 90);
			path.CloseAllFigures();
			dc.DrawPath(pen, path);
		}
		public static void Draw2ColorBar(Graphics dc, RectangleF r, Orientation orientation, Color c1, Color c2)
		{
			RectangleF lr1 = r;
			float angle = 0;

			if (orientation == Orientation.Vertical)
				angle = 270;
			if (orientation == Orientation.Horizontal)
				angle = 0;

			if (lr1.Height > 0 && lr1.Width > 0)
			{
				LinearGradientBrush lb1 = new LinearGradientBrush(lr1, c1, c2, angle, false);
				dc.FillRectangle(lb1, lr1);
				lb1.Dispose();
			}
		}
		public static void Draw3ColorBar(Graphics dc, RectangleF r, Orientation orientation, Color c1, Color c2, Color c3)
		{
			// to draw a 3 color bar 2 gradient brushes are needed
			// one from c1 - c2 and c2 - c3
			RectangleF lr1 = r;
			RectangleF lr2 = r;
			float angle = 0;

			if (orientation == Orientation.Vertical)
			{
				angle = 270;

				lr1.Height = lr1.Height / 2;
				lr2.Height = r.Height - lr1.Height;
				lr2.Y += lr1.Height;
			}
			if (orientation == Orientation.Horizontal)
			{
				angle = 0;

				lr1.Width = lr1.Width / 2;
				lr2.Width = r.Width - lr1.Width;
				lr1.X = lr2.Right;
			}

			if (lr1.Height > 0 && lr1.Width > 0)
			{
				LinearGradientBrush lb2 = new LinearGradientBrush(lr2, c1, c2, angle, false);
				LinearGradientBrush lb1 = new LinearGradientBrush(lr1, c2, c3, angle, false);

				dc.FillRectangle(lb1, lr1);
				dc.FillRectangle(lb2, lr2);

				lb1.Dispose();
				lb2.Dispose();
			}
			// with some sizes the first pixel in the gradient rectangle shows the opposite color
			// this is a workaround for that problem
			if (orientation == Orientation.Vertical)
			{
				Pen pc2 = new Pen(c2, 1);
				Pen pc3 = new Pen(c3, 1);
				dc.DrawLine(pc3, lr1.Left, lr1.Top, lr1.Right - 1, lr1.Top);
				dc.DrawLine(pc2, lr2.Left, lr2.Top, lr2.Right - 1, lr2.Top);
				pc2.Dispose();
				pc3.Dispose();
			}
			
			if (orientation == Orientation.Horizontal)
			{
				Pen pc1 = new Pen(c1, 1);
				Pen pc2 = new Pen(c2, 1);
				Pen pc3 = new Pen(c3, 1);
				dc.DrawLine(pc1, lr2.Left, lr2.Top, lr2.Left, lr2.Bottom-1);
				dc.DrawLine(pc2, lr2.Right, lr2.Top, lr2.Right, lr2.Bottom-1);
				dc.DrawLine(pc3, lr1.Right, lr1.Top, lr1.Right, lr1.Bottom-1);
				pc1.Dispose();
				pc2.Dispose();
				pc3.Dispose();
			}
		}

        public static List<Control> controlsSorted(this Control panel) {
            var controls = panel.Controls.OfType<Control>().ToList();
            controls.Sort((c1, c2) => c1.TabIndex.CompareTo(c2.TabIndex));
            return controls;
        }
    }

	class WinUtil
	{
		public const int WM_KEYDOWN = 0x0100;
		public const int WM_KEYUP   = 0x0101;
		public const int WM_CHAR    = 0x0102;

		public const int SWP_NOSIZE = 0x0001;
		public const int SWP_NOMOVE = 0x0002;
		public const int SWP_NOZORDER = 0x0004;
		public const int SWP_NOREDRAW = 0x0008;
		public const int SWP_NOACTIVATE = 0x0010;
		public const int SWP_FRAMECHANGED = 0x0020;  /* The frame changed: send WM_NCCALCSIZE */
		public const int SWP_SHOWWINDOW = 0x0040;
		public const int SWP_HIDEWINDOW = 0x0080;
		public const int SWP_NOCOPYBITS = 0x0100;
		public const int SWP_NOOWNERZORDER = 0x0200;  /* Don't do owner Z ordering */
		public const int SWP_NOSENDCHANGING = 0x0400;  /* Don't send WM_WINDOWPOSCHANGING */

		[DllImport("user32.dll", CharSet = CharSet.Auto, ExactSpelling = true)]
		public static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter, int x, int y, int cx, int cy, int flags);

		public const uint WS_OVERLAPPED   =    WS_BORDER | WS_CAPTION;
		public const uint WS_CLIPSIBLINGS =    0x04000000;
		public const uint WS_CLIPCHILDREN =    0x02000000;
		public const uint WS_CAPTION      =    0x00C00000;     /* WS_BORDER | WS_DLGFRAME  */
		public const uint WS_BORDER       =    0x00800000;
		public const uint WS_DLGFRAME     =    0x00400000;
		public const uint WS_VSCROLL      =    0x00200000;
		public const uint WS_HSCROLL      =    0x00100000;
		public const uint WS_SYSMENU      =    0x00080000;
		public const uint WS_THICKFRAME   =	   0x00040000;
		public const uint WS_MAXIMIZEBOX  =	   0x00020000;
		public const uint WS_MINIMIZEBOX  =	   0x00010000;
		public const uint WS_SIZEBOX      =    WS_THICKFRAME;
		public const uint WS_POPUP        =    0x80000000;
		public const uint WS_CHILD        =    0x40000000;
		public const uint WS_VISIBLE      =    0x10000000;
		public const uint WS_DISABLED     =    0x08000000;

		public const uint WS_EX_DLGMODALFRAME=	0x00000001;
		public const uint WS_EX_TOPMOST      =	0x00000008;
		public const uint WS_EX_TOOLWINDOW   =	0x00000080;
		public const uint WS_EX_WINDOWEDGE   =	0x00000100;
		public const uint WS_EX_CLIENTEDGE   =	0x00000200;

		public const uint WS_EX_CONTEXTHELP	 =	0x00000400;
		public const uint WS_EX_STATICEDGE   =	0x00020000;
		public const uint WS_EX_OVERLAPPEDWINDOW  =(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE);

		public const int GWL_STYLE      =     (-16);
		public const int GWL_EXSTYLE    =     (-20);

		[DllImport("user32.dll", EntryPoint="GetWindowLong", CharSet=CharSet.Auto)]
		public static extern IntPtr GetWindowLong32(IntPtr hWnd, int nIndex);
 
		[DllImport("user32.dll", EntryPoint="SetWindowLong", CharSet=CharSet.Auto)]
		public static extern IntPtr SetWindowLongPtr32(IntPtr hWnd, int nIndex, int dwNewLong);


		public delegate int HookProc(int nCode, IntPtr wParam, IntPtr lParam);

		public const int WH_KEYBOARD	= 2;
		public const int WH_MOUSE		= 7;
		public const int WH_KEYBOARD_LL = 13;
		public const int WH_MOUSE_LL    = 14;

		/// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/windowing/hooks/hookreference/hookfunctions/setwindowshookex.asp
		[DllImport("user32.dll", CharSet = CharSet.Auto,CallingConvention = CallingConvention.StdCall)]
		public static extern int SetWindowsHookEx(int idHook, HookProc lpfn, IntPtr hInstance, int threadId);

		/// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/windowing/hooks/hookreference/hookfunctions/setwindowshookex.asp
		[DllImport("user32.dll", CharSet = CharSet.Auto,CallingConvention = CallingConvention.StdCall)]
		public static extern bool UnhookWindowsHookEx(int idHook);

		/// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/windowing/hooks/hookreference/hookfunctions/setwindowshookex.asp
		[DllImport("user32.dll", CharSet = CharSet.Auto,CallingConvention = CallingConvention.StdCall)]
		public static extern int CallNextHookEx(int idHook, int nCode, IntPtr wParam, IntPtr lParam);

		//Declare the wrapper managed POINT class.
		[StructLayout(LayoutKind.Sequential)]
		public class POINT
		{
			public int x;
			public int y;
		}

		//Declare the wrapper managed MouseHookStruct class.
		[StructLayout(LayoutKind.Sequential)]
		public class MouseHookStruct
		{
			public POINT pt;
			public int hwnd;
			public int wHitTestCode;
			public int dwExtraInfo;
		}

		//Declare the wrapper managed KeyboardHookStruct class.
        [StructLayout(LayoutKind.Sequential)]
        public class KeyboardHookStruct
        {
            public int vkCode;
            public int scanCode;
            public int flags;
            public int time;
            public int dwExtraInfo;
        }
	}


	/// http://support.microsoft.com/kb/318804
	/// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/windowing/hooks/hookreference/hookfunctions/setwindowshookex.asp
    /// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/windowing/hooks/hookreference/hookstructures/cwpstruct.asp
	/// http://www.codeproject.com/KB/cs/globalhook.aspx
	public class Hook
	{
		public delegate void KeyboardDelegate(KeyEventArgs e);
		public KeyboardDelegate OnKeyDown;
		int					m_hHook = 0;
		WinUtil.HookProc	m_HookCallback;

		public void SetHook(bool enable)
		{
			if (enable && m_hHook == 0)
			{
				m_HookCallback = new WinUtil.HookProc(HookCallbackProc);
                Module module = Assembly.GetExecutingAssembly().GetModules()[0];
				m_hHook = WinUtil.SetWindowsHookEx(WinUtil.WH_KEYBOARD_LL, m_HookCallback, Marshal.GetHINSTANCE(module),0);
				if (m_hHook == 0)
				{
					MessageBox.Show("SetHook Failed. Please make sure the 'Visual Studio Host Process' on the debug setting page is disabled");
					return;
				}
				return;
			}

			if (enable == false && m_hHook != 0)
			{
				WinUtil.UnhookWindowsHookEx(m_hHook);
				m_hHook = 0;
			}
		}
		int HookCallbackProc(int nCode, IntPtr wParam, IntPtr lParam)
		{
			if (nCode < 0)
			{
				return WinUtil.CallNextHookEx(m_hHook, nCode, wParam, lParam);
			}
			else
			{
				//Marshall the data from the callback.
				WinUtil.KeyboardHookStruct hookstruct = (WinUtil.KeyboardHookStruct)Marshal.PtrToStructure(lParam, typeof(WinUtil.KeyboardHookStruct));
				
				if (OnKeyDown != null && wParam.ToInt32() == WinUtil.WM_KEYDOWN)
				{
					Keys key = (Keys)hookstruct.vkCode;
					if ((Control.ModifierKeys & Keys.Shift) == Keys.Shift)
						key |= Keys.Shift;
					if ((Control.ModifierKeys & Keys.Control) == Keys.Control)
						key |= Keys.Control;

					KeyEventArgs e = new KeyEventArgs(key);
					e.Handled = false;
					OnKeyDown(e);
					if (e.Handled)
						return 1;
				}
				int result = 0;
				if (m_hHook != 0)
					result = WinUtil.CallNextHookEx(m_hHook, nCode, wParam, lParam);
				return result;
			}
		}
	}
}
