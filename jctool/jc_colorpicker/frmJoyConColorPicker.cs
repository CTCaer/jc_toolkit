/******************************************************************/
/*****                                                        *****/
/*****     Project:           Joy-Con Color Picker            *****/
/*****     Filename:          frmJoyConColorPicker.cs         *****/
/*****     Original Project:  Adobe Color Picker Clone 1      *****/
/*****     Original File:     frmColorPicker.cs               *****/
/*****     Original Author:   Danny Blanchard                 *****/
/*****                        - scrabcakes@gmail.com          *****/
/*****     Updates:	                                          *****/
/*****      3/28/2005 - Initial Version : Danny Blanchard     *****/
/*****       Mar 2018 - JoyCon Version  : CTCaer              *****/
/*****                                                        *****/
/******************************************************************/

/******************************************************************/
/*****                                                        *****/
/*****     This version is heavily optimised for use in       *****/
/*****     CTCaer's Joy-Con Toolkit and other projects.       *****/
/*****                                                        *****/
/******************************************************************/

using System;
using System.Drawing;
using System.ComponentModel;
using System.Windows.Forms;
using System.Collections.Generic;

namespace jcColor {

    /// <summary>
    /// Joy-Con Color Picker allows you to pick a Primary and a Secondary color.
    /// It's a Adobe based color picker with Preset support.
    /// </summary>
    public class JoyConColorPicker : System.Windows.Forms.Form {

        #region Class Variables

        private AdobeColors.HSL m_hsl;
        private Color m_rgb;
        private bool errorCreatingPresets = false;
        private bool gripsColor;
        private bool showGripsPanel = false;

        public enum eDrawStyle {
            Hue,
            Saturation,
            Brightness,
            Red,
            Green,
            Blue
        }


        #endregion

        #region Designer Generated Variables

        private System.Windows.Forms.Label m_lbl_SelectColor;
        public System.Windows.Forms.Button m_cmd_OK;
        public System.Windows.Forms.Button m_cmd_Cancel;
        private System.Windows.Forms.TextBox m_txt_Hue;
        private System.Windows.Forms.TextBox m_txt_Sat;
        private System.Windows.Forms.TextBox m_txt_Black;
        private System.Windows.Forms.TextBox m_txt_Red;
        private System.Windows.Forms.TextBox m_txt_Green;
        private System.Windows.Forms.TextBox m_txt_Blue;
        private System.Windows.Forms.TextBox m_txt_Hex;
        private System.Windows.Forms.RadioButton m_rbtn_Hue;
        private System.Windows.Forms.RadioButton m_rbtn_Sat;
        private System.Windows.Forms.RadioButton m_rbtn_Black;
        private System.Windows.Forms.RadioButton m_rbtn_Red;
        private System.Windows.Forms.RadioButton m_rbtn_Green;
        private System.Windows.Forms.RadioButton m_rbtn_Blue;
        private System.Windows.Forms.Label m_lbl_HexPound;
        private System.Windows.Forms.Label m_lbl_New_Color;
        private System.Windows.Forms.Label m_lbl_Old_Color;
        private ctrlVerticalColorSlider m_ctrl_ThinBox;
        private ctrl2DColorBox m_ctrl_BigBox;
        private System.Windows.Forms.Label m_lbl_Hue_Symbol;
        private System.Windows.Forms.Label m_lbl_Saturation_Symbol;
        private System.Windows.Forms.Label m_lbl_Black_Symbol;
        private EyedropColorPicker m_eyedropColorPicker;
        private ToolTip toolTip1;
        private Label m_lbl_Screen_Picker;
        private RadioButton m_radio_btn_Buttons;
        private RadioButton m_radio_btn_Body;
        private Label m_lbl_Old_ButtonColor_btn;
        private Label m_lbl_New_ButtonColor;
        private Panel panel1;
        private Panel panel2;
        private Buttons.RoundButton rbtn_RetailColors01;
        private Buttons.RoundButton rbtn_RetailColors02;
        private Buttons.RoundButton rbtn_RetailColors03;
        private Buttons.RoundButton rbtn_RetailColors04;
        private Buttons.RoundButton rbtn_RetailColors05;
        private Buttons.RoundButton rbtn_RetailColors06;
        private Buttons.RoundButton rbtn_RetailColors07;
        private Buttons.RoundButton rbtn_RetailColors08;
        private Buttons.RoundButton rbtn_RetailColors09;
        private Buttons.RoundButton rbtn_RetailColors10;
        private Buttons.RoundButton rbtn_RetailColors11;
        private Buttons.RoundButton rbtn_RetailColors12;
        private Buttons.RoundButton rbtn_RetailColors13;
        private Buttons.RoundButton rbtn_RetailColors14;
        private Buttons.RoundButton rbtn_RetailColors15;
        private Buttons.RoundButton rbtn_RetailColors16;
        private Label m_lbl_SelectPreset;
        private Button btn_Update;
        private Buttons.RoundButton rbtn_RetailColors17;
        private Buttons.RoundButton rbtn_RetailColors18;
        private Buttons.RoundButton rbtn_RetailColors19;
        private Buttons.RoundButton rbtn_RetailColors20;
        private Buttons.RoundButton rbtn_RetailColors21;
        private Buttons.RoundButton rbtn_RetailColors22;
        private Buttons.RoundButton rbtn_RetailColors23;
        private Buttons.RoundButton rbtn_RetailColors24;
        private Buttons.RoundButton rbtn_RetailColors25;
        private Buttons.RoundButton rbtn_RetailColors26;
        private Buttons.RoundButton rbtn_RetailColors27;
        private Buttons.RoundButton rbtn_RetailColors28;
        private Buttons.RoundButton rbtn_RetailColors29;
        private Buttons.RoundButton rbtn_RetailColors30;
        private Buttons.RoundButton rbtn_RetailColors31;
        private Buttons.RoundButton rbtn_RetailColors32;
        private Buttons.RoundButton rbtn_UserColors01;
        private Buttons.RoundButton rbtn_UserColors02;
        private Buttons.RoundButton rbtn_UserColors03;
        private Buttons.RoundButton rbtn_UserColors04;
        private Buttons.RoundButton rbtn_UserColors05;
        private Buttons.RoundButton rbtn_UserColors06;
        private Buttons.RoundButton rbtn_UserColors07;
        private Buttons.RoundButton rbtn_UserColors08;
        private Buttons.RoundButton rbtn_UserColors09;
        private Buttons.RoundButton rbtn_UserColors10;
        private Buttons.RoundButton rbtn_UserColors11;
        private Buttons.RoundButton rbtn_UserColors12;
        private Buttons.RoundButton rbtn_UserColors13;
        private Buttons.RoundButton rbtn_UserColors14;
        private Buttons.RoundButton rbtn_UserColors15;
        private Buttons.RoundButton rbtn_UserColors16;
        private Buttons.RoundButton rbtn_UserColors17;
        private Buttons.RoundButton rbtn_UserColors18;
        private Buttons.RoundButton rbtn_UserColors19;
        private Buttons.RoundButton rbtn_UserColors20;
        private Buttons.RoundButton rbtn_UserColors21;
        private Buttons.RoundButton rbtn_UserColors22;
        private Buttons.RoundButton rbtn_UserColors23;
        private Buttons.RoundButton rbtn_UserColors24;
        private Buttons.RoundButton rbtn_UserColors25;
        private Buttons.RoundButton rbtn_UserColors26;
        private Buttons.RoundButton rbtn_UserColors27;
        private Buttons.RoundButton rbtn_UserColors28;
        private Buttons.RoundButton rbtn_UserColors29;
        private Buttons.RoundButton rbtn_UserColors30;
        private Buttons.RoundButton rbtn_UserColors31;
        private Buttons.RoundButton rbtn_UserColors32;
        private Label m_lbl_UserColors;
        private Label m_lbl_RetailColors;
        private Button btn_Clear;
        private Panel panel_UserColors;
        private Panel panel_RetailUserColors;
        private Panel panel_RetailColors;
        private Panel panel_RetailGripColors;
        private Buttons.RoundButton rbtn_RetailGripColors04;
        private Buttons.RoundButton rbtn_RetailGripColors01;
        private Buttons.RoundButton rbtn_RetailGripColors02;
        private Buttons.RoundButton rbtn_RetailGripColors03;
        private Buttons.RoundButton rbtn_RetailGripColors16;
        private Buttons.RoundButton rbtn_RetailGripColors05;
        private Buttons.RoundButton rbtn_RetailGripColors15;
        private Buttons.RoundButton rbtn_RetailGripColors06;
        private Buttons.RoundButton rbtn_RetailGripColors14;
        private Buttons.RoundButton rbtn_RetailGripColors07;
        private Buttons.RoundButton rbtn_RetailGripColors13;
        private Buttons.RoundButton rbtn_RetailGripColors08;
        private Buttons.RoundButton rbtn_RetailGripColors12;
        private Buttons.RoundButton rbtn_RetailGripColors09;
        private Buttons.RoundButton rbtn_RetailGripColors11;
        private Buttons.RoundButton rbtn_RetailGripColors10;
        private Button btn_switchGripsPanel;
        private IContainer components;

        #endregion

        #region Constructors / Destructors

        /// <summary>
        /// Joy-Con Color Picker
        /// </summary>
        /// <param name="starting_color">Primary color</param>
        /// <param name="starting_color2">Secondary color</param>
        /// <param name="gripColorOptionsOn">Grips mode</param>
        public JoyConColorPicker(Color starting_color, Color starting_color2, bool gripColorOptionsOn) {
            InitializeComponent();

            gripsColor = gripColorOptionsOn;

            m_rgb = starting_color;
            m_hsl = AdobeColors.RGB_to_HSL(m_rgb);

            m_txt_Hue.Text = Round(m_hsl.H * 360).ToString();
            m_txt_Sat.Text = Round(m_hsl.S * 100).ToString();
            m_txt_Black.Text = Round(m_hsl.L * 100).ToString();
            m_txt_Red.Text = m_rgb.R.ToString();
            m_txt_Green.Text = m_rgb.G.ToString();
            m_txt_Blue.Text = m_rgb.B.ToString();

            m_txt_Hue.Update();
            m_txt_Sat.Update();
            m_txt_Red.Update();
            m_txt_Green.Update();
            m_txt_Blue.Update();

            m_ctrl_BigBox.HSL = m_hsl;
            m_ctrl_ThinBox.HSL = m_hsl;

            m_lbl_New_Color.BackColor = starting_color;
            m_lbl_Old_Color.BackColor = starting_color;

            m_lbl_New_ButtonColor.BackColor = starting_color2;
            m_lbl_Old_ButtonColor_btn.BackColor = starting_color2;

            m_rbtn_Hue.Checked = true;

            this.WriteHexData(m_rgb);

            m_eyedropColorPicker.MouseUp += new MouseEventHandler(OnEyeDropperSelectionChanged);

            this.m_radio_btn_Body.Checked = true;
        }


        protected override void Dispose(bool disposing) {
            if (disposing) {
                if (components != null) {
                    components.Dispose();
                }
            }
            base.Dispose(disposing);
        }


        #endregion

        #region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent() {
            this.components = new System.ComponentModel.Container();
            jcColor.AdobeColors.HSL hsl1 = new jcColor.AdobeColors.HSL();
            jcColor.AdobeColors.HSL hsl2 = new jcColor.AdobeColors.HSL();
            this.m_lbl_SelectColor = new System.Windows.Forms.Label();
            this.m_cmd_OK = new System.Windows.Forms.Button();
            this.m_cmd_Cancel = new System.Windows.Forms.Button();
            this.m_txt_Hue = new System.Windows.Forms.TextBox();
            this.m_txt_Sat = new System.Windows.Forms.TextBox();
            this.m_txt_Black = new System.Windows.Forms.TextBox();
            this.m_txt_Red = new System.Windows.Forms.TextBox();
            this.m_txt_Green = new System.Windows.Forms.TextBox();
            this.m_txt_Blue = new System.Windows.Forms.TextBox();
            this.m_txt_Hex = new System.Windows.Forms.TextBox();
            this.m_rbtn_Hue = new System.Windows.Forms.RadioButton();
            this.m_rbtn_Sat = new System.Windows.Forms.RadioButton();
            this.m_rbtn_Black = new System.Windows.Forms.RadioButton();
            this.m_rbtn_Red = new System.Windows.Forms.RadioButton();
            this.m_rbtn_Green = new System.Windows.Forms.RadioButton();
            this.m_rbtn_Blue = new System.Windows.Forms.RadioButton();
            this.m_lbl_HexPound = new System.Windows.Forms.Label();
            this.m_lbl_New_Color = new System.Windows.Forms.Label();
            this.m_lbl_Old_Color = new System.Windows.Forms.Label();
            this.m_lbl_Hue_Symbol = new System.Windows.Forms.Label();
            this.m_lbl_Saturation_Symbol = new System.Windows.Forms.Label();
            this.m_lbl_Black_Symbol = new System.Windows.Forms.Label();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.rbtn_RetailGripColors04 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailGripColors01 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailGripColors02 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailGripColors03 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors04 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors01 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors02 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors03 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors16 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors05 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors15 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors06 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors14 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors07 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors13 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors08 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors12 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors09 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors11 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors10 = new jcColor.Buttons.RoundButton();
            this.m_eyedropColorPicker = new jcColor.EyedropColorPicker();
            this.m_lbl_Screen_Picker = new System.Windows.Forms.Label();
            this.m_lbl_Old_ButtonColor_btn = new System.Windows.Forms.Label();
            this.m_lbl_New_ButtonColor = new System.Windows.Forms.Label();
            this.m_radio_btn_Body = new System.Windows.Forms.RadioButton();
            this.m_radio_btn_Buttons = new System.Windows.Forms.RadioButton();
            this.panel1 = new System.Windows.Forms.Panel();
            this.panel2 = new System.Windows.Forms.Panel();
            this.panel_RetailGripColors = new System.Windows.Forms.Panel();
            this.rbtn_RetailGripColors16 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailGripColors05 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailGripColors15 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailGripColors06 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailGripColors14 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailGripColors07 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailGripColors13 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailGripColors08 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailGripColors12 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailGripColors09 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailGripColors11 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailGripColors10 = new jcColor.Buttons.RoundButton();
            this.panel_RetailColors = new System.Windows.Forms.Panel();
            this.panel_RetailUserColors = new System.Windows.Forms.Panel();
            this.rbtn_RetailColors17 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors18 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors19 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors20 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors21 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors22 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors23 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors24 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors25 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors26 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors27 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors28 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors29 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors30 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors31 = new jcColor.Buttons.RoundButton();
            this.rbtn_RetailColors32 = new jcColor.Buttons.RoundButton();
            this.panel_UserColors = new System.Windows.Forms.Panel();
            this.m_lbl_UserColors = new System.Windows.Forms.Label();
            this.rbtn_UserColors01 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors02 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors03 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors04 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors05 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors06 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors07 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors08 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors09 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors10 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors11 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors12 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors13 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors14 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors15 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors16 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors17 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors18 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors19 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors20 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors21 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors22 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors23 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors24 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors25 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors26 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors27 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors28 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors29 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors30 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors31 = new jcColor.Buttons.RoundButton();
            this.rbtn_UserColors32 = new jcColor.Buttons.RoundButton();
            this.m_lbl_RetailColors = new System.Windows.Forms.Label();
            this.btn_Update = new System.Windows.Forms.Button();
            this.m_lbl_SelectPreset = new System.Windows.Forms.Label();
            this.btn_Clear = new System.Windows.Forms.Button();
            this.m_ctrl_BigBox = new jcColor.ctrl2DColorBox();
            this.m_ctrl_ThinBox = new jcColor.ctrlVerticalColorSlider();
            this.btn_switchGripsPanel = new System.Windows.Forms.Button();
            this.panel1.SuspendLayout();
            this.panel2.SuspendLayout();
            this.panel_RetailGripColors.SuspendLayout();
            this.panel_RetailColors.SuspendLayout();
            this.panel_RetailUserColors.SuspendLayout();
            this.panel_UserColors.SuspendLayout();
            this.SuspendLayout();
            // 
            // m_lbl_SelectColor
            // 
            this.m_lbl_SelectColor.Font = new System.Drawing.Font("Segoe UI Semibold", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_lbl_SelectColor.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(251)))), ((int)(((byte)(251)))), ((int)(((byte)(251)))));
            this.m_lbl_SelectColor.Location = new System.Drawing.Point(8, 6);
            this.m_lbl_SelectColor.Name = "m_lbl_SelectColor";
            this.m_lbl_SelectColor.Size = new System.Drawing.Size(260, 20);
            this.m_lbl_SelectColor.TabIndex = 0;
            this.m_lbl_SelectColor.Text = "Select Color:";
            // 
            // m_cmd_OK
            // 
            this.m_cmd_OK.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.m_cmd_OK.FlatAppearance.BorderSize = 0;
            this.m_cmd_OK.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.m_cmd_OK.Font = new System.Drawing.Font("Segoe UI Semibold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_cmd_OK.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(188)))), ((int)(((byte)(0)))));
            this.m_cmd_OK.Location = new System.Drawing.Point(13, 304);
            this.m_cmd_OK.Name = "m_cmd_OK";
            this.m_cmd_OK.Size = new System.Drawing.Size(87, 30);
            this.m_cmd_OK.TabIndex = 4;
            this.m_cmd_OK.Text = "OK";
            this.m_cmd_OK.UseVisualStyleBackColor = false;
            this.m_cmd_OK.Click += new System.EventHandler(this.m_cmd_OK_Click);
            // 
            // m_cmd_Cancel
            // 
            this.m_cmd_Cancel.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.m_cmd_Cancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.m_cmd_Cancel.FlatAppearance.BorderSize = 0;
            this.m_cmd_Cancel.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.m_cmd_Cancel.Font = new System.Drawing.Font("Segoe UI Semibold", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_cmd_Cancel.ForeColor = System.Drawing.Color.OrangeRed;
            this.m_cmd_Cancel.Location = new System.Drawing.Point(13, 379);
            this.m_cmd_Cancel.Name = "m_cmd_Cancel";
            this.m_cmd_Cancel.Size = new System.Drawing.Size(87, 30);
            this.m_cmd_Cancel.TabIndex = 5;
            this.m_cmd_Cancel.Text = "Cancel";
            this.m_cmd_Cancel.UseVisualStyleBackColor = false;
            this.m_cmd_Cancel.Click += new System.EventHandler(this.m_cmd_Cancel_Click);
            // 
            // m_txt_Hue
            // 
            this.m_txt_Hue.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.m_txt_Hue.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.m_txt_Hue.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_txt_Hue.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(9)))), ((int)(((byte)(255)))), ((int)(((byte)(206)))));
            this.m_txt_Hue.Location = new System.Drawing.Point(352, 29);
            this.m_txt_Hue.Margin = new System.Windows.Forms.Padding(0);
            this.m_txt_Hue.MaxLength = 3;
            this.m_txt_Hue.Multiline = true;
            this.m_txt_Hue.Name = "m_txt_Hue";
            this.m_txt_Hue.Size = new System.Drawing.Size(35, 21);
            this.m_txt_Hue.TabIndex = 6;
            this.m_txt_Hue.Text = "255";
            this.m_txt_Hue.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.m_txt_Hue.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.NumericTextBox_KeyPress);
            this.m_txt_Hue.KeyUp += new System.Windows.Forms.KeyEventHandler(this.Validate_Color_TextValues);
            this.m_txt_Hue.MouseWheel += new System.Windows.Forms.MouseEventHandler(this.Hue_MouseWheel);
            // 
            // m_txt_Sat
            // 
            this.m_txt_Sat.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.m_txt_Sat.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.m_txt_Sat.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_txt_Sat.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(9)))), ((int)(((byte)(255)))), ((int)(((byte)(206)))));
            this.m_txt_Sat.Location = new System.Drawing.Point(352, 54);
            this.m_txt_Sat.Margin = new System.Windows.Forms.Padding(0);
            this.m_txt_Sat.MaxLength = 3;
            this.m_txt_Sat.Multiline = true;
            this.m_txt_Sat.Name = "m_txt_Sat";
            this.m_txt_Sat.Size = new System.Drawing.Size(35, 21);
            this.m_txt_Sat.TabIndex = 7;
            this.m_txt_Sat.Text = "255";
            this.m_txt_Sat.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.m_txt_Sat.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.NumericTextBox_KeyPress);
            this.m_txt_Sat.KeyUp += new System.Windows.Forms.KeyEventHandler(this.Validate_Color_TextValues);
            this.m_txt_Sat.MouseWheel += new System.Windows.Forms.MouseEventHandler(this.Sat_Bri_MouseWheel);
            // 
            // m_txt_Black
            // 
            this.m_txt_Black.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.m_txt_Black.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.m_txt_Black.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_txt_Black.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(9)))), ((int)(((byte)(255)))), ((int)(((byte)(206)))));
            this.m_txt_Black.Location = new System.Drawing.Point(352, 79);
            this.m_txt_Black.Margin = new System.Windows.Forms.Padding(0);
            this.m_txt_Black.MaxLength = 3;
            this.m_txt_Black.Multiline = true;
            this.m_txt_Black.Name = "m_txt_Black";
            this.m_txt_Black.Size = new System.Drawing.Size(35, 21);
            this.m_txt_Black.TabIndex = 8;
            this.m_txt_Black.Text = "255";
            this.m_txt_Black.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.m_txt_Black.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.NumericTextBox_KeyPress);
            this.m_txt_Black.KeyUp += new System.Windows.Forms.KeyEventHandler(this.Validate_Color_TextValues);
            this.m_txt_Black.MouseWheel += new System.Windows.Forms.MouseEventHandler(this.Sat_Bri_MouseWheel);
            // 
            // m_txt_Red
            // 
            this.m_txt_Red.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.m_txt_Red.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.m_txt_Red.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_txt_Red.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(9)))), ((int)(((byte)(255)))), ((int)(((byte)(206)))));
            this.m_txt_Red.Location = new System.Drawing.Point(352, 114);
            this.m_txt_Red.Margin = new System.Windows.Forms.Padding(0);
            this.m_txt_Red.MaxLength = 4;
            this.m_txt_Red.Multiline = true;
            this.m_txt_Red.Name = "m_txt_Red";
            this.m_txt_Red.Size = new System.Drawing.Size(35, 21);
            this.m_txt_Red.TabIndex = 9;
            this.m_txt_Red.Text = "255";
            this.m_txt_Red.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.m_txt_Red.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.NumericTextBox_KeyPress);
            this.m_txt_Red.KeyUp += new System.Windows.Forms.KeyEventHandler(this.Validate_Color_TextValues);
            this.m_txt_Red.MouseWheel += new System.Windows.Forms.MouseEventHandler(this.RGB_MouseWheel);
            // 
            // m_txt_Green
            // 
            this.m_txt_Green.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.m_txt_Green.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.m_txt_Green.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_txt_Green.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(9)))), ((int)(((byte)(255)))), ((int)(((byte)(206)))));
            this.m_txt_Green.Location = new System.Drawing.Point(352, 139);
            this.m_txt_Green.Margin = new System.Windows.Forms.Padding(0);
            this.m_txt_Green.MaxLength = 3;
            this.m_txt_Green.Multiline = true;
            this.m_txt_Green.Name = "m_txt_Green";
            this.m_txt_Green.Size = new System.Drawing.Size(35, 21);
            this.m_txt_Green.TabIndex = 10;
            this.m_txt_Green.Text = "255";
            this.m_txt_Green.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.m_txt_Green.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.NumericTextBox_KeyPress);
            this.m_txt_Green.KeyUp += new System.Windows.Forms.KeyEventHandler(this.Validate_Color_TextValues);
            this.m_txt_Green.MouseWheel += new System.Windows.Forms.MouseEventHandler(this.RGB_MouseWheel);
            // 
            // m_txt_Blue
            // 
            this.m_txt_Blue.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.m_txt_Blue.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.m_txt_Blue.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_txt_Blue.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(9)))), ((int)(((byte)(255)))), ((int)(((byte)(206)))));
            this.m_txt_Blue.Location = new System.Drawing.Point(352, 164);
            this.m_txt_Blue.Margin = new System.Windows.Forms.Padding(0);
            this.m_txt_Blue.MaxLength = 3;
            this.m_txt_Blue.Multiline = true;
            this.m_txt_Blue.Name = "m_txt_Blue";
            this.m_txt_Blue.Size = new System.Drawing.Size(35, 21);
            this.m_txt_Blue.TabIndex = 11;
            this.m_txt_Blue.Text = "255";
            this.m_txt_Blue.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.m_txt_Blue.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.NumericTextBox_KeyPress);
            this.m_txt_Blue.KeyUp += new System.Windows.Forms.KeyEventHandler(this.Validate_Color_TextValues);
            this.m_txt_Blue.MouseWheel += new System.Windows.Forms.MouseEventHandler(this.RGB_MouseWheel);
            // 
            // m_txt_Hex
            // 
            this.m_txt_Hex.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.m_txt_Hex.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.m_txt_Hex.CharacterCasing = System.Windows.Forms.CharacterCasing.Upper;
            this.m_txt_Hex.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_txt_Hex.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(9)))), ((int)(((byte)(255)))), ((int)(((byte)(206)))));
            this.m_txt_Hex.Location = new System.Drawing.Point(329, 225);
            this.m_txt_Hex.MaxLength = 6;
            this.m_txt_Hex.Multiline = true;
            this.m_txt_Hex.Name = "m_txt_Hex";
            this.m_txt_Hex.Size = new System.Drawing.Size(73, 21);
            this.m_txt_Hex.TabIndex = 19;
            this.m_txt_Hex.Text = "FFFFFF";
            this.m_txt_Hex.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.HexTextBox_KeyPress);
            this.m_txt_Hex.KeyUp += new System.Windows.Forms.KeyEventHandler(this.Validate_Color_HexValue);
            // 
            // m_rbtn_Hue
            // 
            this.m_rbtn_Hue.AutoSize = true;
            this.m_rbtn_Hue.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(70)))), ((int)(((byte)(70)))), ((int)(((byte)(70)))));
            this.m_rbtn_Hue.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_rbtn_Hue.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(188)))), ((int)(((byte)(0)))));
            this.m_rbtn_Hue.Location = new System.Drawing.Point(315, 29);
            this.m_rbtn_Hue.Margin = new System.Windows.Forms.Padding(0);
            this.m_rbtn_Hue.Name = "m_rbtn_Hue";
            this.m_rbtn_Hue.Size = new System.Drawing.Size(37, 19);
            this.m_rbtn_Hue.TabIndex = 20;
            this.m_rbtn_Hue.Text = "H:";
            this.m_rbtn_Hue.UseVisualStyleBackColor = false;
            this.m_rbtn_Hue.CheckedChanged += new System.EventHandler(this.m_rbtn_Hue_CheckedChanged);
            // 
            // m_rbtn_Sat
            // 
            this.m_rbtn_Sat.AutoSize = true;
            this.m_rbtn_Sat.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(70)))), ((int)(((byte)(70)))), ((int)(((byte)(70)))));
            this.m_rbtn_Sat.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_rbtn_Sat.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(188)))), ((int)(((byte)(0)))));
            this.m_rbtn_Sat.Location = new System.Drawing.Point(315, 54);
            this.m_rbtn_Sat.Margin = new System.Windows.Forms.Padding(0);
            this.m_rbtn_Sat.Name = "m_rbtn_Sat";
            this.m_rbtn_Sat.Size = new System.Drawing.Size(36, 19);
            this.m_rbtn_Sat.TabIndex = 21;
            this.m_rbtn_Sat.Text = "S:";
            this.m_rbtn_Sat.UseVisualStyleBackColor = false;
            this.m_rbtn_Sat.CheckedChanged += new System.EventHandler(this.m_rbtn_Sat_CheckedChanged);
            // 
            // m_rbtn_Black
            // 
            this.m_rbtn_Black.AutoSize = true;
            this.m_rbtn_Black.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(70)))), ((int)(((byte)(70)))), ((int)(((byte)(70)))));
            this.m_rbtn_Black.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_rbtn_Black.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(188)))), ((int)(((byte)(0)))));
            this.m_rbtn_Black.Location = new System.Drawing.Point(315, 79);
            this.m_rbtn_Black.Margin = new System.Windows.Forms.Padding(0);
            this.m_rbtn_Black.Name = "m_rbtn_Black";
            this.m_rbtn_Black.Size = new System.Drawing.Size(36, 19);
            this.m_rbtn_Black.TabIndex = 22;
            this.m_rbtn_Black.Text = "B:";
            this.m_rbtn_Black.UseVisualStyleBackColor = false;
            this.m_rbtn_Black.CheckedChanged += new System.EventHandler(this.m_rbtn_Black_CheckedChanged);
            // 
            // m_rbtn_Red
            // 
            this.m_rbtn_Red.AutoSize = true;
            this.m_rbtn_Red.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(70)))), ((int)(((byte)(70)))), ((int)(((byte)(70)))));
            this.m_rbtn_Red.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_rbtn_Red.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(188)))), ((int)(((byte)(0)))));
            this.m_rbtn_Red.Location = new System.Drawing.Point(315, 114);
            this.m_rbtn_Red.Margin = new System.Windows.Forms.Padding(0);
            this.m_rbtn_Red.Name = "m_rbtn_Red";
            this.m_rbtn_Red.Size = new System.Drawing.Size(37, 19);
            this.m_rbtn_Red.TabIndex = 23;
            this.m_rbtn_Red.Text = "R:";
            this.m_rbtn_Red.UseVisualStyleBackColor = false;
            this.m_rbtn_Red.CheckedChanged += new System.EventHandler(this.m_rbtn_Red_CheckedChanged);
            // 
            // m_rbtn_Green
            // 
            this.m_rbtn_Green.AutoSize = true;
            this.m_rbtn_Green.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(70)))), ((int)(((byte)(70)))), ((int)(((byte)(70)))));
            this.m_rbtn_Green.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_rbtn_Green.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(188)))), ((int)(((byte)(0)))));
            this.m_rbtn_Green.Location = new System.Drawing.Point(315, 139);
            this.m_rbtn_Green.Margin = new System.Windows.Forms.Padding(0);
            this.m_rbtn_Green.Name = "m_rbtn_Green";
            this.m_rbtn_Green.Size = new System.Drawing.Size(37, 19);
            this.m_rbtn_Green.TabIndex = 24;
            this.m_rbtn_Green.Text = "G:";
            this.m_rbtn_Green.UseVisualStyleBackColor = false;
            this.m_rbtn_Green.CheckedChanged += new System.EventHandler(this.m_rbtn_Green_CheckedChanged);
            // 
            // m_rbtn_Blue
            // 
            this.m_rbtn_Blue.AutoSize = true;
            this.m_rbtn_Blue.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(70)))), ((int)(((byte)(70)))), ((int)(((byte)(70)))));
            this.m_rbtn_Blue.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_rbtn_Blue.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(188)))), ((int)(((byte)(0)))));
            this.m_rbtn_Blue.Location = new System.Drawing.Point(315, 164);
            this.m_rbtn_Blue.Margin = new System.Windows.Forms.Padding(0);
            this.m_rbtn_Blue.Name = "m_rbtn_Blue";
            this.m_rbtn_Blue.Size = new System.Drawing.Size(36, 19);
            this.m_rbtn_Blue.TabIndex = 25;
            this.m_rbtn_Blue.Text = "B:";
            this.m_rbtn_Blue.UseVisualStyleBackColor = false;
            this.m_rbtn_Blue.CheckedChanged += new System.EventHandler(this.m_rbtn_Blue_CheckedChanged);
            // 
            // m_lbl_HexPound
            // 
            this.m_lbl_HexPound.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_lbl_HexPound.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(188)))), ((int)(((byte)(0)))));
            this.m_lbl_HexPound.Location = new System.Drawing.Point(314, 227);
            this.m_lbl_HexPound.Margin = new System.Windows.Forms.Padding(0);
            this.m_lbl_HexPound.Name = "m_lbl_HexPound";
            this.m_lbl_HexPound.Size = new System.Drawing.Size(12, 14);
            this.m_lbl_HexPound.TabIndex = 27;
            this.m_lbl_HexPound.Text = "#";
            // 
            // m_lbl_New_Color
            // 
            this.m_lbl_New_Color.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(120)))), ((int)(((byte)(120)))), ((int)(((byte)(120)))));
            this.m_lbl_New_Color.CausesValidation = false;
            this.m_lbl_New_Color.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_lbl_New_Color.Location = new System.Drawing.Point(26, 33);
            this.m_lbl_New_Color.Margin = new System.Windows.Forms.Padding(0);
            this.m_lbl_New_Color.Name = "m_lbl_New_Color";
            this.m_lbl_New_Color.Size = new System.Drawing.Size(68, 42);
            this.m_lbl_New_Color.TabIndex = 36;
            this.m_lbl_New_Color.Click += new System.EventHandler(this.m_lbl_NewColor_Click);
            // 
            // m_lbl_Old_Color
            // 
            this.m_lbl_Old_Color.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(120)))), ((int)(((byte)(120)))), ((int)(((byte)(120)))));
            this.m_lbl_Old_Color.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_lbl_Old_Color.Location = new System.Drawing.Point(26, 75);
            this.m_lbl_Old_Color.Margin = new System.Windows.Forms.Padding(0);
            this.m_lbl_Old_Color.Name = "m_lbl_Old_Color";
            this.m_lbl_Old_Color.Size = new System.Drawing.Size(68, 42);
            this.m_lbl_Old_Color.TabIndex = 37;
            this.m_lbl_Old_Color.Click += new System.EventHandler(this.m_lbl_OldColor_Click);
            // 
            // m_lbl_Hue_Symbol
            // 
            this.m_lbl_Hue_Symbol.AutoSize = true;
            this.m_lbl_Hue_Symbol.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_lbl_Hue_Symbol.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(188)))), ((int)(((byte)(0)))));
            this.m_lbl_Hue_Symbol.Location = new System.Drawing.Point(388, 28);
            this.m_lbl_Hue_Symbol.Name = "m_lbl_Hue_Symbol";
            this.m_lbl_Hue_Symbol.Size = new System.Drawing.Size(14, 18);
            this.m_lbl_Hue_Symbol.TabIndex = 40;
            this.m_lbl_Hue_Symbol.Text = "°";
            // 
            // m_lbl_Saturation_Symbol
            // 
            this.m_lbl_Saturation_Symbol.AutoSize = true;
            this.m_lbl_Saturation_Symbol.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_lbl_Saturation_Symbol.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(188)))), ((int)(((byte)(0)))));
            this.m_lbl_Saturation_Symbol.Location = new System.Drawing.Point(388, 55);
            this.m_lbl_Saturation_Symbol.Name = "m_lbl_Saturation_Symbol";
            this.m_lbl_Saturation_Symbol.Size = new System.Drawing.Size(20, 16);
            this.m_lbl_Saturation_Symbol.TabIndex = 41;
            this.m_lbl_Saturation_Symbol.Text = "%";
            // 
            // m_lbl_Black_Symbol
            // 
            this.m_lbl_Black_Symbol.AutoSize = true;
            this.m_lbl_Black_Symbol.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_lbl_Black_Symbol.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(188)))), ((int)(((byte)(0)))));
            this.m_lbl_Black_Symbol.Location = new System.Drawing.Point(388, 80);
            this.m_lbl_Black_Symbol.Name = "m_lbl_Black_Symbol";
            this.m_lbl_Black_Symbol.Size = new System.Drawing.Size(20, 16);
            this.m_lbl_Black_Symbol.TabIndex = 42;
            this.m_lbl_Black_Symbol.Text = "%";
            // 
            // rbtn_RetailGripColors04
            // 
            this.rbtn_RetailGripColors04.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(50)))), ((int)(((byte)(120)))));
            this.rbtn_RetailGripColors04.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailGripColors04.Location = new System.Drawing.Point(118, 7);
            this.rbtn_RetailGripColors04.Name = "rbtn_RetailGripColors04";
            this.rbtn_RetailGripColors04.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailGripColors04.TabIndex = 3;
            this.rbtn_RetailGripColors04.Text = "roundButton4";
            this.toolTip1.SetToolTip(this.rbtn_RetailGripColors04, "Splatoon Right Grip");
            this.rbtn_RetailGripColors04.UseVisualStyleBackColor = false;
            this.rbtn_RetailGripColors04.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailGripColors01
            // 
            this.rbtn_RetailGripColors01.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(70)))), ((int)(((byte)(70)))), ((int)(((byte)(70)))));
            this.rbtn_RetailGripColors01.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailGripColors01.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(70)))), ((int)(((byte)(70)))), ((int)(((byte)(70)))));
            this.rbtn_RetailGripColors01.Location = new System.Drawing.Point(4, 7);
            this.rbtn_RetailGripColors01.Name = "rbtn_RetailGripColors01";
            this.rbtn_RetailGripColors01.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailGripColors01.TabIndex = 0;
            this.rbtn_RetailGripColors01.Text = "roundButton1";
            this.toolTip1.SetToolTip(this.rbtn_RetailGripColors01, "Normal Grips");
            this.rbtn_RetailGripColors01.UseVisualStyleBackColor = false;
            this.rbtn_RetailGripColors01.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailGripColors02
            // 
            this.rbtn_RetailGripColors02.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(221)))), ((int)(((byte)(59)))), ((int)(((byte)(100)))));
            this.rbtn_RetailGripColors02.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailGripColors02.Location = new System.Drawing.Point(42, 7);
            this.rbtn_RetailGripColors02.Name = "rbtn_RetailGripColors02";
            this.rbtn_RetailGripColors02.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailGripColors02.TabIndex = 1;
            this.rbtn_RetailGripColors02.Text = "roundButton2";
            this.toolTip1.SetToolTip(this.rbtn_RetailGripColors02, "Xenoblade Grips");
            this.rbtn_RetailGripColors02.UseVisualStyleBackColor = false;
            this.rbtn_RetailGripColors02.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailGripColors03
            // 
            this.rbtn_RetailGripColors03.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(220)))), ((int)(((byte)(0)))));
            this.rbtn_RetailGripColors03.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailGripColors03.Location = new System.Drawing.Point(80, 7);
            this.rbtn_RetailGripColors03.Name = "rbtn_RetailGripColors03";
            this.rbtn_RetailGripColors03.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailGripColors03.TabIndex = 2;
            this.rbtn_RetailGripColors03.Text = "roundButton3";
            this.toolTip1.SetToolTip(this.rbtn_RetailGripColors03, "Splatoon Left Grip");
            this.rbtn_RetailGripColors03.UseVisualStyleBackColor = false;
            this.rbtn_RetailGripColors03.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors04
            // 
            this.rbtn_RetailColors04.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(230)))), ((int)(((byte)(255)))), ((int)(((byte)(0)))));
            this.rbtn_RetailColors04.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors04.Location = new System.Drawing.Point(118, 7);
            this.rbtn_RetailColors04.Name = "rbtn_RetailColors04";
            this.rbtn_RetailColors04.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors04.TabIndex = 3;
            this.rbtn_RetailColors04.Text = "roundButton4";
            this.toolTip1.SetToolTip(this.rbtn_RetailColors04, "Neon Yellow");
            this.rbtn_RetailColors04.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors04.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors01
            // 
            this.rbtn_RetailColors01.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(130)))), ((int)(((byte)(130)))), ((int)(((byte)(130)))));
            this.rbtn_RetailColors01.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors01.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(130)))), ((int)(((byte)(130)))), ((int)(((byte)(130)))));
            this.rbtn_RetailColors01.Location = new System.Drawing.Point(4, 7);
            this.rbtn_RetailColors01.Name = "rbtn_RetailColors01";
            this.rbtn_RetailColors01.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors01.TabIndex = 0;
            this.rbtn_RetailColors01.Text = "roundButton1";
            this.toolTip1.SetToolTip(this.rbtn_RetailColors01, "Grey");
            this.rbtn_RetailColors01.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors01.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors02
            // 
            this.rbtn_RetailColors02.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(10)))), ((int)(((byte)(185)))), ((int)(((byte)(230)))));
            this.rbtn_RetailColors02.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors02.Location = new System.Drawing.Point(42, 7);
            this.rbtn_RetailColors02.Name = "rbtn_RetailColors02";
            this.rbtn_RetailColors02.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors02.TabIndex = 1;
            this.rbtn_RetailColors02.Text = "roundButton2";
            this.toolTip1.SetToolTip(this.rbtn_RetailColors02, "Neon Blue");
            this.rbtn_RetailColors02.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors02.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors03
            // 
            this.rbtn_RetailColors03.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(60)))), ((int)(((byte)(40)))));
            this.rbtn_RetailColors03.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors03.Location = new System.Drawing.Point(80, 7);
            this.rbtn_RetailColors03.Name = "rbtn_RetailColors03";
            this.rbtn_RetailColors03.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors03.TabIndex = 2;
            this.rbtn_RetailColors03.Text = "roundButton3";
            this.toolTip1.SetToolTip(this.rbtn_RetailColors03, "Neon Red");
            this.rbtn_RetailColors03.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors03.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors16
            // 
            this.rbtn_RetailColors16.BackColor = System.Drawing.Color.White;
            this.rbtn_RetailColors16.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors16.Location = new System.Drawing.Point(270, 43);
            this.rbtn_RetailColors16.Name = "rbtn_RetailColors16";
            this.rbtn_RetailColors16.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors16.TabIndex = 15;
            this.rbtn_RetailColors16.Text = "roundButton9";
            this.toolTip1.SetToolTip(this.rbtn_RetailColors16, "Pro Black");
            this.rbtn_RetailColors16.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors16.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors05
            // 
            this.rbtn_RetailColors05.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(50)))), ((int)(((byte)(120)))));
            this.rbtn_RetailColors05.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors05.Location = new System.Drawing.Point(156, 7);
            this.rbtn_RetailColors05.Name = "rbtn_RetailColors05";
            this.rbtn_RetailColors05.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors05.TabIndex = 4;
            this.rbtn_RetailColors05.Text = "roundButton5";
            this.toolTip1.SetToolTip(this.rbtn_RetailColors05, "Neon Pink");
            this.rbtn_RetailColors05.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors05.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors15
            // 
            this.rbtn_RetailColors15.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(40)))), ((int)(((byte)(10)))), ((int)(((byte)(10)))));
            this.rbtn_RetailColors15.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors15.Location = new System.Drawing.Point(232, 43);
            this.rbtn_RetailColors15.Name = "rbtn_RetailColors15";
            this.rbtn_RetailColors15.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors15.TabIndex = 14;
            this.rbtn_RetailColors15.Text = "roundButton10";
            this.toolTip1.SetToolTip(this.rbtn_RetailColors15, "Red");
            this.rbtn_RetailColors15.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors15.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors06
            // 
            this.rbtn_RetailColors06.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(220)))), ((int)(((byte)(0)))));
            this.rbtn_RetailColors06.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors06.Location = new System.Drawing.Point(194, 7);
            this.rbtn_RetailColors06.Name = "rbtn_RetailColors06";
            this.rbtn_RetailColors06.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors06.TabIndex = 5;
            this.rbtn_RetailColors06.Text = "roundButton6";
            this.toolTip1.SetToolTip(this.rbtn_RetailColors06, "Neon Green");
            this.rbtn_RetailColors06.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors06.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors14
            // 
            this.rbtn_RetailColors14.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(40)))), ((int)(((byte)(0)))));
            this.rbtn_RetailColors14.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors14.Location = new System.Drawing.Point(194, 43);
            this.rbtn_RetailColors14.Name = "rbtn_RetailColors14";
            this.rbtn_RetailColors14.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors14.TabIndex = 13;
            this.rbtn_RetailColors14.Text = "roundButton11";
            this.toolTip1.SetToolTip(this.rbtn_RetailColors14, "Neon Green");
            this.rbtn_RetailColors14.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors14.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors07
            // 
            this.rbtn_RetailColors07.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(225)))), ((int)(((byte)(15)))), ((int)(((byte)(0)))));
            this.rbtn_RetailColors07.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors07.Location = new System.Drawing.Point(232, 7);
            this.rbtn_RetailColors07.Name = "rbtn_RetailColors07";
            this.rbtn_RetailColors07.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors07.TabIndex = 6;
            this.rbtn_RetailColors07.Text = "roundButton7";
            this.toolTip1.SetToolTip(this.rbtn_RetailColors07, "Red");
            this.rbtn_RetailColors07.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors07.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors13
            // 
            this.rbtn_RetailColors13.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(40)))), ((int)(((byte)(0)))), ((int)(((byte)(30)))));
            this.rbtn_RetailColors13.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors13.Location = new System.Drawing.Point(156, 43);
            this.rbtn_RetailColors13.Name = "rbtn_RetailColors13";
            this.rbtn_RetailColors13.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors13.TabIndex = 12;
            this.rbtn_RetailColors13.Text = "roundButton12";
            this.toolTip1.SetToolTip(this.rbtn_RetailColors13, "Neon Pink");
            this.rbtn_RetailColors13.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors13.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors08
            // 
            this.rbtn_RetailColors08.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(50)))), ((int)(((byte)(50)))));
            this.rbtn_RetailColors08.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors08.Location = new System.Drawing.Point(270, 7);
            this.rbtn_RetailColors08.Name = "rbtn_RetailColors08";
            this.rbtn_RetailColors08.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors08.TabIndex = 7;
            this.rbtn_RetailColors08.Text = "roundButton8";
            this.toolTip1.SetToolTip(this.rbtn_RetailColors08, "Pro Black");
            this.rbtn_RetailColors08.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors08.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors12
            // 
            this.rbtn_RetailColors12.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(20)))), ((int)(((byte)(40)))), ((int)(((byte)(0)))));
            this.rbtn_RetailColors12.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors12.Location = new System.Drawing.Point(118, 43);
            this.rbtn_RetailColors12.Name = "rbtn_RetailColors12";
            this.rbtn_RetailColors12.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors12.TabIndex = 11;
            this.rbtn_RetailColors12.Text = "roundButton13";
            this.toolTip1.SetToolTip(this.rbtn_RetailColors12, "Neon Yellow");
            this.rbtn_RetailColors12.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors12.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors09
            // 
            this.rbtn_RetailColors09.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(15)))), ((int)(((byte)(15)))), ((int)(((byte)(15)))));
            this.rbtn_RetailColors09.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors09.Location = new System.Drawing.Point(4, 43);
            this.rbtn_RetailColors09.Name = "rbtn_RetailColors09";
            this.rbtn_RetailColors09.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors09.TabIndex = 8;
            this.rbtn_RetailColors09.Text = "roundButton16";
            this.toolTip1.SetToolTip(this.rbtn_RetailColors09, "Grey");
            this.rbtn_RetailColors09.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors09.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors11
            // 
            this.rbtn_RetailColors11.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(10)))), ((int)(((byte)(10)))));
            this.rbtn_RetailColors11.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors11.Location = new System.Drawing.Point(80, 43);
            this.rbtn_RetailColors11.Name = "rbtn_RetailColors11";
            this.rbtn_RetailColors11.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors11.TabIndex = 10;
            this.rbtn_RetailColors11.Text = "roundButton14";
            this.toolTip1.SetToolTip(this.rbtn_RetailColors11, "Neon Red");
            this.rbtn_RetailColors11.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors11.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors10
            // 
            this.rbtn_RetailColors10.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
            this.rbtn_RetailColors10.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors10.Location = new System.Drawing.Point(42, 43);
            this.rbtn_RetailColors10.Name = "rbtn_RetailColors10";
            this.rbtn_RetailColors10.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors10.TabIndex = 9;
            this.rbtn_RetailColors10.Text = "roundButton15";
            this.toolTip1.SetToolTip(this.rbtn_RetailColors10, "Neon Blue");
            this.rbtn_RetailColors10.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors10.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // m_eyedropColorPicker
            // 
            this.m_eyedropColorPicker.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.m_eyedropColorPicker.Location = new System.Drawing.Point(317, 324);
            this.m_eyedropColorPicker.Name = "m_eyedropColorPicker";
            this.m_eyedropColorPicker.SelectedColor = System.Drawing.Color.Empty;
            this.m_eyedropColorPicker.Size = new System.Drawing.Size(85, 85);
            this.m_eyedropColorPicker.TabIndex = 2;
            this.m_eyedropColorPicker.TabStop = false;
            this.toolTip1.SetToolTip(this.m_eyedropColorPicker, "Color Selector. Click and Drag to pick a color from the screen");
            this.m_eyedropColorPicker.Zoom = 4;
            // 
            // m_lbl_Screen_Picker
            // 
            this.m_lbl_Screen_Picker.AutoSize = true;
            this.m_lbl_Screen_Picker.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_lbl_Screen_Picker.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(251)))), ((int)(((byte)(251)))), ((int)(((byte)(251)))));
            this.m_lbl_Screen_Picker.Location = new System.Drawing.Point(313, 302);
            this.m_lbl_Screen_Picker.Name = "m_lbl_Screen_Picker";
            this.m_lbl_Screen_Picker.Size = new System.Drawing.Size(80, 17);
            this.m_lbl_Screen_Picker.TabIndex = 48;
            this.m_lbl_Screen_Picker.Text = "Eyedropper:";
            // 
            // m_lbl_Old_ButtonColor_btn
            // 
            this.m_lbl_Old_ButtonColor_btn.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(120)))), ((int)(((byte)(120)))), ((int)(((byte)(120)))));
            this.m_lbl_Old_ButtonColor_btn.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_lbl_Old_ButtonColor_btn.Location = new System.Drawing.Point(109, 75);
            this.m_lbl_Old_ButtonColor_btn.Margin = new System.Windows.Forms.Padding(0);
            this.m_lbl_Old_ButtonColor_btn.Name = "m_lbl_Old_ButtonColor_btn";
            this.m_lbl_Old_ButtonColor_btn.Size = new System.Drawing.Size(68, 42);
            this.m_lbl_Old_ButtonColor_btn.TabIndex = 51;
            this.m_lbl_Old_ButtonColor_btn.Click += new System.EventHandler(this.m_lbl_OldBtnColor_Click);
            // 
            // m_lbl_New_ButtonColor
            // 
            this.m_lbl_New_ButtonColor.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(120)))), ((int)(((byte)(120)))), ((int)(((byte)(120)))));
            this.m_lbl_New_ButtonColor.CausesValidation = false;
            this.m_lbl_New_ButtonColor.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_lbl_New_ButtonColor.Location = new System.Drawing.Point(109, 33);
            this.m_lbl_New_ButtonColor.Margin = new System.Windows.Forms.Padding(0);
            this.m_lbl_New_ButtonColor.Name = "m_lbl_New_ButtonColor";
            this.m_lbl_New_ButtonColor.Size = new System.Drawing.Size(68, 42);
            this.m_lbl_New_ButtonColor.TabIndex = 50;
            this.m_lbl_New_ButtonColor.Click += new System.EventHandler(this.m_lbl_NewBtnColor_Click);
            // 
            // m_radio_btn_Body
            // 
            this.m_radio_btn_Body.AutoSize = true;
            this.m_radio_btn_Body.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_radio_btn_Body.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(188)))), ((int)(((byte)(0)))));
            this.m_radio_btn_Body.Location = new System.Drawing.Point(27, 8);
            this.m_radio_btn_Body.Margin = new System.Windows.Forms.Padding(0);
            this.m_radio_btn_Body.Name = "m_radio_btn_Body";
            this.m_radio_btn_Body.Size = new System.Drawing.Size(55, 21);
            this.m_radio_btn_Body.TabIndex = 52;
            this.m_radio_btn_Body.TabStop = true;
            this.m_radio_btn_Body.Text = "Body";
            this.m_radio_btn_Body.UseVisualStyleBackColor = true;
            this.m_radio_btn_Body.CheckedChanged += new System.EventHandler(this.m_radio_btn_ColorsTypeChanged);
            // 
            // m_radio_btn_Buttons
            // 
            this.m_radio_btn_Buttons.AutoSize = true;
            this.m_radio_btn_Buttons.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_radio_btn_Buttons.Location = new System.Drawing.Point(110, 8);
            this.m_radio_btn_Buttons.Margin = new System.Windows.Forms.Padding(0);
            this.m_radio_btn_Buttons.Name = "m_radio_btn_Buttons";
            this.m_radio_btn_Buttons.Size = new System.Drawing.Size(69, 21);
            this.m_radio_btn_Buttons.TabIndex = 53;
            this.m_radio_btn_Buttons.Text = "Buttons";
            this.m_radio_btn_Buttons.UseVisualStyleBackColor = true;
            this.m_radio_btn_Buttons.CheckedChanged += new System.EventHandler(this.m_radio_btn_ColorsTypeChanged);
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.m_lbl_New_Color);
            this.panel1.Controls.Add(this.m_lbl_Old_ButtonColor_btn);
            this.panel1.Controls.Add(this.m_lbl_Old_Color);
            this.panel1.Controls.Add(this.m_lbl_New_ButtonColor);
            this.panel1.Controls.Add(this.m_radio_btn_Buttons);
            this.panel1.Controls.Add(this.m_radio_btn_Body);
            this.panel1.Location = new System.Drawing.Point(107, 292);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(204, 128);
            this.panel1.TabIndex = 55;
            // 
            // panel2
            // 
            this.panel2.Controls.Add(this.panel_RetailGripColors);
            this.panel2.Controls.Add(this.panel_RetailColors);
            this.panel2.Controls.Add(this.panel_RetailUserColors);
            this.panel2.Controls.Add(this.panel_UserColors);
            this.panel2.Controls.Add(this.m_lbl_RetailColors);
            this.panel2.Location = new System.Drawing.Point(414, 33);
            this.panel2.Margin = new System.Windows.Forms.Padding(0);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(313, 383);
            this.panel2.TabIndex = 56;
            // 
            // panel_RetailGripColors
            // 
            this.panel_RetailGripColors.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(60)))), ((int)(((byte)(60)))), ((int)(((byte)(60)))));
            this.panel_RetailGripColors.Controls.Add(this.rbtn_RetailGripColors04);
            this.panel_RetailGripColors.Controls.Add(this.rbtn_RetailGripColors01);
            this.panel_RetailGripColors.Controls.Add(this.rbtn_RetailGripColors02);
            this.panel_RetailGripColors.Controls.Add(this.rbtn_RetailGripColors03);
            this.panel_RetailGripColors.Controls.Add(this.rbtn_RetailGripColors16);
            this.panel_RetailGripColors.Controls.Add(this.rbtn_RetailGripColors05);
            this.panel_RetailGripColors.Controls.Add(this.rbtn_RetailGripColors15);
            this.panel_RetailGripColors.Controls.Add(this.rbtn_RetailGripColors06);
            this.panel_RetailGripColors.Controls.Add(this.rbtn_RetailGripColors14);
            this.panel_RetailGripColors.Controls.Add(this.rbtn_RetailGripColors07);
            this.panel_RetailGripColors.Controls.Add(this.rbtn_RetailGripColors13);
            this.panel_RetailGripColors.Controls.Add(this.rbtn_RetailGripColors08);
            this.panel_RetailGripColors.Controls.Add(this.rbtn_RetailGripColors12);
            this.panel_RetailGripColors.Controls.Add(this.rbtn_RetailGripColors09);
            this.panel_RetailGripColors.Controls.Add(this.rbtn_RetailGripColors11);
            this.panel_RetailGripColors.Controls.Add(this.rbtn_RetailGripColors10);
            this.panel_RetailGripColors.Location = new System.Drawing.Point(3, 105);
            this.panel_RetailGripColors.Name = "panel_RetailGripColors";
            this.panel_RetailGripColors.Size = new System.Drawing.Size(307, 82);
            this.panel_RetailGripColors.TabIndex = 69;
            // 
            // rbtn_RetailGripColors16
            // 
            this.rbtn_RetailGripColors16.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailGripColors16.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailGripColors16.Location = new System.Drawing.Point(270, 43);
            this.rbtn_RetailGripColors16.Name = "rbtn_RetailGripColors16";
            this.rbtn_RetailGripColors16.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailGripColors16.TabIndex = 15;
            this.rbtn_RetailGripColors16.Text = "roundButton9";
            this.rbtn_RetailGripColors16.UseVisualStyleBackColor = false;
            this.rbtn_RetailGripColors16.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailGripColors05
            // 
            this.rbtn_RetailGripColors05.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailGripColors05.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailGripColors05.Location = new System.Drawing.Point(156, 7);
            this.rbtn_RetailGripColors05.Name = "rbtn_RetailGripColors05";
            this.rbtn_RetailGripColors05.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailGripColors05.TabIndex = 4;
            this.rbtn_RetailGripColors05.Text = "roundButton5";
            this.rbtn_RetailGripColors05.UseVisualStyleBackColor = false;
            this.rbtn_RetailGripColors05.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailGripColors15
            // 
            this.rbtn_RetailGripColors15.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailGripColors15.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailGripColors15.Location = new System.Drawing.Point(232, 43);
            this.rbtn_RetailGripColors15.Name = "rbtn_RetailGripColors15";
            this.rbtn_RetailGripColors15.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailGripColors15.TabIndex = 14;
            this.rbtn_RetailGripColors15.Text = "roundButton10";
            this.rbtn_RetailGripColors15.UseVisualStyleBackColor = false;
            this.rbtn_RetailGripColors15.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailGripColors06
            // 
            this.rbtn_RetailGripColors06.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailGripColors06.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailGripColors06.Location = new System.Drawing.Point(194, 7);
            this.rbtn_RetailGripColors06.Name = "rbtn_RetailGripColors06";
            this.rbtn_RetailGripColors06.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailGripColors06.TabIndex = 5;
            this.rbtn_RetailGripColors06.Text = "roundButton6";
            this.rbtn_RetailGripColors06.UseVisualStyleBackColor = false;
            this.rbtn_RetailGripColors06.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailGripColors14
            // 
            this.rbtn_RetailGripColors14.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailGripColors14.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailGripColors14.Location = new System.Drawing.Point(194, 43);
            this.rbtn_RetailGripColors14.Name = "rbtn_RetailGripColors14";
            this.rbtn_RetailGripColors14.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailGripColors14.TabIndex = 13;
            this.rbtn_RetailGripColors14.Text = "roundButton11";
            this.rbtn_RetailGripColors14.UseVisualStyleBackColor = false;
            this.rbtn_RetailGripColors14.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailGripColors07
            // 
            this.rbtn_RetailGripColors07.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailGripColors07.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailGripColors07.Location = new System.Drawing.Point(232, 7);
            this.rbtn_RetailGripColors07.Name = "rbtn_RetailGripColors07";
            this.rbtn_RetailGripColors07.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailGripColors07.TabIndex = 6;
            this.rbtn_RetailGripColors07.Text = "roundButton7";
            this.rbtn_RetailGripColors07.UseVisualStyleBackColor = false;
            this.rbtn_RetailGripColors07.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailGripColors13
            // 
            this.rbtn_RetailGripColors13.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailGripColors13.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailGripColors13.Location = new System.Drawing.Point(156, 43);
            this.rbtn_RetailGripColors13.Name = "rbtn_RetailGripColors13";
            this.rbtn_RetailGripColors13.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailGripColors13.TabIndex = 12;
            this.rbtn_RetailGripColors13.Text = "roundButton12";
            this.rbtn_RetailGripColors13.UseVisualStyleBackColor = false;
            this.rbtn_RetailGripColors13.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailGripColors08
            // 
            this.rbtn_RetailGripColors08.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailGripColors08.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailGripColors08.Location = new System.Drawing.Point(270, 7);
            this.rbtn_RetailGripColors08.Name = "rbtn_RetailGripColors08";
            this.rbtn_RetailGripColors08.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailGripColors08.TabIndex = 7;
            this.rbtn_RetailGripColors08.Text = "roundButton8";
            this.rbtn_RetailGripColors08.UseVisualStyleBackColor = false;
            this.rbtn_RetailGripColors08.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailGripColors12
            // 
            this.rbtn_RetailGripColors12.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailGripColors12.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailGripColors12.Location = new System.Drawing.Point(118, 43);
            this.rbtn_RetailGripColors12.Name = "rbtn_RetailGripColors12";
            this.rbtn_RetailGripColors12.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailGripColors12.TabIndex = 11;
            this.rbtn_RetailGripColors12.Text = "roundButton13";
            this.rbtn_RetailGripColors12.UseVisualStyleBackColor = false;
            this.rbtn_RetailGripColors12.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailGripColors09
            // 
            this.rbtn_RetailGripColors09.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailGripColors09.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailGripColors09.Location = new System.Drawing.Point(4, 43);
            this.rbtn_RetailGripColors09.Name = "rbtn_RetailGripColors09";
            this.rbtn_RetailGripColors09.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailGripColors09.TabIndex = 8;
            this.rbtn_RetailGripColors09.Text = "roundButton16";
            this.rbtn_RetailGripColors09.UseVisualStyleBackColor = false;
            this.rbtn_RetailGripColors09.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailGripColors11
            // 
            this.rbtn_RetailGripColors11.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailGripColors11.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailGripColors11.Location = new System.Drawing.Point(80, 43);
            this.rbtn_RetailGripColors11.Name = "rbtn_RetailGripColors11";
            this.rbtn_RetailGripColors11.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailGripColors11.TabIndex = 10;
            this.rbtn_RetailGripColors11.Text = "roundButton14";
            this.rbtn_RetailGripColors11.UseVisualStyleBackColor = false;
            this.rbtn_RetailGripColors11.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailGripColors10
            // 
            this.rbtn_RetailGripColors10.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailGripColors10.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailGripColors10.Location = new System.Drawing.Point(42, 43);
            this.rbtn_RetailGripColors10.Name = "rbtn_RetailGripColors10";
            this.rbtn_RetailGripColors10.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailGripColors10.TabIndex = 9;
            this.rbtn_RetailGripColors10.Text = "roundButton15";
            this.rbtn_RetailGripColors10.UseVisualStyleBackColor = false;
            this.rbtn_RetailGripColors10.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // panel_RetailColors
            // 
            this.panel_RetailColors.Controls.Add(this.rbtn_RetailColors04);
            this.panel_RetailColors.Controls.Add(this.rbtn_RetailColors01);
            this.panel_RetailColors.Controls.Add(this.rbtn_RetailColors02);
            this.panel_RetailColors.Controls.Add(this.rbtn_RetailColors03);
            this.panel_RetailColors.Controls.Add(this.rbtn_RetailColors16);
            this.panel_RetailColors.Controls.Add(this.rbtn_RetailColors05);
            this.panel_RetailColors.Controls.Add(this.rbtn_RetailColors15);
            this.panel_RetailColors.Controls.Add(this.rbtn_RetailColors06);
            this.panel_RetailColors.Controls.Add(this.rbtn_RetailColors14);
            this.panel_RetailColors.Controls.Add(this.rbtn_RetailColors07);
            this.panel_RetailColors.Controls.Add(this.rbtn_RetailColors13);
            this.panel_RetailColors.Controls.Add(this.rbtn_RetailColors08);
            this.panel_RetailColors.Controls.Add(this.rbtn_RetailColors12);
            this.panel_RetailColors.Controls.Add(this.rbtn_RetailColors09);
            this.panel_RetailColors.Controls.Add(this.rbtn_RetailColors11);
            this.panel_RetailColors.Controls.Add(this.rbtn_RetailColors10);
            this.panel_RetailColors.Location = new System.Drawing.Point(3, 22);
            this.panel_RetailColors.Name = "panel_RetailColors";
            this.panel_RetailColors.Size = new System.Drawing.Size(307, 82);
            this.panel_RetailColors.TabIndex = 68;
            // 
            // panel_RetailUserColors
            // 
            this.panel_RetailUserColors.Controls.Add(this.rbtn_RetailColors17);
            this.panel_RetailUserColors.Controls.Add(this.rbtn_RetailColors18);
            this.panel_RetailUserColors.Controls.Add(this.rbtn_RetailColors19);
            this.panel_RetailUserColors.Controls.Add(this.rbtn_RetailColors20);
            this.panel_RetailUserColors.Controls.Add(this.rbtn_RetailColors21);
            this.panel_RetailUserColors.Controls.Add(this.rbtn_RetailColors22);
            this.panel_RetailUserColors.Controls.Add(this.rbtn_RetailColors23);
            this.panel_RetailUserColors.Controls.Add(this.rbtn_RetailColors24);
            this.panel_RetailUserColors.Controls.Add(this.rbtn_RetailColors25);
            this.panel_RetailUserColors.Controls.Add(this.rbtn_RetailColors26);
            this.panel_RetailUserColors.Controls.Add(this.rbtn_RetailColors27);
            this.panel_RetailUserColors.Controls.Add(this.rbtn_RetailColors28);
            this.panel_RetailUserColors.Controls.Add(this.rbtn_RetailColors29);
            this.panel_RetailUserColors.Controls.Add(this.rbtn_RetailColors30);
            this.panel_RetailUserColors.Controls.Add(this.rbtn_RetailColors31);
            this.panel_RetailUserColors.Controls.Add(this.rbtn_RetailColors32);
            this.panel_RetailUserColors.Location = new System.Drawing.Point(3, 105);
            this.panel_RetailUserColors.Name = "panel_RetailUserColors";
            this.panel_RetailUserColors.Size = new System.Drawing.Size(307, 87);
            this.panel_RetailUserColors.TabIndex = 67;
            // 
            // rbtn_RetailColors17
            // 
            this.rbtn_RetailColors17.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors17.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors17.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors17.Location = new System.Drawing.Point(4, 7);
            this.rbtn_RetailColors17.Name = "rbtn_RetailColors17";
            this.rbtn_RetailColors17.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors17.TabIndex = 16;
            this.rbtn_RetailColors17.Text = "roundButton16";
            this.rbtn_RetailColors17.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors17.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors18
            // 
            this.rbtn_RetailColors18.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors18.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors18.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors18.Location = new System.Drawing.Point(42, 7);
            this.rbtn_RetailColors18.Name = "rbtn_RetailColors18";
            this.rbtn_RetailColors18.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors18.TabIndex = 17;
            this.rbtn_RetailColors18.Text = "roundButton16";
            this.rbtn_RetailColors18.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors18.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors19
            // 
            this.rbtn_RetailColors19.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors19.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors19.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors19.Location = new System.Drawing.Point(80, 7);
            this.rbtn_RetailColors19.Name = "rbtn_RetailColors19";
            this.rbtn_RetailColors19.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors19.TabIndex = 18;
            this.rbtn_RetailColors19.Text = "roundButton16";
            this.rbtn_RetailColors19.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors19.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors20
            // 
            this.rbtn_RetailColors20.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors20.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors20.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors20.Location = new System.Drawing.Point(118, 7);
            this.rbtn_RetailColors20.Name = "rbtn_RetailColors20";
            this.rbtn_RetailColors20.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors20.TabIndex = 19;
            this.rbtn_RetailColors20.Text = "roundButton16";
            this.rbtn_RetailColors20.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors20.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors21
            // 
            this.rbtn_RetailColors21.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors21.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors21.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors21.Location = new System.Drawing.Point(156, 7);
            this.rbtn_RetailColors21.Name = "rbtn_RetailColors21";
            this.rbtn_RetailColors21.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors21.TabIndex = 20;
            this.rbtn_RetailColors21.Text = "roundButton16";
            this.rbtn_RetailColors21.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors21.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors22
            // 
            this.rbtn_RetailColors22.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors22.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors22.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors22.Location = new System.Drawing.Point(194, 7);
            this.rbtn_RetailColors22.Name = "rbtn_RetailColors22";
            this.rbtn_RetailColors22.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors22.TabIndex = 21;
            this.rbtn_RetailColors22.Text = "roundButton16";
            this.rbtn_RetailColors22.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors22.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors23
            // 
            this.rbtn_RetailColors23.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors23.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors23.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors23.Location = new System.Drawing.Point(232, 7);
            this.rbtn_RetailColors23.Name = "rbtn_RetailColors23";
            this.rbtn_RetailColors23.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors23.TabIndex = 22;
            this.rbtn_RetailColors23.Text = "roundButton16";
            this.rbtn_RetailColors23.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors23.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors24
            // 
            this.rbtn_RetailColors24.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors24.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors24.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors24.Location = new System.Drawing.Point(271, 7);
            this.rbtn_RetailColors24.Name = "rbtn_RetailColors24";
            this.rbtn_RetailColors24.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors24.TabIndex = 23;
            this.rbtn_RetailColors24.Text = "roundButton16";
            this.rbtn_RetailColors24.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors24.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors25
            // 
            this.rbtn_RetailColors25.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors25.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors25.Location = new System.Drawing.Point(4, 43);
            this.rbtn_RetailColors25.Name = "rbtn_RetailColors25";
            this.rbtn_RetailColors25.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors25.TabIndex = 24;
            this.rbtn_RetailColors25.Text = "roundButton16";
            this.rbtn_RetailColors25.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors25.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors26
            // 
            this.rbtn_RetailColors26.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors26.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors26.Location = new System.Drawing.Point(42, 43);
            this.rbtn_RetailColors26.Name = "rbtn_RetailColors26";
            this.rbtn_RetailColors26.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors26.TabIndex = 25;
            this.rbtn_RetailColors26.Text = "roundButton16";
            this.rbtn_RetailColors26.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors26.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors27
            // 
            this.rbtn_RetailColors27.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors27.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors27.Location = new System.Drawing.Point(80, 43);
            this.rbtn_RetailColors27.Name = "rbtn_RetailColors27";
            this.rbtn_RetailColors27.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors27.TabIndex = 26;
            this.rbtn_RetailColors27.Text = "roundButton16";
            this.rbtn_RetailColors27.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors27.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors28
            // 
            this.rbtn_RetailColors28.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors28.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors28.Location = new System.Drawing.Point(118, 43);
            this.rbtn_RetailColors28.Name = "rbtn_RetailColors28";
            this.rbtn_RetailColors28.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors28.TabIndex = 27;
            this.rbtn_RetailColors28.Text = "roundButton16";
            this.rbtn_RetailColors28.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors28.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors29
            // 
            this.rbtn_RetailColors29.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors29.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors29.Location = new System.Drawing.Point(156, 43);
            this.rbtn_RetailColors29.Name = "rbtn_RetailColors29";
            this.rbtn_RetailColors29.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors29.TabIndex = 28;
            this.rbtn_RetailColors29.Text = "roundButton16";
            this.rbtn_RetailColors29.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors29.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors30
            // 
            this.rbtn_RetailColors30.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors30.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors30.Location = new System.Drawing.Point(194, 43);
            this.rbtn_RetailColors30.Name = "rbtn_RetailColors30";
            this.rbtn_RetailColors30.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors30.TabIndex = 29;
            this.rbtn_RetailColors30.Text = "roundButton16";
            this.rbtn_RetailColors30.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors30.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors31
            // 
            this.rbtn_RetailColors31.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors31.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors31.Location = new System.Drawing.Point(232, 43);
            this.rbtn_RetailColors31.Name = "rbtn_RetailColors31";
            this.rbtn_RetailColors31.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors31.TabIndex = 30;
            this.rbtn_RetailColors31.Text = "roundButton16";
            this.rbtn_RetailColors31.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors31.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_RetailColors32
            // 
            this.rbtn_RetailColors32.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_RetailColors32.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_RetailColors32.Location = new System.Drawing.Point(271, 43);
            this.rbtn_RetailColors32.Name = "rbtn_RetailColors32";
            this.rbtn_RetailColors32.Size = new System.Drawing.Size(32, 32);
            this.rbtn_RetailColors32.TabIndex = 31;
            this.rbtn_RetailColors32.Text = "roundButton16";
            this.rbtn_RetailColors32.UseVisualStyleBackColor = false;
            this.rbtn_RetailColors32.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // panel_UserColors
            // 
            this.panel_UserColors.Controls.Add(this.m_lbl_UserColors);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors01);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors02);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors03);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors04);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors05);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors06);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors07);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors08);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors09);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors10);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors11);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors12);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors13);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors14);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors15);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors16);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors17);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors18);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors19);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors20);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors21);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors22);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors23);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors24);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors25);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors26);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors27);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors28);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors29);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors30);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors31);
            this.panel_UserColors.Controls.Add(this.rbtn_UserColors32);
            this.panel_UserColors.Location = new System.Drawing.Point(3, 195);
            this.panel_UserColors.Name = "panel_UserColors";
            this.panel_UserColors.Size = new System.Drawing.Size(307, 188);
            this.panel_UserColors.TabIndex = 66;
            // 
            // m_lbl_UserColors
            // 
            this.m_lbl_UserColors.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_lbl_UserColors.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(251)))), ((int)(((byte)(251)))), ((int)(((byte)(251)))));
            this.m_lbl_UserColors.Location = new System.Drawing.Point(5, 4);
            this.m_lbl_UserColors.Name = "m_lbl_UserColors";
            this.m_lbl_UserColors.Size = new System.Drawing.Size(260, 20);
            this.m_lbl_UserColors.TabIndex = 65;
            this.m_lbl_UserColors.Text = "Custom Color Sets:";
            // 
            // rbtn_UserColors01
            // 
            this.rbtn_UserColors01.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors01.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors01.Location = new System.Drawing.Point(4, 29);
            this.rbtn_UserColors01.Name = "rbtn_UserColors01";
            this.rbtn_UserColors01.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors01.TabIndex = 32;
            this.rbtn_UserColors01.Text = "roundButton16";
            this.rbtn_UserColors01.UseVisualStyleBackColor = false;
            this.rbtn_UserColors01.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors02
            // 
            this.rbtn_UserColors02.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors02.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors02.Location = new System.Drawing.Point(42, 29);
            this.rbtn_UserColors02.Name = "rbtn_UserColors02";
            this.rbtn_UserColors02.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors02.TabIndex = 33;
            this.rbtn_UserColors02.Text = "roundButton16";
            this.rbtn_UserColors02.UseVisualStyleBackColor = false;
            this.rbtn_UserColors02.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors03
            // 
            this.rbtn_UserColors03.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors03.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors03.Location = new System.Drawing.Point(80, 29);
            this.rbtn_UserColors03.Name = "rbtn_UserColors03";
            this.rbtn_UserColors03.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors03.TabIndex = 34;
            this.rbtn_UserColors03.Text = "roundButton16";
            this.rbtn_UserColors03.UseVisualStyleBackColor = false;
            this.rbtn_UserColors03.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors04
            // 
            this.rbtn_UserColors04.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors04.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors04.Location = new System.Drawing.Point(118, 29);
            this.rbtn_UserColors04.Name = "rbtn_UserColors04";
            this.rbtn_UserColors04.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors04.TabIndex = 35;
            this.rbtn_UserColors04.Text = "roundButton16";
            this.rbtn_UserColors04.UseVisualStyleBackColor = false;
            this.rbtn_UserColors04.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors05
            // 
            this.rbtn_UserColors05.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors05.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors05.Location = new System.Drawing.Point(156, 29);
            this.rbtn_UserColors05.Name = "rbtn_UserColors05";
            this.rbtn_UserColors05.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors05.TabIndex = 36;
            this.rbtn_UserColors05.Text = "roundButton16";
            this.rbtn_UserColors05.UseVisualStyleBackColor = false;
            this.rbtn_UserColors05.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors06
            // 
            this.rbtn_UserColors06.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors06.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors06.Location = new System.Drawing.Point(194, 29);
            this.rbtn_UserColors06.Name = "rbtn_UserColors06";
            this.rbtn_UserColors06.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors06.TabIndex = 37;
            this.rbtn_UserColors06.Text = "roundButton16";
            this.rbtn_UserColors06.UseVisualStyleBackColor = false;
            this.rbtn_UserColors06.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors07
            // 
            this.rbtn_UserColors07.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors07.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors07.Location = new System.Drawing.Point(232, 29);
            this.rbtn_UserColors07.Name = "rbtn_UserColors07";
            this.rbtn_UserColors07.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors07.TabIndex = 38;
            this.rbtn_UserColors07.Text = "roundButton16";
            this.rbtn_UserColors07.UseVisualStyleBackColor = false;
            this.rbtn_UserColors07.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors08
            // 
            this.rbtn_UserColors08.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors08.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors08.Location = new System.Drawing.Point(270, 29);
            this.rbtn_UserColors08.Name = "rbtn_UserColors08";
            this.rbtn_UserColors08.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors08.TabIndex = 39;
            this.rbtn_UserColors08.Text = "roundButton16";
            this.rbtn_UserColors08.UseVisualStyleBackColor = false;
            this.rbtn_UserColors08.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors09
            // 
            this.rbtn_UserColors09.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors09.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors09.Location = new System.Drawing.Point(4, 112);
            this.rbtn_UserColors09.Name = "rbtn_UserColors09";
            this.rbtn_UserColors09.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors09.TabIndex = 48;
            this.rbtn_UserColors09.Text = "roundButton16";
            this.rbtn_UserColors09.UseVisualStyleBackColor = false;
            this.rbtn_UserColors09.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors10
            // 
            this.rbtn_UserColors10.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors10.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors10.Location = new System.Drawing.Point(42, 112);
            this.rbtn_UserColors10.Name = "rbtn_UserColors10";
            this.rbtn_UserColors10.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors10.TabIndex = 49;
            this.rbtn_UserColors10.Text = "roundButton16";
            this.rbtn_UserColors10.UseVisualStyleBackColor = false;
            this.rbtn_UserColors10.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors11
            // 
            this.rbtn_UserColors11.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors11.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors11.Location = new System.Drawing.Point(80, 112);
            this.rbtn_UserColors11.Name = "rbtn_UserColors11";
            this.rbtn_UserColors11.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors11.TabIndex = 50;
            this.rbtn_UserColors11.Text = "roundButton16";
            this.rbtn_UserColors11.UseVisualStyleBackColor = false;
            this.rbtn_UserColors11.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors12
            // 
            this.rbtn_UserColors12.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors12.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors12.Location = new System.Drawing.Point(118, 112);
            this.rbtn_UserColors12.Name = "rbtn_UserColors12";
            this.rbtn_UserColors12.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors12.TabIndex = 51;
            this.rbtn_UserColors12.Text = "roundButton16";
            this.rbtn_UserColors12.UseVisualStyleBackColor = false;
            this.rbtn_UserColors12.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors13
            // 
            this.rbtn_UserColors13.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors13.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors13.Location = new System.Drawing.Point(156, 112);
            this.rbtn_UserColors13.Name = "rbtn_UserColors13";
            this.rbtn_UserColors13.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors13.TabIndex = 52;
            this.rbtn_UserColors13.Text = "roundButton16";
            this.rbtn_UserColors13.UseVisualStyleBackColor = false;
            this.rbtn_UserColors13.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors14
            // 
            this.rbtn_UserColors14.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors14.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors14.Location = new System.Drawing.Point(194, 112);
            this.rbtn_UserColors14.Name = "rbtn_UserColors14";
            this.rbtn_UserColors14.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors14.TabIndex = 53;
            this.rbtn_UserColors14.Text = "roundButton16";
            this.rbtn_UserColors14.UseVisualStyleBackColor = false;
            this.rbtn_UserColors14.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors15
            // 
            this.rbtn_UserColors15.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors15.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors15.Location = new System.Drawing.Point(232, 112);
            this.rbtn_UserColors15.Name = "rbtn_UserColors15";
            this.rbtn_UserColors15.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors15.TabIndex = 54;
            this.rbtn_UserColors15.Text = "roundButton16";
            this.rbtn_UserColors15.UseVisualStyleBackColor = false;
            this.rbtn_UserColors15.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors16
            // 
            this.rbtn_UserColors16.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors16.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors16.Location = new System.Drawing.Point(270, 112);
            this.rbtn_UserColors16.Name = "rbtn_UserColors16";
            this.rbtn_UserColors16.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors16.TabIndex = 55;
            this.rbtn_UserColors16.Text = "roundButton16";
            this.rbtn_UserColors16.UseVisualStyleBackColor = false;
            this.rbtn_UserColors16.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors17
            // 
            this.rbtn_UserColors17.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors17.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors17.Location = new System.Drawing.Point(4, 65);
            this.rbtn_UserColors17.Name = "rbtn_UserColors17";
            this.rbtn_UserColors17.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors17.TabIndex = 40;
            this.rbtn_UserColors17.Text = "roundButton16";
            this.rbtn_UserColors17.UseVisualStyleBackColor = false;
            this.rbtn_UserColors17.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors18
            // 
            this.rbtn_UserColors18.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors18.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors18.Location = new System.Drawing.Point(42, 65);
            this.rbtn_UserColors18.Name = "rbtn_UserColors18";
            this.rbtn_UserColors18.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors18.TabIndex = 41;
            this.rbtn_UserColors18.Text = "roundButton16";
            this.rbtn_UserColors18.UseVisualStyleBackColor = false;
            this.rbtn_UserColors18.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors19
            // 
            this.rbtn_UserColors19.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors19.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors19.Location = new System.Drawing.Point(80, 65);
            this.rbtn_UserColors19.Name = "rbtn_UserColors19";
            this.rbtn_UserColors19.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors19.TabIndex = 42;
            this.rbtn_UserColors19.Text = "roundButton16";
            this.rbtn_UserColors19.UseVisualStyleBackColor = false;
            this.rbtn_UserColors19.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors20
            // 
            this.rbtn_UserColors20.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors20.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors20.Location = new System.Drawing.Point(118, 65);
            this.rbtn_UserColors20.Name = "rbtn_UserColors20";
            this.rbtn_UserColors20.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors20.TabIndex = 43;
            this.rbtn_UserColors20.Text = "roundButton16";
            this.rbtn_UserColors20.UseVisualStyleBackColor = false;
            this.rbtn_UserColors20.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors21
            // 
            this.rbtn_UserColors21.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors21.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors21.Location = new System.Drawing.Point(156, 65);
            this.rbtn_UserColors21.Name = "rbtn_UserColors21";
            this.rbtn_UserColors21.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors21.TabIndex = 44;
            this.rbtn_UserColors21.Text = "roundButton16";
            this.rbtn_UserColors21.UseVisualStyleBackColor = false;
            this.rbtn_UserColors21.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors22
            // 
            this.rbtn_UserColors22.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors22.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors22.Location = new System.Drawing.Point(194, 65);
            this.rbtn_UserColors22.Name = "rbtn_UserColors22";
            this.rbtn_UserColors22.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors22.TabIndex = 45;
            this.rbtn_UserColors22.Text = "roundButton16";
            this.rbtn_UserColors22.UseVisualStyleBackColor = false;
            this.rbtn_UserColors22.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors23
            // 
            this.rbtn_UserColors23.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors23.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors23.Location = new System.Drawing.Point(232, 65);
            this.rbtn_UserColors23.Name = "rbtn_UserColors23";
            this.rbtn_UserColors23.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors23.TabIndex = 46;
            this.rbtn_UserColors23.Text = "roundButton16";
            this.rbtn_UserColors23.UseVisualStyleBackColor = false;
            this.rbtn_UserColors23.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors24
            // 
            this.rbtn_UserColors24.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors24.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors24.Location = new System.Drawing.Point(270, 65);
            this.rbtn_UserColors24.Name = "rbtn_UserColors24";
            this.rbtn_UserColors24.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors24.TabIndex = 47;
            this.rbtn_UserColors24.Text = "roundButton16";
            this.rbtn_UserColors24.UseVisualStyleBackColor = false;
            this.rbtn_UserColors24.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors25
            // 
            this.rbtn_UserColors25.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors25.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors25.Location = new System.Drawing.Point(4, 148);
            this.rbtn_UserColors25.Name = "rbtn_UserColors25";
            this.rbtn_UserColors25.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors25.TabIndex = 56;
            this.rbtn_UserColors25.Text = "roundButton16";
            this.rbtn_UserColors25.UseVisualStyleBackColor = false;
            this.rbtn_UserColors25.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors26
            // 
            this.rbtn_UserColors26.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors26.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors26.Location = new System.Drawing.Point(42, 148);
            this.rbtn_UserColors26.Name = "rbtn_UserColors26";
            this.rbtn_UserColors26.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors26.TabIndex = 57;
            this.rbtn_UserColors26.Text = "roundButton16";
            this.rbtn_UserColors26.UseVisualStyleBackColor = false;
            this.rbtn_UserColors26.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors27
            // 
            this.rbtn_UserColors27.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors27.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors27.Location = new System.Drawing.Point(80, 148);
            this.rbtn_UserColors27.Name = "rbtn_UserColors27";
            this.rbtn_UserColors27.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors27.TabIndex = 58;
            this.rbtn_UserColors27.Text = "roundButton16";
            this.rbtn_UserColors27.UseVisualStyleBackColor = false;
            this.rbtn_UserColors27.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors28
            // 
            this.rbtn_UserColors28.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors28.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors28.Location = new System.Drawing.Point(118, 148);
            this.rbtn_UserColors28.Name = "rbtn_UserColors28";
            this.rbtn_UserColors28.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors28.TabIndex = 59;
            this.rbtn_UserColors28.Text = "roundButton16";
            this.rbtn_UserColors28.UseVisualStyleBackColor = false;
            this.rbtn_UserColors28.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors29
            // 
            this.rbtn_UserColors29.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors29.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors29.Location = new System.Drawing.Point(156, 148);
            this.rbtn_UserColors29.Name = "rbtn_UserColors29";
            this.rbtn_UserColors29.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors29.TabIndex = 60;
            this.rbtn_UserColors29.Text = "roundButton16";
            this.rbtn_UserColors29.UseVisualStyleBackColor = false;
            this.rbtn_UserColors29.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors30
            // 
            this.rbtn_UserColors30.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors30.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors30.Location = new System.Drawing.Point(194, 148);
            this.rbtn_UserColors30.Name = "rbtn_UserColors30";
            this.rbtn_UserColors30.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors30.TabIndex = 61;
            this.rbtn_UserColors30.Text = "roundButton16";
            this.rbtn_UserColors30.UseVisualStyleBackColor = false;
            this.rbtn_UserColors30.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors31
            // 
            this.rbtn_UserColors31.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors31.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors31.Location = new System.Drawing.Point(231, 148);
            this.rbtn_UserColors31.Name = "rbtn_UserColors31";
            this.rbtn_UserColors31.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors31.TabIndex = 62;
            this.rbtn_UserColors31.Text = "roundButton16";
            this.rbtn_UserColors31.UseVisualStyleBackColor = false;
            this.rbtn_UserColors31.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // rbtn_UserColors32
            // 
            this.rbtn_UserColors32.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.rbtn_UserColors32.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.rbtn_UserColors32.Location = new System.Drawing.Point(270, 148);
            this.rbtn_UserColors32.Name = "rbtn_UserColors32";
            this.rbtn_UserColors32.Size = new System.Drawing.Size(32, 32);
            this.rbtn_UserColors32.TabIndex = 63;
            this.rbtn_UserColors32.Text = "roundButton16";
            this.rbtn_UserColors32.UseVisualStyleBackColor = false;
            this.rbtn_UserColors32.Click += new System.EventHandler(this.m_lbl_Preset_Click);
            // 
            // m_lbl_RetailColors
            // 
            this.m_lbl_RetailColors.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_lbl_RetailColors.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(251)))), ((int)(((byte)(251)))), ((int)(((byte)(251)))));
            this.m_lbl_RetailColors.Location = new System.Drawing.Point(8, 4);
            this.m_lbl_RetailColors.Name = "m_lbl_RetailColors";
            this.m_lbl_RetailColors.Size = new System.Drawing.Size(260, 20);
            this.m_lbl_RetailColors.TabIndex = 59;
            this.m_lbl_RetailColors.Text = "Retail Color Sets:";
            // 
            // btn_Update
            // 
            this.btn_Update.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.btn_Update.FlatAppearance.BorderSize = 0;
            this.btn_Update.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.btn_Update.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.btn_Update.Location = new System.Drawing.Point(664, 19);
            this.btn_Update.Name = "btn_Update";
            this.btn_Update.Size = new System.Drawing.Size(55, 26);
            this.btn_Update.TabIndex = 57;
            this.btn_Update.Text = "Update";
            this.btn_Update.UseVisualStyleBackColor = false;
            this.btn_Update.Click += new System.EventHandler(this.SetPresetColorName);
            // 
            // m_lbl_SelectPreset
            // 
            this.m_lbl_SelectPreset.Font = new System.Drawing.Font("Segoe UI Semibold", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.m_lbl_SelectPreset.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(251)))), ((int)(((byte)(251)))), ((int)(((byte)(251)))));
            this.m_lbl_SelectPreset.Location = new System.Drawing.Point(411, 6);
            this.m_lbl_SelectPreset.Name = "m_lbl_SelectPreset";
            this.m_lbl_SelectPreset.Size = new System.Drawing.Size(101, 20);
            this.m_lbl_SelectPreset.TabIndex = 58;
            this.m_lbl_SelectPreset.Text = "Select Preset:";
            // 
            // btn_Clear
            // 
            this.btn_Clear.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.btn_Clear.FlatAppearance.BorderSize = 0;
            this.btn_Clear.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.btn_Clear.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.btn_Clear.ForeColor = System.Drawing.Color.OrangeRed;
            this.btn_Clear.Location = new System.Drawing.Point(603, 19);
            this.btn_Clear.Name = "btn_Clear";
            this.btn_Clear.Size = new System.Drawing.Size(55, 26);
            this.btn_Clear.TabIndex = 60;
            this.btn_Clear.Text = "Clear";
            this.btn_Clear.UseVisualStyleBackColor = false;
            this.btn_Clear.Click += new System.EventHandler(this.ClearPreset);
            // 
            // m_ctrl_BigBox
            // 
            this.m_ctrl_BigBox.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(70)))), ((int)(((byte)(70)))), ((int)(((byte)(70)))));
            this.m_ctrl_BigBox.CausesValidation = false;
            this.m_ctrl_BigBox.DrawStyle = jcColor.ctrl2DColorBox.eDrawStyle.Hue;
            this.m_ctrl_BigBox.ForeColor = System.Drawing.Color.DeepSkyBlue;
            hsl1.H = 0D;
            hsl1.L = 1D;
            hsl1.S = 1D;
            this.m_ctrl_BigBox.HSL = hsl1;
            this.m_ctrl_BigBox.Location = new System.Drawing.Point(11, 26);
            this.m_ctrl_BigBox.Name = "m_ctrl_BigBox";
            this.m_ctrl_BigBox.RGB = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.m_ctrl_BigBox.Size = new System.Drawing.Size(260, 260);
            this.m_ctrl_BigBox.TabIndex = 39;
            this.m_ctrl_BigBox.Scroll += new jcColor.EventHandler(this.m_ctrl_BigBox_Scroll);
            // 
            // m_ctrl_ThinBox
            // 
            this.m_ctrl_ThinBox.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(70)))), ((int)(((byte)(70)))), ((int)(((byte)(70)))));
            this.m_ctrl_ThinBox.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
            this.m_ctrl_ThinBox.CausesValidation = false;
            this.m_ctrl_ThinBox.DrawStyle = jcColor.ctrlVerticalColorSlider.eDrawStyle.Hue;
            this.m_ctrl_ThinBox.ForeColor = System.Drawing.Color.DeepSkyBlue;
            hsl2.H = 0D;
            hsl2.L = 1D;
            hsl2.S = 1D;
            this.m_ctrl_ThinBox.HSL = hsl2;
            this.m_ctrl_ThinBox.Location = new System.Drawing.Point(272, 24);
            this.m_ctrl_ThinBox.Name = "m_ctrl_ThinBox";
            this.m_ctrl_ThinBox.RGB = System.Drawing.Color.Red;
            this.m_ctrl_ThinBox.Size = new System.Drawing.Size(40, 264);
            this.m_ctrl_ThinBox.TabIndex = 38;
            this.m_ctrl_ThinBox.Scroll += new jcColor.EventHandler(this.m_ctrl_ThinBox_Scroll);
            // 
            // btn_switchGripsPanel
            // 
            this.btn_switchGripsPanel.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.btn_switchGripsPanel.FlatAppearance.BorderSize = 0;
            this.btn_switchGripsPanel.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.btn_switchGripsPanel.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(161)));
            this.btn_switchGripsPanel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(188)))), ((int)(((byte)(0)))));
            this.btn_switchGripsPanel.Location = new System.Drawing.Point(542, 19);
            this.btn_switchGripsPanel.Name = "btn_switchGripsPanel";
            this.btn_switchGripsPanel.Size = new System.Drawing.Size(55, 26);
            this.btn_switchGripsPanel.TabIndex = 61;
            this.btn_switchGripsPanel.Text = "Grips";
            this.btn_switchGripsPanel.UseVisualStyleBackColor = false;
            this.btn_switchGripsPanel.Click += new System.EventHandler(this.btn_switchGripsPanel_Click);
            // 
            // JoyConColorPicker
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Inherit;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(70)))), ((int)(((byte)(70)))), ((int)(((byte)(70)))));
            this.ClientSize = new System.Drawing.Size(727, 424);
            this.ControlBox = false;
            this.Controls.Add(this.btn_switchGripsPanel);
            this.Controls.Add(this.btn_Clear);
            this.Controls.Add(this.m_lbl_SelectPreset);
            this.Controls.Add(this.btn_Update);
            this.Controls.Add(this.panel2);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.m_lbl_Screen_Picker);
            this.Controls.Add(this.m_lbl_Black_Symbol);
            this.Controls.Add(this.m_lbl_Saturation_Symbol);
            this.Controls.Add(this.m_lbl_Hue_Symbol);
            this.Controls.Add(this.m_ctrl_BigBox);
            this.Controls.Add(this.m_ctrl_ThinBox);
            this.Controls.Add(this.m_lbl_HexPound);
            this.Controls.Add(this.m_txt_Hex);
            this.Controls.Add(this.m_txt_Blue);
            this.Controls.Add(this.m_txt_Green);
            this.Controls.Add(this.m_txt_Red);
            this.Controls.Add(this.m_txt_Black);
            this.Controls.Add(this.m_txt_Sat);
            this.Controls.Add(this.m_txt_Hue);
            this.Controls.Add(this.m_rbtn_Blue);
            this.Controls.Add(this.m_rbtn_Green);
            this.Controls.Add(this.m_rbtn_Red);
            this.Controls.Add(this.m_rbtn_Black);
            this.Controls.Add(this.m_rbtn_Sat);
            this.Controls.Add(this.m_rbtn_Hue);
            this.Controls.Add(this.m_cmd_Cancel);
            this.Controls.Add(this.m_cmd_OK);
            this.Controls.Add(this.m_lbl_SelectColor);
            this.Controls.Add(this.m_eyedropColorPicker);
            this.DoubleBuffered = true;
            this.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(9)))), ((int)(((byte)(255)))), ((int)(((byte)(206)))));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "JoyConColorPicker";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Color Picker";
            this.Load += new System.EventHandler(this.frmColorPicker_Load);
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.panel2.ResumeLayout(false);
            this.panel_RetailGripColors.ResumeLayout(false);
            this.panel_RetailColors.ResumeLayout(false);
            this.panel_RetailUserColors.ResumeLayout(false);
            this.panel_UserColors.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }


        #endregion

        #region Events

        #region General Events

        /// <summary>
        /// Runs when the form is loaded
        /// </summary>
        private void frmColorPicker_Load(object sender, System.EventArgs e) {
            System.Xml.XmlDocument olddoc = new System.Xml.XmlDocument();
            try {
                olddoc.Load(System.IO.Path.GetDirectoryName(Application.ExecutablePath) + "\\Colors.config");
            }
            catch {
                getCustomColorFromConfig();
                return;
            }
            // Old format config found. Create new file format colors.xml, load old colors and save them.
            // Order of functions is important!
            getCustomColorFromConfig();
            convertOldFormatCustomConfig();
            saveCustomColorToConfig();
        }


        public void loadGripsPanel() {
            if (gripsColor) {
                this.m_radio_btn_Body.Text = "Grip (L)";
                this.m_radio_btn_Buttons.Text = "Grip (R)";
                showGripsPanel = true;
                this.btn_switchGripsPanel.Text = "Retail";
            }
            else {
                this.panel2.Controls.Remove(this.panel_RetailGripColors);
                showGripsPanel = false;
            }
        }


        private void m_cmd_OK_Click(object sender, System.EventArgs e) {
            saveCustomColorToConfig();
            this.DialogResult = DialogResult.OK;
            this.Close();
        }


        private void m_cmd_Cancel_Click(object sender, System.EventArgs e) {
            saveCustomColorToConfig();
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }


        #endregion

        #region Primary Picture Box (m_ctrl_BigBox)

        private void m_ctrl_BigBox_Scroll(object sender, System.EventArgs e) {
            m_hsl = m_ctrl_BigBox.HSL;
            m_rgb = AdobeColors.HSL_to_RGB(m_hsl);

            m_txt_Hue.Text = Round(m_hsl.H * 360).ToString();
            m_txt_Sat.Text = Round(m_hsl.S * 100).ToString();
            m_txt_Black.Text = Round(m_hsl.L * 100).ToString();
            m_txt_Red.Text = m_rgb.R.ToString();
            m_txt_Green.Text = m_rgb.G.ToString();
            m_txt_Blue.Text = m_rgb.B.ToString();

            m_txt_Hue.Update();
            m_txt_Sat.Update();
            m_txt_Black.Update();
            m_txt_Red.Update();
            m_txt_Green.Update();
            m_txt_Blue.Update();

            m_ctrl_ThinBox.HSL = m_hsl;

            SetColorBasedOnType(m_rgb);

            WriteHexData(m_rgb);
        }


        #endregion

        #region Secondary Picture Box (m_ctrl_ThinBox)

        private void m_ctrl_ThinBox_Scroll(object sender, System.EventArgs e) {
            m_hsl = m_ctrl_ThinBox.HSL;
            m_rgb = AdobeColors.HSL_to_RGB(m_hsl);

            m_txt_Hue.Text = Round(m_hsl.H * 360).ToString();
            m_txt_Sat.Text = Round(m_hsl.S * 100).ToString();
            m_txt_Black.Text = Round(m_hsl.L * 100).ToString();
            m_txt_Red.Text = m_rgb.R.ToString();
            m_txt_Green.Text = m_rgb.G.ToString();
            m_txt_Blue.Text = m_rgb.B.ToString();

            m_txt_Hue.Update();
            m_txt_Sat.Update();
            m_txt_Black.Update();
            m_txt_Red.Update();
            m_txt_Green.Update();
            m_txt_Blue.Update();

            m_ctrl_BigBox.HSL = m_hsl;

            SetColorBasedOnType(m_rgb);

            WriteHexData(m_rgb);
        }


        #endregion

        #region Hex Box (m_txt_Hex)

        private void Validate_Color_HexValue(object sender, KeyEventArgs e) {
            string text = m_txt_Hex.Text.ToUpper();

            if (text.Length <= 0) {
                WriteHexData(m_rgb);
                return;
            }
            foreach (char letter in text) {
                if (!char.IsNumber(letter)) {
                    if (letter < 'A' || letter > 'F') {
                        WriteHexData(m_rgb);
                        return;
                    }
                }
            }

            m_rgb = ParseHexData(text);
            m_hsl = AdobeColors.RGB_to_HSL(m_rgb);

            m_ctrl_BigBox.HSL = m_hsl;
            m_ctrl_ThinBox.HSL = m_hsl;

            SetColorBasedOnType(m_rgb);

            UpdateTextBoxes();
        }


        private void HexTextBox_KeyPress(object sender, KeyPressEventArgs e) {
            int n;
            if (!char.IsDigit(e.KeyChar) && !char.IsControl(e.KeyChar)) {
                if (!int.TryParse(e.KeyChar.ToString(), System.Globalization.NumberStyles.HexNumber, System.Globalization.NumberFormatInfo.CurrentInfo, out n))
                    e.Handled = true;
            }
        }



        #endregion

        #region Color Boxes

        private void m_lbl_OldColor_Click(object sender, System.EventArgs e) {
            m_lbl_New_Color.BackColor = m_lbl_Old_Color.BackColor;
            if (!m_radio_btn_Body.Checked)
                m_radio_btn_Body.Checked = true;
        }

        private void m_lbl_OldBtnColor_Click(object sender, System.EventArgs e) {
            m_lbl_New_ButtonColor.BackColor = m_lbl_Old_ButtonColor_btn.BackColor;
            if (!m_radio_btn_Buttons.Checked)
                m_radio_btn_Buttons.Checked = true;
        }

        private void m_lbl_NewColor_Click(object sender, EventArgs e) {
            if (!m_radio_btn_Body.Checked)
                m_radio_btn_Body.Checked = true;
        }


        private void m_lbl_NewBtnColor_Click(object sender, EventArgs e) {
            if (!m_radio_btn_Buttons.Checked)
                m_radio_btn_Buttons.Checked = true;
        }


        #endregion

        #region Radio Buttons

        private void m_rbtn_Hue_CheckedChanged(object sender, System.EventArgs e) {
            if (m_rbtn_Hue.Checked) {
                m_ctrl_ThinBox.DrawStyle = ctrlVerticalColorSlider.eDrawStyle.Hue;
                m_ctrl_BigBox.DrawStyle = ctrl2DColorBox.eDrawStyle.Hue;
            }
        }


        private void m_rbtn_Sat_CheckedChanged(object sender, System.EventArgs e) {
            if (m_rbtn_Sat.Checked) {
                m_ctrl_ThinBox.DrawStyle = ctrlVerticalColorSlider.eDrawStyle.Saturation;
                m_ctrl_BigBox.DrawStyle = ctrl2DColorBox.eDrawStyle.Saturation;
            }
        }


        private void m_rbtn_Black_CheckedChanged(object sender, System.EventArgs e) {
            if (m_rbtn_Black.Checked) {
                m_ctrl_ThinBox.DrawStyle = ctrlVerticalColorSlider.eDrawStyle.Brightness;
                m_ctrl_BigBox.DrawStyle = ctrl2DColorBox.eDrawStyle.Brightness;
            }
        }


        private void m_rbtn_Red_CheckedChanged(object sender, System.EventArgs e) {
            if (m_rbtn_Red.Checked) {
                m_ctrl_ThinBox.DrawStyle = ctrlVerticalColorSlider.eDrawStyle.Red;
                m_ctrl_BigBox.DrawStyle = ctrl2DColorBox.eDrawStyle.Red;
            }
        }


        private void m_rbtn_Green_CheckedChanged(object sender, System.EventArgs e) {
            if (m_rbtn_Green.Checked) {
                m_ctrl_ThinBox.DrawStyle = ctrlVerticalColorSlider.eDrawStyle.Green;
                m_ctrl_BigBox.DrawStyle = ctrl2DColorBox.eDrawStyle.Green;
            }
        }


        private void m_rbtn_Blue_CheckedChanged(object sender, System.EventArgs e) {
            if (m_rbtn_Blue.Checked) {
                m_ctrl_ThinBox.DrawStyle = ctrlVerticalColorSlider.eDrawStyle.Blue;
                m_ctrl_BigBox.DrawStyle = ctrl2DColorBox.eDrawStyle.Blue;
            }
        }


        private void m_radio_btn_ColorsTypeChanged(object sender, EventArgs e) {
            if (m_radio_btn_Body.Checked) {
                m_rgb = m_lbl_New_Color.BackColor;


            }
            else {
                m_rgb = m_lbl_New_ButtonColor.BackColor;
            }
            m_hsl = AdobeColors.RGB_to_HSL(m_rgb);

            m_ctrl_BigBox.HSL = m_hsl;
            m_ctrl_ThinBox.HSL = m_hsl;

            UpdateTextBoxes();

            WriteHexData(m_rgb);
        }


        #endregion

        #region Text Boxes

        private void NumericTextBox_KeyPress(object sender, KeyPressEventArgs e) {
            if (!char.IsDigit(e.KeyChar) && !char.IsControl(e.KeyChar)) {
                e.Handled = true;
            }
        }


        private void Validate_Color_TextValues(object sender, KeyEventArgs e) {
            TextBox senderTxt = (TextBox)sender;
            string text = senderTxt.Text;

            if (text.Length <= 0) {
                UpdateTextBoxes();
                WriteHexData(m_rgb);
                return;
            }
            foreach (char letter in text) {
                if (!char.IsNumber(letter)) {
                    UpdateTextBoxes();
                    WriteHexData(m_rgb);
                    return;
                }
            }

            int senderValue = int.Parse(text);
            switch (senderTxt.Name) {
                case "m_txt_Hue":
                    if (senderValue > 360) {
                        m_txt_Hue.Text = "360";
                        m_hsl.H = 1.0;
                    }
                    else {
                        m_hsl.H = (double)senderValue / 360;
                    }
                    m_rgb = AdobeColors.HSL_to_RGB(m_hsl);
                    break;
                case "m_txt_Sat":
                    if (senderValue > 100) {
                        m_txt_Sat.Text = "100";
                        m_hsl.S = 1.0;
                    }
                    else {
                        m_hsl.S = (double)senderValue / 100;
                    }
                    m_rgb = AdobeColors.HSL_to_RGB(m_hsl);
                    break;
                case "m_txt_Black":
                    if (senderValue > 100) {
                        m_txt_Black.Text = "100";
                        m_hsl.L = 1.0;
                    }
                    else {
                        m_hsl.L = (double)senderValue / 100;
                    }
                    m_rgb = AdobeColors.HSL_to_RGB(m_hsl);
                    break;
                case "m_txt_Red":
                    if (senderValue > 255) {
                        m_txt_Red.Text = "255";
                        m_rgb = Color.FromArgb(255, m_rgb.G, m_rgb.B);
                        e.Handled = true;
                    }
                    else {
                        m_rgb = Color.FromArgb(senderValue, m_rgb.G, m_rgb.B);
                    }
                    m_hsl = AdobeColors.RGB_to_HSL(m_rgb);
                    break;
                case "m_txt_Green":
                    if (senderValue > 255) {
                        m_txt_Green.Text = "255";
                        m_rgb = Color.FromArgb(m_rgb.R, 255, m_rgb.B);
                    }
                    else {
                        m_rgb = Color.FromArgb(m_rgb.R, senderValue, m_rgb.B);
                    }
                    m_hsl = AdobeColors.RGB_to_HSL(m_rgb);
                    break;
                case "m_txt_Blue":
                    if (senderValue > 255) {
                        m_txt_Blue.Text = "255";
                        m_rgb = Color.FromArgb(m_rgb.R, m_rgb.G, 255);
                    }
                    else {
                        m_rgb = Color.FromArgb(m_rgb.R, m_rgb.G, senderValue);
                    }
                    m_hsl = AdobeColors.RGB_to_HSL(m_rgb);

                    break;

            }
            m_ctrl_BigBox.HSL = m_hsl;
            m_ctrl_ThinBox.HSL = m_hsl;
            SetColorBasedOnType(m_rgb);

            UpdateTextBoxes();
            WriteHexData(m_rgb);
        }


        void RGB_MouseWheel(object sender, MouseEventArgs e) {
            TextBox test = (TextBox)sender;
            int test_value = int.Parse(test.Text);
            test_value += e.Delta / 120;
            if (test_value > 255 || test_value < 0) {
                test_value -= e.Delta / 120;
            }
            test.Text = test_value.ToString();
        }


        void Hue_MouseWheel(object sender, MouseEventArgs e) {
            TextBox test = (TextBox)sender;
            int test_value = int.Parse(test.Text);
            test_value += e.Delta / 120;
            if (test_value > 360 || test_value < 0) {
                test_value -= e.Delta / 120;
            }
            test.Text = test_value.ToString();
        }


        void Sat_Bri_MouseWheel(object sender, MouseEventArgs e) {
            TextBox test = (TextBox)sender;
            int test_value = int.Parse(test.Text);
            test_value += e.Delta / 120;
            if (test_value > 100 || test_value < 0) {
                test_value -= e.Delta / 120;
            }
            test.Text = test_value.ToString();
        }


        #endregion

        #region Presets

        private void m_lbl_Preset_Click(object sender, System.EventArgs e) {
            Buttons.RoundButton senderButton = (Buttons.RoundButton)sender;

            UncheckPreset(this.panel2);
            senderButton.checkedFocus = true;

            m_rgb = senderButton.BackColor;
            m_hsl = AdobeColors.RGB_to_HSL(m_rgb);

            m_ctrl_BigBox.HSL = m_hsl;
            m_ctrl_ThinBox.HSL = m_hsl;

            SetColorBasedOnType(m_rgb);

            m_txt_Hue.Text = Round(m_hsl.H * 360).ToString();
            m_txt_Sat.Text = Round(m_hsl.S * 100).ToString();
            m_txt_Black.Text = Round(m_hsl.L * 100).ToString();
            m_txt_Red.Text = m_rgb.R.ToString();
            m_txt_Green.Text = m_rgb.G.ToString();
            m_txt_Blue.Text = m_rgb.B.ToString();

            WriteHexData(m_rgb);
        }


        private void UncheckPreset(Control ctrl) {
            Buttons.RoundButton chkBtn = ctrl as Buttons.RoundButton;
            if (chkBtn == null) {
                foreach (Control child in ctrl.Controls) {
                    UncheckPreset(child);
                }
            }
            else {
                chkBtn.checkedFocus = false;
                chkBtn.buttonPressOffset = 0;
                chkBtn.Invalidate();
            }
        }


        private void SetPresetColorName(object sender, System.EventArgs e) {
            // Check Retail user colors
            foreach (Control child in panel_RetailUserColors.Controls) {
                Buttons.RoundButton rB = child as Buttons.RoundButton;
                if (rB != null && rB.checkedFocus == true) {
                    PresetNameDialog presetDialog = new PresetNameDialog();
                    presetDialog.txtbox_presetName.Text = this.toolTip1.GetToolTip(rB);
                    if (presetDialog.ShowDialog(this) == DialogResult.OK) {
                        // Read the contents of presetNameDialog's TextBox.
                        rB.BackColor = m_rgb;
                        this.toolTip1.SetToolTip(rB, presetDialog.txtbox_presetName.Text);
                        presetDialog.Dispose();
                    }
                    // Only one is checked. Return.
                    return;
                }
            }
            // Check User colors
            foreach (Control child in panel_UserColors.Controls) {
                Buttons.RoundButton rB = child as Buttons.RoundButton;
                if (rB != null && rB.checkedFocus == true) {
                    PresetNameDialog presetDialog = new PresetNameDialog();
                    presetDialog.txtbox_presetName.Text = this.toolTip1.GetToolTip(rB);
                    if (presetDialog.ShowDialog(this) == DialogResult.OK) {
                        // Read the contents of presetNameDialog's TextBox.
                        rB.BackColor = m_rgb;
                        this.toolTip1.SetToolTip(rB, presetDialog.txtbox_presetName.Text);
                        presetDialog.Dispose();
                    }
                    // Only one is checked. Return.
                    return;
                }
            }
        }


        private void ClearPreset(object sender, System.EventArgs e) {
            // Check Retail user colors
            foreach (Control child in panel_RetailUserColors.Controls) {
                Buttons.RoundButton rB = child as Buttons.RoundButton;
                if (rB != null && rB.checkedFocus == true) {
                    if (MessageBox.Show("Do you really want to clear this color preset?", "Delete preset",
                        MessageBoxButtons.YesNo, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button2) == DialogResult.Yes) {
                        rB.BackColor = Color.FromArgb(80, 80, 80);
                        this.toolTip1.SetToolTip(rB, "");
                    }
                    // Only one is checked. Return.
                    return;

                }
            }
            // Check User colors
            foreach (Control child in panel_UserColors.Controls) {
                Buttons.RoundButton rB = child as Buttons.RoundButton;
                if (rB != null && rB.checkedFocus == true) {
                    if (MessageBox.Show("Do you really want to clear this color preset?", "Delete preset",
                        MessageBoxButtons.YesNo, MessageBoxIcon.Asterisk, MessageBoxDefaultButton.Button2) == DialogResult.Yes) {
                        rB.BackColor = Color.FromArgb(80, 80, 80);
                        this.toolTip1.SetToolTip(rB, "");
                    }
                    // Only one is checked. Return.
                    return;

                }
            }
        }

        #endregion

        #endregion

        #region Private Functions

        private int Round(double val) {
            int ret_val = (int)val;

            int temp = (int)(val * 100);

            if ((temp % 100) >= 50)
                ret_val += 1;

            return ret_val;
        }


        private void WriteHexData(Color rgb) {
            string red = Convert.ToString(rgb.R, 16);
            if (red.Length < 2) red = "0" + red;
            string green = Convert.ToString(rgb.G, 16);
            if (green.Length < 2) green = "0" + green;
            string blue = Convert.ToString(rgb.B, 16);
            if (blue.Length < 2) blue = "0" + blue;

            m_txt_Hex.Text = red.ToUpper() + green.ToUpper() + blue.ToUpper();
            m_txt_Hex.Update();
        }


        private void btn_switchGripsPanel_Click(object sender, EventArgs e) {
            this.AutoScaleDimensions = new System.Drawing.SizeF(96, 96);
            if (showGripsPanel) {
                showGripsPanel = false;
                this.btn_switchGripsPanel.Text = "Grips";
                this.panel2.Controls.Remove(this.panel_RetailGripColors);
            }
            else {
                showGripsPanel = true;
                this.AutoScaleDimensions = new System.Drawing.SizeF(96, 96);
                this.btn_switchGripsPanel.Text = "Retail";
                this.panel2.Controls.Add(this.panel_RetailGripColors);
                this.panel_RetailGripColors.BringToFront();
            }
        }


        private Color ParseHexData(string hex_data) {
            if (hex_data.Length > 6 || hex_data.Length < 0)
                return Color.Black;

            string r_text, g_text, b_text;
            int r, g, b;

            switch (hex_data.Length) {
                case 0:
                    return Color.Black;
                case 1:
                case 2:
                    r_text = g_text = "00";
                    b_text = hex_data;
                    break;
                case 3:
                    r_text = hex_data.Substring(0, 1);
                    g_text = hex_data.Substring(1, 1);
                    b_text = hex_data.Substring(2, 1);
                    r = 17 * int.Parse(r_text, System.Globalization.NumberStyles.HexNumber);
                    g = 17 * int.Parse(g_text, System.Globalization.NumberStyles.HexNumber);
                    b = 17 * int.Parse(b_text, System.Globalization.NumberStyles.HexNumber);
                    return Color.FromArgb(r, g, b);
                case 4:
                    r_text = "00";
                    g_text = hex_data.Substring(0, 2);
                    b_text = hex_data.Substring(2, 2);
                    break;
                case 5:
                    r_text = hex_data.Substring(0, 1);
                    g_text = hex_data.Substring(1, 2);
                    b_text = hex_data.Substring(3, 2);
                    break;
                case 6:
                    r_text = hex_data.Substring(0, 2);
                    g_text = hex_data.Substring(2, 2);
                    b_text = hex_data.Substring(4, 2);
                    break;
                default:
                    r_text = hex_data.Substring(0, 2);
                    g_text = hex_data.Substring(2, 2);
                    b_text = hex_data.Substring(4, 2);
                    break;

            }

            r = int.Parse(r_text, System.Globalization.NumberStyles.HexNumber);
            g = int.Parse(g_text, System.Globalization.NumberStyles.HexNumber);
            b = int.Parse(b_text, System.Globalization.NumberStyles.HexNumber);

            return Color.FromArgb(r, g, b);
        }


        private void UpdateTextBoxes() {
            m_txt_Hue.Text = Round(m_hsl.H * 360).ToString();
            m_txt_Sat.Text = Round(m_hsl.S * 100).ToString();
            m_txt_Black.Text = Round(m_hsl.L * 100).ToString();
            m_txt_Red.Text = m_rgb.R.ToString();
            m_txt_Green.Text = m_rgb.G.ToString();
            m_txt_Blue.Text = m_rgb.B.ToString();
        }


        private void OnEyeDropperSelectionChanged(object sender, EventArgs e) {
            m_rgb = m_eyedropColorPicker.SelectedColor;
            m_hsl = AdobeColors.RGB_to_HSL(m_rgb);

            UpdateTextBoxes();

            m_ctrl_BigBox.HSL = m_hsl;
            m_ctrl_ThinBox.HSL = m_hsl;

            SetColorBasedOnType(m_rgb);

            this.WriteHexData(m_rgb);

        }


        private void getCustomColorFromConfig() {
            System.Xml.XmlDocument doc = new System.Xml.XmlDocument();
            try {
                doc.Load(System.IO.Path.GetDirectoryName(Application.ExecutablePath) + "\\colors.xml");
            }
            catch {
                // If colors.xml not found, create it
                System.Xml.XmlWriterSettings settings = new System.Xml.XmlWriterSettings();
                settings.Indent = true;
                settings.NewLineOnAttributes = true;
                settings.IndentChars = "\t";
                settings.ConformanceLevel = System.Xml.ConformanceLevel.Document;
                using (System.Xml.XmlWriter writer = System.Xml.XmlWriter.Create("colors.xml", settings)) {
                    string newXml = "\n<configuration>\n\t<startup>\n\t\t<supportedRuntime version='v4.0' sku='.NETFramework,Version=v4.7.1' />" +
                    "\n\t</startup>\n\t<!--Values are in RGB-->\n</configuration>";
                    writer.WriteRaw(newXml);
                }
                try {
                    doc.Load(System.IO.Path.GetDirectoryName(Application.ExecutablePath) + "\\colors.xml");
                }
                catch {
                    errorCreatingPresets = true;
                    MessageBox.Show("Error creating colors.xml!\nPlease check that you have write permissions.",
                    "Error writing!", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
                // File is new, nothing to load
                return;
            }
            //get all retail user created colors
            System.Xml.XmlNodeList nodes = doc.GetElementsByTagName("RC");
            int alphaChannel = -16777216;
            if (nodes != null) {
                int i = 0;
                foreach (Control child in Util.controlsSorted(panel_RetailUserColors)) {
                    Buttons.RoundButton rB = child as Buttons.RoundButton;
                    if (rB != null) {
                        if (i > (nodes.Count - 1))
                            break;
                        //get key attribute
                        System.Xml.XmlAttribute att = nodes[i].Attributes["key"];
                        if (att.Value == ("retailpreset_" + (i + 1).ToString("D2"))) {
                            //get value attribute
                            att = nodes[i].Attributes["rgb_color"];
                            if (att != null)
                                rB.BackColor = Color.FromArgb(Convert.ToInt32(att.Value, 16) + alphaChannel);
                            att = nodes[i].Attributes["tooltip"];
                            if (att != null)
                                this.toolTip1.SetToolTip(rB, att.Value.ToString());
                        }
                        i++;
                    }
                }
            }
            //get all user custom colors
            System.Xml.XmlNodeList nodes2 = doc.GetElementsByTagName("UC");
            if (nodes2 != null) {
                int i = 0;
                foreach (Control child in Util.controlsSorted(panel_UserColors)) {
                    Buttons.RoundButton rB = child as Buttons.RoundButton;
                    if (rB != null) {
                        if (i > (nodes2.Count - 1))
                            break;
                        //get key attribute
                        System.Xml.XmlAttribute att = nodes2[i].Attributes["key"];
                        if (att.Value == ("userpreset_" + (i + 1).ToString("D2"))) {
                            //get value attribute
                            att = nodes2[i].Attributes["rgb_color"];
                            if (att != null)
                                rB.BackColor = Color.FromArgb(Convert.ToInt32(att.Value, 16) + alphaChannel);
                            att = nodes2[i].Attributes["tooltip"];
                            if (att != null)
                                this.toolTip1.SetToolTip(rB, att.Value.ToString());
                        }
                        i++;
                    }
                }
            }
        }


        private void convertOldFormatCustomConfig() {
            if (errorCreatingPresets)
                return;
            MessageBox.Show("An old preset saving file 'Colors.config' was found!\n\n" +
                "The old colors will be converted into the new format.\n\nThe old file will be backed up into Colors.backup.",
                "Old color presets found!", MessageBoxButtons.OK, MessageBoxIcon.Asterisk);
            System.Xml.XmlDocument doc = new System.Xml.XmlDocument();
            try {
                doc.Load(System.IO.Path.GetDirectoryName(Application.ExecutablePath) + "\\Colors.config");
            }
            catch {
                return;
            }
            List<Int32> oldColors = new List<Int32>();
            Int32 tempColor;
            int alphaChannel = -16777216;
            int j = 0;
            //get all retail user created colors
            System.Xml.XmlNodeList nodes = doc.GetElementsByTagName("bodycolors_v");
            if (nodes != null) {
                for (j = 0; j < nodes.Count; j++) {
                    System.Xml.XmlAttribute att = nodes[j].Attributes["key"];
                    if (att.Value == "CustomColor") {
                        //get value attribute
                        att = nodes[j].Attributes["value"];
                        if (att != null) {
                            tempColor = Convert.ToInt32(swapBytesColor(Convert.ToUInt32(att.Value, 16))) + alphaChannel;
                            oldColors.Add(tempColor);
                        }
                    }
                }
            }
            System.Xml.XmlNodeList nodes2 = doc.GetElementsByTagName("bodycolors_pro_v");
            if (nodes2 != null) {
                for (j = 0; j < nodes2.Count; j++) {
                    System.Xml.XmlAttribute att = nodes2[j].Attributes["key"];
                    if (att.Value == "CustomColor") {
                        //get value attribute
                        att = nodes2[j].Attributes["value"];
                        if (att != null) {
                            tempColor = Convert.ToInt32(swapBytesColor(Convert.ToUInt32(att.Value, 16))) + alphaChannel;
                            oldColors.Add(tempColor);
                        }
                    }
                }
            }
            System.Xml.XmlNodeList nodes3 = doc.GetElementsByTagName("buttoncolors_v");
            if (nodes3 != null) {
                for (j = 0; j < nodes3.Count; j++) {
                    System.Xml.XmlAttribute att = nodes3[j].Attributes["key"];
                    if (att.Value == "CustomColor") {
                        //get value attribute
                        att = nodes3[j].Attributes["value"];
                        if (att != null) {
                            tempColor = Convert.ToInt32(swapBytesColor(Convert.ToUInt32(att.Value, 16))) + alphaChannel;
                            oldColors.Add(tempColor);
                        }
                    }
                }
            }
            for (j = 0; j < oldColors.Count; j++) {
                foreach (Control child in Util.controlsSorted(panel_RetailColors)) {
                    Buttons.RoundButton rB = child as Buttons.RoundButton;
                    if (rB != null) {
                        // Remove colors that exist in Retail panel, plus the 31,32,32 color
                        if (rB.BackColor.ToArgb() == oldColors[j] || oldColors[j] == -13553102) {
                            oldColors.RemoveAt(j);
                            j--;
                            break;
                        }
                    }
                }
            }
            j = 0;
            if (32 < oldColors.Count)
                MessageBox.Show("The saved colors are more than 32.\nSome of them were not loaded.\n\n");
            foreach (Control child in Util.controlsSorted(panel_UserColors)) {
                Buttons.RoundButton rB = child as Buttons.RoundButton;
                if (rB != null) {
                    if (j > (oldColors.Count - 1))
                        break;
                    // Load color from the list
                    rB.BackColor = Color.FromArgb(oldColors[j]);
                    this.toolTip1.SetToolTip(rB, "Old Preset " + (j + 1).ToString());
                }
                j++;
            }
            try {
                System.IO.File.Move(System.IO.Path.GetDirectoryName(Application.ExecutablePath) + "\\Colors.config",
                    System.IO.Path.GetDirectoryName(Application.ExecutablePath) + "\\Colors.backup");
            }
            catch {
                return;
            }
        }


        private void saveCustomColorToConfig() {
            if (errorCreatingPresets)
                return;
            System.Xml.XmlDocument doc = new System.Xml.XmlDocument();
            try {
                doc.Load(System.IO.Path.GetDirectoryName(Application.ExecutablePath) + "\\colors.xml");
            }
            catch {
                MessageBox.Show("Error saving colors.xml!\nPlease check that you have write permissions.",
                    "Error writing!", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            int alphaChannel = -16777216;
            System.Xml.XmlNode node = doc.SelectSingleNode("//RetailColors");
            if (node == null) {
                node = doc.CreateNode(System.Xml.XmlNodeType.Element, "RetailColors", "");
                System.Xml.XmlElement root = doc.DocumentElement;
                root.AppendChild(node);
                System.Xml.XmlComment newComment;
                newComment = doc.CreateComment("Color Presets for Retail Colors");
                doc.DocumentElement.InsertBefore(newComment, doc.DocumentElement.SelectSingleNode("//RetailColors"));
            }
            else
                node.RemoveAll();
            //add custom colors
            int presetKey = 1;
            foreach (Control child in Util.controlsSorted(panel_RetailUserColors)) {
                Buttons.RoundButton rB = child as Buttons.RoundButton;
                if (rB != null) {
                    System.Xml.XmlElement xe = doc.CreateElement("RC");
                    xe.SetAttribute("key", "retailpreset_" + presetKey.ToString("D2"));
                    presetKey++;
                    xe.SetAttribute("rgb_color", String.Format("{0:X6}", (rB.BackColor.ToArgb() - alphaChannel)));
                    xe.SetAttribute("tooltip", this.toolTip1.GetToolTip(rB));
                    node.AppendChild(xe);
                }
            }
            try {
                doc.Save(System.IO.Path.GetDirectoryName(Application.ExecutablePath) + "\\colors.xml");
            }
            catch {
                MessageBox.Show("Error saving colors.xml!\nPlease check that the file is writable.\n\n" +
                    "The custom color presets were not saved.", "File not found!", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            System.Xml.XmlNode node2 = doc.SelectSingleNode("//UserColors");
            if (node2 == null) {
                node2 = doc.CreateNode(System.Xml.XmlNodeType.Element, "UserColors", "");
                System.Xml.XmlElement root = doc.DocumentElement;
                root.AppendChild(node2);
                System.Xml.XmlComment newComment;
                newComment = doc.CreateComment("Color Presets for User Colors");
                doc.DocumentElement.InsertBefore(newComment, doc.DocumentElement.SelectSingleNode("//UserColors"));
            }
            else
                node2.RemoveAll();
            presetKey = 1;
            foreach (Control child in Util.controlsSorted(panel_UserColors)) {
                Buttons.RoundButton rB = child as Buttons.RoundButton;
                if (rB != null) {
                    System.Xml.XmlElement xe = doc.CreateElement("UC");
                    xe.SetAttribute("key", "userpreset_" + presetKey.ToString("D2"));
                    presetKey++;
                    xe.SetAttribute("rgb_color", String.Format("{0:X6}", (rB.BackColor.ToArgb() - alphaChannel)));
                    xe.SetAttribute("tooltip", this.toolTip1.GetToolTip(rB));
                    node2.AppendChild(xe);
                }
            }
            try {
                doc.Save(System.IO.Path.GetDirectoryName(Application.ExecutablePath) + "\\colors.xml");
            }
            catch {
                MessageBox.Show("Error saving colors.xml!\nPlease check that the file is writable.\n\n" +
                    "The custom color presets were not saved.", "File not found!", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
        }


        private UInt32 swapBytesColor(UInt32 x) {
            x = (x << 8);
            // swap adjacent 16-bit blocks
            x = (x >> 16) | (x << 16);
            // swap adjacent 8-bit blocks
            return ((x & 0xFF00FF00) >> 8) | ((x & 0x00FF00FF) << 8);
        }


        private void SetColorBasedOnType(Color m_RGB_color) {
            if (m_radio_btn_Body.Checked == true) {
                this.m_lbl_New_Color.BackColor = m_RGB_color;
                this.m_lbl_New_Color.Update();
            }
            else {
                this.m_lbl_New_ButtonColor.BackColor = m_RGB_color;
                this.m_lbl_New_ButtonColor.Update();
            }
        }

        #endregion

        #region Public Methods

        public Color PrimaryColor {
            get {
                return m_lbl_New_Color.BackColor;
            }
            set {
                m_rgb = value;
                m_hsl = AdobeColors.RGB_to_HSL(m_rgb);

                m_txt_Hue.Text = Round(m_hsl.H * 360).ToString();
                m_txt_Sat.Text = Round(m_hsl.S * 100).ToString();
                m_txt_Black.Text = Round(m_hsl.L * 100).ToString();
                m_txt_Red.Text = m_rgb.R.ToString();
                m_txt_Green.Text = m_rgb.G.ToString();
                m_txt_Blue.Text = m_rgb.B.ToString();

                m_ctrl_BigBox.HSL = m_hsl;
                m_ctrl_ThinBox.HSL = m_hsl;

                m_lbl_New_Color.BackColor = m_rgb;

                this.WriteHexData(value);
            }
        }


        public Color SecondaryColor {
            get {
                return m_lbl_New_ButtonColor.BackColor;
            }
            set {
                m_rgb = value;
                m_hsl = AdobeColors.RGB_to_HSL(m_rgb);

                m_txt_Hue.Text = Round(m_hsl.H * 360).ToString();
                m_txt_Sat.Text = Round(m_hsl.S * 100).ToString();
                m_txt_Black.Text = Round(m_hsl.L * 100).ToString();
                m_txt_Red.Text = m_rgb.R.ToString();
                m_txt_Green.Text = m_rgb.G.ToString();
                m_txt_Blue.Text = m_rgb.B.ToString();

                m_ctrl_BigBox.HSL = m_hsl;
                m_ctrl_ThinBox.HSL = m_hsl;

                m_lbl_New_ButtonColor.BackColor = m_rgb;

                this.WriteHexData(value);
            }
        }

        public bool GripsColorValue {
            get {
                return gripsColor;
            }
            set {
                gripsColor = value;

            }
        }


        public eDrawStyle DrawStyle {
            get {
                if (m_rbtn_Hue.Checked)
                    return eDrawStyle.Hue;
                else if (m_rbtn_Sat.Checked)
                    return eDrawStyle.Saturation;
                else if (m_rbtn_Black.Checked)
                    return eDrawStyle.Brightness;
                else if (m_rbtn_Red.Checked)
                    return eDrawStyle.Red;
                else if (m_rbtn_Green.Checked)
                    return eDrawStyle.Green;
                else if (m_rbtn_Blue.Checked)
                    return eDrawStyle.Blue;
                else
                    return eDrawStyle.Hue;
            }
            set {
                switch (value) {
                    case eDrawStyle.Hue:
                        m_rbtn_Hue.Checked = true;
                        break;
                    case eDrawStyle.Saturation:
                        m_rbtn_Sat.Checked = true;
                        break;
                    case eDrawStyle.Brightness:
                        m_rbtn_Black.Checked = true;
                        break;
                    case eDrawStyle.Red:
                        m_rbtn_Red.Checked = true;
                        break;
                    case eDrawStyle.Green:
                        m_rbtn_Green.Checked = true;
                        break;
                    case eDrawStyle.Blue:
                        m_rbtn_Blue.Checked = true;
                        break;
                    default:
                        m_rbtn_Hue.Checked = true;
                        break;
                }
            }
        }

        #endregion

    }

}
