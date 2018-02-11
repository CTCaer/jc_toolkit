using System;
using System.Windows.Forms;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Drawing2D;

internal class resfinder
{
    // Trick from Bob Powell
}

namespace jcColor.Buttons
{
    [Description("Round (Elliptical) Button Control")]
    public class RoundButton : System.Windows.Forms.Button
	{
		#region Public Properties
		public override string Text
		{
			get
			{
				return buttonText;
			}
			set
			{
				buttonText = value;
                this.Invalidate();
			}
		}

        public bool checkedFocus;
        [Bindable(true), Category("Button Appearance"),
        DefaultValue(false),
        Description("Specifies if the button is checked.")]
        public bool CheckedFocus {
            get {
                return checkedFocus;
            }
            set {
                    checkedFocus = value;
                this.Invalidate();
            }
        }

        public int buttonPressOffset;
        [Bindable(true), Category("Button Appearance"),
        DefaultValue(0),
        Description("Specifies if the button is pressed.")]
        public int ButtonPressOffset {
            get {
                return buttonPressOffset;
            }
            set {
                buttonPressOffset = value;
                this.Invalidate();
            }
        }

        #endregion Public Properties

        /// <summary>
        /// Windows form control derived from Button, to create round and elliptical buttons
        /// </summary>
        public RoundButton()
		{
			this.Name = "RoundButton";
			this.Size = new System.Drawing.Size(50, 50);		// Default size of control
			
            this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.mouseDown);
			this.MouseUp += new System.Windows.Forms.MouseEventHandler(this.mouseUp);
			
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.keyDown);
			this.KeyUp += new System.Windows.Forms.KeyEventHandler(this.keyUp);
        }

        #region Private properties

        private Color buttonColor;
        private String buttonText;

        private Color edgeColor1;                                       // Change if button pressed
        private Color edgeColor2;                                       //
        private int edgeWidth;                                          // Width of button edge    
        private Color cColor = Color.White;                             // Centre colour for Path Gradient brushed

        private GraphicsPath bpath;
        private GraphicsPath gpath;

        #endregion 

		#region Base draw functions

        protected override void OnPaint(PaintEventArgs e)
		{
            buttonColor = this.BackColor;
            edgeColor1 = ControlPaint.Light(buttonColor);
            edgeColor2 = ControlPaint.Dark(buttonColor);

			Graphics g = e.Graphics;
			g.SmoothingMode = SmoothingMode.AntiAlias;

            Rectangle buttonRect = this.ClientRectangle;
			edgeWidth = GetEdgeWidth(buttonRect);
			
            FillBackground(g, buttonRect);

			ShrinkShape(ref g, ref buttonRect, edgeWidth);

			DrawButton(g, buttonRect);
			 
 			SetClickableRegion();						

		}
	
        /// <summary>
        /// Fill in the control background
        /// </summary>
        /// <param name="g">Graphics object</param>
        /// <param name="rect">Rectangle defining the button background</param>
		protected void FillBackground(Graphics g, Rectangle rect)
		{
			// Fill in the control with the background colour, to overwrite anything
			// that may have been drawn already.
			Rectangle bgRect = rect;
			bgRect.Inflate(1,1);
			SolidBrush bgBrush = new SolidBrush(Color.FromKnownColor(KnownColor.Control));
			bgBrush.Color = Parent.BackColor;
			g.FillRectangle(bgBrush, bgRect);
			bgBrush.Dispose();

		}

        /// <summary>
        /// Fill in the main button colour
        /// </summary>
        /// <param name="g">Graphics Object</param>
        /// <param name="buttonRect">Rectangle defining the button top</param>
		protected virtual void DrawButton(Graphics g, Rectangle buttonRect)
		{
			BuildGraphicsPath(buttonRect);
						
			PathGradientBrush pgb = new PathGradientBrush(bpath);
			pgb.SurroundColors = new Color[] {buttonColor};
			
			buttonRect.Offset(buttonPressOffset, buttonPressOffset);
            
			pgb.CenterColor = buttonColor;
			
			FillShape(g, pgb, buttonRect);
			
			// If we have focus, draw line around control to indicate this.
			if (CheckedFocus)
			{
				DrawFocus(g, buttonRect);
			}
		}
				
		#endregion Base draw functions
		
		#region Overrideable shape-specific methods
		
		protected virtual void BuildGraphicsPath(Rectangle buttonRect)
		{
			bpath = new GraphicsPath();
			// Adding this second smaller rectangle to the graphics path smooths the edges - don't know why...?
			Rectangle rect2 = new Rectangle(buttonRect.X - 1, buttonRect.Y - 1, buttonRect.Width + 2, buttonRect.Height + 2);
			AddShape(bpath, rect2);
			AddShape(bpath, buttonRect);
		}
		
        /// <summary>
        /// Create region the same shape as the button. This will respond to the mouse click
        /// </summary>
		protected virtual void SetClickableRegion()
		{
			gpath = new GraphicsPath();
			gpath.AddEllipse(this.ClientRectangle);
			this.Region = new Region(gpath);			// Click only activates on elipse
		}
		
        /// <summary>
        /// Method to fill the specified rectangle with either LinearGradient or PathGradient
        /// </summary>
        /// <param name="g">Graphics Object</param>
        /// <param name="brush">Either a PathGradient- or LinearGradientBrush</param>
        /// <param name="rect">The rectangle bounding the ellipse to be filled</param>
		protected virtual void FillShape(Graphics g, Object brush, Rectangle rect)
		{
			if (brush.GetType().ToString() == "System.Drawing.Drawing2D.LinearGradientBrush")
			{
				g.FillEllipse((LinearGradientBrush)brush, rect);	
			}
			else if (brush.GetType().ToString() == "System.Drawing.Drawing2D.PathGradientBrush")
			{
				g.FillEllipse((PathGradientBrush)brush, rect);	
			}
		}
		
		/// <summary>
		/// Add an ellipse to the Graphics Path
		/// </summary>
		/// <param name="gpath">The Graphics Path</param>
        /// <param name="rect">The rectangle defining the ellipse to be added</param>
        protected virtual void AddShape(GraphicsPath gpath, Rectangle rect)
		{
			gpath.AddEllipse(rect);	
		}
		
        /// <summary>
        /// Draw the specified shape
        /// </summary>
        /// <param name="g">Graphics Object</param>
        /// <param name="pen">Pen</param>
        /// <param name="rect">The rectangle bounding the ellipse to be drawn</param>
		protected virtual void DrawShape(Graphics g, Pen pen, Rectangle rect)
		{
			g.DrawEllipse(pen, rect);	
		}
		
        /// <summary>
        /// Shrink the shape 
        /// </summary>
        /// <param name="g">Graphics Object</param>
        /// <param name="rect">The rectangle defining the shape to be shrunk</param>
        /// <param name="amount">The amount to shrink it by</param>
        protected virtual void ShrinkShape(ref Graphics g, ref Rectangle rect, int amount)
		{
            rect.Inflate(-amount, -amount);	
		}
		
        /// <summary>
        /// Indicate that this button has focus
        /// </summary>
        /// <param name="g">Graphics Object</param>
        /// <param name="rect">The rectangle defining the button face</param>
		protected virtual void DrawFocus(Graphics g, Rectangle rect)
		{
            rect.Inflate(-3, -3);
            Pen fPen = new Pen(Color.FromArgb(150, 255, 255, 255));
			fPen.DashStyle = DashStyle.Solid;
            fPen.Width = 2;
            DrawShape(g, fPen, rect);
            rect.Inflate(2, 2);
            fPen = new Pen(Color.FromArgb(150, 0, 0, 0));
            fPen.DashStyle = DashStyle.Solid;
            fPen.Width = 3;
            DrawShape(g, fPen, rect);
        }
		
		#endregion Overrideable shape-specific methods
		
		#region Help functions
		
        /// <summary>
		/// Calculate button edge width depending on button size
		/// </summary>
		/// <param name="rect">Rectangle defining the button</param>
		/// <returns>The width of the edge</returns>
		protected int GetEdgeWidth(Rectangle rect)
		{
			if (rect.Width < 50 | rect.Height < 50) return 1;
			else return 2;
		}

		#endregion Private Help functions
		
		#region Event Handlers
				
		protected void mouseDown(object sender, System.Windows.Forms.MouseEventArgs e) 
		{
			buttonDown();
		}
		
		protected void mouseUp(object sender, System.Windows.Forms.MouseEventArgs e) 
		{
    		buttonUp();
		}

		protected void buttonDown() 
		{
			buttonPressOffset = 1;
            this.Invalidate();
		}
		
		protected void buttonUp() 
		{
			buttonPressOffset = 0;
			this.Invalidate();
		}
		
		protected void keyDown(object sender, System.Windows.Forms.KeyEventArgs e) 
		{
			if (e.KeyCode.ToString() == "Space")
			{
				buttonDown();
			}
		}
		
		protected void keyUp(object sender, System.Windows.Forms.KeyEventArgs e) 
		{
			if (e.KeyCode.ToString() == "Space")
			{
				buttonUp();
			}
		}


		#endregion Event Handlers			
	}
}


