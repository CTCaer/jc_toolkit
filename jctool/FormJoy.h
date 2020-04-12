// Copyright (c) 2018 CTCaer. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once
#include <iomanip>
#include <sstream>

#include <msclr\marshal_cppstd.h>

#include "overrides.h"
#include "luts.h"

#include "jctool_api.hpp"

using namespace System;

namespace CppWinFormJoy {
    using namespace System::ComponentModel;
    using namespace System::Drawing;
    using namespace System::Globalization;
    using namespace System::IO;
    using namespace System::Resources;
    using namespace System::Windows::Forms;

/// <summary>
/// Summary for FormJoy
/// </summary>
public ref class FormJoy : public System::Windows::Forms::Form
{
    public: static FormJoy^ myform1;

    FormJoy(void)
    {
        handler_close   = 0;
        option_is_on    = 0;
        vib_file_type   = 0;
        vib_sample_rate = 0;
        vib_samples     = 0;
        vib_loop_start  = 0;
        vib_loop_end    = 0;
        vib_loop_wait   = 0;
        disable_expert_mode = true;
        temp_celsius        = true;

        silence_input_report();
        set_led_busy();

        InitializeComponent();

        // Set static form, to allow calling functions from unmanaged code.
        myform1 = this;

        //Initialise locations on start for easy designing
        this->grpBox_DebugCmd->Location         = System::Drawing::Point(494, 36);
        this->grpBox_Restore->Location          = System::Drawing::Point(494, 36);
        this->grpBox_ChangeSN->Location         = System::Drawing::Point(494, 36);
        this->grpBox_VibPlayer->Location        = System::Drawing::Point(494, 36);
        this->grpBox_ButtonTest->Location       = System::Drawing::Point(494, 36);
        this->grpBox_nfc->Location              = System::Drawing::Point(724, 36);
        this->grpBox_IR->Location               = System::Drawing::Point(521, 36);
        this->grpBox_IRSettings->Location       = System::Drawing::Point(14, 120);
        this->grpBox_editCalModel->Location     = System::Drawing::Point(14, 120);

        // Get controller info
        full_refresh(false);
        
        // Set properties that otherwise get removed by Designer in InitializeComponent()
        this->comboBox_rstOption->Items->Add("Restore Color");
        this->comboBox_rstOption->Items->Add("Restore S/N");
        this->comboBox_rstOption->Items->Add("Restore User Calibration");
        this->comboBox_rstOption->Items->Add("Factory Reset User Calibration");
        this->comboBox_rstOption->Items->Add("Full Restore");
        this->comboBox_rstOption->DrawItem +=
            gcnew System::Windows::Forms::DrawItemEventHandler(this, &FormJoy::comboBox_darkTheme_DrawItem);
            
        this->menuStrip1->Renderer =
            gcnew System::Windows::Forms::ToolStripProfessionalRenderer(gcnew Overrides::TestColorTable());
        this->toolStrip1->Renderer = gcnew Overrides::OverrideTSSR();

        this->textBoxDbg_cmd->Validating         += gcnew CancelEventHandler(this, &FormJoy::textBoxDbg_Validating);
        this->textBoxDbg_cmd->Validated          += gcnew EventHandler(this, &FormJoy::textBoxDbg_Validated);
        this->textBoxDbg_subcmd->Validating      += gcnew CancelEventHandler(this, &FormJoy::textBoxDbg_subcmd_Validating);
        this->textBoxDbg_subcmd->Validated       += gcnew EventHandler(this, &FormJoy::textBoxDbg_subcmd_Validated);
        this->textBoxDbg_SubcmdArg->Validating   += gcnew CancelEventHandler(this, &FormJoy::textBoxDbg_SubcmdArg_Validating);
        this->textBoxDbg_SubcmdArg->Validated    += gcnew EventHandler(this, &FormJoy::textBoxDbg_SubcmdArg_Validated);
        this->textBoxDbg_lfamp->Validating       += gcnew CancelEventHandler(this, &FormJoy::textBoxDbg_Validating);
        this->textBoxDbg_lfamp->Validated        += gcnew EventHandler(this, &FormJoy::textBoxDbg_Validated);
        this->textBoxDbg_lfreq->Validating       += gcnew CancelEventHandler(this, &FormJoy::textBoxDbg_Validating);
        this->textBoxDbg_lfreq->Validated        += gcnew EventHandler(this, &FormJoy::textBoxDbg_Validated);
        this->textBoxDbg_hamp->Validating        += gcnew CancelEventHandler(this, &FormJoy::textBoxDbg_Validating);
        this->textBoxDbg_hamp->Validated         += gcnew EventHandler(this, &FormJoy::textBoxDbg_Validated);
        this->textBoxDbg_hfreq->Validating       += gcnew CancelEventHandler(this, &FormJoy::textBoxDbg_Validating);
        this->textBoxDbg_hfreq->Validated        += gcnew EventHandler(this, &FormJoy::textBoxDbg_Validated);
        this->textBox_chg_sn->Validating         += gcnew CancelEventHandler(this, &FormJoy::textBox_chg_sn_Validating);
        this->textBox_chg_sn->Validated          += gcnew EventHandler(this, &FormJoy::textBox_chg_sn_Validated);
        this->textBox_vib_loop_times->Validating += gcnew CancelEventHandler(this, &FormJoy::textBox_loop_Validating);
        this->textBox_vib_loop_times->Validated  += gcnew EventHandler(this, &FormJoy::textBox_loop_Validated);

        this->chkBox_IRFlashlight->CheckedChanged   += gcnew EventHandler(this, &FormJoy::IRFlashlight_checkedChanged);
        this->chkBox_IRBrightLeds->CheckedChanged   += gcnew EventHandler(this, &FormJoy::IRLeds_checkedChanged);
        this->chkBox_IRDimLeds->CheckedChanged      += gcnew EventHandler(this, &FormJoy::IRLeds_checkedChanged);
        this->chkBox_IRDenoise->CheckedChanged      += gcnew EventHandler(this, &FormJoy::IRDenoise_checkedChanged);
        this->chkBox_IRAutoExposure->CheckedChanged += gcnew EventHandler(this, &FormJoy::IRAutoExposure_checkedChanged);

        this->toolTip1->SetToolTip(this->label_sn, L"Click here to change your S/N");
        this->toolTip1->SetToolTip(this->textBox_vib_loop_times,
            L"Set how many additional times the loop will be played.\n\nChoose a number from 0 to 999");
        this->toolTip1->SetToolTip(this->label_loop_times,
            L"Set how many additional times the loop will be played.\n\nChoose a number from 0 to 999");

        // Unicode escapes
        this->chkBox_IRBrightLeds->Text = L"Far/Narrow   (75\u00B0)  Leds 1/2";
        this->chkBox_IRDimLeds->Text    = L"Near/Wide  (130\u00B0)  Leds 3/4";
        
        // Final form window adjustments
        this->CenterToScreen();
        this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
        reset_window_option(true);

        //Done drawing!
        send_rumble();
    }

    protected:
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        ~FormJoy()
        {
            if (components)
            {
                delete components;
            }
        }
    

#pragma region Class Variables
    private: int handler_close;
    private: int option_is_on;
    private: bool allow_full_restore;
    private: bool disable_expert_mode;
    private: bool temp_celsius;
    private: array<byte>^ backup_spi;
    public:  array<byte>^ vib_loaded_file;
    public:  array<byte>^ vib_file_converted;
    private: u16 vib_sample_rate;
    private: u32 vib_samples;
    private: u32 vib_loop_start;
    private: u32 vib_loop_end;
    private: u32 vib_loop_wait;
    private: int vib_converted;
    //file type: 1 = Raw, 2 = bnvib (0x4), 3 = bnvib loop (0xC), 4 = bnvib loop (0x10)
    private: int vib_file_type;
    private: float lf_gain;
    private: float lf_pitch;
    private: float hf_gain;
    private: float hf_pitch;
    private: Color jcBodyColor;
    private: Color jcButtonsColor;
    private: Color jcGripLeftColor;
    private: Color jcGripRightColor;
    private: int ir_image_width;
    private: int ir_image_height;
    private: System::Windows::Forms::GroupBox^  grpBox_Color;
    private: System::Windows::Forms::Button^ btn_writeColorsToSpi;
    private: System::Windows::Forms::TextBox^ textBoxSN;
    private: System::Windows::Forms::Label^ label_sn;
    private: System::Windows::Forms::Label^  label_mac;
    private: System::Windows::Forms::Label^  label_fw;
    private: System::Windows::Forms::TextBox^  textBoxMAC;
    private: System::Windows::Forms::TextBox^  textBoxFW;
    private: System::Windows::Forms::TextBox^  textBoxDev;
    private: System::Windows::Forms::Label^  label_dev;
    private: System::Windows::Forms::Button^  btn_makeSPIBackup;
    private: System::Windows::Forms::GroupBox^  grpBox_SPI;
    private: System::Windows::Forms::Label^  lbl_spiProggressDesc;
    private: System::Windows::Forms::PictureBox^  pictureBoxPreview;
    public:  System::Windows::Forms::Label^  label_progress;
    private: System::Windows::Forms::MenuStrip^  menuStrip1;
    private: System::Windows::Forms::ToolStripMenuItem^  menuToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  debugToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  aboutToolStripMenuItem;
    private: System::Windows::Forms::Label^  lbl_subcmdArgs;
    private: System::Windows::Forms::TextBox^  textBoxDbg_SubcmdArg;
    private: System::Windows::Forms::Button^  btn_dbgSendCmd;
    private: System::Windows::Forms::Label^  lbl_cmd;
    private: System::Windows::Forms::Label^  lbl_subcmd;
    private: System::Windows::Forms::TextBox^  textBoxDbg_cmd;
    private: System::Windows::Forms::TextBox^  textBoxDbg_subcmd;
    private: System::Windows::Forms::Label^  lbl_dbgDisclaimer;
    private: System::Windows::Forms::GroupBox^  grpBox_Restore;
    private: System::Windows::Forms::Button^  btn_loadSPIBackup;
    private: System::Windows::Forms::Label^  label_rst_mac;
    private: System::Windows::Forms::TextBox^  txtBox_fileLoaded;
    private: System::Windows::Forms::ComboBox^  comboBox_rstOption;
    private: System::Windows::Forms::Button^  btn_restore;
    private: System::Windows::Forms::GroupBox^  grpBox_RstUser;
    private: System::Windows::Forms::CheckBox^  checkBox_rst_accGyroCal;
    private: System::Windows::Forms::CheckBox^  checkBox_rst_R_StickCal;
    private: System::Windows::Forms::CheckBox^  checkBox_rst_L_StickCal;
    private: System::Windows::Forms::Label^  lbl_rstDisclaimer;
    private: System::Windows::Forms::Label^  lbl_rstDesc;
    private: System::Windows::Forms::Button^  btn_RestoreEnable;
    private: System::Windows::Forms::ErrorProvider^  errorProvider1;
    private: System::Windows::Forms::ErrorProvider^  errorProvider2;
    private: System::Windows::Forms::Label^  lbl_dbgVib;
    private: System::Windows::Forms::TextBox^  textBoxDbg_lfamp;
    private: System::Windows::Forms::TextBox^  textBoxDbg_lfreq;
    private: System::Windows::Forms::TextBox^  textBoxDbg_hamp;
    private: System::Windows::Forms::TextBox^  textBoxDbg_hfreq;
    private: System::Windows::Forms::Label^  lbl_AmpL;
    private: System::Windows::Forms::Label^  lbl_AmpH;
    private: System::Windows::Forms::Label^  lbl_FreqL;
    private: System::Windows::Forms::Label^  lbl_FreqH;
    private: System::Windows::Forms::GroupBox^  grpBox_DebugCmd;
    private: System::Windows::Forms::GroupBox^  grpBox_ChangeSN;
    private: System::Windows::Forms::Button^  btn_changeSN;
    private: System::Windows::Forms::TextBox^  textBox_chg_sn;
    private: System::Windows::Forms::Label^  lbl_loadedMAC;
    private: System::Windows::Forms::Label^  label_sn_change_warning;
    private: System::Windows::Forms::ToolTip^  toolTip1;
    public:  System::Windows::Forms::TextBox^  textBoxDbg_sent;
    public:  System::Windows::Forms::TextBox^  textBoxDbg_reply;
    private: System::Windows::Forms::GroupBox^  grpBox_VibPlayer;
    private: System::Windows::Forms::Button^  btn_vibPlay;
    private: System::Windows::Forms::Button^  btn_loadVib;
    private: System::Windows::Forms::Label^  label_vib_loaded;
    private: System::Windows::Forms::Label^  label_samplerate;
    public:  System::Windows::Forms::Label^  label_samples;
    private: System::Windows::Forms::ToolStripMenuItem^  hDRumblePlayerToolStripMenuItem;
    private: System::Windows::Forms::Label^  label_hdrumble_filename;
    public:  System::Windows::Forms::TextBox^  textBoxDbg_reply_cmd;
    private: System::ComponentModel::IContainer^  components;
    private: System::Windows::Forms::TrackBar^  trackBar_lf_amp;
    private: System::Windows::Forms::TrackBar^  trackBar_hf_freq;
    private: System::Windows::Forms::TrackBar^  trackBar_hf_amp;
    private: System::Windows::Forms::TrackBar^  trackBar_lf_freq;
    private: System::Windows::Forms::Label^  label_eq_info;
    private: System::Windows::Forms::Button^  btn_vibResetEQ;
    private: System::Windows::Forms::GroupBox^  groupBox_vib_eq;
    private: System::Windows::Forms::Button^  btn_enableExpertMode;
    private: System::Windows::Forms::Label^  label_loop_times;
    private: System::Windows::Forms::Button^  btn_restoreSN;
    private: System::Windows::Forms::GroupBox^  groupBox_vib_info;
    private: System::Windows::Forms::TextBox^  textBox_vib_loop_times;
    private: System::Windows::Forms::GroupBox^  grpBox_ButtonTest;
    public:  System::Windows::Forms::TextBox^  textBox_btn_test_reply;
    public:  System::Windows::Forms::TextBox^  textBox_btn_test_subreply;
    private: System::Windows::Forms::ToolStripMenuItem^  buttonTestToolStripMenuItem;
    private: System::Windows::Forms::Button^  btn_runBtnTest;
    private: System::Windows::Forms::GroupBox^  grpBox_StickCal;
    public:  System::Windows::Forms::TextBox^  textBox_lstick_ucal;
    public:  System::Windows::Forms::TextBox^  textBox_rstick_ucal;
    public:  System::Windows::Forms::TextBox^  textBox_lstick_fcal;
    public:  System::Windows::Forms::TextBox^  textBox_rstick_fcal;
    public:  System::Windows::Forms::TextBox^  txtBox_devParameters;
    public:  System::Windows::Forms::TextBox^  textBox_6axis_cal;
    public:  System::Windows::Forms::TextBox^  textBox_6axis_ucal;
    public:  System::Windows::Forms::TextBox^  txtBox_devParameters2;
    private: System::Windows::Forms::GroupBox^  grpBox_dev_param;
    private: System::Windows::Forms::Button^  btn_spiCancel;
    private: System::Windows::Forms::ToolStrip^  toolStrip1;
    private: System::Windows::Forms::ToolStripLabel^  toolStripLabel_temp;
    private: System::Windows::Forms::ToolStripButton^  toolStripBtn_refresh;
    private: System::Windows::Forms::ToolStripButton^  toolStripBtn_batt;
    private: System::Windows::Forms::ToolStripLabel^  toolStripLabel_batt;
    private: System::Windows::Forms::ToolStripButton^  toolStripBtn_Disconnect;
    private: System::Windows::Forms::Button^  btn_changeColor;
    private: System::Windows::Forms::Panel^  panel_filler;
    private: jcColor::JoyConColorPicker^ JCColorPicker;
    private: System::Windows::Forms::Label^  lbl_Buttons_hex_txt;
    private: System::Windows::Forms::Label^  lbl_Body_hex_txt;
    private: System::Windows::Forms::GroupBox^  grpBox_accGyroCal;
    private: System::Windows::Forms::GroupBox^  grpBox_IR;
    private: System::Windows::Forms::Button^  btn_getIRImage;
    private: System::Windows::Forms::PictureBox^  pictureBoxIR;
    private: System::Windows::Forms::ToolStripMenuItem^  iRCameraToolStripMenuItem;
    private: System::Windows::Forms::GroupBox^  grpBox_IRRes;
    private: System::Windows::Forms::RadioButton^  radioBtn_IR60p;
    private: System::Windows::Forms::RadioButton^  radioBtn_IR120p;
    private: System::Windows::Forms::RadioButton^  radioBtn_IR240p;
    private: System::Windows::Forms::TrackBar^  trackBar_IRGain;
    private: System::Windows::Forms::CheckBox^  chkBox_IRDimLeds;
    private: System::Windows::Forms::CheckBox^  chkBox_IRBrightLeds;
    private: System::Windows::Forms::GroupBox^  grpBox_IRColorize;
    private: System::Windows::Forms::RadioButton^  radioBtn_IRColorHeat;
    private: System::Windows::Forms::RadioButton^  radioBtn_IRColorGreen;
    private: System::Windows::Forms::RadioButton^  radioBtn_IRColorRed;
    private: System::Windows::Forms::RadioButton^  radioBtn_IRColorGrey;
    public:  System::Windows::Forms::NumericUpDown^  numeric_IRExposure;
    private: System::Windows::Forms::Label^  lbl_digitalGain;
    private: System::Windows::Forms::RadioButton^  radioBtn_IR30p;
    public:  System::Windows::Forms::Label^  lbl_IRStatus;
    public:  System::Windows::Forms::Label^  lbl_IRHelp;
    private: System::Windows::Forms::Label^  lbl_exposure;
    private: System::Windows::Forms::Button^  btn_getIRStream;
    private: System::Windows::Forms::GroupBox^  grpBox_nfc;
    private: System::Windows::Forms::Button^  btn_NFC;
    private: System::Windows::Forms::Label^  lbl_nfcHelp;
    public:  System::Windows::Forms::TextBox^  txtBox_nfcUid;
    private: System::Windows::Forms::Button^  btn_refreshUserCal;
    private: System::Windows::Forms::NumericUpDown^  numeric_leftUserCal_x_minus;
    private: System::Windows::Forms::NumericUpDown^  numeric_leftUserCal_y_plus;
    private: System::Windows::Forms::NumericUpDown^  numeric_leftUserCal_y_center;
    private: System::Windows::Forms::NumericUpDown^  numeric_leftUserCal_y_minus;
    private: System::Windows::Forms::NumericUpDown^  numeric_leftUserCal_x_plus;
    private: System::Windows::Forms::NumericUpDown^  numeric_leftUserCal_x_center;
    private: System::Windows::Forms::CheckBox^  checkBox_enableLeftUserCal;
    private: System::Windows::Forms::CheckBox^  checkBox_enableRightUserCal;
    private: System::Windows::Forms::NumericUpDown^  numeric_rightUserCal_y_plus;
    private: System::Windows::Forms::NumericUpDown^  numeric_rightUserCal_y_center;
    private: System::Windows::Forms::NumericUpDown^  numeric_rightUserCal_y_minus;
    private: System::Windows::Forms::NumericUpDown^  numeric_rightUserCal_x_plus;
    private: System::Windows::Forms::NumericUpDown^  numeric_rightUserCal_x_center;
    private: System::Windows::Forms::NumericUpDown^  numeric_rightUserCal_x_minus;
    private: System::Windows::Forms::GroupBox^  grpBox_leftStickUCal;
    private: System::Windows::Forms::Label^  lbl_userCalMinCenterMax;
    private: System::Windows::Forms::GroupBox^  grpBox_rightStickUCal;
    private: System::Windows::Forms::Label^  lbl_userCalMinCenterMax2;
    private: System::Windows::Forms::Button^  btn_writeUserCal;
    private: System::Windows::Forms::Button^  btn_changeGripsColor;
    private: System::Windows::Forms::Button^  btn_IRConfigLive;
    private: System::Windows::Forms::NumericUpDown^  numeric_IRCustomRegVal;
    private: System::Windows::Forms::NumericUpDown^  numeric_IRCustomRegAddr;
    private: System::Windows::Forms::GroupBox^  grpBox_IRSettings;
    private: System::Windows::Forms::CheckBox^  chkBox_IRExFilter;
    private: System::Windows::Forms::Label^  lbl_IRLed2Int;
    private: System::Windows::Forms::Label^  lbl_IRLed1Int;
    private: System::Windows::Forms::TrackBar^  trackBar_IRDimLeds;
    private: System::Windows::Forms::TrackBar^  trackBar_IRBrightLeds;
    private: System::Windows::Forms::CheckBox^  chkBox_IRStrobe;
    private: System::Windows::Forms::CheckBox^  chkBox_IRFlashlight;
    private: System::Windows::Forms::CheckBox^  chkBox_IRSelfie;
    private: System::Windows::Forms::CheckBox^  chkBox_IRDenoise;
    private: System::Windows::Forms::NumericUpDown^  numeric_IRDenoiseEdgeSmoothing;
    private: System::Windows::Forms::NumericUpDown^  numeric_IRDenoiseColorInterpolation;
    private: System::Windows::Forms::Label^  lbl_IRDenoise2;
    private: System::Windows::Forms::Label^  lbl_IRDenoise1;
    private: System::Windows::Forms::Label^  lbl_IRCustomReg;
    private: System::Windows::Forms::CheckBox^  chkBox_IRAutoExposure;
    private: System::Windows::Forms::GroupBox^  grpBox_IRSettingsDenoise;
    private: System::Windows::Forms::GroupBox^  grpBox_IRInfraredLight;
    private: System::Windows::Forms::GroupBox^  grpBox_editCalModel;
    private: System::Windows::Forms::Button^  btn_writeStickParams;
    private: System::Windows::Forms::GroupBox^  grpBox_CalUserAcc;
    private: System::Windows::Forms::Label^  lbl_CalGyroHelp;
    private: System::Windows::Forms::Label^  lbl_CalAccHelp;
    private: System::Windows::Forms::NumericUpDown^  numeric_CalEditAccX;
    private: System::Windows::Forms::NumericUpDown^  numeric_CalEditGyroX;
    private: System::Windows::Forms::NumericUpDown^  numeric_CalEditGyroY;
    private: System::Windows::Forms::NumericUpDown^  numeric_CalEditAccY;
    private: System::Windows::Forms::NumericUpDown^  numeric_CalEditGyroZ;
    private: System::Windows::Forms::NumericUpDown^  numeric_CalEditAccZ;
    private: System::Windows::Forms::CheckBox^  checkBox_enableSensorUserCal;
    private: System::Windows::Forms::GroupBox^  grpBox_StickDevParam;
    private: System::Windows::Forms::ToolStripMenuItem^  editCalibrationToolStripMenuItem;
    private: System::Windows::Forms::Label^  lbl_deadzone;
    private: System::Windows::Forms::NumericUpDown^  numeric_StickParamDeadzone;
    private: System::Windows::Forms::Label^  lbl_rangeReatio;
    private: System::Windows::Forms::NumericUpDown^  numeric_StickParamRangeRatio;
    private: System::Windows::Forms::Label^  lbl_editStickDevHelp;
    private: System::Windows::Forms::Label^  lbl_proStickHelp;
    private: System::Windows::Forms::Label^  lbl_mainStickHelp;
    private: System::Windows::Forms::NumericUpDown^  numeric_StickParamRangeRatio2;
    private: System::Windows::Forms::NumericUpDown^  numeric_StickParamDeadzone2;


    private: System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(images::typeid));

#pragma endregion

    private:
    /// <summary>
    /// Required designer variable.
    /// </summary>


#pragma region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        void InitializeComponent(void)
        {
            this->components = (gcnew System::ComponentModel::Container());
            System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(FormJoy::typeid));
            this->btn_writeColorsToSpi = (gcnew System::Windows::Forms::Button());
            this->grpBox_Color = (gcnew System::Windows::Forms::GroupBox());
            this->btn_changeGripsColor = (gcnew System::Windows::Forms::Button());
            this->lbl_Buttons_hex_txt = (gcnew System::Windows::Forms::Label());
            this->lbl_Body_hex_txt = (gcnew System::Windows::Forms::Label());
            this->btn_RestoreEnable = (gcnew System::Windows::Forms::Button());
            this->pictureBoxPreview = (gcnew System::Windows::Forms::PictureBox());
            this->btn_changeColor = (gcnew System::Windows::Forms::Button());
            this->btn_makeSPIBackup = (gcnew System::Windows::Forms::Button());
            this->grpBox_SPI = (gcnew System::Windows::Forms::GroupBox());
            this->btn_spiCancel = (gcnew System::Windows::Forms::Button());
            this->label_progress = (gcnew System::Windows::Forms::Label());
            this->lbl_spiProggressDesc = (gcnew System::Windows::Forms::Label());
            this->textBoxSN = (gcnew System::Windows::Forms::TextBox());
            this->label_sn = (gcnew System::Windows::Forms::Label());
            this->label_mac = (gcnew System::Windows::Forms::Label());
            this->label_fw = (gcnew System::Windows::Forms::Label());
            this->textBoxMAC = (gcnew System::Windows::Forms::TextBox());
            this->textBoxFW = (gcnew System::Windows::Forms::TextBox());
            this->textBoxDev = (gcnew System::Windows::Forms::TextBox());
            this->label_dev = (gcnew System::Windows::Forms::Label());
            this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
            this->hDRumblePlayerToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->iRCameraToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->menuToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->buttonTestToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->editCalibrationToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->debugToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->aboutToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->lbl_subcmdArgs = (gcnew System::Windows::Forms::Label());
            this->textBoxDbg_SubcmdArg = (gcnew System::Windows::Forms::TextBox());
            this->btn_dbgSendCmd = (gcnew System::Windows::Forms::Button());
            this->lbl_cmd = (gcnew System::Windows::Forms::Label());
            this->lbl_subcmd = (gcnew System::Windows::Forms::Label());
            this->textBoxDbg_cmd = (gcnew System::Windows::Forms::TextBox());
            this->textBoxDbg_subcmd = (gcnew System::Windows::Forms::TextBox());
            this->lbl_dbgDisclaimer = (gcnew System::Windows::Forms::Label());
            this->grpBox_DebugCmd = (gcnew System::Windows::Forms::GroupBox());
            this->textBoxDbg_reply_cmd = (gcnew System::Windows::Forms::TextBox());
            this->textBoxDbg_reply = (gcnew System::Windows::Forms::TextBox());
            this->textBoxDbg_sent = (gcnew System::Windows::Forms::TextBox());
            this->lbl_AmpL = (gcnew System::Windows::Forms::Label());
            this->lbl_AmpH = (gcnew System::Windows::Forms::Label());
            this->lbl_FreqL = (gcnew System::Windows::Forms::Label());
            this->lbl_FreqH = (gcnew System::Windows::Forms::Label());
            this->textBoxDbg_lfamp = (gcnew System::Windows::Forms::TextBox());
            this->textBoxDbg_lfreq = (gcnew System::Windows::Forms::TextBox());
            this->textBoxDbg_hamp = (gcnew System::Windows::Forms::TextBox());
            this->textBoxDbg_hfreq = (gcnew System::Windows::Forms::TextBox());
            this->lbl_dbgVib = (gcnew System::Windows::Forms::Label());
            this->grpBox_Restore = (gcnew System::Windows::Forms::GroupBox());
            this->comboBox_rstOption = (gcnew System::Windows::Forms::ComboBox());
            this->lbl_rstDesc = (gcnew System::Windows::Forms::Label());
            this->grpBox_RstUser = (gcnew System::Windows::Forms::GroupBox());
            this->checkBox_rst_accGyroCal = (gcnew System::Windows::Forms::CheckBox());
            this->checkBox_rst_R_StickCal = (gcnew System::Windows::Forms::CheckBox());
            this->checkBox_rst_L_StickCal = (gcnew System::Windows::Forms::CheckBox());
            this->btn_restore = (gcnew System::Windows::Forms::Button());
            this->txtBox_fileLoaded = (gcnew System::Windows::Forms::TextBox());
            this->label_rst_mac = (gcnew System::Windows::Forms::Label());
            this->btn_loadSPIBackup = (gcnew System::Windows::Forms::Button());
            this->lbl_rstDisclaimer = (gcnew System::Windows::Forms::Label());
            this->errorProvider1 = (gcnew System::Windows::Forms::ErrorProvider(this->components));
            this->errorProvider2 = (gcnew System::Windows::Forms::ErrorProvider(this->components));
            this->grpBox_ChangeSN = (gcnew System::Windows::Forms::GroupBox());
            this->btn_restoreSN = (gcnew System::Windows::Forms::Button());
            this->lbl_loadedMAC = (gcnew System::Windows::Forms::Label());
            this->label_sn_change_warning = (gcnew System::Windows::Forms::Label());
            this->btn_changeSN = (gcnew System::Windows::Forms::Button());
            this->textBox_chg_sn = (gcnew System::Windows::Forms::TextBox());
            this->toolTip1 = (gcnew System::Windows::Forms::ToolTip(this->components));
            this->lbl_exposure = (gcnew System::Windows::Forms::Label());
            this->numeric_IRExposure = (gcnew System::Windows::Forms::NumericUpDown());
            this->trackBar_IRGain = (gcnew System::Windows::Forms::TrackBar());
            this->grpBox_VibPlayer = (gcnew System::Windows::Forms::GroupBox());
            this->groupBox_vib_info = (gcnew System::Windows::Forms::GroupBox());
            this->textBox_vib_loop_times = (gcnew System::Windows::Forms::TextBox());
            this->label_hdrumble_filename = (gcnew System::Windows::Forms::Label());
            this->label_loop_times = (gcnew System::Windows::Forms::Label());
            this->label_vib_loaded = (gcnew System::Windows::Forms::Label());
            this->label_samplerate = (gcnew System::Windows::Forms::Label());
            this->label_samples = (gcnew System::Windows::Forms::Label());
            this->groupBox_vib_eq = (gcnew System::Windows::Forms::GroupBox());
            this->btn_vibResetEQ = (gcnew System::Windows::Forms::Button());
            this->label_eq_info = (gcnew System::Windows::Forms::Label());
            this->trackBar_hf_amp = (gcnew System::Windows::Forms::TrackBar());
            this->trackBar_lf_amp = (gcnew System::Windows::Forms::TrackBar());
            this->trackBar_hf_freq = (gcnew System::Windows::Forms::TrackBar());
            this->trackBar_lf_freq = (gcnew System::Windows::Forms::TrackBar());
            this->btn_vibPlay = (gcnew System::Windows::Forms::Button());
            this->btn_loadVib = (gcnew System::Windows::Forms::Button());
            this->btn_enableExpertMode = (gcnew System::Windows::Forms::Button());
            this->textBox_btn_test_reply = (gcnew System::Windows::Forms::TextBox());
            this->textBox_btn_test_subreply = (gcnew System::Windows::Forms::TextBox());
            this->grpBox_ButtonTest = (gcnew System::Windows::Forms::GroupBox());
            this->btn_runBtnTest = (gcnew System::Windows::Forms::Button());
            this->grpBox_StickCal = (gcnew System::Windows::Forms::GroupBox());
            this->textBox_lstick_fcal = (gcnew System::Windows::Forms::TextBox());
            this->textBox_rstick_fcal = (gcnew System::Windows::Forms::TextBox());
            this->textBox_rstick_ucal = (gcnew System::Windows::Forms::TextBox());
            this->textBox_lstick_ucal = (gcnew System::Windows::Forms::TextBox());
            this->textBox_6axis_ucal = (gcnew System::Windows::Forms::TextBox());
            this->textBox_6axis_cal = (gcnew System::Windows::Forms::TextBox());
            this->txtBox_devParameters = (gcnew System::Windows::Forms::TextBox());
            this->txtBox_devParameters2 = (gcnew System::Windows::Forms::TextBox());
            this->grpBox_dev_param = (gcnew System::Windows::Forms::GroupBox());
            this->toolStrip1 = (gcnew System::Windows::Forms::ToolStrip());
            this->toolStripBtn_batt = (gcnew System::Windows::Forms::ToolStripButton());
            this->toolStripLabel_batt = (gcnew System::Windows::Forms::ToolStripLabel());
            this->toolStripLabel_temp = (gcnew System::Windows::Forms::ToolStripLabel());
            this->toolStripBtn_refresh = (gcnew System::Windows::Forms::ToolStripButton());
            this->toolStripBtn_Disconnect = (gcnew System::Windows::Forms::ToolStripButton());
            this->panel_filler = (gcnew System::Windows::Forms::Panel());
            this->grpBox_accGyroCal = (gcnew System::Windows::Forms::GroupBox());
            this->grpBox_IR = (gcnew System::Windows::Forms::GroupBox());
            this->lbl_IRStatus = (gcnew System::Windows::Forms::Label());
            this->lbl_IRHelp = (gcnew System::Windows::Forms::Label());
            this->pictureBoxIR = (gcnew System::Windows::Forms::PictureBox());
            this->btn_getIRImage = (gcnew System::Windows::Forms::Button());
            this->btn_getIRStream = (gcnew System::Windows::Forms::Button());
            this->numeric_IRCustomRegVal = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_IRCustomRegAddr = (gcnew System::Windows::Forms::NumericUpDown());
            this->btn_IRConfigLive = (gcnew System::Windows::Forms::Button());
            this->lbl_digitalGain = (gcnew System::Windows::Forms::Label());
            this->grpBox_IRColorize = (gcnew System::Windows::Forms::GroupBox());
            this->radioBtn_IRColorHeat = (gcnew System::Windows::Forms::RadioButton());
            this->radioBtn_IRColorGreen = (gcnew System::Windows::Forms::RadioButton());
            this->radioBtn_IRColorRed = (gcnew System::Windows::Forms::RadioButton());
            this->radioBtn_IRColorGrey = (gcnew System::Windows::Forms::RadioButton());
            this->chkBox_IRDimLeds = (gcnew System::Windows::Forms::CheckBox());
            this->chkBox_IRBrightLeds = (gcnew System::Windows::Forms::CheckBox());
            this->grpBox_IRRes = (gcnew System::Windows::Forms::GroupBox());
            this->radioBtn_IR30p = (gcnew System::Windows::Forms::RadioButton());
            this->radioBtn_IR60p = (gcnew System::Windows::Forms::RadioButton());
            this->radioBtn_IR120p = (gcnew System::Windows::Forms::RadioButton());
            this->radioBtn_IR240p = (gcnew System::Windows::Forms::RadioButton());
            this->grpBox_nfc = (gcnew System::Windows::Forms::GroupBox());
            this->txtBox_nfcUid = (gcnew System::Windows::Forms::TextBox());
            this->btn_NFC = (gcnew System::Windows::Forms::Button());
            this->lbl_nfcHelp = (gcnew System::Windows::Forms::Label());
            this->checkBox_enableLeftUserCal = (gcnew System::Windows::Forms::CheckBox());
            this->numeric_leftUserCal_y_plus = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_leftUserCal_y_center = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_leftUserCal_y_minus = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_leftUserCal_x_plus = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_leftUserCal_x_center = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_leftUserCal_x_minus = (gcnew System::Windows::Forms::NumericUpDown());
            this->btn_refreshUserCal = (gcnew System::Windows::Forms::Button());
            this->checkBox_enableRightUserCal = (gcnew System::Windows::Forms::CheckBox());
            this->numeric_rightUserCal_y_plus = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_rightUserCal_y_center = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_rightUserCal_y_minus = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_rightUserCal_x_plus = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_rightUserCal_x_center = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_rightUserCal_x_minus = (gcnew System::Windows::Forms::NumericUpDown());
            this->grpBox_leftStickUCal = (gcnew System::Windows::Forms::GroupBox());
            this->lbl_userCalMinCenterMax = (gcnew System::Windows::Forms::Label());
            this->grpBox_rightStickUCal = (gcnew System::Windows::Forms::GroupBox());
            this->lbl_userCalMinCenterMax2 = (gcnew System::Windows::Forms::Label());
            this->btn_writeUserCal = (gcnew System::Windows::Forms::Button());
            this->grpBox_IRSettings = (gcnew System::Windows::Forms::GroupBox());
            this->grpBox_IRInfraredLight = (gcnew System::Windows::Forms::GroupBox());
            this->chkBox_IRExFilter = (gcnew System::Windows::Forms::CheckBox());
            this->trackBar_IRBrightLeds = (gcnew System::Windows::Forms::TrackBar());
            this->trackBar_IRDimLeds = (gcnew System::Windows::Forms::TrackBar());
            this->chkBox_IRStrobe = (gcnew System::Windows::Forms::CheckBox());
            this->lbl_IRLed1Int = (gcnew System::Windows::Forms::Label());
            this->chkBox_IRFlashlight = (gcnew System::Windows::Forms::CheckBox());
            this->lbl_IRLed2Int = (gcnew System::Windows::Forms::Label());
            this->chkBox_IRAutoExposure = (gcnew System::Windows::Forms::CheckBox());
            this->grpBox_IRSettingsDenoise = (gcnew System::Windows::Forms::GroupBox());
            this->numeric_IRDenoiseEdgeSmoothing = (gcnew System::Windows::Forms::NumericUpDown());
            this->chkBox_IRDenoise = (gcnew System::Windows::Forms::CheckBox());
            this->lbl_IRDenoise2 = (gcnew System::Windows::Forms::Label());
            this->numeric_IRDenoiseColorInterpolation = (gcnew System::Windows::Forms::NumericUpDown());
            this->lbl_IRDenoise1 = (gcnew System::Windows::Forms::Label());
            this->lbl_IRCustomReg = (gcnew System::Windows::Forms::Label());
            this->chkBox_IRSelfie = (gcnew System::Windows::Forms::CheckBox());
            this->grpBox_editCalModel = (gcnew System::Windows::Forms::GroupBox());
            this->lbl_editStickDevHelp = (gcnew System::Windows::Forms::Label());
            this->grpBox_CalUserAcc = (gcnew System::Windows::Forms::GroupBox());
            this->lbl_CalGyroHelp = (gcnew System::Windows::Forms::Label());
            this->lbl_CalAccHelp = (gcnew System::Windows::Forms::Label());
            this->numeric_CalEditAccX = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_CalEditGyroX = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_CalEditGyroY = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_CalEditAccY = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_CalEditGyroZ = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_CalEditAccZ = (gcnew System::Windows::Forms::NumericUpDown());
            this->checkBox_enableSensorUserCal = (gcnew System::Windows::Forms::CheckBox());
            this->grpBox_StickDevParam = (gcnew System::Windows::Forms::GroupBox());
            this->lbl_proStickHelp = (gcnew System::Windows::Forms::Label());
            this->lbl_mainStickHelp = (gcnew System::Windows::Forms::Label());
            this->numeric_StickParamRangeRatio2 = (gcnew System::Windows::Forms::NumericUpDown());
            this->numeric_StickParamDeadzone2 = (gcnew System::Windows::Forms::NumericUpDown());
            this->btn_writeStickParams = (gcnew System::Windows::Forms::Button());
            this->lbl_rangeReatio = (gcnew System::Windows::Forms::Label());
            this->numeric_StickParamRangeRatio = (gcnew System::Windows::Forms::NumericUpDown());
            this->lbl_deadzone = (gcnew System::Windows::Forms::Label());
            this->numeric_StickParamDeadzone = (gcnew System::Windows::Forms::NumericUpDown());
            this->grpBox_Color->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxPreview))->BeginInit();
            this->grpBox_SPI->SuspendLayout();
            this->menuStrip1->SuspendLayout();
            this->grpBox_DebugCmd->SuspendLayout();
            this->grpBox_Restore->SuspendLayout();
            this->grpBox_RstUser->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->errorProvider1))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->errorProvider2))->BeginInit();
            this->grpBox_ChangeSN->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_IRExposure))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_IRGain))->BeginInit();
            this->grpBox_VibPlayer->SuspendLayout();
            this->groupBox_vib_info->SuspendLayout();
            this->groupBox_vib_eq->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_hf_amp))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_lf_amp))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_hf_freq))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_lf_freq))->BeginInit();
            this->grpBox_ButtonTest->SuspendLayout();
            this->grpBox_StickCal->SuspendLayout();
            this->grpBox_dev_param->SuspendLayout();
            this->toolStrip1->SuspendLayout();
            this->grpBox_accGyroCal->SuspendLayout();
            this->grpBox_IR->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxIR))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_IRCustomRegVal))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_IRCustomRegAddr))->BeginInit();
            this->grpBox_IRColorize->SuspendLayout();
            this->grpBox_IRRes->SuspendLayout();
            this->grpBox_nfc->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_leftUserCal_y_plus))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_leftUserCal_y_center))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_leftUserCal_y_minus))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_leftUserCal_x_plus))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_leftUserCal_x_center))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_leftUserCal_x_minus))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_rightUserCal_y_plus))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_rightUserCal_y_center))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_rightUserCal_y_minus))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_rightUserCal_x_plus))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_rightUserCal_x_center))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_rightUserCal_x_minus))->BeginInit();
            this->grpBox_leftStickUCal->SuspendLayout();
            this->grpBox_rightStickUCal->SuspendLayout();
            this->grpBox_IRSettings->SuspendLayout();
            this->grpBox_IRInfraredLight->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_IRBrightLeds))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_IRDimLeds))->BeginInit();
            this->grpBox_IRSettingsDenoise->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_IRDenoiseEdgeSmoothing))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_IRDenoiseColorInterpolation))->BeginInit();
            this->grpBox_editCalModel->SuspendLayout();
            this->grpBox_CalUserAcc->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_CalEditAccX))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_CalEditGyroX))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_CalEditGyroY))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_CalEditAccY))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_CalEditGyroZ))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_CalEditAccZ))->BeginInit();
            this->grpBox_StickDevParam->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_StickParamRangeRatio2))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_StickParamDeadzone2))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_StickParamRangeRatio))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_StickParamDeadzone))->BeginInit();
            this->SuspendLayout();
            // 
            // btn_writeColorsToSpi
            // 
            this->btn_writeColorsToSpi->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_writeColorsToSpi->Enabled = false;
            this->btn_writeColorsToSpi->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_writeColorsToSpi->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_writeColorsToSpi->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->btn_writeColorsToSpi->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->btn_writeColorsToSpi->Location = System::Drawing::Point(316, 252);
            this->btn_writeColorsToSpi->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->btn_writeColorsToSpi->Name = L"btn_writeColorsToSpi";
            this->btn_writeColorsToSpi->Size = System::Drawing::Size(128, 48);
            this->btn_writeColorsToSpi->TabIndex = 1;
            this->btn_writeColorsToSpi->Text = L"Write Colors";
            this->btn_writeColorsToSpi->UseVisualStyleBackColor = false;
            this->btn_writeColorsToSpi->Click += gcnew System::EventHandler(this, &FormJoy::btn_writeColorsToSpi_Click);
            // 
            // grpBox_Color
            // 
            this->grpBox_Color->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_Color->Controls->Add(this->btn_changeGripsColor);
            this->grpBox_Color->Controls->Add(this->lbl_Buttons_hex_txt);
            this->grpBox_Color->Controls->Add(this->lbl_Body_hex_txt);
            this->grpBox_Color->Controls->Add(this->btn_RestoreEnable);
            this->grpBox_Color->Controls->Add(this->pictureBoxPreview);
            this->grpBox_Color->Controls->Add(this->btn_changeColor);
            this->grpBox_Color->Controls->Add(this->btn_makeSPIBackup);
            this->grpBox_Color->Controls->Add(this->btn_writeColorsToSpi);
            this->grpBox_Color->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_Color->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->grpBox_Color->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_Color->Location = System::Drawing::Point(14, 120);
            this->grpBox_Color->Margin = System::Windows::Forms::Padding(0, 0, 14, 39);
            this->grpBox_Color->Name = L"grpBox_Color";
            this->grpBox_Color->Padding = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->grpBox_Color->Size = System::Drawing::Size(456, 315);
            this->grpBox_Color->TabIndex = 0;
            this->grpBox_Color->TabStop = false;
            this->grpBox_Color->Text = L"Device colors";
            // 
            // btn_changeGripsColor
            // 
            this->btn_changeGripsColor->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_changeGripsColor->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(80)),
                static_cast<System::Int32>(static_cast<System::Byte>(80)), static_cast<System::Int32>(static_cast<System::Byte>(80)));
            this->btn_changeGripsColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_changeGripsColor->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 9.75F));
            this->btn_changeGripsColor->Location = System::Drawing::Point(316, 114);
            this->btn_changeGripsColor->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->btn_changeGripsColor->Name = L"btn_changeGripsColor";
            this->btn_changeGripsColor->Size = System::Drawing::Size(128, 48);
            this->btn_changeGripsColor->TabIndex = 22;
            this->btn_changeGripsColor->Text = L"Grips Color";
            this->btn_changeGripsColor->UseVisualStyleBackColor = false;
            this->btn_changeGripsColor->Click += gcnew System::EventHandler(this, &FormJoy::btn_changeGripsColor_Click);
            // 
            // lbl_Buttons_hex_txt
            // 
            this->lbl_Buttons_hex_txt->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->lbl_Buttons_hex_txt->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_Buttons_hex_txt->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(150)),
                static_cast<System::Int32>(static_cast<System::Byte>(150)), static_cast<System::Int32>(static_cast<System::Byte>(150)));
            this->lbl_Buttons_hex_txt->Location = System::Drawing::Point(316, 208);
            this->lbl_Buttons_hex_txt->Margin = System::Windows::Forms::Padding(0);
            this->lbl_Buttons_hex_txt->Name = L"lbl_Buttons_hex_txt";
            this->lbl_Buttons_hex_txt->Size = System::Drawing::Size(128, 24);
            this->lbl_Buttons_hex_txt->TabIndex = 21;
            this->lbl_Buttons_hex_txt->Text = L"#Buttons Color";
            this->lbl_Buttons_hex_txt->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
            // 
            // lbl_Body_hex_txt
            // 
            this->lbl_Body_hex_txt->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->lbl_Body_hex_txt->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_Body_hex_txt->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(150)),
                static_cast<System::Int32>(static_cast<System::Byte>(150)), static_cast<System::Int32>(static_cast<System::Byte>(150)));
            this->lbl_Body_hex_txt->Location = System::Drawing::Point(316, 184);
            this->lbl_Body_hex_txt->Margin = System::Windows::Forms::Padding(0);
            this->lbl_Body_hex_txt->Name = L"lbl_Body_hex_txt";
            this->lbl_Body_hex_txt->Size = System::Drawing::Size(128, 24);
            this->lbl_Body_hex_txt->TabIndex = 20;
            this->lbl_Body_hex_txt->Text = L"#Body Color";
            this->lbl_Body_hex_txt->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
            // 
            // btn_RestoreEnable
            // 
            this->btn_RestoreEnable->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_RestoreEnable->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_RestoreEnable->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_RestoreEnable->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 9.75F, System::Drawing::FontStyle::Bold));
            this->btn_RestoreEnable->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->btn_RestoreEnable->Location = System::Drawing::Point(11, 25);
            this->btn_RestoreEnable->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->btn_RestoreEnable->Name = L"btn_RestoreEnable";
            this->btn_RestoreEnable->Size = System::Drawing::Size(144, 36);
            this->btn_RestoreEnable->TabIndex = 18;
            this->btn_RestoreEnable->Text = L"Restore SPI";
            this->btn_RestoreEnable->UseVisualStyleBackColor = false;
            this->btn_RestoreEnable->Click += gcnew System::EventHandler(this, &FormJoy::btn_RestoreEnable_Click);
            // 
            // pictureBoxPreview
            // 
            this->pictureBoxPreview->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->pictureBoxPreview->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Zoom;
            this->pictureBoxPreview->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->pictureBoxPreview->Location = System::Drawing::Point(2, 90);
            this->pictureBoxPreview->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->pictureBoxPreview->Name = L"pictureBoxPreview";
            this->pictureBoxPreview->Size = System::Drawing::Size(312, 192);
            this->pictureBoxPreview->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
            this->pictureBoxPreview->TabIndex = 15;
            this->pictureBoxPreview->TabStop = false;
            // 
            // btn_changeColor
            // 
            this->btn_changeColor->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_changeColor->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(80)),
                static_cast<System::Int32>(static_cast<System::Byte>(80)), static_cast<System::Int32>(static_cast<System::Byte>(80)));
            this->btn_changeColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_changeColor->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 9.75F));
            this->btn_changeColor->Location = System::Drawing::Point(316, 66);
            this->btn_changeColor->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->btn_changeColor->Name = L"btn_changeColor";
            this->btn_changeColor->Size = System::Drawing::Size(128, 48);
            this->btn_changeColor->TabIndex = 19;
            this->btn_changeColor->Text = L"Body && Buttons Color";
            this->btn_changeColor->UseVisualStyleBackColor = false;
            this->btn_changeColor->Click += gcnew System::EventHandler(this, &FormJoy::btn_changeNormalColor_Click);
            // 
            // btn_makeSPIBackup
            // 
            this->btn_makeSPIBackup->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_makeSPIBackup->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_makeSPIBackup->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_makeSPIBackup->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 9.75F, System::Drawing::FontStyle::Bold));
            this->btn_makeSPIBackup->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->btn_makeSPIBackup->Location = System::Drawing::Point(169, 25);
            this->btn_makeSPIBackup->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->btn_makeSPIBackup->Name = L"btn_makeSPIBackup";
            this->btn_makeSPIBackup->Size = System::Drawing::Size(144, 36);
            this->btn_makeSPIBackup->TabIndex = 1;
            this->btn_makeSPIBackup->Text = L"Backup SPI";
            this->btn_makeSPIBackup->UseVisualStyleBackColor = false;
            this->btn_makeSPIBackup->Click += gcnew System::EventHandler(this, &FormJoy::btn_makeSPIBackup_Click);
            // 
            // grpBox_SPI
            // 
            this->grpBox_SPI->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_SPI->Controls->Add(this->btn_spiCancel);
            this->grpBox_SPI->Controls->Add(this->label_progress);
            this->grpBox_SPI->Controls->Add(this->lbl_spiProggressDesc);
            this->grpBox_SPI->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_SPI->Location = System::Drawing::Point(14, 120);
            this->grpBox_SPI->Margin = System::Windows::Forms::Padding(0, 0, 14, 39);
            this->grpBox_SPI->Name = L"grpBox_SPI";
            this->grpBox_SPI->Padding = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->grpBox_SPI->Size = System::Drawing::Size(456, 315);
            this->grpBox_SPI->TabIndex = 14;
            this->grpBox_SPI->TabStop = false;
            // 
            // btn_spiCancel
            // 
            this->btn_spiCancel->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_spiCancel->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->btn_spiCancel->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_spiCancel->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btn_spiCancel->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->btn_spiCancel->Location = System::Drawing::Point(187, 250);
            this->btn_spiCancel->Name = L"btn_spiCancel";
            this->btn_spiCancel->Size = System::Drawing::Size(75, 34);
            this->btn_spiCancel->TabIndex = 35;
            this->btn_spiCancel->Text = L"Cancel";
            this->btn_spiCancel->UseVisualStyleBackColor = false;
            this->btn_spiCancel->Click += gcnew System::EventHandler(this, &FormJoy::btn_spiCancel_Click);
            // 
            // label_progress
            // 
            this->label_progress->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->label_progress->Location = System::Drawing::Point(87, 194);
            this->label_progress->Name = L"label_progress";
            this->label_progress->Size = System::Drawing::Size(279, 31);
            this->label_progress->TabIndex = 1;
            this->label_progress->Text = L"Please wait..";
            this->label_progress->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
            // 
            // lbl_spiProggressDesc
            // 
            this->lbl_spiProggressDesc->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->lbl_spiProggressDesc->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_spiProggressDesc->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->lbl_spiProggressDesc->Location = System::Drawing::Point(87, 87);
            this->lbl_spiProggressDesc->Name = L"lbl_spiProggressDesc";
            this->lbl_spiProggressDesc->Size = System::Drawing::Size(279, 97);
            this->lbl_spiProggressDesc->TabIndex = 0;
            this->lbl_spiProggressDesc->Text = L"Dumping SPI flash chip!\nThis will take around 10 minutes...\n\nDon\'t disconnect you"
                L"r device!";
            this->lbl_spiProggressDesc->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
            // 
            // textBoxSN
            // 
            this->textBoxSN->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBoxSN->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBoxSN->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBoxSN->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->textBoxSN->Location = System::Drawing::Point(72, 42);
            this->textBoxSN->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->textBoxSN->MaxLength = 16;
            this->textBoxSN->Name = L"textBoxSN";
            this->textBoxSN->ReadOnly = true;
            this->textBoxSN->Size = System::Drawing::Size(138, 20);
            this->textBoxSN->TabIndex = 2;
            this->textBoxSN->TabStop = false;
            this->textBoxSN->Text = L"No S/N";
            // 
            // label_sn
            // 
            this->label_sn->AutoSize = true;
            this->label_sn->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->label_sn->Cursor = System::Windows::Forms::Cursors::Hand;
            this->label_sn->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->label_sn->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->label_sn->Location = System::Drawing::Point(10, 42);
            this->label_sn->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->label_sn->Name = L"label_sn";
            this->label_sn->Size = System::Drawing::Size(39, 20);
            this->label_sn->TabIndex = 3;
            this->label_sn->Text = L"S/N:";
            this->label_sn->Click += gcnew System::EventHandler(this, &FormJoy::label_sn_Click);
            // 
            // label_mac
            // 
            this->label_mac->AutoSize = true;
            this->label_mac->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->label_mac->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->label_mac->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->label_mac->Location = System::Drawing::Point(10, 83);
            this->label_mac->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->label_mac->Name = L"label_mac";
            this->label_mac->Size = System::Drawing::Size(46, 20);
            this->label_mac->TabIndex = 8;
            this->label_mac->Text = L"MAC:";
            // 
            // label_fw
            // 
            this->label_fw->AutoSize = true;
            this->label_fw->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->label_fw->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->label_fw->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->label_fw->Location = System::Drawing::Point(234, 42);
            this->label_fw->Name = L"label_fw";
            this->label_fw->Size = System::Drawing::Size(90, 20);
            this->label_fw->TabIndex = 9;
            this->label_fw->Text = L"FW Version:";
            // 
            // textBoxMAC
            // 
            this->textBoxMAC->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBoxMAC->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBoxMAC->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBoxMAC->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->textBoxMAC->Location = System::Drawing::Point(72, 83);
            this->textBoxMAC->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->textBoxMAC->Name = L"textBoxMAC";
            this->textBoxMAC->ReadOnly = true;
            this->textBoxMAC->Size = System::Drawing::Size(138, 20);
            this->textBoxMAC->TabIndex = 10;
            this->textBoxMAC->TabStop = false;
            this->textBoxMAC->Text = L"00:00:00:00:00:00";
            // 
            // textBoxFW
            // 
            this->textBoxFW->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBoxFW->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBoxFW->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBoxFW->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->textBoxFW->Location = System::Drawing::Point(351, 42);
            this->textBoxFW->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->textBoxFW->Name = L"textBoxFW";
            this->textBoxFW->ReadOnly = true;
            this->textBoxFW->Size = System::Drawing::Size(114, 20);
            this->textBoxFW->TabIndex = 11;
            this->textBoxFW->TabStop = false;
            this->textBoxFW->Text = L"0.00";
            // 
            // textBoxDev
            // 
            this->textBoxDev->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBoxDev->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBoxDev->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBoxDev->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->textBoxDev->Location = System::Drawing::Point(351, 83);
            this->textBoxDev->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->textBoxDev->Name = L"textBoxDev";
            this->textBoxDev->ReadOnly = true;
            this->textBoxDev->Size = System::Drawing::Size(114, 20);
            this->textBoxDev->TabIndex = 13;
            this->textBoxDev->TabStop = false;
            this->textBoxDev->Text = L"Controller";
            // 
            // label_dev
            // 
            this->label_dev->AutoSize = true;
            this->label_dev->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->label_dev->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->label_dev->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->label_dev->Location = System::Drawing::Point(234, 83);
            this->label_dev->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->label_dev->Name = L"label_dev";
            this->label_dev->Size = System::Drawing::Size(83, 20);
            this->label_dev->TabIndex = 12;
            this->label_dev->Text = L"Controller:";
            // 
            // menuStrip1
            // 
            this->menuStrip1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(55)), static_cast<System::Int32>(static_cast<System::Byte>(55)),
                static_cast<System::Int32>(static_cast<System::Byte>(55)));
            this->menuStrip1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {
                this->hDRumblePlayerToolStripMenuItem,
                    this->iRCameraToolStripMenuItem, this->menuToolStripMenuItem
            });
            this->menuStrip1->Location = System::Drawing::Point(0, 0);
            this->menuStrip1->Name = L"menuStrip1";
            this->menuStrip1->Size = System::Drawing::Size(1925, 25);
            this->menuStrip1->TabIndex = 0;
            this->menuStrip1->Text = L"menuStrip1";
            // 
            // hDRumblePlayerToolStripMenuItem
            // 
            this->hDRumblePlayerToolStripMenuItem->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(55)),
                static_cast<System::Int32>(static_cast<System::Byte>(55)), static_cast<System::Int32>(static_cast<System::Byte>(55)));
            this->hDRumblePlayerToolStripMenuItem->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->hDRumblePlayerToolStripMenuItem->Name = L"hDRumblePlayerToolStripMenuItem";
            this->hDRumblePlayerToolStripMenuItem->Size = System::Drawing::Size(125, 21);
            this->hDRumblePlayerToolStripMenuItem->Text = L"HD Rumble Player";
            this->hDRumblePlayerToolStripMenuItem->Click += gcnew System::EventHandler(this, &FormJoy::btnPlayVibEnable_Click);
            // 
            // iRCameraToolStripMenuItem
            // 
            this->iRCameraToolStripMenuItem->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(55)),
                static_cast<System::Int32>(static_cast<System::Byte>(55)), static_cast<System::Int32>(static_cast<System::Byte>(55)));
            this->iRCameraToolStripMenuItem->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->iRCameraToolStripMenuItem->Name = L"iRCameraToolStripMenuItem";
            this->iRCameraToolStripMenuItem->Size = System::Drawing::Size(80, 21);
            this->iRCameraToolStripMenuItem->Text = L"IR Camera";
            this->iRCameraToolStripMenuItem->Click += gcnew System::EventHandler(this, &FormJoy::iRCameraToolStripMenuItem_Click);
            // 
            // menuToolStripMenuItem
            // 
            this->menuToolStripMenuItem->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(55)),
                static_cast<System::Int32>(static_cast<System::Byte>(55)), static_cast<System::Int32>(static_cast<System::Byte>(55)));
            this->menuToolStripMenuItem->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->menuToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {
                this->buttonTestToolStripMenuItem,
                    this->editCalibrationToolStripMenuItem, this->debugToolStripMenuItem, this->aboutToolStripMenuItem
            });
            this->menuToolStripMenuItem->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->menuToolStripMenuItem->Name = L"menuToolStripMenuItem";
            this->menuToolStripMenuItem->Size = System::Drawing::Size(61, 21);
            this->menuToolStripMenuItem->Text = L"More...";
            // 
            // buttonTestToolStripMenuItem
            // 
            this->buttonTestToolStripMenuItem->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->buttonTestToolStripMenuItem->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->buttonTestToolStripMenuItem->Name = L"buttonTestToolStripMenuItem";
            this->buttonTestToolStripMenuItem->ShowShortcutKeys = false;
            this->buttonTestToolStripMenuItem->Size = System::Drawing::Size(177, 22);
            this->buttonTestToolStripMenuItem->Text = L"Playground testing";
            this->buttonTestToolStripMenuItem->Click += gcnew System::EventHandler(this, &FormJoy::buttonTestToolStripMenuItem_Click);
            // 
            // editCalibrationToolStripMenuItem
            // 
            this->editCalibrationToolStripMenuItem->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->editCalibrationToolStripMenuItem->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->editCalibrationToolStripMenuItem->Name = L"editCalibrationToolStripMenuItem";
            this->editCalibrationToolStripMenuItem->ShowShortcutKeys = false;
            this->editCalibrationToolStripMenuItem->Size = System::Drawing::Size(177, 22);
            this->editCalibrationToolStripMenuItem->Text = L"Edit Calibration";
            this->editCalibrationToolStripMenuItem->Click += gcnew System::EventHandler(this, &FormJoy::editCalibrationToolStripMenuItem_Click);
            // 
            // debugToolStripMenuItem
            // 
            this->debugToolStripMenuItem->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->debugToolStripMenuItem->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->debugToolStripMenuItem->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->debugToolStripMenuItem->Name = L"debugToolStripMenuItem";
            this->debugToolStripMenuItem->ShowShortcutKeys = false;
            this->debugToolStripMenuItem->Size = System::Drawing::Size(177, 22);
            this->debugToolStripMenuItem->Text = L"Debug";
            this->debugToolStripMenuItem->Click += gcnew System::EventHandler(this, &FormJoy::debugToolStripMenuItem_Click);
            // 
            // aboutToolStripMenuItem
            // 
            this->aboutToolStripMenuItem->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->aboutToolStripMenuItem->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->aboutToolStripMenuItem->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->aboutToolStripMenuItem->Name = L"aboutToolStripMenuItem";
            this->aboutToolStripMenuItem->ShowShortcutKeys = false;
            this->aboutToolStripMenuItem->Size = System::Drawing::Size(177, 22);
            this->aboutToolStripMenuItem->Text = L"About";
            this->aboutToolStripMenuItem->Click += gcnew System::EventHandler(this, &FormJoy::aboutToolStripMenuItem_Click);
            // 
            // lbl_subcmdArgs
            // 
            this->lbl_subcmdArgs->AutoSize = true;
            this->lbl_subcmdArgs->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_subcmdArgs->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->lbl_subcmdArgs->Location = System::Drawing::Point(13, 121);
            this->lbl_subcmdArgs->Name = L"lbl_subcmdArgs";
            this->lbl_subcmdArgs->Size = System::Drawing::Size(124, 17);
            this->lbl_subcmdArgs->TabIndex = 15;
            this->lbl_subcmdArgs->Text = L"Subcmd arguments:";
            // 
            // textBoxDbg_SubcmdArg
            // 
            this->textBoxDbg_SubcmdArg->AutoCompleteSource = System::Windows::Forms::AutoCompleteSource::HistoryList;
            this->textBoxDbg_SubcmdArg->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->textBoxDbg_SubcmdArg->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->textBoxDbg_SubcmdArg->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->textBoxDbg_SubcmdArg->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->textBoxDbg_SubcmdArg->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->textBoxDbg_SubcmdArg->Location = System::Drawing::Point(16, 141);
            this->textBoxDbg_SubcmdArg->Margin = System::Windows::Forms::Padding(0);
            this->textBoxDbg_SubcmdArg->MaxLength = 76;
            this->textBoxDbg_SubcmdArg->Multiline = true;
            this->textBoxDbg_SubcmdArg->Name = L"textBoxDbg_SubcmdArg";
            this->textBoxDbg_SubcmdArg->Size = System::Drawing::Size(188, 44);
            this->textBoxDbg_SubcmdArg->TabIndex = 25;
            this->textBoxDbg_SubcmdArg->Text = L"00";
            this->textBoxDbg_SubcmdArg->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            // 
            // btn_dbgSendCmd
            // 
            this->btn_dbgSendCmd->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_dbgSendCmd->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->btn_dbgSendCmd->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_dbgSendCmd->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btn_dbgSendCmd->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->btn_dbgSendCmd->Location = System::Drawing::Point(73, 193);
            this->btn_dbgSendCmd->Name = L"btn_dbgSendCmd";
            this->btn_dbgSendCmd->Size = System::Drawing::Size(75, 34);
            this->btn_dbgSendCmd->TabIndex = 17;
            this->btn_dbgSendCmd->Text = L"Send";
            this->btn_dbgSendCmd->UseVisualStyleBackColor = false;
            this->btn_dbgSendCmd->Click += gcnew System::EventHandler(this, &FormJoy::btn_dbgSendCmd_Click);
            // 
            // lbl_cmd
            // 
            this->lbl_cmd->AutoSize = true;
            this->lbl_cmd->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_cmd->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->lbl_cmd->Location = System::Drawing::Point(13, 94);
            this->lbl_cmd->Name = L"lbl_cmd";
            this->lbl_cmd->Size = System::Drawing::Size(38, 17);
            this->lbl_cmd->TabIndex = 18;
            this->lbl_cmd->Text = L"Cmd:";
            // 
            // lbl_subcmd
            // 
            this->lbl_subcmd->AutoSize = true;
            this->lbl_subcmd->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_subcmd->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->lbl_subcmd->Location = System::Drawing::Point(106, 94);
            this->lbl_subcmd->Name = L"lbl_subcmd";
            this->lbl_subcmd->Size = System::Drawing::Size(58, 17);
            this->lbl_subcmd->TabIndex = 19;
            this->lbl_subcmd->Text = L"Subcmd:";
            // 
            // textBoxDbg_cmd
            // 
            this->textBoxDbg_cmd->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->textBoxDbg_cmd->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->textBoxDbg_cmd->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->textBoxDbg_cmd->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->textBoxDbg_cmd->Location = System::Drawing::Point(64, 92);
            this->textBoxDbg_cmd->MaxLength = 2;
            this->textBoxDbg_cmd->Name = L"textBoxDbg_cmd";
            this->textBoxDbg_cmd->Size = System::Drawing::Size(35, 25);
            this->textBoxDbg_cmd->TabIndex = 16;
            this->textBoxDbg_cmd->Text = L"01";
            this->textBoxDbg_cmd->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            // 
            // textBoxDbg_subcmd
            // 
            this->textBoxDbg_subcmd->AutoCompleteSource = System::Windows::Forms::AutoCompleteSource::HistoryList;
            this->textBoxDbg_subcmd->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->textBoxDbg_subcmd->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->textBoxDbg_subcmd->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->textBoxDbg_subcmd->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->textBoxDbg_subcmd->Location = System::Drawing::Point(169, 92);
            this->textBoxDbg_subcmd->MaxLength = 2;
            this->textBoxDbg_subcmd->Name = L"textBoxDbg_subcmd";
            this->textBoxDbg_subcmd->Size = System::Drawing::Size(35, 25);
            this->textBoxDbg_subcmd->TabIndex = 24;
            this->textBoxDbg_subcmd->Text = L"00";
            this->textBoxDbg_subcmd->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            // 
            // lbl_dbgDisclaimer
            // 
            this->lbl_dbgDisclaimer->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_dbgDisclaimer->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->lbl_dbgDisclaimer->Location = System::Drawing::Point(26, 275);
            this->lbl_dbgDisclaimer->Name = L"lbl_dbgDisclaimer";
            this->lbl_dbgDisclaimer->Size = System::Drawing::Size(170, 97);
            this->lbl_dbgDisclaimer->TabIndex = 22;
            this->lbl_dbgDisclaimer->Text = L"The debug feature is only for developer use!\r\n\r\nNo one is responsible for any com"
                L"mand sent.";
            // 
            // grpBox_DebugCmd
            // 
            this->grpBox_DebugCmd->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_DebugCmd->Controls->Add(this->textBoxDbg_reply_cmd);
            this->grpBox_DebugCmd->Controls->Add(this->textBoxDbg_reply);
            this->grpBox_DebugCmd->Controls->Add(this->textBoxDbg_sent);
            this->grpBox_DebugCmd->Controls->Add(this->lbl_AmpL);
            this->grpBox_DebugCmd->Controls->Add(this->lbl_AmpH);
            this->grpBox_DebugCmd->Controls->Add(this->lbl_FreqL);
            this->grpBox_DebugCmd->Controls->Add(this->lbl_FreqH);
            this->grpBox_DebugCmd->Controls->Add(this->textBoxDbg_lfamp);
            this->grpBox_DebugCmd->Controls->Add(this->textBoxDbg_lfreq);
            this->grpBox_DebugCmd->Controls->Add(this->textBoxDbg_hamp);
            this->grpBox_DebugCmd->Controls->Add(this->textBoxDbg_hfreq);
            this->grpBox_DebugCmd->Controls->Add(this->lbl_dbgVib);
            this->grpBox_DebugCmd->Controls->Add(this->lbl_subcmdArgs);
            this->grpBox_DebugCmd->Controls->Add(this->lbl_dbgDisclaimer);
            this->grpBox_DebugCmd->Controls->Add(this->textBoxDbg_SubcmdArg);
            this->grpBox_DebugCmd->Controls->Add(this->textBoxDbg_subcmd);
            this->grpBox_DebugCmd->Controls->Add(this->btn_dbgSendCmd);
            this->grpBox_DebugCmd->Controls->Add(this->textBoxDbg_cmd);
            this->grpBox_DebugCmd->Controls->Add(this->lbl_cmd);
            this->grpBox_DebugCmd->Controls->Add(this->lbl_subcmd);
            this->grpBox_DebugCmd->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_DebugCmd->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_DebugCmd->Location = System::Drawing::Point(494, 36);
            this->grpBox_DebugCmd->Margin = System::Windows::Forms::Padding(0, 0, 14, 0);
            this->grpBox_DebugCmd->Name = L"grpBox_DebugCmd";
            this->grpBox_DebugCmd->Size = System::Drawing::Size(220, 399);
            this->grpBox_DebugCmd->TabIndex = 23;
            this->grpBox_DebugCmd->TabStop = false;
            this->grpBox_DebugCmd->Text = L"Debug: Custom Command";
            // 
            // textBoxDbg_reply_cmd
            // 
            this->textBoxDbg_reply_cmd->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBoxDbg_reply_cmd->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBoxDbg_reply_cmd->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->textBoxDbg_reply_cmd->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)),
                static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->textBoxDbg_reply_cmd->Location = System::Drawing::Point(26, 278);
            this->textBoxDbg_reply_cmd->Margin = System::Windows::Forms::Padding(0);
            this->textBoxDbg_reply_cmd->Multiline = true;
            this->textBoxDbg_reply_cmd->Name = L"textBoxDbg_reply_cmd";
            this->textBoxDbg_reply_cmd->ReadOnly = true;
            this->textBoxDbg_reply_cmd->Size = System::Drawing::Size(170, 45);
            this->textBoxDbg_reply_cmd->TabIndex = 34;
            this->textBoxDbg_reply_cmd->TabStop = false;
            this->textBoxDbg_reply_cmd->Text = L"Reply buttons and sticks";
            this->textBoxDbg_reply_cmd->Visible = false;
            // 
            // textBoxDbg_reply
            // 
            this->textBoxDbg_reply->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBoxDbg_reply->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBoxDbg_reply->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->textBoxDbg_reply->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->textBoxDbg_reply->Location = System::Drawing::Point(26, 324);
            this->textBoxDbg_reply->Margin = System::Windows::Forms::Padding(0);
            this->textBoxDbg_reply->Multiline = true;
            this->textBoxDbg_reply->Name = L"textBoxDbg_reply";
            this->textBoxDbg_reply->ReadOnly = true;
            this->textBoxDbg_reply->Size = System::Drawing::Size(170, 69);
            this->textBoxDbg_reply->TabIndex = 33;
            this->textBoxDbg_reply->TabStop = false;
            this->textBoxDbg_reply->Text = L"Reply text";
            this->textBoxDbg_reply->Visible = false;
            // 
            // textBoxDbg_sent
            // 
            this->textBoxDbg_sent->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBoxDbg_sent->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBoxDbg_sent->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->textBoxDbg_sent->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->textBoxDbg_sent->Location = System::Drawing::Point(26, 234);
            this->textBoxDbg_sent->Margin = System::Windows::Forms::Padding(0);
            this->textBoxDbg_sent->Multiline = true;
            this->textBoxDbg_sent->Name = L"textBoxDbg_sent";
            this->textBoxDbg_sent->ReadOnly = true;
            this->textBoxDbg_sent->Size = System::Drawing::Size(170, 47);
            this->textBoxDbg_sent->TabIndex = 32;
            this->textBoxDbg_sent->TabStop = false;
            this->textBoxDbg_sent->Text = L"Sent text";
            this->textBoxDbg_sent->Visible = false;
            // 
            // lbl_AmpL
            // 
            this->lbl_AmpL->AutoSize = true;
            this->lbl_AmpL->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_AmpL->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(200)),
                static_cast<System::Int32>(static_cast<System::Byte>(200)));
            this->lbl_AmpL->Location = System::Drawing::Point(114, 64);
            this->lbl_AmpL->Name = L"lbl_AmpL";
            this->lbl_AmpL->Size = System::Drawing::Size(47, 17);
            this->lbl_AmpL->TabIndex = 31;
            this->lbl_AmpL->Text = L"L.Amp:";
            // 
            // lbl_AmpH
            // 
            this->lbl_AmpH->AutoSize = true;
            this->lbl_AmpH->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_AmpH->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(200)),
                static_cast<System::Int32>(static_cast<System::Byte>(200)));
            this->lbl_AmpH->Location = System::Drawing::Point(114, 42);
            this->lbl_AmpH->Name = L"lbl_AmpH";
            this->lbl_AmpH->Size = System::Drawing::Size(50, 17);
            this->lbl_AmpH->TabIndex = 30;
            this->lbl_AmpH->Text = L"H.Amp:";
            // 
            // lbl_FreqL
            // 
            this->lbl_FreqL->AutoSize = true;
            this->lbl_FreqL->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_FreqL->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(200)),
                static_cast<System::Int32>(static_cast<System::Byte>(200)));
            this->lbl_FreqL->Location = System::Drawing::Point(13, 64);
            this->lbl_FreqL->Name = L"lbl_FreqL";
            this->lbl_FreqL->Size = System::Drawing::Size(46, 17);
            this->lbl_FreqL->TabIndex = 29;
            this->lbl_FreqL->Text = L"L.Freq:";
            // 
            // lbl_FreqH
            // 
            this->lbl_FreqH->AutoSize = true;
            this->lbl_FreqH->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_FreqH->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(200)),
                static_cast<System::Int32>(static_cast<System::Byte>(200)));
            this->lbl_FreqH->Location = System::Drawing::Point(13, 42);
            this->lbl_FreqH->Name = L"lbl_FreqH";
            this->lbl_FreqH->Size = System::Drawing::Size(49, 17);
            this->lbl_FreqH->TabIndex = 28;
            this->lbl_FreqH->Text = L"H.Freq:";
            // 
            // textBoxDbg_lfamp
            // 
            this->textBoxDbg_lfamp->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->textBoxDbg_lfamp->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBoxDbg_lfamp->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->textBoxDbg_lfamp->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)),
                static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)));
            this->textBoxDbg_lfamp->Location = System::Drawing::Point(169, 64);
            this->textBoxDbg_lfamp->MaxLength = 2;
            this->textBoxDbg_lfamp->Name = L"textBoxDbg_lfamp";
            this->textBoxDbg_lfamp->Size = System::Drawing::Size(35, 18);
            this->textBoxDbg_lfamp->TabIndex = 23;
            this->textBoxDbg_lfamp->Text = L"40";
            this->textBoxDbg_lfamp->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            // 
            // textBoxDbg_lfreq
            // 
            this->textBoxDbg_lfreq->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->textBoxDbg_lfreq->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBoxDbg_lfreq->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->textBoxDbg_lfreq->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)),
                static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)));
            this->textBoxDbg_lfreq->Location = System::Drawing::Point(64, 64);
            this->textBoxDbg_lfreq->MaxLength = 2;
            this->textBoxDbg_lfreq->Name = L"textBoxDbg_lfreq";
            this->textBoxDbg_lfreq->Size = System::Drawing::Size(35, 18);
            this->textBoxDbg_lfreq->TabIndex = 22;
            this->textBoxDbg_lfreq->Text = L"40";
            this->textBoxDbg_lfreq->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            // 
            // textBoxDbg_hamp
            // 
            this->textBoxDbg_hamp->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->textBoxDbg_hamp->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBoxDbg_hamp->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->textBoxDbg_hamp->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)),
                static_cast<System::Int32>(static_cast<System::Byte>(250)));
            this->textBoxDbg_hamp->Location = System::Drawing::Point(169, 42);
            this->textBoxDbg_hamp->MaxLength = 2;
            this->textBoxDbg_hamp->Name = L"textBoxDbg_hamp";
            this->textBoxDbg_hamp->Size = System::Drawing::Size(35, 18);
            this->textBoxDbg_hamp->TabIndex = 21;
            this->textBoxDbg_hamp->Text = L"01";
            this->textBoxDbg_hamp->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            // 
            // textBoxDbg_hfreq
            // 
            this->textBoxDbg_hfreq->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->textBoxDbg_hfreq->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBoxDbg_hfreq->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
            this->textBoxDbg_hfreq->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)),
                static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)));
            this->textBoxDbg_hfreq->Location = System::Drawing::Point(64, 42);
            this->textBoxDbg_hfreq->MaxLength = 2;
            this->textBoxDbg_hfreq->Name = L"textBoxDbg_hfreq";
            this->textBoxDbg_hfreq->Size = System::Drawing::Size(35, 18);
            this->textBoxDbg_hfreq->TabIndex = 20;
            this->textBoxDbg_hfreq->Text = L"00";
            this->textBoxDbg_hfreq->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            // 
            // lbl_dbgVib
            // 
            this->lbl_dbgVib->AutoSize = true;
            this->lbl_dbgVib->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_dbgVib->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->lbl_dbgVib->Location = System::Drawing::Point(13, 21);
            this->lbl_dbgVib->Name = L"lbl_dbgVib";
            this->lbl_dbgVib->Size = System::Drawing::Size(64, 17);
            this->lbl_dbgVib->TabIndex = 23;
            this->lbl_dbgVib->Text = L"Vibration:";
            // 
            // grpBox_Restore
            // 
            this->grpBox_Restore->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_Restore->Controls->Add(this->comboBox_rstOption);
            this->grpBox_Restore->Controls->Add(this->lbl_rstDesc);
            this->grpBox_Restore->Controls->Add(this->grpBox_RstUser);
            this->grpBox_Restore->Controls->Add(this->btn_restore);
            this->grpBox_Restore->Controls->Add(this->txtBox_fileLoaded);
            this->grpBox_Restore->Controls->Add(this->label_rst_mac);
            this->grpBox_Restore->Controls->Add(this->btn_loadSPIBackup);
            this->grpBox_Restore->Controls->Add(this->lbl_rstDisclaimer);
            this->grpBox_Restore->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_Restore->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_Restore->Location = System::Drawing::Point(724, 36);
            this->grpBox_Restore->Margin = System::Windows::Forms::Padding(0, 0, 14, 0);
            this->grpBox_Restore->Name = L"grpBox_Restore";
            this->grpBox_Restore->Size = System::Drawing::Size(220, 399);
            this->grpBox_Restore->TabIndex = 24;
            this->grpBox_Restore->TabStop = false;
            this->grpBox_Restore->Text = L"Restore";
            // 
            // comboBox_rstOption
            // 
            this->comboBox_rstOption->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->comboBox_rstOption->DrawMode = System::Windows::Forms::DrawMode::OwnerDrawFixed;
            this->comboBox_rstOption->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->comboBox_rstOption->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->comboBox_rstOption->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)),
                static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->comboBox_rstOption->FormattingEnabled = true;
            this->comboBox_rstOption->Location = System::Drawing::Point(8, 69);
            this->comboBox_rstOption->Name = L"comboBox_rstOption";
            this->comboBox_rstOption->Size = System::Drawing::Size(203, 26);
            this->comboBox_rstOption->TabIndex = 5;
            this->comboBox_rstOption->Visible = false;
            this->comboBox_rstOption->SelectedIndexChanged += gcnew System::EventHandler(this, &FormJoy::comboBox_rstOption_SelectedIndexChanged);
            // 
            // lbl_rstDesc
            // 
            this->lbl_rstDesc->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_rstDesc->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->lbl_rstDesc->Location = System::Drawing::Point(15, 101);
            this->lbl_rstDesc->Name = L"lbl_rstDesc";
            this->lbl_rstDesc->Size = System::Drawing::Size(188, 167);
            this->lbl_rstDesc->TabIndex = 4;
            this->lbl_rstDesc->Visible = false;
            // 
            // grpBox_RstUser
            // 
            this->grpBox_RstUser->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_RstUser->Controls->Add(this->checkBox_rst_accGyroCal);
            this->grpBox_RstUser->Controls->Add(this->checkBox_rst_R_StickCal);
            this->grpBox_RstUser->Controls->Add(this->checkBox_rst_L_StickCal);
            this->grpBox_RstUser->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_RstUser->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_RstUser->Location = System::Drawing::Point(8, 85);
            this->grpBox_RstUser->Name = L"grpBox_RstUser";
            this->grpBox_RstUser->Size = System::Drawing::Size(203, 265);
            this->grpBox_RstUser->TabIndex = 7;
            this->grpBox_RstUser->TabStop = false;
            this->grpBox_RstUser->Visible = false;
            // 
            // checkBox_rst_accGyroCal
            // 
            this->checkBox_rst_accGyroCal->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->checkBox_rst_accGyroCal->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->checkBox_rst_accGyroCal->Location = System::Drawing::Point(6, 238);
            this->checkBox_rst_accGyroCal->Name = L"checkBox_rst_accGyroCal";
            this->checkBox_rst_accGyroCal->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->checkBox_rst_accGyroCal->Size = System::Drawing::Size(188, 21);
            this->checkBox_rst_accGyroCal->TabIndex = 2;
            this->checkBox_rst_accGyroCal->Text = L"6-Axis Sensor";
            // 
            // checkBox_rst_R_StickCal
            // 
            this->checkBox_rst_R_StickCal->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->checkBox_rst_R_StickCal->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->checkBox_rst_R_StickCal->Location = System::Drawing::Point(6, 213);
            this->checkBox_rst_R_StickCal->Name = L"checkBox_rst_R_StickCal";
            this->checkBox_rst_R_StickCal->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->checkBox_rst_R_StickCal->Size = System::Drawing::Size(188, 21);
            this->checkBox_rst_R_StickCal->TabIndex = 1;
            this->checkBox_rst_R_StickCal->Text = L"Right Stick";
            // 
            // checkBox_rst_L_StickCal
            // 
            this->checkBox_rst_L_StickCal->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->checkBox_rst_L_StickCal->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->checkBox_rst_L_StickCal->Location = System::Drawing::Point(6, 188);
            this->checkBox_rst_L_StickCal->Name = L"checkBox_rst_L_StickCal";
            this->checkBox_rst_L_StickCal->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->checkBox_rst_L_StickCal->Size = System::Drawing::Size(188, 21);
            this->checkBox_rst_L_StickCal->TabIndex = 0;
            this->checkBox_rst_L_StickCal->Text = L"Left Stick";
            // 
            // btn_restore
            // 
            this->btn_restore->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_restore->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_restore->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_restore->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btn_restore->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->btn_restore->Location = System::Drawing::Point(124, 359);
            this->btn_restore->Name = L"btn_restore";
            this->btn_restore->Size = System::Drawing::Size(87, 30);
            this->btn_restore->TabIndex = 6;
            this->btn_restore->Text = L"Restore";
            this->btn_restore->UseVisualStyleBackColor = false;
            this->btn_restore->Visible = false;
            this->btn_restore->Click += gcnew System::EventHandler(this, &FormJoy::btn_restore_Click);
            // 
            // txtBox_fileLoaded
            // 
            this->txtBox_fileLoaded->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->txtBox_fileLoaded->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->txtBox_fileLoaded->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->txtBox_fileLoaded->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->txtBox_fileLoaded->Location = System::Drawing::Point(111, 38);
            this->txtBox_fileLoaded->Name = L"txtBox_fileLoaded";
            this->txtBox_fileLoaded->Size = System::Drawing::Size(100, 18);
            this->txtBox_fileLoaded->TabIndex = 2;
            this->txtBox_fileLoaded->Text = L"No file loaded";
            // 
            // label_rst_mac
            // 
            this->label_rst_mac->AutoSize = true;
            this->label_rst_mac->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->label_rst_mac->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->label_rst_mac->Location = System::Drawing::Point(108, 19);
            this->label_rst_mac->Name = L"label_rst_mac";
            this->label_rst_mac->Size = System::Drawing::Size(87, 17);
            this->label_rst_mac->TabIndex = 1;
            this->label_rst_mac->Text = L"Loaded MAC:";
            // 
            // btn_loadSPIBackup
            // 
            this->btn_loadSPIBackup->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_loadSPIBackup->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_loadSPIBackup->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_loadSPIBackup->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->btn_loadSPIBackup->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->btn_loadSPIBackup->Location = System::Drawing::Point(8, 22);
            this->btn_loadSPIBackup->Name = L"btn_loadSPIBackup";
            this->btn_loadSPIBackup->Size = System::Drawing::Size(94, 32);
            this->btn_loadSPIBackup->TabIndex = 0;
            this->btn_loadSPIBackup->Text = L"Load Backup";
            this->btn_loadSPIBackup->UseVisualStyleBackColor = false;
            this->btn_loadSPIBackup->Click += gcnew System::EventHandler(this, &FormJoy::btn_loadSPIBackup_Click);
            // 
            // lbl_rstDisclaimer
            // 
            this->lbl_rstDisclaimer->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_rstDisclaimer->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->lbl_rstDisclaimer->Location = System::Drawing::Point(15, 101);
            this->lbl_rstDisclaimer->Name = L"lbl_rstDisclaimer";
            this->lbl_rstDisclaimer->Size = System::Drawing::Size(188, 167);
            this->lbl_rstDisclaimer->TabIndex = 3;
            this->lbl_rstDisclaimer->Text = L"This will restore the user calibration from the selected backup.\n\nIf you have any"
                L" problem with the analog stick or the 6-Axis sensor you can choose to factory re"
                L"set user calibration.";
            // 
            // errorProvider1
            // 
            this->errorProvider1->ContainerControl = this;
            // 
            // errorProvider2
            // 
            this->errorProvider2->ContainerControl = this;
            // 
            // grpBox_ChangeSN
            // 
            this->grpBox_ChangeSN->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_ChangeSN->Controls->Add(this->btn_restoreSN);
            this->grpBox_ChangeSN->Controls->Add(this->lbl_loadedMAC);
            this->grpBox_ChangeSN->Controls->Add(this->label_sn_change_warning);
            this->grpBox_ChangeSN->Controls->Add(this->btn_changeSN);
            this->grpBox_ChangeSN->Controls->Add(this->textBox_chg_sn);
            this->grpBox_ChangeSN->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_ChangeSN->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_ChangeSN->Location = System::Drawing::Point(954, 36);
            this->grpBox_ChangeSN->Margin = System::Windows::Forms::Padding(0, 0, 14, 0);
            this->grpBox_ChangeSN->Name = L"grpBox_ChangeSN";
            this->grpBox_ChangeSN->Size = System::Drawing::Size(220, 399);
            this->grpBox_ChangeSN->TabIndex = 25;
            this->grpBox_ChangeSN->TabStop = false;
            this->grpBox_ChangeSN->Text = L"Change S/N";
            // 
            // btn_restoreSN
            // 
            this->btn_restoreSN->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_restoreSN->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_restoreSN->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_restoreSN->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btn_restoreSN->Location = System::Drawing::Point(17, 355);
            this->btn_restoreSN->Name = L"btn_restoreSN";
            this->btn_restoreSN->Size = System::Drawing::Size(87, 30);
            this->btn_restoreSN->TabIndex = 34;
            this->btn_restoreSN->Text = L"Restore";
            this->btn_restoreSN->UseVisualStyleBackColor = false;
            this->btn_restoreSN->Click += gcnew System::EventHandler(this, &FormJoy::btn_restoreSN_Click);
            // 
            // lbl_loadedMAC
            // 
            this->lbl_loadedMAC->AutoSize = true;
            this->lbl_loadedMAC->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_loadedMAC->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->lbl_loadedMAC->Location = System::Drawing::Point(14, 293);
            this->lbl_loadedMAC->Name = L"lbl_loadedMAC";
            this->lbl_loadedMAC->Size = System::Drawing::Size(63, 17);
            this->lbl_loadedMAC->TabIndex = 33;
            this->lbl_loadedMAC->Text = L"New S/N:";
            // 
            // label_sn_change_warning
            // 
            this->label_sn_change_warning->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->label_sn_change_warning->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->label_sn_change_warning->Location = System::Drawing::Point(14, 28);
            this->label_sn_change_warning->Name = L"label_sn_change_warning";
            this->label_sn_change_warning->Size = System::Drawing::Size(189, 240);
            this->label_sn_change_warning->TabIndex = 32;
            this->label_sn_change_warning->Text = resources->GetString(L"label_sn_change_warning.Text");
            // 
            // btn_changeSN
            // 
            this->btn_changeSN->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_changeSN->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_changeSN->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_changeSN->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btn_changeSN->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->btn_changeSN->Location = System::Drawing::Point(116, 355);
            this->btn_changeSN->Name = L"btn_changeSN";
            this->btn_changeSN->Size = System::Drawing::Size(87, 30);
            this->btn_changeSN->TabIndex = 1;
            this->btn_changeSN->Text = L"Change";
            this->btn_changeSN->UseVisualStyleBackColor = false;
            this->btn_changeSN->Click += gcnew System::EventHandler(this, &FormJoy::btn_changeSN_Click);
            // 
            // textBox_chg_sn
            // 
            this->textBox_chg_sn->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBox_chg_sn->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->textBox_chg_sn->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->textBox_chg_sn->Location = System::Drawing::Point(17, 317);
            this->textBox_chg_sn->MaxLength = 15;
            this->textBox_chg_sn->Name = L"textBox_chg_sn";
            this->textBox_chg_sn->Size = System::Drawing::Size(186, 25);
            this->textBox_chg_sn->TabIndex = 0;
            // 
            // lbl_exposure
            // 
            this->lbl_exposure->AutoSize = true;
            this->lbl_exposure->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_exposure->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->lbl_exposure->Location = System::Drawing::Point(6, 160);
            this->lbl_exposure->Name = L"lbl_exposure";
            this->lbl_exposure->Size = System::Drawing::Size(57, 13);
            this->lbl_exposure->TabIndex = 31;
            this->lbl_exposure->Text = L"Exposure:";
            this->lbl_exposure->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
            // 
            // numeric_IRExposure
            // 
            this->numeric_IRExposure->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_IRExposure->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_IRExposure->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_IRExposure->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 20, 0, 0, 0 });
            this->numeric_IRExposure->Location = System::Drawing::Point(10, 180);
            this->numeric_IRExposure->Margin = System::Windows::Forms::Padding(0);
            this->numeric_IRExposure->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 600, 0, 0, 0 });
            this->numeric_IRExposure->Name = L"numeric_IRExposure";
            this->numeric_IRExposure->Size = System::Drawing::Size(53, 25);
            this->numeric_IRExposure->TabIndex = 29;
            this->numeric_IRExposure->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_IRExposure->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            this->numeric_IRExposure->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 300, 0, 0, 0 });
            // 
            // trackBar_IRGain
            // 
            this->trackBar_IRGain->AutoSize = false;
            this->trackBar_IRGain->LargeChange = 1;
            this->trackBar_IRGain->Location = System::Drawing::Point(8, 239);
            this->trackBar_IRGain->Margin = System::Windows::Forms::Padding(0);
            this->trackBar_IRGain->Maximum = 20;
            this->trackBar_IRGain->Minimum = 1;
            this->trackBar_IRGain->Name = L"trackBar_IRGain";
            this->trackBar_IRGain->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->trackBar_IRGain->Size = System::Drawing::Size(257, 28);
            this->trackBar_IRGain->TabIndex = 4;
            this->trackBar_IRGain->Value = 2;
            this->trackBar_IRGain->ValueChanged += gcnew System::EventHandler(this, &FormJoy::TrackBarIR_ValueChanged);
            // 
            // grpBox_VibPlayer
            // 
            this->grpBox_VibPlayer->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_VibPlayer->Controls->Add(this->groupBox_vib_info);
            this->grpBox_VibPlayer->Controls->Add(this->groupBox_vib_eq);
            this->grpBox_VibPlayer->Controls->Add(this->btn_vibPlay);
            this->grpBox_VibPlayer->Controls->Add(this->btn_loadVib);
            this->grpBox_VibPlayer->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_VibPlayer->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_VibPlayer->Location = System::Drawing::Point(1183, 36);
            this->grpBox_VibPlayer->Margin = System::Windows::Forms::Padding(0, 0, 14, 0);
            this->grpBox_VibPlayer->Name = L"grpBox_VibPlayer";
            this->grpBox_VibPlayer->Size = System::Drawing::Size(220, 399);
            this->grpBox_VibPlayer->TabIndex = 26;
            this->grpBox_VibPlayer->TabStop = false;
            this->grpBox_VibPlayer->Text = L"HD Rumble Player";
            // 
            // groupBox_vib_info
            // 
            this->groupBox_vib_info->Controls->Add(this->textBox_vib_loop_times);
            this->groupBox_vib_info->Controls->Add(this->label_hdrumble_filename);
            this->groupBox_vib_info->Controls->Add(this->label_loop_times);
            this->groupBox_vib_info->Controls->Add(this->label_vib_loaded);
            this->groupBox_vib_info->Controls->Add(this->label_samplerate);
            this->groupBox_vib_info->Controls->Add(this->label_samples);
            this->groupBox_vib_info->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->groupBox_vib_info->Location = System::Drawing::Point(6, 62);
            this->groupBox_vib_info->Name = L"groupBox_vib_info";
            this->groupBox_vib_info->Size = System::Drawing::Size(208, 130);
            this->groupBox_vib_info->TabIndex = 14;
            this->groupBox_vib_info->TabStop = false;
            this->groupBox_vib_info->Text = L"Info";
            // 
            // textBox_vib_loop_times
            // 
            this->textBox_vib_loop_times->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->textBox_vib_loop_times->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBox_vib_loop_times->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->textBox_vib_loop_times->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->textBox_vib_loop_times->Location = System::Drawing::Point(82, 106);
            this->textBox_vib_loop_times->MaxLength = 3;
            this->textBox_vib_loop_times->Name = L"textBox_vib_loop_times";
            this->textBox_vib_loop_times->Size = System::Drawing::Size(30, 16);
            this->textBox_vib_loop_times->TabIndex = 14;
            this->textBox_vib_loop_times->Text = L"0";
            this->textBox_vib_loop_times->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->textBox_vib_loop_times->Visible = false;
            // 
            // label_hdrumble_filename
            // 
            this->label_hdrumble_filename->AutoSize = true;
            this->label_hdrumble_filename->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->label_hdrumble_filename->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->label_hdrumble_filename->Location = System::Drawing::Point(6, 20);
            this->label_hdrumble_filename->Name = L"label_hdrumble_filename";
            this->label_hdrumble_filename->Size = System::Drawing::Size(92, 17);
            this->label_hdrumble_filename->TabIndex = 5;
            this->label_hdrumble_filename->Text = L"No file loaded";
            // 
            // label_loop_times
            // 
            this->label_loop_times->AutoSize = true;
            this->label_loop_times->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->label_loop_times->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->label_loop_times->Location = System::Drawing::Point(6, 104);
            this->label_loop_times->Name = L"label_loop_times";
            this->label_loop_times->Size = System::Drawing::Size(77, 17);
            this->label_loop_times->TabIndex = 13;
            this->label_loop_times->Text = L"Extra loops:";
            this->label_loop_times->Visible = false;
            // 
            // label_vib_loaded
            // 
            this->label_vib_loaded->AutoSize = true;
            this->label_vib_loaded->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->label_vib_loaded->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->label_vib_loaded->Location = System::Drawing::Point(6, 41);
            this->label_vib_loaded->Name = L"label_vib_loaded";
            this->label_vib_loaded->Size = System::Drawing::Size(38, 17);
            this->label_vib_loaded->TabIndex = 2;
            this->label_vib_loaded->Text = L"Type:";
            // 
            // label_samplerate
            // 
            this->label_samplerate->AutoSize = true;
            this->label_samplerate->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->label_samplerate->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->label_samplerate->Location = System::Drawing::Point(6, 62);
            this->label_samplerate->Name = L"label_samplerate";
            this->label_samplerate->Size = System::Drawing::Size(81, 17);
            this->label_samplerate->TabIndex = 3;
            this->label_samplerate->Text = L"Sample rate:";
            // 
            // label_samples
            // 
            this->label_samples->AutoSize = true;
            this->label_samples->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->label_samples->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->label_samples->Location = System::Drawing::Point(6, 83);
            this->label_samples->Name = L"label_samples";
            this->label_samples->Size = System::Drawing::Size(64, 17);
            this->label_samples->TabIndex = 4;
            this->label_samples->Text = L"Samples: ";
            // 
            // groupBox_vib_eq
            // 
            this->groupBox_vib_eq->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->groupBox_vib_eq->Controls->Add(this->btn_vibResetEQ);
            this->groupBox_vib_eq->Controls->Add(this->label_eq_info);
            this->groupBox_vib_eq->Controls->Add(this->trackBar_hf_amp);
            this->groupBox_vib_eq->Controls->Add(this->trackBar_lf_amp);
            this->groupBox_vib_eq->Controls->Add(this->trackBar_hf_freq);
            this->groupBox_vib_eq->Controls->Add(this->trackBar_lf_freq);
            this->groupBox_vib_eq->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->groupBox_vib_eq->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->groupBox_vib_eq->Location = System::Drawing::Point(6, 196);
            this->groupBox_vib_eq->Name = L"groupBox_vib_eq";
            this->groupBox_vib_eq->Size = System::Drawing::Size(208, 197);
            this->groupBox_vib_eq->TabIndex = 12;
            this->groupBox_vib_eq->TabStop = false;
            this->groupBox_vib_eq->Text = L"Equalizer";
            this->groupBox_vib_eq->Visible = false;
            // 
            // btn_vibResetEQ
            // 
            this->btn_vibResetEQ->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_vibResetEQ->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_vibResetEQ->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_vibResetEQ->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->btn_vibResetEQ->Location = System::Drawing::Point(66, 27);
            this->btn_vibResetEQ->Name = L"btn_vibResetEQ";
            this->btn_vibResetEQ->Size = System::Drawing::Size(75, 30);
            this->btn_vibResetEQ->TabIndex = 11;
            this->btn_vibResetEQ->Text = L"Reset EQ";
            this->btn_vibResetEQ->UseVisualStyleBackColor = false;
            this->btn_vibResetEQ->Click += gcnew System::EventHandler(this, &FormJoy::btn_vibResetEQ_Click);
            // 
            // label_eq_info
            // 
            this->label_eq_info->AutoSize = true;
            this->label_eq_info->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->label_eq_info->Location = System::Drawing::Point(7, 68);
            this->label_eq_info->Name = L"label_eq_info";
            this->label_eq_info->Size = System::Drawing::Size(192, 26);
            this->label_eq_info->TabIndex = 10;
            this->label_eq_info->Text = L"  Low Frequency      High Frequency \r\n Gain        Pitch        Gain        Pitch"
                L"";
            this->label_eq_info->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
            // 
            // trackBar_hf_amp
            // 
            this->trackBar_hf_amp->AutoSize = false;
            this->trackBar_hf_amp->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->trackBar_hf_amp->LargeChange = 1;
            this->trackBar_hf_amp->Location = System::Drawing::Point(111, 91);
            this->trackBar_hf_amp->Margin = System::Windows::Forms::Padding(0);
            this->trackBar_hf_amp->Maximum = 20;
            this->trackBar_hf_amp->Name = L"trackBar_hf_amp";
            this->trackBar_hf_amp->Orientation = System::Windows::Forms::Orientation::Vertical;
            this->trackBar_hf_amp->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->trackBar_hf_amp->Size = System::Drawing::Size(35, 104);
            this->trackBar_hf_amp->TabIndex = 8;
            this->trackBar_hf_amp->TickStyle = System::Windows::Forms::TickStyle::TopLeft;
            this->trackBar_hf_amp->ValueChanged += gcnew System::EventHandler(this, &FormJoy::TrackBar_ValueChanged);
            // 
            // trackBar_lf_amp
            // 
            this->trackBar_lf_amp->AutoSize = false;
            this->trackBar_lf_amp->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->trackBar_lf_amp->LargeChange = 1;
            this->trackBar_lf_amp->Location = System::Drawing::Point(13, 91);
            this->trackBar_lf_amp->Margin = System::Windows::Forms::Padding(0);
            this->trackBar_lf_amp->Maximum = 20;
            this->trackBar_lf_amp->Name = L"trackBar_lf_amp";
            this->trackBar_lf_amp->Orientation = System::Windows::Forms::Orientation::Vertical;
            this->trackBar_lf_amp->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->trackBar_lf_amp->Size = System::Drawing::Size(35, 104);
            this->trackBar_lf_amp->TabIndex = 6;
            this->trackBar_lf_amp->TickStyle = System::Windows::Forms::TickStyle::TopLeft;
            this->trackBar_lf_amp->ValueChanged += gcnew System::EventHandler(this, &FormJoy::TrackBar_ValueChanged);
            // 
            // trackBar_hf_freq
            // 
            this->trackBar_hf_freq->AutoSize = false;
            this->trackBar_hf_freq->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->trackBar_hf_freq->LargeChange = 1;
            this->trackBar_hf_freq->Location = System::Drawing::Point(160, 91);
            this->trackBar_hf_freq->Margin = System::Windows::Forms::Padding(0);
            this->trackBar_hf_freq->Maximum = 20;
            this->trackBar_hf_freq->Name = L"trackBar_hf_freq";
            this->trackBar_hf_freq->Orientation = System::Windows::Forms::Orientation::Vertical;
            this->trackBar_hf_freq->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->trackBar_hf_freq->Size = System::Drawing::Size(35, 104);
            this->trackBar_hf_freq->TabIndex = 9;
            this->trackBar_hf_freq->TickStyle = System::Windows::Forms::TickStyle::TopLeft;
            this->trackBar_hf_freq->ValueChanged += gcnew System::EventHandler(this, &FormJoy::TrackBar_ValueChanged);
            // 
            // trackBar_lf_freq
            // 
            this->trackBar_lf_freq->AutoSize = false;
            this->trackBar_lf_freq->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->trackBar_lf_freq->LargeChange = 1;
            this->trackBar_lf_freq->Location = System::Drawing::Point(62, 91);
            this->trackBar_lf_freq->Margin = System::Windows::Forms::Padding(0);
            this->trackBar_lf_freq->Maximum = 20;
            this->trackBar_lf_freq->Name = L"trackBar_lf_freq";
            this->trackBar_lf_freq->Orientation = System::Windows::Forms::Orientation::Vertical;
            this->trackBar_lf_freq->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->trackBar_lf_freq->Size = System::Drawing::Size(35, 104);
            this->trackBar_lf_freq->TabIndex = 7;
            this->trackBar_lf_freq->TickStyle = System::Windows::Forms::TickStyle::TopLeft;
            this->trackBar_lf_freq->ValueChanged += gcnew System::EventHandler(this, &FormJoy::TrackBar_ValueChanged);
            // 
            // btn_vibPlay
            // 
            this->btn_vibPlay->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_vibPlay->Enabled = false;
            this->btn_vibPlay->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_vibPlay->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_vibPlay->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btn_vibPlay->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->btn_vibPlay->Location = System::Drawing::Point(113, 26);
            this->btn_vibPlay->Name = L"btn_vibPlay";
            this->btn_vibPlay->Size = System::Drawing::Size(99, 32);
            this->btn_vibPlay->TabIndex = 1;
            this->btn_vibPlay->Text = L"Play";
            this->btn_vibPlay->UseVisualStyleBackColor = false;
            this->btn_vibPlay->Click += gcnew System::EventHandler(this, &FormJoy::btn_vibPlay_Click);
            // 
            // btn_loadVib
            // 
            this->btn_loadVib->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_loadVib->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_loadVib->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_loadVib->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->btn_loadVib->Location = System::Drawing::Point(8, 26);
            this->btn_loadVib->Name = L"btn_loadVib";
            this->btn_loadVib->Size = System::Drawing::Size(99, 32);
            this->btn_loadVib->TabIndex = 0;
            this->btn_loadVib->Text = L"Load Rumble";
            this->btn_loadVib->UseVisualStyleBackColor = false;
            this->btn_loadVib->Click += gcnew System::EventHandler(this, &FormJoy::btn_loadVib_Click);
            // 
            // btn_enableExpertMode
            // 
            this->btn_enableExpertMode->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->btn_enableExpertMode->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->btn_enableExpertMode->FlatAppearance->BorderSize = 0;
            this->btn_enableExpertMode->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_enableExpertMode->Location = System::Drawing::Point(500, 438);
            this->btn_enableExpertMode->Margin = System::Windows::Forms::Padding(0);
            this->btn_enableExpertMode->Name = L"btn_enableExpertMode";
            this->btn_enableExpertMode->Size = System::Drawing::Size(10, 8);
            this->btn_enableExpertMode->TabIndex = 27;
            this->btn_enableExpertMode->UseVisualStyleBackColor = false;
            this->btn_enableExpertMode->Click += gcnew System::EventHandler(this, &FormJoy::btn_enableExpertMode_Click);
            // 
            // textBox_btn_test_reply
            // 
            this->textBox_btn_test_reply->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBox_btn_test_reply->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBox_btn_test_reply->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F, System::Drawing::FontStyle::Regular,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(161)));
            this->textBox_btn_test_reply->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->textBox_btn_test_reply->Location = System::Drawing::Point(8, 24);
            this->textBox_btn_test_reply->Multiline = true;
            this->textBox_btn_test_reply->Name = L"textBox_btn_test_reply";
            this->textBox_btn_test_reply->ReadOnly = true;
            this->textBox_btn_test_reply->Size = System::Drawing::Size(205, 172);
            this->textBox_btn_test_reply->TabIndex = 35;
            this->textBox_btn_test_reply->TabStop = false;
            this->textBox_btn_test_reply->Text = L"Conn:\r\nBatt:       Charging: Yes\r\nVibration decision:\r\n\r\nButtons:\r\n\r\nL Stick (Raw"
                L"/Cal):\r\nX:         Y:\r\nX:         Y:\r\n\r\n\r\nR Stick (Raw/Cal):\r\nX:         Y:\r\nX: "
                L"        Y:";
            // 
            // textBox_btn_test_subreply
            // 
            this->textBox_btn_test_subreply->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBox_btn_test_subreply->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBox_btn_test_subreply->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F));
            this->textBox_btn_test_subreply->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->textBox_btn_test_subreply->Location = System::Drawing::Point(8, 202);
            this->textBox_btn_test_subreply->Multiline = true;
            this->textBox_btn_test_subreply->Name = L"textBox_btn_test_subreply";
            this->textBox_btn_test_subreply->ReadOnly = true;
            this->textBox_btn_test_subreply->Size = System::Drawing::Size(205, 140);
            this->textBox_btn_test_subreply->TabIndex = 35;
            this->textBox_btn_test_subreply->TabStop = false;
            this->textBox_btn_test_subreply->Text = L"Acc/meter (Raw/Cal):\r\nX: \r\nY: \r\nX: \r\n\r\nGyroscope (Raw/Cal):\r\nX: \r\nY: \r\nZ: ";
            // 
            // grpBox_ButtonTest
            // 
            this->grpBox_ButtonTest->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_ButtonTest->Controls->Add(this->btn_runBtnTest);
            this->grpBox_ButtonTest->Controls->Add(this->textBox_btn_test_reply);
            this->grpBox_ButtonTest->Controls->Add(this->textBox_btn_test_subreply);
            this->grpBox_ButtonTest->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_ButtonTest->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_ButtonTest->Location = System::Drawing::Point(1418, 36);
            this->grpBox_ButtonTest->Margin = System::Windows::Forms::Padding(0, 0, 14, 0);
            this->grpBox_ButtonTest->Name = L"grpBox_ButtonTest";
            this->grpBox_ButtonTest->Size = System::Drawing::Size(220, 399);
            this->grpBox_ButtonTest->TabIndex = 36;
            this->grpBox_ButtonTest->TabStop = false;
            this->grpBox_ButtonTest->Text = L"Button test";
            // 
            // btn_runBtnTest
            // 
            this->btn_runBtnTest->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_runBtnTest->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_runBtnTest->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_runBtnTest->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->btn_runBtnTest->Location = System::Drawing::Point(66, 359);
            this->btn_runBtnTest->Name = L"btn_runBtnTest";
            this->btn_runBtnTest->Size = System::Drawing::Size(88, 30);
            this->btn_runBtnTest->TabIndex = 36;
            this->btn_runBtnTest->Text = L"Turn on";
            this->btn_runBtnTest->UseVisualStyleBackColor = false;
            this->btn_runBtnTest->Click += gcnew System::EventHandler(this, &FormJoy::btn_runBtnTest_Click);
            // 
            // grpBox_StickCal
            // 
            this->grpBox_StickCal->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_StickCal->Controls->Add(this->textBox_lstick_fcal);
            this->grpBox_StickCal->Controls->Add(this->textBox_rstick_fcal);
            this->grpBox_StickCal->Controls->Add(this->textBox_rstick_ucal);
            this->grpBox_StickCal->Controls->Add(this->textBox_lstick_ucal);
            this->grpBox_StickCal->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_StickCal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_StickCal->Location = System::Drawing::Point(14, 445);
            this->grpBox_StickCal->Margin = System::Windows::Forms::Padding(0, 0, 0, 39);
            this->grpBox_StickCal->Name = L"grpBox_StickCal";
            this->grpBox_StickCal->Size = System::Drawing::Size(218, 215);
            this->grpBox_StickCal->TabIndex = 37;
            this->grpBox_StickCal->TabStop = false;
            this->grpBox_StickCal->Text = L"Stick Calibration";
            // 
            // textBox_lstick_fcal
            // 
            this->textBox_lstick_fcal->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->textBox_lstick_fcal->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBox_lstick_fcal->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBox_lstick_fcal->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F));
            this->textBox_lstick_fcal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->textBox_lstick_fcal->Location = System::Drawing::Point(6, 22);
            this->textBox_lstick_fcal->Margin = System::Windows::Forms::Padding(0);
            this->textBox_lstick_fcal->Multiline = true;
            this->textBox_lstick_fcal->Name = L"textBox_lstick_fcal";
            this->textBox_lstick_fcal->ReadOnly = true;
            this->textBox_lstick_fcal->Size = System::Drawing::Size(207, 44);
            this->textBox_lstick_fcal->TabIndex = 40;
            this->textBox_lstick_fcal->TabStop = false;
            this->textBox_lstick_fcal->Text = L"L Stick Factory:\r\nCenter X,Y: (   ,    )\r\nX: [    -    ] Y: [    -    ]";
            // 
            // textBox_rstick_fcal
            // 
            this->textBox_rstick_fcal->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->textBox_rstick_fcal->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBox_rstick_fcal->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBox_rstick_fcal->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F));
            this->textBox_rstick_fcal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->textBox_rstick_fcal->Location = System::Drawing::Point(6, 119);
            this->textBox_rstick_fcal->Margin = System::Windows::Forms::Padding(0);
            this->textBox_rstick_fcal->Multiline = true;
            this->textBox_rstick_fcal->Name = L"textBox_rstick_fcal";
            this->textBox_rstick_fcal->ReadOnly = true;
            this->textBox_rstick_fcal->Size = System::Drawing::Size(207, 44);
            this->textBox_rstick_fcal->TabIndex = 39;
            this->textBox_rstick_fcal->TabStop = false;
            this->textBox_rstick_fcal->Text = L"R Stick Factory:\r\nCenter X,Y: (   ,    )\r\nX: [    -    ] Y: [    -    ]";
            // 
            // textBox_rstick_ucal
            // 
            this->textBox_rstick_ucal->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->textBox_rstick_ucal->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBox_rstick_ucal->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBox_rstick_ucal->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F));
            this->textBox_rstick_ucal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->textBox_rstick_ucal->Location = System::Drawing::Point(6, 163);
            this->textBox_rstick_ucal->Margin = System::Windows::Forms::Padding(0);
            this->textBox_rstick_ucal->Multiline = true;
            this->textBox_rstick_ucal->Name = L"textBox_rstick_ucal";
            this->textBox_rstick_ucal->ReadOnly = true;
            this->textBox_rstick_ucal->Size = System::Drawing::Size(207, 44);
            this->textBox_rstick_ucal->TabIndex = 38;
            this->textBox_rstick_ucal->TabStop = false;
            this->textBox_rstick_ucal->Text = L"R Stick User:\r\nCenter X,Y: (   ,    )\r\nX: [    -    ] Y: [    -    ]";
            // 
            // textBox_lstick_ucal
            // 
            this->textBox_lstick_ucal->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->textBox_lstick_ucal->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBox_lstick_ucal->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBox_lstick_ucal->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F));
            this->textBox_lstick_ucal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->textBox_lstick_ucal->Location = System::Drawing::Point(6, 66);
            this->textBox_lstick_ucal->Margin = System::Windows::Forms::Padding(0);
            this->textBox_lstick_ucal->Multiline = true;
            this->textBox_lstick_ucal->Name = L"textBox_lstick_ucal";
            this->textBox_lstick_ucal->ReadOnly = true;
            this->textBox_lstick_ucal->Size = System::Drawing::Size(207, 44);
            this->textBox_lstick_ucal->TabIndex = 37;
            this->textBox_lstick_ucal->TabStop = false;
            this->textBox_lstick_ucal->Text = L"L Stick User:\r\nCenter X,Y: (   ,    )\r\nX: [    -    ] Y: [    -    ]";
            // 
            // textBox_6axis_ucal
            // 
            this->textBox_6axis_ucal->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->textBox_6axis_ucal->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBox_6axis_ucal->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBox_6axis_ucal->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F));
            this->textBox_6axis_ucal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->textBox_6axis_ucal->Location = System::Drawing::Point(6, 119);
            this->textBox_6axis_ucal->Margin = System::Windows::Forms::Padding(0);
            this->textBox_6axis_ucal->Multiline = true;
            this->textBox_6axis_ucal->Name = L"textBox_6axis_ucal";
            this->textBox_6axis_ucal->ReadOnly = true;
            this->textBox_6axis_ucal->Size = System::Drawing::Size(156, 88);
            this->textBox_6axis_ucal->TabIndex = 43;
            this->textBox_6axis_ucal->TabStop = false;
            this->textBox_6axis_ucal->Text = L"6-Axis User (XYZ):\r\nAcc:\r\n\r\n\r\nGyro:";
            // 
            // textBox_6axis_cal
            // 
            this->textBox_6axis_cal->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->textBox_6axis_cal->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBox_6axis_cal->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBox_6axis_cal->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F));
            this->textBox_6axis_cal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->textBox_6axis_cal->Location = System::Drawing::Point(6, 22);
            this->textBox_6axis_cal->Margin = System::Windows::Forms::Padding(0);
            this->textBox_6axis_cal->Multiline = true;
            this->textBox_6axis_cal->Name = L"textBox_6axis_cal";
            this->textBox_6axis_cal->ReadOnly = true;
            this->textBox_6axis_cal->Size = System::Drawing::Size(156, 88);
            this->textBox_6axis_cal->TabIndex = 42;
            this->textBox_6axis_cal->TabStop = false;
            this->textBox_6axis_cal->Text = L"6-Axis Factory (XYZ):\r\nAcc:\r\n\r\n\r\nGyro:";
            // 
            // txtBox_devParameters
            // 
            this->txtBox_devParameters->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->txtBox_devParameters->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->txtBox_devParameters->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F));
            this->txtBox_devParameters->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->txtBox_devParameters->Location = System::Drawing::Point(7, 22);
            this->txtBox_devParameters->Margin = System::Windows::Forms::Padding(0);
            this->txtBox_devParameters->Multiline = true;
            this->txtBox_devParameters->Name = L"txtBox_devParameters";
            this->txtBox_devParameters->ReadOnly = true;
            this->txtBox_devParameters->Size = System::Drawing::Size(135, 185);
            this->txtBox_devParameters->TabIndex = 41;
            this->txtBox_devParameters->TabStop = false;
            this->txtBox_devParameters->Text = L"Flat surface ACC Offsets:\r\n\r\n\r\n\r\nStick Parameters:";
            // 
            // txtBox_devParameters2
            // 
            this->txtBox_devParameters2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->txtBox_devParameters2->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->txtBox_devParameters2->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F));
            this->txtBox_devParameters2->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->txtBox_devParameters2->Location = System::Drawing::Point(155, 77);
            this->txtBox_devParameters2->Margin = System::Windows::Forms::Padding(0);
            this->txtBox_devParameters2->Multiline = true;
            this->txtBox_devParameters2->Name = L"txtBox_devParameters2";
            this->txtBox_devParameters2->ReadOnly = true;
            this->txtBox_devParameters2->Size = System::Drawing::Size(140, 130);
            this->txtBox_devParameters2->TabIndex = 42;
            this->txtBox_devParameters2->TabStop = false;
            this->txtBox_devParameters2->Text = L"Stick Parameters 2:";
            // 
            // grpBox_dev_param
            // 
            this->grpBox_dev_param->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_dev_param->Controls->Add(this->txtBox_devParameters);
            this->grpBox_dev_param->Controls->Add(this->txtBox_devParameters2);
            this->grpBox_dev_param->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_dev_param->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_dev_param->Location = System::Drawing::Point(408, 445);
            this->grpBox_dev_param->Margin = System::Windows::Forms::Padding(0);
            this->grpBox_dev_param->Name = L"grpBox_dev_param";
            this->grpBox_dev_param->Size = System::Drawing::Size(306, 215);
            this->grpBox_dev_param->TabIndex = 43;
            this->grpBox_dev_param->TabStop = false;
            this->grpBox_dev_param->Text = L"6-Axis and Stick Device Parameters";
            // 
            // toolStrip1
            // 
            this->toolStrip1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(55)), static_cast<System::Int32>(static_cast<System::Byte>(55)),
                static_cast<System::Int32>(static_cast<System::Byte>(55)));
            this->toolStrip1->Dock = System::Windows::Forms::DockStyle::Bottom;
            this->toolStrip1->GripMargin = System::Windows::Forms::Padding(0);
            this->toolStrip1->GripStyle = System::Windows::Forms::ToolStripGripStyle::Hidden;
            this->toolStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(5) {
                this->toolStripBtn_batt,
                    this->toolStripLabel_batt, this->toolStripLabel_temp, this->toolStripBtn_refresh, this->toolStripBtn_Disconnect
            });
            this->toolStrip1->Location = System::Drawing::Point(0, 1065);
            this->toolStrip1->Name = L"toolStrip1";
            this->toolStrip1->Padding = System::Windows::Forms::Padding(6, 1, 6, 3);
            this->toolStrip1->Size = System::Drawing::Size(1925, 25);
            this->toolStrip1->Stretch = true;
            this->toolStrip1->TabIndex = 44;
            this->toolStrip1->Text = L"toolStrip1";
            // 
            // toolStripBtn_batt
            // 
            this->toolStripBtn_batt->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->toolStripBtn_batt->AutoToolTip = false;
            this->toolStripBtn_batt->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
            this->toolStripBtn_batt->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
            this->toolStripBtn_batt->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
            this->toolStripBtn_batt->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->toolStripBtn_batt->Margin = System::Windows::Forms::Padding(0);
            this->toolStripBtn_batt->Name = L"toolStripBtn_batt";
            this->toolStripBtn_batt->Padding = System::Windows::Forms::Padding(0, 0, 4, 0);
            this->toolStripBtn_batt->Size = System::Drawing::Size(23, 21);
            this->toolStripBtn_batt->Text = L"toolStripBtn_batt";
            this->toolStripBtn_batt->Click += gcnew System::EventHandler(this, &FormJoy::pictureBoxBattery_Click);
            // 
            // toolStripLabel_batt
            // 
            this->toolStripLabel_batt->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->toolStripLabel_batt->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->toolStripLabel_batt->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->toolStripLabel_batt->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->toolStripLabel_batt->Margin = System::Windows::Forms::Padding(0);
            this->toolStripLabel_batt->Name = L"toolStripLabel_batt";
            this->toolStripLabel_batt->Padding = System::Windows::Forms::Padding(4, 0, 0, 0);
            this->toolStripLabel_batt->Size = System::Drawing::Size(63, 21);
            this->toolStripLabel_batt->Text = L"Batt_V_%";
            this->toolStripLabel_batt->Click += gcnew System::EventHandler(this, &FormJoy::toolStripLabel_batt_Click);
            // 
            // toolStripLabel_temp
            // 
            this->toolStripLabel_temp->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->toolStripLabel_temp->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->toolStripLabel_temp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->toolStripLabel_temp->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->toolStripLabel_temp->Margin = System::Windows::Forms::Padding(0);
            this->toolStripLabel_temp->Name = L"toolStripLabel_temp";
            this->toolStripLabel_temp->Padding = System::Windows::Forms::Padding(4, 0, 4, 0);
            this->toolStripLabel_temp->Size = System::Drawing::Size(56, 21);
            this->toolStripLabel_temp->Text = L"0.00oC";
            this->toolStripLabel_temp->ToolTipText = resources->GetString(L"toolStripLabel_temp.ToolTipText");
            this->toolStripLabel_temp->Click += gcnew System::EventHandler(this, &FormJoy::toolStripLabel_temp_Click);
            // 
            // toolStripBtn_refresh
            // 
            this->toolStripBtn_refresh->AutoToolTip = false;
            this->toolStripBtn_refresh->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->toolStripBtn_refresh->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->toolStripBtn_refresh->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->toolStripBtn_refresh->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"toolStripBtn_refresh.Image")));
            this->toolStripBtn_refresh->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->toolStripBtn_refresh->Margin = System::Windows::Forms::Padding(0);
            this->toolStripBtn_refresh->Name = L"toolStripBtn_refresh";
            this->toolStripBtn_refresh->Padding = System::Windows::Forms::Padding(4, 0, 4, 0);
            this->toolStripBtn_refresh->Size = System::Drawing::Size(64, 21);
            this->toolStripBtn_refresh->Text = L"Refresh";
            this->toolStripBtn_refresh->ToolTipText = L"Refresh connected controller info.\r\n\r\nIf you connected a new controller and disco"
                L"nnected the old one,\r\nit will show the new controller.";
            this->toolStripBtn_refresh->Click += gcnew System::EventHandler(this, &FormJoy::toolStripBtn_refresh_Click);
            // 
            // toolStripBtn_Disconnect
            // 
            this->toolStripBtn_Disconnect->AutoToolTip = false;
            this->toolStripBtn_Disconnect->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->toolStripBtn_Disconnect->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->toolStripBtn_Disconnect->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(60)), static_cast<System::Int32>(static_cast<System::Byte>(40)));
            this->toolStripBtn_Disconnect->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"toolStripBtn_Disconnect.Image")));
            this->toolStripBtn_Disconnect->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->toolStripBtn_Disconnect->Margin = System::Windows::Forms::Padding(0);
            this->toolStripBtn_Disconnect->Name = L"toolStripBtn_Disconnect";
            this->toolStripBtn_Disconnect->Padding = System::Windows::Forms::Padding(4, 0, 4, 0);
            this->toolStripBtn_Disconnect->Size = System::Drawing::Size(83, 21);
            this->toolStripBtn_Disconnect->Text = L"Disconnect";
            this->toolStripBtn_Disconnect->ToolTipText = L"Disconnects the device.\r\n\r\nAdditionally performs a reboot.\r\nAfter 4s it\'s ready t"
                L"o connect again.";
            this->toolStripBtn_Disconnect->Click += gcnew System::EventHandler(this, &FormJoy::toolStripBtn_Disconnect_Click);
            // 
            // panel_filler
            // 
            this->panel_filler->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
            this->panel_filler->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->panel_filler->BackgroundImageLayout = System::Windows::Forms::ImageLayout::None;
            this->panel_filler->Location = System::Drawing::Point(0, 0);
            this->panel_filler->Margin = System::Windows::Forms::Padding(0);
            this->panel_filler->Name = L"panel_filler";
            this->panel_filler->Size = System::Drawing::Size(0, 0);
            this->panel_filler->TabIndex = 45;
            // 
            // grpBox_accGyroCal
            // 
            this->grpBox_accGyroCal->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_accGyroCal->Controls->Add(this->textBox_6axis_ucal);
            this->grpBox_accGyroCal->Controls->Add(this->textBox_6axis_cal);
            this->grpBox_accGyroCal->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_accGyroCal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_accGyroCal->Location = System::Drawing::Point(236, 445);
            this->grpBox_accGyroCal->Margin = System::Windows::Forms::Padding(0);
            this->grpBox_accGyroCal->Name = L"grpBox_accGyroCal";
            this->grpBox_accGyroCal->Size = System::Drawing::Size(168, 215);
            this->grpBox_accGyroCal->TabIndex = 47;
            this->grpBox_accGyroCal->TabStop = false;
            this->grpBox_accGyroCal->Text = L"Acc/Gyro Calibration";
            // 
            // grpBox_IR
            // 
            this->grpBox_IR->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_IR->Controls->Add(this->lbl_IRStatus);
            this->grpBox_IR->Controls->Add(this->lbl_IRHelp);
            this->grpBox_IR->Controls->Add(this->pictureBoxIR);
            this->grpBox_IR->Controls->Add(this->btn_getIRImage);
            this->grpBox_IR->Controls->Add(this->btn_getIRStream);
            this->grpBox_IR->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_IR->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_IR->Location = System::Drawing::Point(1652, 36);
            this->grpBox_IR->Margin = System::Windows::Forms::Padding(0, 0, 14, 0);
            this->grpBox_IR->Name = L"grpBox_IR";
            this->grpBox_IR->Size = System::Drawing::Size(252, 444);
            this->grpBox_IR->TabIndex = 49;
            this->grpBox_IR->TabStop = false;
            this->grpBox_IR->Text = L"IR Camera";
            // 
            // lbl_IRStatus
            // 
            this->lbl_IRStatus->AutoSize = true;
            this->lbl_IRStatus->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_IRStatus->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->lbl_IRStatus->Location = System::Drawing::Point(2, 377);
            this->lbl_IRStatus->Name = L"lbl_IRStatus";
            this->lbl_IRStatus->Size = System::Drawing::Size(97, 17);
            this->lbl_IRStatus->TabIndex = 4;
            this->lbl_IRStatus->Text = L"Status: Standby";
            // 
            // lbl_IRHelp
            // 
            this->lbl_IRHelp->AutoSize = true;
            this->lbl_IRHelp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_IRHelp->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->lbl_IRHelp->Location = System::Drawing::Point(2, 346);
            this->lbl_IRHelp->Name = L"lbl_IRHelp";
            this->lbl_IRHelp->Size = System::Drawing::Size(223, 13);
            this->lbl_IRHelp->TabIndex = 5;
            this->lbl_IRHelp->Text = L"The photo will be taken after initialization\r\n";
            // 
            // pictureBoxIR
            // 
            this->pictureBoxIR->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(50)), static_cast<System::Int32>(static_cast<System::Byte>(50)),
                static_cast<System::Int32>(static_cast<System::Byte>(50)));
            this->pictureBoxIR->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->pictureBoxIR->Location = System::Drawing::Point(5, 21);
            this->pictureBoxIR->Margin = System::Windows::Forms::Padding(0);
            this->pictureBoxIR->Name = L"pictureBoxIR";
            this->pictureBoxIR->Size = System::Drawing::Size(242, 322);
            this->pictureBoxIR->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
            this->pictureBoxIR->TabIndex = 2;
            this->pictureBoxIR->TabStop = false;
            // 
            // btn_getIRImage
            // 
            this->btn_getIRImage->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_getIRImage->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_getIRImage->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_getIRImage->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btn_getIRImage->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->btn_getIRImage->Location = System::Drawing::Point(28, 403);
            this->btn_getIRImage->Name = L"btn_getIRImage";
            this->btn_getIRImage->Size = System::Drawing::Size(87, 30);
            this->btn_getIRImage->TabIndex = 1;
            this->btn_getIRImage->Text = L"Capture";
            this->btn_getIRImage->UseVisualStyleBackColor = false;
            this->btn_getIRImage->Click += gcnew System::EventHandler(this, &FormJoy::btn_getImage_Click);
            // 
            // btn_getIRStream
            // 
            this->btn_getIRStream->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_getIRStream->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_getIRStream->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_getIRStream->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btn_getIRStream->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(60)),
                static_cast<System::Int32>(static_cast<System::Byte>(40)));
            this->btn_getIRStream->Location = System::Drawing::Point(137, 403);
            this->btn_getIRStream->Name = L"btn_getIRStream";
            this->btn_getIRStream->Size = System::Drawing::Size(87, 30);
            this->btn_getIRStream->TabIndex = 32;
            this->btn_getIRStream->Text = L"Stream";
            this->btn_getIRStream->UseVisualStyleBackColor = false;
            this->btn_getIRStream->Click += gcnew System::EventHandler(this, &FormJoy::btn_getVideo_Click);
            // 
            // numeric_IRCustomRegVal
            // 
            this->numeric_IRCustomRegVal->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_IRCustomRegVal->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_IRCustomRegVal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_IRCustomRegVal->Hexadecimal = true;
            this->numeric_IRCustomRegVal->Location = System::Drawing::Point(364, 283);
            this->numeric_IRCustomRegVal->Margin = System::Windows::Forms::Padding(0);
            this->numeric_IRCustomRegVal->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
            this->numeric_IRCustomRegVal->Name = L"numeric_IRCustomRegVal";
            this->numeric_IRCustomRegVal->Size = System::Drawing::Size(53, 25);
            this->numeric_IRCustomRegVal->TabIndex = 37;
            this->numeric_IRCustomRegVal->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_IRCustomRegVal->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            // 
            // numeric_IRCustomRegAddr
            // 
            this->numeric_IRCustomRegAddr->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_IRCustomRegAddr->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_IRCustomRegAddr->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_IRCustomRegAddr->Hexadecimal = true;
            this->numeric_IRCustomRegAddr->Location = System::Drawing::Point(277, 283);
            this->numeric_IRCustomRegAddr->Margin = System::Windows::Forms::Padding(0);
            this->numeric_IRCustomRegAddr->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1535, 0, 0, 0 });
            this->numeric_IRCustomRegAddr->Name = L"numeric_IRCustomRegAddr";
            this->numeric_IRCustomRegAddr->Size = System::Drawing::Size(74, 25);
            this->numeric_IRCustomRegAddr->TabIndex = 36;
            this->numeric_IRCustomRegAddr->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_IRCustomRegAddr->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            // 
            // btn_IRConfigLive
            // 
            this->btn_IRConfigLive->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_IRConfigLive->Enabled = false;
            this->btn_IRConfigLive->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_IRConfigLive->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_IRConfigLive->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 9));
            this->btn_IRConfigLive->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(60)), static_cast<System::Int32>(static_cast<System::Byte>(40)));
            this->btn_IRConfigLive->Location = System::Drawing::Point(303, 318);
            this->btn_IRConfigLive->Name = L"btn_IRConfigLive";
            this->btn_IRConfigLive->Size = System::Drawing::Size(140, 30);
            this->btn_IRConfigLive->TabIndex = 35;
            this->btn_IRConfigLive->Text = L"Commit Changes";
            this->btn_IRConfigLive->UseVisualStyleBackColor = false;
            this->btn_IRConfigLive->Click += gcnew System::EventHandler(this, &FormJoy::btn_IRConfigLive_Click);
            // 
            // lbl_digitalGain
            // 
            this->lbl_digitalGain->AutoSize = true;
            this->lbl_digitalGain->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_digitalGain->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->lbl_digitalGain->Location = System::Drawing::Point(6, 221);
            this->lbl_digitalGain->Name = L"lbl_digitalGain";
            this->lbl_digitalGain->Size = System::Drawing::Size(105, 13);
            this->lbl_digitalGain->TabIndex = 30;
            this->lbl_digitalGain->Text = L"Digital Gain (lossy):";
            // 
            // grpBox_IRColorize
            // 
            this->grpBox_IRColorize->Controls->Add(this->radioBtn_IRColorHeat);
            this->grpBox_IRColorize->Controls->Add(this->radioBtn_IRColorGreen);
            this->grpBox_IRColorize->Controls->Add(this->radioBtn_IRColorRed);
            this->grpBox_IRColorize->Controls->Add(this->radioBtn_IRColorGrey);
            this->grpBox_IRColorize->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_IRColorize->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_IRColorize->Location = System::Drawing::Point(138, 20);
            this->grpBox_IRColorize->Name = L"grpBox_IRColorize";
            this->grpBox_IRColorize->Size = System::Drawing::Size(125, 130);
            this->grpBox_IRColorize->TabIndex = 4;
            this->grpBox_IRColorize->TabStop = false;
            this->grpBox_IRColorize->Text = L"Colorize";
            // 
            // radioBtn_IRColorHeat
            // 
            this->radioBtn_IRColorHeat->AutoSize = true;
            this->radioBtn_IRColorHeat->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->radioBtn_IRColorHeat->Checked = true;
            this->radioBtn_IRColorHeat->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->radioBtn_IRColorHeat->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(207)),
                static_cast<System::Int32>(static_cast<System::Byte>(179)), static_cast<System::Int32>(static_cast<System::Byte>(255)));
            this->radioBtn_IRColorHeat->Location = System::Drawing::Point(10, 75);
            this->radioBtn_IRColorHeat->Margin = System::Windows::Forms::Padding(0);
            this->radioBtn_IRColorHeat->Name = L"radioBtn_IRColorHeat";
            this->radioBtn_IRColorHeat->Size = System::Drawing::Size(74, 21);
            this->radioBtn_IRColorHeat->TabIndex = 3;
            this->radioBtn_IRColorHeat->TabStop = true;
            this->radioBtn_IRColorHeat->Text = L"Ironbow";
            this->radioBtn_IRColorHeat->UseVisualStyleBackColor = false;
            // 
            // radioBtn_IRColorGreen
            // 
            this->radioBtn_IRColorGreen->AutoSize = true;
            this->radioBtn_IRColorGreen->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->radioBtn_IRColorGreen->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->radioBtn_IRColorGreen->ForeColor = System::Drawing::Color::Lime;
            this->radioBtn_IRColorGreen->Location = System::Drawing::Point(10, 48);
            this->radioBtn_IRColorGreen->Margin = System::Windows::Forms::Padding(0);
            this->radioBtn_IRColorGreen->Name = L"radioBtn_IRColorGreen";
            this->radioBtn_IRColorGreen->Size = System::Drawing::Size(95, 21);
            this->radioBtn_IRColorGreen->TabIndex = 2;
            this->radioBtn_IRColorGreen->Text = L"Night vision";
            this->radioBtn_IRColorGreen->UseVisualStyleBackColor = false;
            // 
            // radioBtn_IRColorRed
            // 
            this->radioBtn_IRColorRed->AutoSize = true;
            this->radioBtn_IRColorRed->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->radioBtn_IRColorRed->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->radioBtn_IRColorRed->ForeColor = System::Drawing::Color::Tomato;
            this->radioBtn_IRColorRed->Location = System::Drawing::Point(10, 102);
            this->radioBtn_IRColorRed->Margin = System::Windows::Forms::Padding(0);
            this->radioBtn_IRColorRed->Name = L"radioBtn_IRColorRed";
            this->radioBtn_IRColorRed->Size = System::Drawing::Size(72, 21);
            this->radioBtn_IRColorRed->TabIndex = 1;
            this->radioBtn_IRColorRed->Text = L"Infrared";
            this->radioBtn_IRColorRed->UseVisualStyleBackColor = false;
            // 
            // radioBtn_IRColorGrey
            // 
            this->radioBtn_IRColorGrey->AutoSize = true;
            this->radioBtn_IRColorGrey->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->radioBtn_IRColorGrey->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->radioBtn_IRColorGrey->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(224)),
                static_cast<System::Int32>(static_cast<System::Byte>(224)), static_cast<System::Int32>(static_cast<System::Byte>(224)));
            this->radioBtn_IRColorGrey->Location = System::Drawing::Point(10, 21);
            this->radioBtn_IRColorGrey->Margin = System::Windows::Forms::Padding(0);
            this->radioBtn_IRColorGrey->Name = L"radioBtn_IRColorGrey";
            this->radioBtn_IRColorGrey->Size = System::Drawing::Size(82, 21);
            this->radioBtn_IRColorGrey->TabIndex = 0;
            this->radioBtn_IRColorGrey->Text = L"Greyscale";
            this->radioBtn_IRColorGrey->UseVisualStyleBackColor = false;
            // 
            // chkBox_IRDimLeds
            // 
            this->chkBox_IRDimLeds->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->chkBox_IRDimLeds->Checked = true;
            this->chkBox_IRDimLeds->CheckState = System::Windows::Forms::CheckState::Checked;
            this->chkBox_IRDimLeds->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->chkBox_IRDimLeds->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->chkBox_IRDimLeds->Location = System::Drawing::Point(4, 76);
            this->chkBox_IRDimLeds->Name = L"chkBox_IRDimLeds";
            this->chkBox_IRDimLeds->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->chkBox_IRDimLeds->Size = System::Drawing::Size(195, 21);
            this->chkBox_IRDimLeds->TabIndex = 27;
            this->chkBox_IRDimLeds->Text = L"Near/Wide  (130°)  Leds 3/4";
            // 
            // chkBox_IRBrightLeds
            // 
            this->chkBox_IRBrightLeds->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->chkBox_IRBrightLeds->Checked = true;
            this->chkBox_IRBrightLeds->CheckState = System::Windows::Forms::CheckState::Checked;
            this->chkBox_IRBrightLeds->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->chkBox_IRBrightLeds->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->chkBox_IRBrightLeds->Location = System::Drawing::Point(4, 19);
            this->chkBox_IRBrightLeds->Name = L"chkBox_IRBrightLeds";
            this->chkBox_IRBrightLeds->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->chkBox_IRBrightLeds->Size = System::Drawing::Size(195, 21);
            this->chkBox_IRBrightLeds->TabIndex = 26;
            this->chkBox_IRBrightLeds->Text = L"Far/Narrow   (75°)  Leds 1/2";
            // 
            // grpBox_IRRes
            // 
            this->grpBox_IRRes->Controls->Add(this->radioBtn_IR30p);
            this->grpBox_IRRes->Controls->Add(this->radioBtn_IR60p);
            this->grpBox_IRRes->Controls->Add(this->radioBtn_IR120p);
            this->grpBox_IRRes->Controls->Add(this->radioBtn_IR240p);
            this->grpBox_IRRes->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_IRRes->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_IRRes->Location = System::Drawing::Point(10, 20);
            this->grpBox_IRRes->Name = L"grpBox_IRRes";
            this->grpBox_IRRes->Size = System::Drawing::Size(125, 130);
            this->grpBox_IRRes->TabIndex = 3;
            this->grpBox_IRRes->TabStop = false;
            this->grpBox_IRRes->Text = L"Resolution";
            // 
            // radioBtn_IR30p
            // 
            this->radioBtn_IR30p->AutoSize = true;
            this->radioBtn_IR30p->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->radioBtn_IR30p->FlatAppearance->BorderSize = 0;
            this->radioBtn_IR30p->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->radioBtn_IR30p->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->radioBtn_IR30p->Location = System::Drawing::Point(10, 102);
            this->radioBtn_IR30p->Margin = System::Windows::Forms::Padding(0);
            this->radioBtn_IR30p->Name = L"radioBtn_IR30p";
            this->radioBtn_IR30p->Size = System::Drawing::Size(68, 21);
            this->radioBtn_IR30p->TabIndex = 3;
            this->radioBtn_IR30p->Text = L"30 x 40";
            this->radioBtn_IR30p->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
            this->radioBtn_IR30p->UseVisualStyleBackColor = false;
            // 
            // radioBtn_IR60p
            // 
            this->radioBtn_IR60p->AutoSize = true;
            this->radioBtn_IR60p->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->radioBtn_IR60p->FlatAppearance->BorderSize = 0;
            this->radioBtn_IR60p->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->radioBtn_IR60p->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->radioBtn_IR60p->Location = System::Drawing::Point(10, 75);
            this->radioBtn_IR60p->Margin = System::Windows::Forms::Padding(0);
            this->radioBtn_IR60p->Name = L"radioBtn_IR60p";
            this->radioBtn_IR60p->Size = System::Drawing::Size(68, 21);
            this->radioBtn_IR60p->TabIndex = 2;
            this->radioBtn_IR60p->Text = L"60 x 80";
            this->radioBtn_IR60p->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
            this->radioBtn_IR60p->UseVisualStyleBackColor = false;
            // 
            // radioBtn_IR120p
            // 
            this->radioBtn_IR120p->AutoSize = true;
            this->radioBtn_IR120p->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->radioBtn_IR120p->FlatAppearance->BorderSize = 0;
            this->radioBtn_IR120p->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->radioBtn_IR120p->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->radioBtn_IR120p->Location = System::Drawing::Point(10, 48);
            this->radioBtn_IR120p->Margin = System::Windows::Forms::Padding(0);
            this->radioBtn_IR120p->Name = L"radioBtn_IR120p";
            this->radioBtn_IR120p->Size = System::Drawing::Size(82, 21);
            this->radioBtn_IR120p->TabIndex = 1;
            this->radioBtn_IR120p->Text = L"120 x 160";
            this->radioBtn_IR120p->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
            this->radioBtn_IR120p->UseVisualStyleBackColor = false;
            // 
            // radioBtn_IR240p
            // 
            this->radioBtn_IR240p->AutoSize = true;
            this->radioBtn_IR240p->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->radioBtn_IR240p->Checked = true;
            this->radioBtn_IR240p->FlatAppearance->BorderColor = System::Drawing::Color::Red;
            this->radioBtn_IR240p->FlatAppearance->BorderSize = 0;
            this->radioBtn_IR240p->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->radioBtn_IR240p->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->radioBtn_IR240p->Location = System::Drawing::Point(10, 21);
            this->radioBtn_IR240p->Margin = System::Windows::Forms::Padding(0);
            this->radioBtn_IR240p->Name = L"radioBtn_IR240p";
            this->radioBtn_IR240p->Size = System::Drawing::Size(82, 21);
            this->radioBtn_IR240p->TabIndex = 0;
            this->radioBtn_IR240p->TabStop = true;
            this->radioBtn_IR240p->Text = L"240 x 320";
            this->radioBtn_IR240p->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
            this->radioBtn_IR240p->UseVisualStyleBackColor = false;
            // 
            // grpBox_nfc
            // 
            this->grpBox_nfc->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_nfc->Controls->Add(this->txtBox_nfcUid);
            this->grpBox_nfc->Controls->Add(this->btn_NFC);
            this->grpBox_nfc->Controls->Add(this->lbl_nfcHelp);
            this->grpBox_nfc->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_nfc->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_nfc->Location = System::Drawing::Point(724, 445);
            this->grpBox_nfc->Margin = System::Windows::Forms::Padding(0, 0, 14, 0);
            this->grpBox_nfc->Name = L"grpBox_nfc";
            this->grpBox_nfc->Size = System::Drawing::Size(220, 215);
            this->grpBox_nfc->TabIndex = 35;
            this->grpBox_nfc->TabStop = false;
            this->grpBox_nfc->Text = L"NFC Simple Tag Info";
            // 
            // txtBox_nfcUid
            // 
            this->txtBox_nfcUid->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->txtBox_nfcUid->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->txtBox_nfcUid->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->txtBox_nfcUid->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->txtBox_nfcUid->Location = System::Drawing::Point(8, 116);
            this->txtBox_nfcUid->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->txtBox_nfcUid->Multiline = true;
            this->txtBox_nfcUid->Name = L"txtBox_nfcUid";
            this->txtBox_nfcUid->ReadOnly = true;
            this->txtBox_nfcUid->Size = System::Drawing::Size(203, 37);
            this->txtBox_nfcUid->TabIndex = 36;
            this->txtBox_nfcUid->TabStop = false;
            this->txtBox_nfcUid->Text = L"Type:\r\nUID:";
            // 
            // btn_NFC
            // 
            this->btn_NFC->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_NFC->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_NFC->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_NFC->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btn_NFC->Location = System::Drawing::Point(65, 166);
            this->btn_NFC->Name = L"btn_NFC";
            this->btn_NFC->Size = System::Drawing::Size(87, 30);
            this->btn_NFC->TabIndex = 34;
            this->btn_NFC->Text = L"Scan";
            this->btn_NFC->UseVisualStyleBackColor = false;
            this->btn_NFC->Click += gcnew System::EventHandler(this, &FormJoy::btn_NFC_Click);
            // 
            // lbl_nfcHelp
            // 
            this->lbl_nfcHelp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_nfcHelp->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->lbl_nfcHelp->Location = System::Drawing::Point(8, 21);
            this->lbl_nfcHelp->Name = L"lbl_nfcHelp";
            this->lbl_nfcHelp->Size = System::Drawing::Size(203, 85);
            this->lbl_nfcHelp->TabIndex = 32;
            this->lbl_nfcHelp->Text = L"Press Scan and try to touch some tags or NFC-enabled phone over the Joy-Con (R)\'s"
                L" analog stick.\r\nWhen done, press stop.";
            // 
            // checkBox_enableLeftUserCal
            // 
            this->checkBox_enableLeftUserCal->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->checkBox_enableLeftUserCal->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->checkBox_enableLeftUserCal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->checkBox_enableLeftUserCal->Location = System::Drawing::Point(8, 111);
            this->checkBox_enableLeftUserCal->Margin = System::Windows::Forms::Padding(0);
            this->checkBox_enableLeftUserCal->Name = L"checkBox_enableLeftUserCal";
            this->checkBox_enableLeftUserCal->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->checkBox_enableLeftUserCal->Size = System::Drawing::Size(189, 22);
            this->checkBox_enableLeftUserCal->TabIndex = 3;
            this->checkBox_enableLeftUserCal->Text = L"Enable Left Stick Cal";
            // 
            // numeric_leftUserCal_y_plus
            // 
            this->numeric_leftUserCal_y_plus->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_leftUserCal_y_plus->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_leftUserCal_y_plus->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_leftUserCal_y_plus->Hexadecimal = true;
            this->numeric_leftUserCal_y_plus->Location = System::Drawing::Point(143, 75);
            this->numeric_leftUserCal_y_plus->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            this->numeric_leftUserCal_y_plus->Name = L"numeric_leftUserCal_y_plus";
            this->numeric_leftUserCal_y_plus->Size = System::Drawing::Size(53, 25);
            this->numeric_leftUserCal_y_plus->TabIndex = 48;
            this->numeric_leftUserCal_y_plus->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_leftUserCal_y_plus->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            this->numeric_leftUserCal_y_plus->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            // 
            // numeric_leftUserCal_y_center
            // 
            this->numeric_leftUserCal_y_center->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_leftUserCal_y_center->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_leftUserCal_y_center->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_leftUserCal_y_center->Hexadecimal = true;
            this->numeric_leftUserCal_y_center->Location = System::Drawing::Point(78, 75);
            this->numeric_leftUserCal_y_center->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            this->numeric_leftUserCal_y_center->Name = L"numeric_leftUserCal_y_center";
            this->numeric_leftUserCal_y_center->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->numeric_leftUserCal_y_center->Size = System::Drawing::Size(53, 25);
            this->numeric_leftUserCal_y_center->TabIndex = 47;
            this->numeric_leftUserCal_y_center->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_leftUserCal_y_center->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            this->numeric_leftUserCal_y_center->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2047, 0, 0, 0 });
            // 
            // numeric_leftUserCal_y_minus
            // 
            this->numeric_leftUserCal_y_minus->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_leftUserCal_y_minus->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_leftUserCal_y_minus->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_leftUserCal_y_minus->Hexadecimal = true;
            this->numeric_leftUserCal_y_minus->Location = System::Drawing::Point(13, 75);
            this->numeric_leftUserCal_y_minus->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            this->numeric_leftUserCal_y_minus->Name = L"numeric_leftUserCal_y_minus";
            this->numeric_leftUserCal_y_minus->Size = System::Drawing::Size(53, 25);
            this->numeric_leftUserCal_y_minus->TabIndex = 46;
            this->numeric_leftUserCal_y_minus->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_leftUserCal_y_minus->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            // 
            // numeric_leftUserCal_x_plus
            // 
            this->numeric_leftUserCal_x_plus->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_leftUserCal_x_plus->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_leftUserCal_x_plus->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_leftUserCal_x_plus->Hexadecimal = true;
            this->numeric_leftUserCal_x_plus->Location = System::Drawing::Point(143, 44);
            this->numeric_leftUserCal_x_plus->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            this->numeric_leftUserCal_x_plus->Name = L"numeric_leftUserCal_x_plus";
            this->numeric_leftUserCal_x_plus->Size = System::Drawing::Size(53, 25);
            this->numeric_leftUserCal_x_plus->TabIndex = 45;
            this->numeric_leftUserCal_x_plus->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_leftUserCal_x_plus->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            this->numeric_leftUserCal_x_plus->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            // 
            // numeric_leftUserCal_x_center
            // 
            this->numeric_leftUserCal_x_center->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_leftUserCal_x_center->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_leftUserCal_x_center->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_leftUserCal_x_center->Hexadecimal = true;
            this->numeric_leftUserCal_x_center->Location = System::Drawing::Point(78, 44);
            this->numeric_leftUserCal_x_center->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            this->numeric_leftUserCal_x_center->Name = L"numeric_leftUserCal_x_center";
            this->numeric_leftUserCal_x_center->Size = System::Drawing::Size(53, 25);
            this->numeric_leftUserCal_x_center->TabIndex = 44;
            this->numeric_leftUserCal_x_center->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_leftUserCal_x_center->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            this->numeric_leftUserCal_x_center->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2047, 0, 0, 0 });
            // 
            // numeric_leftUserCal_x_minus
            // 
            this->numeric_leftUserCal_x_minus->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_leftUserCal_x_minus->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_leftUserCal_x_minus->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_leftUserCal_x_minus->Hexadecimal = true;
            this->numeric_leftUserCal_x_minus->Location = System::Drawing::Point(13, 44);
            this->numeric_leftUserCal_x_minus->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            this->numeric_leftUserCal_x_minus->Name = L"numeric_leftUserCal_x_minus";
            this->numeric_leftUserCal_x_minus->Size = System::Drawing::Size(53, 25);
            this->numeric_leftUserCal_x_minus->TabIndex = 43;
            this->numeric_leftUserCal_x_minus->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_leftUserCal_x_minus->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            // 
            // btn_refreshUserCal
            // 
            this->btn_refreshUserCal->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_refreshUserCal->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_refreshUserCal->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_refreshUserCal->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btn_refreshUserCal->Location = System::Drawing::Point(243, 344);
            this->btn_refreshUserCal->Name = L"btn_refreshUserCal";
            this->btn_refreshUserCal->Size = System::Drawing::Size(93, 30);
            this->btn_refreshUserCal->TabIndex = 37;
            this->btn_refreshUserCal->Text = L"Refresh All";
            this->btn_refreshUserCal->UseVisualStyleBackColor = false;
            this->btn_refreshUserCal->Click += gcnew System::EventHandler(this, &FormJoy::btn_refreshUserCal_Click);
            // 
            // checkBox_enableRightUserCal
            // 
            this->checkBox_enableRightUserCal->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->checkBox_enableRightUserCal->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->checkBox_enableRightUserCal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->checkBox_enableRightUserCal->Location = System::Drawing::Point(8, 111);
            this->checkBox_enableRightUserCal->Margin = System::Windows::Forms::Padding(0);
            this->checkBox_enableRightUserCal->Name = L"checkBox_enableRightUserCal";
            this->checkBox_enableRightUserCal->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->checkBox_enableRightUserCal->Size = System::Drawing::Size(189, 22);
            this->checkBox_enableRightUserCal->TabIndex = 49;
            this->checkBox_enableRightUserCal->Text = L"Enable Right Stick Cal";
            // 
            // numeric_rightUserCal_y_plus
            // 
            this->numeric_rightUserCal_y_plus->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_rightUserCal_y_plus->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_rightUserCal_y_plus->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_rightUserCal_y_plus->Hexadecimal = true;
            this->numeric_rightUserCal_y_plus->Location = System::Drawing::Point(143, 75);
            this->numeric_rightUserCal_y_plus->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            this->numeric_rightUserCal_y_plus->Name = L"numeric_rightUserCal_y_plus";
            this->numeric_rightUserCal_y_plus->Size = System::Drawing::Size(53, 25);
            this->numeric_rightUserCal_y_plus->TabIndex = 55;
            this->numeric_rightUserCal_y_plus->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_rightUserCal_y_plus->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            this->numeric_rightUserCal_y_plus->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            // 
            // numeric_rightUserCal_y_center
            // 
            this->numeric_rightUserCal_y_center->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_rightUserCal_y_center->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_rightUserCal_y_center->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_rightUserCal_y_center->Hexadecimal = true;
            this->numeric_rightUserCal_y_center->Location = System::Drawing::Point(78, 75);
            this->numeric_rightUserCal_y_center->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            this->numeric_rightUserCal_y_center->Name = L"numeric_rightUserCal_y_center";
            this->numeric_rightUserCal_y_center->Size = System::Drawing::Size(53, 25);
            this->numeric_rightUserCal_y_center->TabIndex = 54;
            this->numeric_rightUserCal_y_center->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_rightUserCal_y_center->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            this->numeric_rightUserCal_y_center->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2047, 0, 0, 0 });
            // 
            // numeric_rightUserCal_y_minus
            // 
            this->numeric_rightUserCal_y_minus->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_rightUserCal_y_minus->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_rightUserCal_y_minus->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_rightUserCal_y_minus->Hexadecimal = true;
            this->numeric_rightUserCal_y_minus->Location = System::Drawing::Point(13, 75);
            this->numeric_rightUserCal_y_minus->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            this->numeric_rightUserCal_y_minus->Name = L"numeric_rightUserCal_y_minus";
            this->numeric_rightUserCal_y_minus->Size = System::Drawing::Size(53, 25);
            this->numeric_rightUserCal_y_minus->TabIndex = 53;
            this->numeric_rightUserCal_y_minus->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_rightUserCal_y_minus->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            // 
            // numeric_rightUserCal_x_plus
            // 
            this->numeric_rightUserCal_x_plus->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_rightUserCal_x_plus->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_rightUserCal_x_plus->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_rightUserCal_x_plus->Hexadecimal = true;
            this->numeric_rightUserCal_x_plus->Location = System::Drawing::Point(143, 44);
            this->numeric_rightUserCal_x_plus->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            this->numeric_rightUserCal_x_plus->Name = L"numeric_rightUserCal_x_plus";
            this->numeric_rightUserCal_x_plus->Size = System::Drawing::Size(53, 25);
            this->numeric_rightUserCal_x_plus->TabIndex = 52;
            this->numeric_rightUserCal_x_plus->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_rightUserCal_x_plus->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            this->numeric_rightUserCal_x_plus->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            // 
            // numeric_rightUserCal_x_center
            // 
            this->numeric_rightUserCal_x_center->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_rightUserCal_x_center->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_rightUserCal_x_center->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_rightUserCal_x_center->Hexadecimal = true;
            this->numeric_rightUserCal_x_center->Location = System::Drawing::Point(78, 44);
            this->numeric_rightUserCal_x_center->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            this->numeric_rightUserCal_x_center->Name = L"numeric_rightUserCal_x_center";
            this->numeric_rightUserCal_x_center->Size = System::Drawing::Size(53, 25);
            this->numeric_rightUserCal_x_center->TabIndex = 51;
            this->numeric_rightUserCal_x_center->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_rightUserCal_x_center->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            this->numeric_rightUserCal_x_center->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2047, 0, 0, 0 });
            // 
            // numeric_rightUserCal_x_minus
            // 
            this->numeric_rightUserCal_x_minus->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_rightUserCal_x_minus->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_rightUserCal_x_minus->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_rightUserCal_x_minus->Hexadecimal = true;
            this->numeric_rightUserCal_x_minus->Location = System::Drawing::Point(13, 44);
            this->numeric_rightUserCal_x_minus->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            this->numeric_rightUserCal_x_minus->Name = L"numeric_rightUserCal_x_minus";
            this->numeric_rightUserCal_x_minus->Size = System::Drawing::Size(53, 25);
            this->numeric_rightUserCal_x_minus->TabIndex = 50;
            this->numeric_rightUserCal_x_minus->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_rightUserCal_x_minus->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            // 
            // grpBox_leftStickUCal
            // 
            this->grpBox_leftStickUCal->Controls->Add(this->lbl_userCalMinCenterMax);
            this->grpBox_leftStickUCal->Controls->Add(this->numeric_leftUserCal_x_minus);
            this->grpBox_leftStickUCal->Controls->Add(this->numeric_leftUserCal_x_center);
            this->grpBox_leftStickUCal->Controls->Add(this->numeric_leftUserCal_x_plus);
            this->grpBox_leftStickUCal->Controls->Add(this->numeric_leftUserCal_y_minus);
            this->grpBox_leftStickUCal->Controls->Add(this->numeric_leftUserCal_y_center);
            this->grpBox_leftStickUCal->Controls->Add(this->numeric_leftUserCal_y_plus);
            this->grpBox_leftStickUCal->Controls->Add(this->checkBox_enableLeftUserCal);
            this->grpBox_leftStickUCal->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_leftStickUCal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)),
                static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_leftStickUCal->Location = System::Drawing::Point(6, 187);
            this->grpBox_leftStickUCal->Name = L"grpBox_leftStickUCal";
            this->grpBox_leftStickUCal->Size = System::Drawing::Size(208, 144);
            this->grpBox_leftStickUCal->TabIndex = 50;
            this->grpBox_leftStickUCal->TabStop = false;
            this->grpBox_leftStickUCal->Text = L"Left Analog Stick X/Y";
            // 
            // lbl_userCalMinCenterMax
            // 
            this->lbl_userCalMinCenterMax->AutoSize = true;
            this->lbl_userCalMinCenterMax->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_userCalMinCenterMax->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->lbl_userCalMinCenterMax->Location = System::Drawing::Point(9, 26);
            this->lbl_userCalMinCenterMax->Name = L"lbl_userCalMinCenterMax";
            this->lbl_userCalMinCenterMax->Size = System::Drawing::Size(161, 13);
            this->lbl_userCalMinCenterMax->TabIndex = 49;
            this->lbl_userCalMinCenterMax->Text = L"Minimum / Center / Maximum:";
            this->lbl_userCalMinCenterMax->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
            // 
            // grpBox_rightStickUCal
            // 
            this->grpBox_rightStickUCal->Controls->Add(this->lbl_userCalMinCenterMax2);
            this->grpBox_rightStickUCal->Controls->Add(this->checkBox_enableRightUserCal);
            this->grpBox_rightStickUCal->Controls->Add(this->numeric_rightUserCal_x_minus);
            this->grpBox_rightStickUCal->Controls->Add(this->numeric_rightUserCal_y_plus);
            this->grpBox_rightStickUCal->Controls->Add(this->numeric_rightUserCal_x_center);
            this->grpBox_rightStickUCal->Controls->Add(this->numeric_rightUserCal_y_center);
            this->grpBox_rightStickUCal->Controls->Add(this->numeric_rightUserCal_x_plus);
            this->grpBox_rightStickUCal->Controls->Add(this->numeric_rightUserCal_y_minus);
            this->grpBox_rightStickUCal->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_rightStickUCal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)),
                static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_rightStickUCal->Location = System::Drawing::Point(239, 187);
            this->grpBox_rightStickUCal->Name = L"grpBox_rightStickUCal";
            this->grpBox_rightStickUCal->Size = System::Drawing::Size(208, 144);
            this->grpBox_rightStickUCal->TabIndex = 51;
            this->grpBox_rightStickUCal->TabStop = false;
            this->grpBox_rightStickUCal->Text = L"Right Analog Stick X/Y";
            // 
            // lbl_userCalMinCenterMax2
            // 
            this->lbl_userCalMinCenterMax2->AutoSize = true;
            this->lbl_userCalMinCenterMax2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_userCalMinCenterMax2->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->lbl_userCalMinCenterMax2->Location = System::Drawing::Point(9, 26);
            this->lbl_userCalMinCenterMax2->Margin = System::Windows::Forms::Padding(0, 0, 3, 0);
            this->lbl_userCalMinCenterMax2->Name = L"lbl_userCalMinCenterMax2";
            this->lbl_userCalMinCenterMax2->Size = System::Drawing::Size(161, 13);
            this->lbl_userCalMinCenterMax2->TabIndex = 56;
            this->lbl_userCalMinCenterMax2->Text = L"Minimum / Center / Maximum:";
            this->lbl_userCalMinCenterMax2->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
            // 
            // btn_writeUserCal
            // 
            this->btn_writeUserCal->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_writeUserCal->Enabled = false;
            this->btn_writeUserCal->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_writeUserCal->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_writeUserCal->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btn_writeUserCal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->btn_writeUserCal->Location = System::Drawing::Point(350, 344);
            this->btn_writeUserCal->Name = L"btn_writeUserCal";
            this->btn_writeUserCal->Size = System::Drawing::Size(93, 30);
            this->btn_writeUserCal->TabIndex = 52;
            this->btn_writeUserCal->Text = L"Write Cal";
            this->btn_writeUserCal->UseVisualStyleBackColor = false;
            this->btn_writeUserCal->Click += gcnew System::EventHandler(this, &FormJoy::btn_writeUserCal_Click);
            // 
            // grpBox_IRSettings
            // 
            this->grpBox_IRSettings->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_IRSettings->Controls->Add(this->grpBox_IRInfraredLight);
            this->grpBox_IRSettings->Controls->Add(this->chkBox_IRAutoExposure);
            this->grpBox_IRSettings->Controls->Add(this->grpBox_IRSettingsDenoise);
            this->grpBox_IRSettings->Controls->Add(this->lbl_IRCustomReg);
            this->grpBox_IRSettings->Controls->Add(this->chkBox_IRSelfie);
            this->grpBox_IRSettings->Controls->Add(this->trackBar_IRGain);
            this->grpBox_IRSettings->Controls->Add(this->btn_IRConfigLive);
            this->grpBox_IRSettings->Controls->Add(this->grpBox_IRRes);
            this->grpBox_IRSettings->Controls->Add(this->grpBox_IRColorize);
            this->grpBox_IRSettings->Controls->Add(this->numeric_IRCustomRegVal);
            this->grpBox_IRSettings->Controls->Add(this->numeric_IRCustomRegAddr);
            this->grpBox_IRSettings->Controls->Add(this->lbl_exposure);
            this->grpBox_IRSettings->Controls->Add(this->numeric_IRExposure);
            this->grpBox_IRSettings->Controls->Add(this->lbl_digitalGain);
            this->grpBox_IRSettings->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_IRSettings->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_IRSettings->Location = System::Drawing::Point(954, 447);
            this->grpBox_IRSettings->Margin = System::Windows::Forms::Padding(0, 0, 14, 39);
            this->grpBox_IRSettings->Name = L"grpBox_IRSettings";
            this->grpBox_IRSettings->Size = System::Drawing::Size(483, 360);
            this->grpBox_IRSettings->TabIndex = 41;
            this->grpBox_IRSettings->TabStop = false;
            this->grpBox_IRSettings->Text = L"IR Camera Settings";
            // 
            // grpBox_IRInfraredLight
            // 
            this->grpBox_IRInfraredLight->Controls->Add(this->chkBox_IRBrightLeds);
            this->grpBox_IRInfraredLight->Controls->Add(this->chkBox_IRDimLeds);
            this->grpBox_IRInfraredLight->Controls->Add(this->chkBox_IRExFilter);
            this->grpBox_IRInfraredLight->Controls->Add(this->trackBar_IRBrightLeds);
            this->grpBox_IRInfraredLight->Controls->Add(this->trackBar_IRDimLeds);
            this->grpBox_IRInfraredLight->Controls->Add(this->chkBox_IRStrobe);
            this->grpBox_IRInfraredLight->Controls->Add(this->lbl_IRLed1Int);
            this->grpBox_IRInfraredLight->Controls->Add(this->chkBox_IRFlashlight);
            this->grpBox_IRInfraredLight->Controls->Add(this->lbl_IRLed2Int);
            this->grpBox_IRInfraredLight->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_IRInfraredLight->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)),
                static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_IRInfraredLight->Location = System::Drawing::Point(269, 20);
            this->grpBox_IRInfraredLight->Name = L"grpBox_IRInfraredLight";
            this->grpBox_IRInfraredLight->Size = System::Drawing::Size(205, 212);
            this->grpBox_IRInfraredLight->TabIndex = 5;
            this->grpBox_IRInfraredLight->TabStop = false;
            this->grpBox_IRInfraredLight->Text = L"Near-Infrared Light";
            // 
            // chkBox_IRExFilter
            // 
            this->chkBox_IRExFilter->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->chkBox_IRExFilter->Checked = true;
            this->chkBox_IRExFilter->CheckState = System::Windows::Forms::CheckState::Checked;
            this->chkBox_IRExFilter->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->chkBox_IRExFilter->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->chkBox_IRExFilter->Location = System::Drawing::Point(4, 183);
            this->chkBox_IRExFilter->Name = L"chkBox_IRExFilter";
            this->chkBox_IRExFilter->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->chkBox_IRExFilter->Size = System::Drawing::Size(195, 21);
            this->chkBox_IRExFilter->TabIndex = 38;
            this->chkBox_IRExFilter->Text = L"External IR Filter";
            // 
            // trackBar_IRBrightLeds
            // 
            this->trackBar_IRBrightLeds->AutoSize = false;
            this->trackBar_IRBrightLeds->LargeChange = 1;
            this->trackBar_IRBrightLeds->Location = System::Drawing::Point(78, 43);
            this->trackBar_IRBrightLeds->Margin = System::Windows::Forms::Padding(0);
            this->trackBar_IRBrightLeds->Maximum = 15;
            this->trackBar_IRBrightLeds->Name = L"trackBar_IRBrightLeds";
            this->trackBar_IRBrightLeds->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->trackBar_IRBrightLeds->Size = System::Drawing::Size(121, 28);
            this->trackBar_IRBrightLeds->TabIndex = 39;
            this->trackBar_IRBrightLeds->Value = 15;
            this->trackBar_IRBrightLeds->ValueChanged += gcnew System::EventHandler(this, &FormJoy::TrackBarIRLedsIntensity_ValueChanged);
            // 
            // trackBar_IRDimLeds
            // 
            this->trackBar_IRDimLeds->AutoSize = false;
            this->trackBar_IRDimLeds->LargeChange = 1;
            this->trackBar_IRDimLeds->Location = System::Drawing::Point(78, 100);
            this->trackBar_IRDimLeds->Margin = System::Windows::Forms::Padding(0);
            this->trackBar_IRDimLeds->Maximum = 16;
            this->trackBar_IRDimLeds->Name = L"trackBar_IRDimLeds";
            this->trackBar_IRDimLeds->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->trackBar_IRDimLeds->Size = System::Drawing::Size(121, 28);
            this->trackBar_IRDimLeds->TabIndex = 40;
            this->trackBar_IRDimLeds->Value = 16;
            this->trackBar_IRDimLeds->ValueChanged += gcnew System::EventHandler(this, &FormJoy::TrackBarIRLedsIntensity_ValueChanged);
            // 
            // chkBox_IRStrobe
            // 
            this->chkBox_IRStrobe->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->chkBox_IRStrobe->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->chkBox_IRStrobe->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->chkBox_IRStrobe->Location = System::Drawing::Point(4, 158);
            this->chkBox_IRStrobe->Name = L"chkBox_IRStrobe";
            this->chkBox_IRStrobe->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->chkBox_IRStrobe->Size = System::Drawing::Size(195, 21);
            this->chkBox_IRStrobe->TabIndex = 44;
            this->chkBox_IRStrobe->Text = L"Strobe Flash mode";
            // 
            // lbl_IRLed1Int
            // 
            this->lbl_IRLed1Int->AutoSize = true;
            this->lbl_IRLed1Int->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_IRLed1Int->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->lbl_IRLed1Int->Location = System::Drawing::Point(21, 45);
            this->lbl_IRLed1Int->Name = L"lbl_IRLed1Int";
            this->lbl_IRLed1Int->Size = System::Drawing::Size(54, 13);
            this->lbl_IRLed1Int->TabIndex = 41;
            this->lbl_IRLed1Int->Text = L"Intensity:";
            this->lbl_IRLed1Int->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
            // 
            // chkBox_IRFlashlight
            // 
            this->chkBox_IRFlashlight->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->chkBox_IRFlashlight->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->chkBox_IRFlashlight->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->chkBox_IRFlashlight->Location = System::Drawing::Point(4, 133);
            this->chkBox_IRFlashlight->Name = L"chkBox_IRFlashlight";
            this->chkBox_IRFlashlight->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->chkBox_IRFlashlight->Size = System::Drawing::Size(195, 21);
            this->chkBox_IRFlashlight->TabIndex = 43;
            this->chkBox_IRFlashlight->Text = L"Flashlight mode";
            // 
            // lbl_IRLed2Int
            // 
            this->lbl_IRLed2Int->AutoSize = true;
            this->lbl_IRLed2Int->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_IRLed2Int->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->lbl_IRLed2Int->Location = System::Drawing::Point(21, 102);
            this->lbl_IRLed2Int->Name = L"lbl_IRLed2Int";
            this->lbl_IRLed2Int->Size = System::Drawing::Size(54, 13);
            this->lbl_IRLed2Int->TabIndex = 42;
            this->lbl_IRLed2Int->Text = L"Intensity:";
            this->lbl_IRLed2Int->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
            // 
            // chkBox_IRAutoExposure
            // 
            this->chkBox_IRAutoExposure->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->chkBox_IRAutoExposure->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->chkBox_IRAutoExposure->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->chkBox_IRAutoExposure->Location = System::Drawing::Point(109, 163);
            this->chkBox_IRAutoExposure->Margin = System::Windows::Forms::Padding(0);
            this->chkBox_IRAutoExposure->Name = L"chkBox_IRAutoExposure";
            this->chkBox_IRAutoExposure->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->chkBox_IRAutoExposure->Size = System::Drawing::Size(115, 52);
            this->chkBox_IRAutoExposure->TabIndex = 51;
            this->chkBox_IRAutoExposure->Text = L"Auto Exposure\r\n(experimental)";
            // 
            // grpBox_IRSettingsDenoise
            // 
            this->grpBox_IRSettingsDenoise->Controls->Add(this->numeric_IRDenoiseEdgeSmoothing);
            this->grpBox_IRSettingsDenoise->Controls->Add(this->chkBox_IRDenoise);
            this->grpBox_IRSettingsDenoise->Controls->Add(this->lbl_IRDenoise2);
            this->grpBox_IRSettingsDenoise->Controls->Add(this->numeric_IRDenoiseColorInterpolation);
            this->grpBox_IRSettingsDenoise->Controls->Add(this->lbl_IRDenoise1);
            this->grpBox_IRSettingsDenoise->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_IRSettingsDenoise->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)),
                static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_IRSettingsDenoise->Location = System::Drawing::Point(10, 279);
            this->grpBox_IRSettingsDenoise->Name = L"grpBox_IRSettingsDenoise";
            this->grpBox_IRSettingsDenoise->Size = System::Drawing::Size(253, 69);
            this->grpBox_IRSettingsDenoise->TabIndex = 5;
            this->grpBox_IRSettingsDenoise->TabStop = false;
            this->grpBox_IRSettingsDenoise->Text = L"De-noise";
            // 
            // numeric_IRDenoiseEdgeSmoothing
            // 
            this->numeric_IRDenoiseEdgeSmoothing->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_IRDenoiseEdgeSmoothing->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_IRDenoiseEdgeSmoothing->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_IRDenoiseEdgeSmoothing->Location = System::Drawing::Point(89, 33);
            this->numeric_IRDenoiseEdgeSmoothing->Margin = System::Windows::Forms::Padding(0);
            this->numeric_IRDenoiseEdgeSmoothing->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 256, 0, 0, 0 });
            this->numeric_IRDenoiseEdgeSmoothing->Name = L"numeric_IRDenoiseEdgeSmoothing";
            this->numeric_IRDenoiseEdgeSmoothing->Size = System::Drawing::Size(62, 25);
            this->numeric_IRDenoiseEdgeSmoothing->TabIndex = 47;
            this->numeric_IRDenoiseEdgeSmoothing->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_IRDenoiseEdgeSmoothing->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            this->numeric_IRDenoiseEdgeSmoothing->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 35, 0, 0, 0 });
            // 
            // chkBox_IRDenoise
            // 
            this->chkBox_IRDenoise->CheckAlign = System::Drawing::ContentAlignment::BottomCenter;
            this->chkBox_IRDenoise->Checked = true;
            this->chkBox_IRDenoise->CheckState = System::Windows::Forms::CheckState::Checked;
            this->chkBox_IRDenoise->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->chkBox_IRDenoise->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->chkBox_IRDenoise->Location = System::Drawing::Point(10, 21);
            this->chkBox_IRDenoise->Name = L"chkBox_IRDenoise";
            this->chkBox_IRDenoise->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->chkBox_IRDenoise->Size = System::Drawing::Size(65, 37);
            this->chkBox_IRDenoise->TabIndex = 46;
            this->chkBox_IRDenoise->Text = L"Enable";
            this->chkBox_IRDenoise->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
            // 
            // lbl_IRDenoise2
            // 
            this->lbl_IRDenoise2->AutoSize = true;
            this->lbl_IRDenoise2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_IRDenoise2->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->lbl_IRDenoise2->Location = System::Drawing::Point(167, 17);
            this->lbl_IRDenoise2->Name = L"lbl_IRDenoise2";
            this->lbl_IRDenoise2->Size = System::Drawing::Size(69, 13);
            this->lbl_IRDenoise2->TabIndex = 50;
            this->lbl_IRDenoise2->Text = L"Color Intrpl:";
            this->lbl_IRDenoise2->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
            // 
            // numeric_IRDenoiseColorInterpolation
            // 
            this->numeric_IRDenoiseColorInterpolation->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_IRDenoiseColorInterpolation->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_IRDenoiseColorInterpolation->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_IRDenoiseColorInterpolation->Location = System::Drawing::Point(171, 34);
            this->numeric_IRDenoiseColorInterpolation->Margin = System::Windows::Forms::Padding(0);
            this->numeric_IRDenoiseColorInterpolation->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 256, 0, 0, 0 });
            this->numeric_IRDenoiseColorInterpolation->Name = L"numeric_IRDenoiseColorInterpolation";
            this->numeric_IRDenoiseColorInterpolation->Size = System::Drawing::Size(62, 25);
            this->numeric_IRDenoiseColorInterpolation->TabIndex = 48;
            this->numeric_IRDenoiseColorInterpolation->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_IRDenoiseColorInterpolation->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            this->numeric_IRDenoiseColorInterpolation->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 68, 0, 0, 0 });
            // 
            // lbl_IRDenoise1
            // 
            this->lbl_IRDenoise1->AutoSize = true;
            this->lbl_IRDenoise1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_IRDenoise1->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->lbl_IRDenoise1->Location = System::Drawing::Point(85, 17);
            this->lbl_IRDenoise1->Name = L"lbl_IRDenoise1";
            this->lbl_IRDenoise1->Size = System::Drawing::Size(65, 13);
            this->lbl_IRDenoise1->TabIndex = 49;
            this->lbl_IRDenoise1->Text = L"Edge Smth:";
            this->lbl_IRDenoise1->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
            // 
            // lbl_IRCustomReg
            // 
            this->lbl_IRCustomReg->AutoSize = true;
            this->lbl_IRCustomReg->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_IRCustomReg->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->lbl_IRCustomReg->Location = System::Drawing::Point(273, 264);
            this->lbl_IRCustomReg->Name = L"lbl_IRCustomReg";
            this->lbl_IRCustomReg->Size = System::Drawing::Size(135, 13);
            this->lbl_IRCustomReg->TabIndex = 51;
            this->lbl_IRCustomReg->Text = L"IR Sensor Register/Value:";
            this->lbl_IRCustomReg->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
            // 
            // chkBox_IRSelfie
            // 
            this->chkBox_IRSelfie->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->chkBox_IRSelfie->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->chkBox_IRSelfie->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->chkBox_IRSelfie->Location = System::Drawing::Point(273, 238);
            this->chkBox_IRSelfie->Margin = System::Windows::Forms::Padding(0);
            this->chkBox_IRSelfie->Name = L"chkBox_IRSelfie";
            this->chkBox_IRSelfie->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->chkBox_IRSelfie->Size = System::Drawing::Size(195, 21);
            this->chkBox_IRSelfie->TabIndex = 45;
            this->chkBox_IRSelfie->Text = L"Selfie mode\r\n(flip)";
            // 
            // grpBox_editCalModel
            // 
            this->grpBox_editCalModel->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_editCalModel->Controls->Add(this->lbl_editStickDevHelp);
            this->grpBox_editCalModel->Controls->Add(this->btn_writeUserCal);
            this->grpBox_editCalModel->Controls->Add(this->btn_refreshUserCal);
            this->grpBox_editCalModel->Controls->Add(this->grpBox_CalUserAcc);
            this->grpBox_editCalModel->Controls->Add(this->grpBox_StickDevParam);
            this->grpBox_editCalModel->Controls->Add(this->grpBox_rightStickUCal);
            this->grpBox_editCalModel->Controls->Add(this->grpBox_leftStickUCal);
            this->grpBox_editCalModel->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_editCalModel->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)),
                static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_editCalModel->Location = System::Drawing::Point(14, 663);
            this->grpBox_editCalModel->Margin = System::Windows::Forms::Padding(0, 0, 14, 39);
            this->grpBox_editCalModel->Name = L"grpBox_editCalModel";
            this->grpBox_editCalModel->Padding = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->grpBox_editCalModel->Size = System::Drawing::Size(456, 389);
            this->grpBox_editCalModel->TabIndex = 52;
            this->grpBox_editCalModel->TabStop = false;
            this->grpBox_editCalModel->Text = L"User Calibration && Stick Device Parameters";
            // 
            // lbl_editStickDevHelp
            // 
            this->lbl_editStickDevHelp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_editStickDevHelp->ForeColor = System::Drawing::Color::Tomato;
            this->lbl_editStickDevHelp->Location = System::Drawing::Point(4, 336);
            this->lbl_editStickDevHelp->Name = L"lbl_editStickDevHelp";
            this->lbl_editStickDevHelp->Size = System::Drawing::Size(211, 46);
            this->lbl_editStickDevHelp->TabIndex = 53;
            this->lbl_editStickDevHelp->Text = L"Warning: The Stick Device Parameters are factory values. Change only when you hav"
                L"e drifting problems.";
            // 
            // grpBox_CalUserAcc
            // 
            this->grpBox_CalUserAcc->Controls->Add(this->lbl_CalGyroHelp);
            this->grpBox_CalUserAcc->Controls->Add(this->lbl_CalAccHelp);
            this->grpBox_CalUserAcc->Controls->Add(this->numeric_CalEditAccX);
            this->grpBox_CalUserAcc->Controls->Add(this->numeric_CalEditGyroX);
            this->grpBox_CalUserAcc->Controls->Add(this->numeric_CalEditGyroY);
            this->grpBox_CalUserAcc->Controls->Add(this->numeric_CalEditAccY);
            this->grpBox_CalUserAcc->Controls->Add(this->numeric_CalEditGyroZ);
            this->grpBox_CalUserAcc->Controls->Add(this->numeric_CalEditAccZ);
            this->grpBox_CalUserAcc->Controls->Add(this->checkBox_enableSensorUserCal);
            this->grpBox_CalUserAcc->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_CalUserAcc->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_CalUserAcc->Location = System::Drawing::Point(6, 24);
            this->grpBox_CalUserAcc->Name = L"grpBox_CalUserAcc";
            this->grpBox_CalUserAcc->Size = System::Drawing::Size(208, 155);
            this->grpBox_CalUserAcc->TabIndex = 50;
            this->grpBox_CalUserAcc->TabStop = false;
            this->grpBox_CalUserAcc->Text = L"Acc/Gyro Calibration";
            // 
            // lbl_CalGyroHelp
            // 
            this->lbl_CalGyroHelp->AutoSize = true;
            this->lbl_CalGyroHelp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_CalGyroHelp->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->lbl_CalGyroHelp->Location = System::Drawing::Point(10, 77);
            this->lbl_CalGyroHelp->Margin = System::Windows::Forms::Padding(0, 0, 3, 0);
            this->lbl_CalGyroHelp->Name = L"lbl_CalGyroHelp";
            this->lbl_CalGyroHelp->Size = System::Drawing::Size(136, 13);
            this->lbl_CalGyroHelp->TabIndex = 56;
            this->lbl_CalGyroHelp->Text = L"Gyro Bias: X / Y / Z  (Int16)";
            this->lbl_CalGyroHelp->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
            // 
            // lbl_CalAccHelp
            // 
            this->lbl_CalAccHelp->AutoSize = true;
            this->lbl_CalAccHelp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_CalAccHelp->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->lbl_CalAccHelp->Location = System::Drawing::Point(9, 26);
            this->lbl_CalAccHelp->Name = L"lbl_CalAccHelp";
            this->lbl_CalAccHelp->Size = System::Drawing::Size(159, 13);
            this->lbl_CalAccHelp->TabIndex = 49;
            this->lbl_CalAccHelp->Text = L"Acc Scaler Bias: X / Y / Z (Int16)";
            this->lbl_CalAccHelp->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
            // 
            // numeric_CalEditAccX
            // 
            this->numeric_CalEditAccX->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_CalEditAccX->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_CalEditAccX->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_CalEditAccX->Hexadecimal = true;
            this->numeric_CalEditAccX->Location = System::Drawing::Point(12, 44);
            this->numeric_CalEditAccX->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65535, 0, 0, 0 });
            this->numeric_CalEditAccX->Name = L"numeric_CalEditAccX";
            this->numeric_CalEditAccX->Size = System::Drawing::Size(53, 25);
            this->numeric_CalEditAccX->TabIndex = 43;
            this->numeric_CalEditAccX->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_CalEditAccX->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            // 
            // numeric_CalEditGyroX
            // 
            this->numeric_CalEditGyroX->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_CalEditGyroX->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_CalEditGyroX->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_CalEditGyroX->Hexadecimal = true;
            this->numeric_CalEditGyroX->Location = System::Drawing::Point(13, 95);
            this->numeric_CalEditGyroX->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65535, 0, 0, 0 });
            this->numeric_CalEditGyroX->Name = L"numeric_CalEditGyroX";
            this->numeric_CalEditGyroX->Size = System::Drawing::Size(53, 25);
            this->numeric_CalEditGyroX->TabIndex = 50;
            this->numeric_CalEditGyroX->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_CalEditGyroX->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            // 
            // numeric_CalEditGyroY
            // 
            this->numeric_CalEditGyroY->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_CalEditGyroY->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_CalEditGyroY->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_CalEditGyroY->Hexadecimal = true;
            this->numeric_CalEditGyroY->Location = System::Drawing::Point(78, 95);
            this->numeric_CalEditGyroY->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65535, 0, 0, 0 });
            this->numeric_CalEditGyroY->Name = L"numeric_CalEditGyroY";
            this->numeric_CalEditGyroY->Size = System::Drawing::Size(53, 25);
            this->numeric_CalEditGyroY->TabIndex = 51;
            this->numeric_CalEditGyroY->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_CalEditGyroY->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            // 
            // numeric_CalEditAccY
            // 
            this->numeric_CalEditAccY->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_CalEditAccY->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_CalEditAccY->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_CalEditAccY->Hexadecimal = true;
            this->numeric_CalEditAccY->Location = System::Drawing::Point(77, 44);
            this->numeric_CalEditAccY->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65535, 0, 0, 0 });
            this->numeric_CalEditAccY->Name = L"numeric_CalEditAccY";
            this->numeric_CalEditAccY->Size = System::Drawing::Size(53, 25);
            this->numeric_CalEditAccY->TabIndex = 44;
            this->numeric_CalEditAccY->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_CalEditAccY->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            // 
            // numeric_CalEditGyroZ
            // 
            this->numeric_CalEditGyroZ->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_CalEditGyroZ->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_CalEditGyroZ->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_CalEditGyroZ->Hexadecimal = true;
            this->numeric_CalEditGyroZ->Location = System::Drawing::Point(143, 95);
            this->numeric_CalEditGyroZ->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65535, 0, 0, 0 });
            this->numeric_CalEditGyroZ->Name = L"numeric_CalEditGyroZ";
            this->numeric_CalEditGyroZ->Size = System::Drawing::Size(53, 25);
            this->numeric_CalEditGyroZ->TabIndex = 52;
            this->numeric_CalEditGyroZ->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_CalEditGyroZ->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            // 
            // numeric_CalEditAccZ
            // 
            this->numeric_CalEditAccZ->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_CalEditAccZ->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_CalEditAccZ->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_CalEditAccZ->Hexadecimal = true;
            this->numeric_CalEditAccZ->Location = System::Drawing::Point(142, 44);
            this->numeric_CalEditAccZ->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65535, 0, 0, 0 });
            this->numeric_CalEditAccZ->Name = L"numeric_CalEditAccZ";
            this->numeric_CalEditAccZ->Size = System::Drawing::Size(53, 25);
            this->numeric_CalEditAccZ->TabIndex = 45;
            this->numeric_CalEditAccZ->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_CalEditAccZ->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            // 
            // checkBox_enableSensorUserCal
            // 
            this->checkBox_enableSensorUserCal->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
            this->checkBox_enableSensorUserCal->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(161)));
            this->checkBox_enableSensorUserCal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->checkBox_enableSensorUserCal->Location = System::Drawing::Point(8, 126);
            this->checkBox_enableSensorUserCal->Margin = System::Windows::Forms::Padding(0);
            this->checkBox_enableSensorUserCal->Name = L"checkBox_enableSensorUserCal";
            this->checkBox_enableSensorUserCal->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->checkBox_enableSensorUserCal->Size = System::Drawing::Size(189, 22);
            this->checkBox_enableSensorUserCal->TabIndex = 3;
            this->checkBox_enableSensorUserCal->Text = L"Enable 6-Axis Cal";
            // 
            // grpBox_StickDevParam
            // 
            this->grpBox_StickDevParam->Controls->Add(this->lbl_proStickHelp);
            this->grpBox_StickDevParam->Controls->Add(this->lbl_mainStickHelp);
            this->grpBox_StickDevParam->Controls->Add(this->numeric_StickParamRangeRatio2);
            this->grpBox_StickDevParam->Controls->Add(this->numeric_StickParamDeadzone2);
            this->grpBox_StickDevParam->Controls->Add(this->btn_writeStickParams);
            this->grpBox_StickDevParam->Controls->Add(this->lbl_rangeReatio);
            this->grpBox_StickDevParam->Controls->Add(this->numeric_StickParamRangeRatio);
            this->grpBox_StickDevParam->Controls->Add(this->lbl_deadzone);
            this->grpBox_StickDevParam->Controls->Add(this->numeric_StickParamDeadzone);
            this->grpBox_StickDevParam->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_StickDevParam->ForeColor = System::Drawing::Color::Tomato;
            this->grpBox_StickDevParam->Location = System::Drawing::Point(240, 25);
            this->grpBox_StickDevParam->Name = L"grpBox_StickDevParam";
            this->grpBox_StickDevParam->Size = System::Drawing::Size(208, 154);
            this->grpBox_StickDevParam->TabIndex = 51;
            this->grpBox_StickDevParam->TabStop = false;
            this->grpBox_StickDevParam->Text = L"Stick Device Parameters";
            // 
            // lbl_proStickHelp
            // 
            this->lbl_proStickHelp->AutoSize = true;
            this->lbl_proStickHelp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_proStickHelp->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->lbl_proStickHelp->Location = System::Drawing::Point(80, 80);
            this->lbl_proStickHelp->Name = L"lbl_proStickHelp";
            this->lbl_proStickHelp->Size = System::Drawing::Size(48, 26);
            this->lbl_proStickHelp->TabIndex = 66;
            this->lbl_proStickHelp->Text = L"Pro Con\r\nRight";
            this->lbl_proStickHelp->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
            // 
            // lbl_mainStickHelp
            // 
            this->lbl_mainStickHelp->AutoSize = true;
            this->lbl_mainStickHelp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_mainStickHelp->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->lbl_mainStickHelp->Location = System::Drawing::Point(88, 37);
            this->lbl_mainStickHelp->Name = L"lbl_mainStickHelp";
            this->lbl_mainStickHelp->Size = System::Drawing::Size(33, 26);
            this->lbl_mainStickHelp->TabIndex = 65;
            this->lbl_mainStickHelp->Text = L"Main\r\nStick";
            this->lbl_mainStickHelp->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
            // 
            // numeric_StickParamRangeRatio2
            // 
            this->numeric_StickParamRangeRatio2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_StickParamRangeRatio2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_StickParamRangeRatio2->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_StickParamRangeRatio2->Hexadecimal = true;
            this->numeric_StickParamRangeRatio2->Location = System::Drawing::Point(134, 81);
            this->numeric_StickParamRangeRatio2->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            this->numeric_StickParamRangeRatio2->Name = L"numeric_StickParamRangeRatio2";
            this->numeric_StickParamRangeRatio2->Size = System::Drawing::Size(62, 25);
            this->numeric_StickParamRangeRatio2->TabIndex = 63;
            this->numeric_StickParamRangeRatio2->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_StickParamRangeRatio2->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            // 
            // numeric_StickParamDeadzone2
            // 
            this->numeric_StickParamDeadzone2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_StickParamDeadzone2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_StickParamDeadzone2->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_StickParamDeadzone2->Hexadecimal = true;
            this->numeric_StickParamDeadzone2->Location = System::Drawing::Point(12, 81);
            this->numeric_StickParamDeadzone2->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            this->numeric_StickParamDeadzone2->Name = L"numeric_StickParamDeadzone2";
            this->numeric_StickParamDeadzone2->Size = System::Drawing::Size(62, 25);
            this->numeric_StickParamDeadzone2->TabIndex = 61;
            this->numeric_StickParamDeadzone2->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_StickParamDeadzone2->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            // 
            // btn_writeStickParams
            // 
            this->btn_writeStickParams->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_writeStickParams->Enabled = false;
            this->btn_writeStickParams->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_writeStickParams->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_writeStickParams->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btn_writeStickParams->ForeColor = System::Drawing::Color::Tomato;
            this->btn_writeStickParams->Location = System::Drawing::Point(49, 116);
            this->btn_writeStickParams->Name = L"btn_writeStickParams";
            this->btn_writeStickParams->Size = System::Drawing::Size(110, 30);
            this->btn_writeStickParams->TabIndex = 52;
            this->btn_writeStickParams->Text = L"Write Params";
            this->btn_writeStickParams->UseVisualStyleBackColor = false;
            this->btn_writeStickParams->Click += gcnew System::EventHandler(this, &FormJoy::btn_writeStickParams_Click);
            // 
            // lbl_rangeReatio
            // 
            this->lbl_rangeReatio->AutoSize = true;
            this->lbl_rangeReatio->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_rangeReatio->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->lbl_rangeReatio->Location = System::Drawing::Point(130, 21);
            this->lbl_rangeReatio->Name = L"lbl_rangeReatio";
            this->lbl_rangeReatio->Size = System::Drawing::Size(70, 13);
            this->lbl_rangeReatio->TabIndex = 60;
            this->lbl_rangeReatio->Text = L"Range ratio:";
            this->lbl_rangeReatio->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
            // 
            // numeric_StickParamRangeRatio
            // 
            this->numeric_StickParamRangeRatio->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_StickParamRangeRatio->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_StickParamRangeRatio->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_StickParamRangeRatio->Hexadecimal = true;
            this->numeric_StickParamRangeRatio->Location = System::Drawing::Point(134, 39);
            this->numeric_StickParamRangeRatio->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            this->numeric_StickParamRangeRatio->Name = L"numeric_StickParamRangeRatio";
            this->numeric_StickParamRangeRatio->Size = System::Drawing::Size(62, 25);
            this->numeric_StickParamRangeRatio->TabIndex = 59;
            this->numeric_StickParamRangeRatio->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_StickParamRangeRatio->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            // 
            // lbl_deadzone
            // 
            this->lbl_deadzone->AutoSize = true;
            this->lbl_deadzone->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_deadzone->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->lbl_deadzone->Location = System::Drawing::Point(8, 21);
            this->lbl_deadzone->Name = L"lbl_deadzone";
            this->lbl_deadzone->Size = System::Drawing::Size(62, 13);
            this->lbl_deadzone->TabIndex = 58;
            this->lbl_deadzone->Text = L"Deadzone:";
            this->lbl_deadzone->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
            // 
            // numeric_StickParamDeadzone
            // 
            this->numeric_StickParamDeadzone->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->numeric_StickParamDeadzone->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
            this->numeric_StickParamDeadzone->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->numeric_StickParamDeadzone->Hexadecimal = true;
            this->numeric_StickParamDeadzone->Location = System::Drawing::Point(12, 39);
            this->numeric_StickParamDeadzone->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4095, 0, 0, 0 });
            this->numeric_StickParamDeadzone->Name = L"numeric_StickParamDeadzone";
            this->numeric_StickParamDeadzone->Size = System::Drawing::Size(62, 25);
            this->numeric_StickParamDeadzone->TabIndex = 57;
            this->numeric_StickParamDeadzone->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            this->numeric_StickParamDeadzone->UpDownAlign = System::Windows::Forms::LeftRightAlignment::Left;
            // 
            // FormJoy
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Dpi;
            this->AutoSize = true;
            this->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
            this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->ClientSize = System::Drawing::Size(1925, 1090);
            this->Controls->Add(this->grpBox_editCalModel);
            this->Controls->Add(this->grpBox_IRSettings);
            this->Controls->Add(this->grpBox_nfc);
            this->Controls->Add(this->grpBox_IR);
            this->Controls->Add(this->menuStrip1);
            this->Controls->Add(this->toolStrip1);
            this->Controls->Add(this->panel_filler);
            this->Controls->Add(this->grpBox_dev_param);
            this->Controls->Add(this->grpBox_StickCal);
            this->Controls->Add(this->grpBox_accGyroCal);
            this->Controls->Add(this->grpBox_ButtonTest);
            this->Controls->Add(this->btn_enableExpertMode);
            this->Controls->Add(this->grpBox_VibPlayer);
            this->Controls->Add(this->grpBox_ChangeSN);
            this->Controls->Add(this->grpBox_Restore);
            this->Controls->Add(this->grpBox_DebugCmd);
            this->Controls->Add(this->grpBox_Color);
            this->Controls->Add(this->textBoxDev);
            this->Controls->Add(this->label_dev);
            this->Controls->Add(this->textBoxFW);
            this->Controls->Add(this->grpBox_SPI);
            this->Controls->Add(this->textBoxMAC);
            this->Controls->Add(this->label_fw);
            this->Controls->Add(this->label_mac);
            this->Controls->Add(this->textBoxSN);
            this->Controls->Add(this->label_sn);
            this->DoubleBuffered = true;
            this->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
            this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
            this->MainMenuStrip = this->menuStrip1;
            this->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->MaximizeBox = false;
            this->Name = L"FormJoy";
            this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
            this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
            this->Text = L"Joy-Con Toolkit v5.2.0";
            this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &FormJoy::Form1_FormClosing);
            this->SizeChanged += gcnew System::EventHandler(this, &FormJoy::fixToolstripOverlap);
            this->grpBox_Color->ResumeLayout(false);
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxPreview))->EndInit();
            this->grpBox_SPI->ResumeLayout(false);
            this->menuStrip1->ResumeLayout(false);
            this->menuStrip1->PerformLayout();
            this->grpBox_DebugCmd->ResumeLayout(false);
            this->grpBox_DebugCmd->PerformLayout();
            this->grpBox_Restore->ResumeLayout(false);
            this->grpBox_Restore->PerformLayout();
            this->grpBox_RstUser->ResumeLayout(false);
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->errorProvider1))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->errorProvider2))->EndInit();
            this->grpBox_ChangeSN->ResumeLayout(false);
            this->grpBox_ChangeSN->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_IRExposure))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_IRGain))->EndInit();
            this->grpBox_VibPlayer->ResumeLayout(false);
            this->groupBox_vib_info->ResumeLayout(false);
            this->groupBox_vib_info->PerformLayout();
            this->groupBox_vib_eq->ResumeLayout(false);
            this->groupBox_vib_eq->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_hf_amp))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_lf_amp))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_hf_freq))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_lf_freq))->EndInit();
            this->grpBox_ButtonTest->ResumeLayout(false);
            this->grpBox_ButtonTest->PerformLayout();
            this->grpBox_StickCal->ResumeLayout(false);
            this->grpBox_StickCal->PerformLayout();
            this->grpBox_dev_param->ResumeLayout(false);
            this->grpBox_dev_param->PerformLayout();
            this->toolStrip1->ResumeLayout(false);
            this->toolStrip1->PerformLayout();
            this->grpBox_accGyroCal->ResumeLayout(false);
            this->grpBox_accGyroCal->PerformLayout();
            this->grpBox_IR->ResumeLayout(false);
            this->grpBox_IR->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxIR))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_IRCustomRegVal))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_IRCustomRegAddr))->EndInit();
            this->grpBox_IRColorize->ResumeLayout(false);
            this->grpBox_IRColorize->PerformLayout();
            this->grpBox_IRRes->ResumeLayout(false);
            this->grpBox_IRRes->PerformLayout();
            this->grpBox_nfc->ResumeLayout(false);
            this->grpBox_nfc->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_leftUserCal_y_plus))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_leftUserCal_y_center))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_leftUserCal_y_minus))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_leftUserCal_x_plus))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_leftUserCal_x_center))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_leftUserCal_x_minus))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_rightUserCal_y_plus))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_rightUserCal_y_center))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_rightUserCal_y_minus))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_rightUserCal_x_plus))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_rightUserCal_x_center))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_rightUserCal_x_minus))->EndInit();
            this->grpBox_leftStickUCal->ResumeLayout(false);
            this->grpBox_leftStickUCal->PerformLayout();
            this->grpBox_rightStickUCal->ResumeLayout(false);
            this->grpBox_rightStickUCal->PerformLayout();
            this->grpBox_IRSettings->ResumeLayout(false);
            this->grpBox_IRSettings->PerformLayout();
            this->grpBox_IRInfraredLight->ResumeLayout(false);
            this->grpBox_IRInfraredLight->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_IRBrightLeds))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_IRDimLeds))->EndInit();
            this->grpBox_IRSettingsDenoise->ResumeLayout(false);
            this->grpBox_IRSettingsDenoise->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_IRDenoiseEdgeSmoothing))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_IRDenoiseColorInterpolation))->EndInit();
            this->grpBox_editCalModel->ResumeLayout(false);
            this->grpBox_CalUserAcc->ResumeLayout(false);
            this->grpBox_CalUserAcc->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_CalEditAccX))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_CalEditGyroX))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_CalEditGyroY))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_CalEditAccY))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_CalEditGyroZ))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_CalEditAccZ))->EndInit();
            this->grpBox_StickDevParam->ResumeLayout(false);
            this->grpBox_StickDevParam->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_StickParamRangeRatio2))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_StickParamDeadzone2))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_StickParamRangeRatio))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numeric_StickParamDeadzone))->EndInit();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion

    //////////////
    // Functions
    //////////////

    private: System::Void full_refresh(bool check_connection) {
        if (check_connection) {
            if (check_if_connected())
                return;

            this->toolStripBtn_Disconnect->Enabled = true;
            this->toolStripLabel_temp->Enabled     = true;
            this->toolStripLabel_batt->Enabled     = true;
            this->toolStripBtn_batt->Enabled       = true;
        }

        this->btn_runBtnTest->Text = L"Turn on";
        enable_button_test = false;

        if (handle_ok != 3) {
            this->textBoxSN->Text = gcnew String(get_sn(0x6001, 0xF).c_str());
            this->textBox_chg_sn->Text = this->textBoxSN->Text;

            this->btn_changeGripsColor->Enabled = false;
        }
        else {
            this->textBoxSN->Text = L"Not supported";
            this->textBox_chg_sn->Text = this->textBoxSN->Text;

            this->btn_changeGripsColor->Enabled = true;
        }

        if (handle_ok != 1) {
            if (handle_ok == 2)
                this->iRCameraToolStripMenuItem->Enabled = true;  // JC (R)
            else
                this->iRCameraToolStripMenuItem->Enabled = false; // Pro con
            this->grpBox_nfc->Enabled = true;
        }
        else {
            this->iRCameraToolStripMenuItem->Enabled = false; // JC (L)
            this->grpBox_nfc->Enabled = false;
        }

        unsigned char device_info[10];
        memset(device_info, 0, sizeof(device_info));

        get_device_info(device_info);

        this->textBoxFW->Text = String::Format("{0:X}.{1:X2}", device_info[0], device_info[1]);
        this->textBoxMAC->Text = String::Format("{0:X2}:{1:X2}:{2:X2}:{3:X2}:{4:X2}:{5:X2}",
            device_info[4], device_info[5], device_info[6], device_info[7], device_info[8], device_info[9]);

        if (handle_ok == 1)
            this->textBoxDev->Text = L"Joy-Con (L)";
        else if (handle_ok == 2)
            this->textBoxDev->Text = L"Joy-Con (R)";
        else if (handle_ok == 3)
            this->textBoxDev->Text = L"Pro Controller";

        update_battery();
        update_temperature();
        update_colors_from_spi(!check_connection);
    }

    private: System::Void btn_writeColorsToSpi_Click(System::Object^ sender, System::EventArgs^ e) {
        if (check_if_connected())
            return;

        set_led_busy();
        if (MessageBox::Show(L"Don't forget to make a backup first!\n" +
            L"You can also find retail colors at the bottom of the colors dialog for each type.\n\n" +
            L"Are you sure you want to continue?",
            L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
        {
            int error = 0;
            this->btn_writeColorsToSpi->Enabled = false;

            unsigned char newColors[12];
            memset(newColors, 0, 12);

            newColors[0]  = (u8)jcBodyColor.R;
            newColors[1]  = (u8)jcBodyColor.G;
            newColors[2]  = (u8)jcBodyColor.B;
            newColors[3]  = (u8)jcButtonsColor.R;
            newColors[4]  = (u8)jcButtonsColor.G;
            newColors[5]  = (u8)jcButtonsColor.B;
            newColors[6]  = (u8)jcGripLeftColor.R;
            newColors[7]  = (u8)jcGripLeftColor.G;
            newColors[8]  = (u8)jcGripLeftColor.B;
            newColors[9]  = (u8)jcGripRightColor.R;
            newColors[10] = (u8)jcGripRightColor.G;
            newColors[11] = (u8)jcGripRightColor.B;

            if (handle_ok != 3)
                error = write_spi_data(0x6050, 6, newColors);
            else
                error = write_spi_data(0x6050, 12, newColors);

            send_rumble();

            if (error == 0) {
                MessageBox::Show(L"The colors were written to the device!", L"Done!",
                    MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
                //Check that the colors were written
                update_colors_from_spi(false);
            }
            else {
                MessageBox::Show(L"Failed to write the colors to the device!", L"Failed!",
                    MessageBoxButtons::OK, MessageBoxIcon::Stop);
            }

            update_battery();
            update_temperature();
        }
    }

    private: System::Void btn_makeSPIBackup_Click(System::Object^  sender, System::EventArgs^  e) {
        if (check_if_connected())
            return;

        this->menuStrip1->Enabled = false;
        this->toolStrip1->Enabled = false;

        int do_backup = 0;
        unsigned char device_info[10];
        memset(device_info, 0, sizeof(device_info));
        get_device_info(device_info);

        String^ filename = L"spi_";
        if (handle_ok == 1)
            filename += "left_";
        else if (handle_ok == 2)
            filename += "right_";
        else if (handle_ok == 3)
            filename += "pro_";

        filename += String::Format("{0:X2}{1:X2}{2:X2}{3:X2}{4:X2}{5:X2}",
            device_info[4], device_info[5], device_info[6], device_info[7], device_info[8], device_info[9]);
        filename += L".bin";

        String^ path = Path::Combine(Path::GetDirectoryName(Application::ExecutablePath), filename);
        if (File::Exists(path))
        {
            if (MessageBox::Show(L"The file " + filename + L" already exists!\n\nDo you want to overwrite it?",
                L"Warning", MessageBoxButtons::YesNo, MessageBoxIcon::Warning, 
                MessageBoxDefaultButton::Button2) ==System::Windows::Forms::DialogResult::Yes)
            {
                do_backup = 1;
            }
        }
        else {
            do_backup = 1;
        }

        if (do_backup) {
            msclr::interop::marshal_context context;
            handler_close = 1;
            this->grpBox_Color->Visible = false;
            reset_window_option(true);
            Application::DoEvents();
            send_rumble();
            set_led_busy();
            cancel_spi_dump = false;

            int error = dump_spi((context.marshal_as<std::string>(filename)).c_str());

            this->grpBox_Color->Visible = true;
            handler_close = 0;
            if (!error && !cancel_spi_dump) {
                send_rumble();
                MessageBox::Show(L"Done dumping SPI!", L"SPI Dumping", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
            }
            else if (error && !cancel_spi_dump)
                MessageBox::Show(L"Failed to dump the SPI chip!", L"SPI Dumping Failed!", MessageBoxButtons::OK, MessageBoxIcon::Stop);

        }
        this->menuStrip1->Enabled = true;
        this->toolStrip1->Enabled = true;
        update_battery();
        update_temperature();
    }

    private: System::Void update_joycon_color(u8 r, u8 g, u8 b, u8 rb, u8 gb, u8 bb, u8 rgl, u8 ggl, u8 bgl, u8 rgr, u8 ggr, u8 bgr) {
        Bitmap^ MyImage;
        Bitmap^ MyImageLayer;
        Bitmap^ MyImageLayer2;

        // Apply body color 
        switch (handle_ok) {
        case 1:
            MyImage = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"base64_l_joy_body")));
            break;
        case 2:
            MyImage = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"base64_r_joy_body")));
            break;
        case 3:
            MyImage = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"base64_pro_body")));
            MyImageLayer = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"base64_pro_grips_l")));
            MyImageLayer2 = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"base64_pro_grips_r")));
            break;
        default:
            MyImage = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"base64_pro_body")));
            break;
        }
        // Skip slow SetPixel(). Reduce latency pixel set latency from 842us -> 260ns.
        System::Drawing::Imaging::BitmapData^ bmd = MyImage->LockBits(System::Drawing::Rectangle(0, 0, MyImage->Width, MyImage->Height), System::Drawing::Imaging::ImageLockMode::ReadOnly, MyImage->PixelFormat);
        int PixelSize = 4;
        for (int y = 0; y < MyImage->Height; y++) {
            byte* row = (byte *)bmd->Scan0.ToPointer() + (y * bmd->Stride);
            for (int x = 0; x < MyImage->Width; x++) {
                // Values are in BGRA in memory. Here in ARGB order.
                //row[x * PixelSize + 3] = alpha;
                row[x * PixelSize + 2] = r;
                row[x * PixelSize + 1] = g;
                row[x * PixelSize]     = b;
            }
        }
        MyImage->UnlockBits(bmd);
        delete bmd;

        // Apply grips color
        if (handle_ok == 3) {
            // Skip slow SetPixel(). Reduce latency pixel set latency from 842us -> 260ns.
            bmd = MyImageLayer->LockBits(System::Drawing::Rectangle(0, 0, MyImageLayer->Width, MyImageLayer->Height), System::Drawing::Imaging::ImageLockMode::ReadOnly, MyImageLayer->PixelFormat);
            System::Drawing::Imaging::BitmapData^ bmd2 = MyImageLayer2->LockBits(System::Drawing::Rectangle(0, 0, MyImageLayer2->Width, MyImageLayer2->Height), System::Drawing::Imaging::ImageLockMode::ReadOnly, MyImageLayer2->PixelFormat);
            for (int y = 0; y < MyImageLayer->Height; y++) {
            byte* row = (byte *)bmd->Scan0.ToPointer() + (y * bmd->Stride);
            byte* row2 = (byte *)bmd2->Scan0.ToPointer() + (y * bmd2->Stride);
            for (int x = 0; x < MyImageLayer->Width; x++) {
                    // White Buttons
                    if (Color::FromArgb(255, 255, 255) == Color::FromArgb(rb, gb, bb)) {
                        // Normal Pro
                        if (Color::FromArgb(0x32, 0x32, 0x32) == Color::FromArgb(r, g, b)) {
                            row[x * PixelSize + 2]  = 0x46;
                            row[x * PixelSize + 1]  = 0x46;
                            row[x * PixelSize]      = 0x46;
                            row2[x * PixelSize + 2] = 0x46;
                            row2[x * PixelSize + 1] = 0x46;
                            row2[x * PixelSize]     = 0x46;
                        }
                        // Xenoblade Pro
                        else if (Color::FromArgb(0x32, 0x31, 0x32) == Color::FromArgb(r, g, b)) {
                            row[x * PixelSize + 2]  = 0xdd;
                            row[x * PixelSize + 1]  = 0x3b;
                            row[x * PixelSize]      = 0x64;
                            row2[x * PixelSize + 2] = 0xdd;
                            row2[x * PixelSize + 1] = 0x3b;
                            row2[x * PixelSize]     = 0x64;
                        }
                        // Splatoon Pro
                        else if (Color::FromArgb(0x31, 0x32, 0x32) == Color::FromArgb(r, g, b)) {
                            row[x * PixelSize + 2]  = 0x1e;
                            row[x * PixelSize + 1]  = 0xdc;
                            row[x * PixelSize]      = 0x00;
                            row2[x * PixelSize + 2] = 0xff;
                            row2[x * PixelSize + 1] = 0x32;
                            row2[x * PixelSize]     = 0x78;
                        }
                        // Custom Pro. Body is not one of the 3 retail colors. Apply the Left Grip/Right Grip Colors from SPI.
                        else {
                            row[x * PixelSize + 2]  = rgl;
                            row[x * PixelSize + 1]  = ggl;
                            row[x * PixelSize]      = bgl;
                            row2[x * PixelSize + 2] = rgr;
                            row2[x * PixelSize + 1] = ggr;
                            row2[x * PixelSize]     = bgr;
                        }
                    }
                    // Custom Grips. Buttons are not white. Apply the Left Grip/Right Grip Colors from SPI.
                    else {
                        row[x * PixelSize + 2]  = rgl;
                        row[x * PixelSize + 1]  = ggl;
                        row[x * PixelSize]      = bgl;
                        row2[x * PixelSize + 2] = rgr;
                        row2[x * PixelSize + 1] = ggr;
                        row2[x * PixelSize]     = bgr;
                    }
                }
            }
            MyImageLayer->UnlockBits(bmd);
            MyImageLayer2->UnlockBits(bmd2);
            delete bmd;
            delete bmd2;
            MyImageLayer = drawLayeredImage(MyImageLayer, MyImageLayer2);
        }

        // Apply buttons color 
        switch (handle_ok) {
        case 1:
            MyImageLayer = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"base64_l_joy_buttons")));
            break;
        case 2:
            MyImageLayer = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"base64_r_joy_buttons")));
            break;
        case 3:
            MyImage = drawLayeredImage(MyImage, MyImageLayer); // Apply grips layer
            MyImageLayer = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"base64_pro_buttons")));
            break;
        default:
            MyImageLayer = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"base64_pro_buttons")));
            break;
        }
        // Skip slow SetPixel(). Reduce latency pixel set latency from 842us -> 260ns.
        bmd = MyImageLayer->LockBits(System::Drawing::Rectangle(0, 0, MyImageLayer->Width, MyImageLayer->Height), System::Drawing::Imaging::ImageLockMode::ReadOnly, MyImageLayer->PixelFormat);
        for (int y = 0; y < MyImageLayer->Height; y++) {
            byte* row = (byte *)bmd->Scan0.ToPointer() + (y * bmd->Stride);
            for (int x = 0; x < MyImageLayer->Width; x++) {
                row[x * PixelSize + 2] = rb;
                row[x * PixelSize + 1] = gb;
                row[x * PixelSize]     = bb;
            }
        }
        MyImageLayer->UnlockBits(bmd);
        delete bmd;
        MyImage = drawLayeredImage(MyImage, MyImageLayer);

        // Apply outlines
        switch (handle_ok) {
        case 1:
            MyImageLayer = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"base64_l_joy_lines")));
            break;
        case 2:
            MyImageLayer = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"base64_r_joy_lines")));
            break;
        case 3:
            MyImageLayer = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"base64_pro_lines")));
            break;
        default:
            MyImageLayer = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"base64_pro_lines")));
            break;
        }
        MyImage = drawLayeredImage(MyImage, MyImageLayer);

        // Draw image
        this->pictureBoxPreview->Image = dynamic_cast<Image^>(MyImage);
        this->pictureBoxPreview->ClientSize = System::Drawing::Size(312, 192);
        this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
    }

    private: Bitmap^ drawLayeredImage(Bitmap^ baseImage, Bitmap^ layerImage) {
        Graphics^ g = System::Drawing::Graphics::FromImage(baseImage);
        g->CompositingMode = System::Drawing::Drawing2D::CompositingMode::SourceOver;
        g->DrawImage(layerImage, 0, 0);
        return baseImage;
    }

    private: System::Void update_colors_from_spi(bool update_color_dialog) {
        unsigned char spiColors[12];
        memset(spiColors, 0, 12);

        int res = get_spi_data(0x6050, 12, spiColors);

        update_joycon_color(
            (u8)spiColors[0], (u8)spiColors[1], (u8)spiColors[2],    // Body Colors
            (u8)spiColors[3], (u8)spiColors[4], (u8)spiColors[5],    // Button Colors
            (u8)spiColors[6], (u8)spiColors[7], (u8)spiColors[8],    // Left Grip Colors (Pro Controller, Switch Update 5.0.0+)
            (u8)spiColors[9], (u8)spiColors[10], (u8)spiColors[11]); // Right Grip Colors (Pro Controller, Switch Update 5.0.0+)

        if (update_color_dialog) {
            this->jcBodyColor = Color::FromArgb(0xFF, (u8)spiColors[0], (u8)spiColors[1], (u8)spiColors[2]);
            this->lbl_Body_hex_txt->Text = L"Body: #" + String::Format("{0:X6}",
                ((u8)spiColors[0] << 16) + ((u8)spiColors[1] << 8) + ((u8)spiColors[2]));
            this->lbl_Body_hex_txt->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_Body_hex_txt->Size = System::Drawing::Size(128, 24);

            this->jcButtonsColor = Color::FromArgb(0xFF, (u8)spiColors[3], (u8)spiColors[4], (u8)spiColors[5]);
            this->lbl_Buttons_hex_txt->Text = L"Buttons: #" + String::Format("{0:X6}",
                ((u8)spiColors[3] << 16) + ((u8)spiColors[4] << 8) + ((u8)spiColors[5]));
            this->lbl_Buttons_hex_txt->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_Buttons_hex_txt->Size = System::Drawing::Size(128, 24);

            this->jcGripLeftColor = Color::FromArgb(0xFF, (u8)spiColors[6], (u8)spiColors[7], (u8)spiColors[8]);
            this->jcGripRightColor = Color::FromArgb(0xFF, (u8)spiColors[9], (u8)spiColors[10], (u8)spiColors[11]);
        }

        if (res) {
            this->lbl_Body_hex_txt->Text = "Error!";
            this->lbl_Buttons_hex_txt->Text = "Error!";
        }
    }

    private: System::Void update_battery() {
        unsigned char batt_info[3];
        memset(batt_info, 0, sizeof(batt_info));

        get_battery(batt_info);

        BatteryData battery_data = parseBatteryData(batt_info);

        this->toolStripLabel_batt->Text = String::Format(" {0:f2}V - {1:D}%", battery_data.voltage, battery_data
.percent);

        // Update Battery icon from input report value.
        switch (battery_data.report) {
            case 0:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"batt_0")));
                this->toolStripBtn_batt->ToolTipText = L"Empty\n\nDisconnected?";
                break;
            case 1:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"batt_0_chr")));
                this->toolStripBtn_batt->ToolTipText = L"Empty, Charging.";
                break;
            case 2:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"batt_25")));
                this->toolStripBtn_batt->ToolTipText = L"Low\n\nPlease charge your device!";
                break;
            case 3:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"batt_25_chr")));
                this->toolStripBtn_batt->ToolTipText = L"Low\n\nCharging";
                break;
            case 4:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"batt_50")));
                this->toolStripBtn_batt->ToolTipText = L"Medium";
                break;
            case 5:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"batt_50_chr")));
                this->toolStripBtn_batt->ToolTipText = L"Medium\n\nCharging";
                break;
            case 6:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"batt_75")));
                this->toolStripBtn_batt->ToolTipText = L"Good";
                break;
            case 7:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"batt_75_chr")));
                this->toolStripBtn_batt->ToolTipText = L"Good\n\nCharging";
                break;
            case 8:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"batt_100")));
                this->toolStripBtn_batt->ToolTipText = L"Full";
                break;
            case 9:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"batt_100_chr")));
                this->toolStripBtn_batt->ToolTipText = L"Almost full\n\nCharging";
                break;
        }
    
    }

    private: System::Void update_temperature() {
        unsigned char temp_info[2];
        memset(temp_info, 0, sizeof(temp_info));
        get_temperature(temp_info);

        TemperatureData temp_data = parseTemperatureData(temp_info);

        if (temp_celsius)
            this->toolStripLabel_temp->Text = String::Format(L"{0:f1}\u2103 ", temp_data.celsius);
        else
            this->toolStripLabel_temp->Text = String::Format(L"{0:f1}\u2109 ", temp_data.farenheight);
    }

    private: System::Void Form1_FormClosing(System::Object^  sender, FormClosingEventArgs^ e)
    {
        //check if spi dumping in progress
        if (handler_close == 1) {
            if (MessageBox::Show(L"SPI dumping in process!\n\nDo you really want to exit?", L"Warning!",
                MessageBoxButtons::YesNo, MessageBoxIcon::Stop) == System::Windows::Forms::DialogResult::Yes) {
                Environment::Exit(0);
            }
            else {
                e->Cancel = true;
            }
        }
        else if (handler_close == 2) {
            if (MessageBox::Show(L"Full restore in process!\n\nDo you really want to exit?", L"Warning!",
                MessageBoxButtons::YesNo, MessageBoxIcon::Stop) == System::Windows::Forms::DialogResult::Yes) {
                Environment::Exit(0);
            }
            else {
                e->Cancel = true;
            }
        }
        else if (handler_close == 0) {
            Environment::Exit(0);
        }
    }

    private: System::Void aboutToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
        if (MessageBox::Show(L"CTCaer's " + this->Text + L"\n\nDo you want to go to the official forum and check for the latest version?\n\n",
            L"About", MessageBoxButtons::YesNo, MessageBoxIcon::Asterisk) == System::Windows::Forms::DialogResult::Yes)
        {
            System::Diagnostics::Process::Start("http://gbatemp.net/threads/tool-joy-con-toolkit-v1-0.478560/");
        }
    }

    private: System::Void debugToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
        if (option_is_on != 1) {
            reset_window_option(false);
            this->Controls->Add(this->grpBox_DebugCmd);
            this->Controls->Add(this->btn_enableExpertMode);
            // Recalculate buggy textboxes for high DPI
            this->textBoxDbg_SubcmdArg->Size = System::Drawing::Size(188, 44);
            this->textBoxDbg_sent->Size      = System::Drawing::Size(170, 47);
            this->textBoxDbg_reply_cmd->Size = System::Drawing::Size(170, 45);
            this->textBoxDbg_reply->Size     = System::Drawing::Size(170, 69);
            option_is_on = 1;
            this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
        }
        else
            reset_window_option(true);
    }

    private: System::Void btn_enableExpertMode_Click(System::Object^  sender, System::EventArgs^  e) {
        disable_expert_mode = false;
        this->grpBox_DebugCmd->Text = L"Debug: Expert Mode";
    }

    private: System::Void btn_RestoreEnable_Click(System::Object^  sender, System::EventArgs^  e) {
        if (option_is_on != 2) {
            reset_window_option(false);
            this->Controls->Add(this->grpBox_Restore);
            option_is_on = 2;
            this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
        }
        else
            reset_window_option(true);
    }

    private: System::Void label_sn_Click(System::Object^  sender, System::EventArgs^  e) {
        if (option_is_on != 3) {
            reset_window_option(false);
            this->Controls->Add(this->grpBox_ChangeSN);
            option_is_on = 3;
            this->textBox_chg_sn->Size = System::Drawing::Size(186, 25);
            this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
        }
        else
            reset_window_option(true);
    }

    private: System::Void btnPlayVibEnable_Click(System::Object^  sender, System::EventArgs^  e) {
        if (option_is_on != 4) {
            reset_window_option(false);
            this->Controls->Add(this->grpBox_VibPlayer);
            option_is_on = 4;
            this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
        }
        else
            reset_window_option(true);
    }

    private: System::Void buttonTestToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
        if (option_is_on != 5) {
            reset_window_option(false);
            this->Controls->Add(this->grpBox_ButtonTest);
            this->Controls->Add(this->grpBox_StickCal);
            this->Controls->Add(this->grpBox_dev_param);
            this->Controls->Add(this->grpBox_accGyroCal);
            this->Controls->Add(this->grpBox_nfc);
            // Recalculate buggy textboxes for high DPI
            this->lbl_nfcHelp->Size   = System::Drawing::Size(203, 85);
            this->txtBox_nfcUid->Size = System::Drawing::Size(208, 53);
            this->textBox_lstick_fcal->Size   = System::Drawing::Size(207, 44);
            this->textBox_lstick_ucal->Size   = System::Drawing::Size(207, 44);
            this->textBox_rstick_fcal->Size   = System::Drawing::Size(207, 44);
            this->textBox_rstick_ucal->Size   = System::Drawing::Size(207, 44);
            this->textBox_6axis_cal->Size     = System::Drawing::Size(156, 88);
            this->textBox_6axis_ucal->Size    = System::Drawing::Size(156, 88);
            this->txtBox_devParameters->Size  = System::Drawing::Size(135, 185);
            this->txtBox_devParameters2->Size = System::Drawing::Size(140, 130);
            this->textBox_btn_test_reply->Size    = System::Drawing::Size(205, 172);
            this->textBox_btn_test_subreply->Size = System::Drawing::Size(205, 140);

            this->btn_runBtnTest->Text = L"Turn on";
            option_is_on = 5;
            this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
        }
        else {
            reset_window_option(true);
            this->btn_runBtnTest->Text = L"Turn on";
        }
    }

    private: System::Void iRCameraToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
        if (option_is_on != 6) {
            reset_window_option(false);
            this->Controls->Add(this->grpBox_IR);
            this->Controls->Add(this->grpBox_IRSettings);
            this->grpBox_IRSettings->BringToFront();
            this->btn_getIRStream->Text = L"Stream";
            option_is_on = 6;
            this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
        }
        else {
            reset_window_option(true);
            this->btn_getIRStream->Text = L"Stream";
        }
    }

    private: System::Void editCalibrationToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
        if (option_is_on != 7) {
            reset_window_option(false);
            this->Controls->Add(this->grpBox_editCalModel);
            this->lbl_editStickDevHelp->Size = System::Drawing::Size(211, 46);
            this->grpBox_editCalModel->BringToFront();
            option_is_on = 7;
            this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
        }
        else
            reset_window_option(true);
    }

    private: System::Void reset_window_option(bool reset_all) {
        if (!check_connection_ok) {
            this->toolStripBtn_refresh->Enabled    = false;
            this->toolStripBtn_Disconnect->Enabled = false;
        }
        enable_button_test  = false;
        enable_IRVideoPhoto = false;
        enable_NFCScanning  = false;
        this->Controls->Remove(this->grpBox_DebugCmd);
        this->Controls->Remove(this->grpBox_Restore);
        this->Controls->Remove(this->grpBox_ChangeSN);
        this->Controls->Remove(this->grpBox_VibPlayer);
        this->Controls->Remove(this->grpBox_ButtonTest);
        this->Controls->Remove(this->grpBox_IR);
        this->Controls->Remove(this->grpBox_IRSettings);
        this->Controls->Remove(this->grpBox_editCalModel);

        this->Controls->Remove(this->grpBox_nfc);
        this->Controls->Remove(this->btn_enableExpertMode);
        this->Controls->Remove(this->grpBox_StickCal);
        this->Controls->Remove(this->grpBox_dev_param);
        this->Controls->Remove(this->grpBox_accGyroCal);

        this->textBoxDbg_sent->Visible      = false;
        this->textBoxDbg_reply->Visible     = false;
        this->textBoxDbg_reply_cmd->Visible = false;

        grpBox_leftStickUCal->Enabled  = false;
        grpBox_rightStickUCal->Enabled = false;
        grpBox_CalUserAcc->Enabled     = false;
        grpBox_StickDevParam->Enabled  = false;

        this->menuStrip1->Refresh();
        this->toolStrip1->Refresh();

        if (reset_all) {
            option_is_on = 0;
        }
    }

    private: System::Void btn_dbgSendCmd_Click(System::Object^  sender, System::EventArgs^  e) {
        if (check_if_connected())
            return;


        msclr::interop::marshal_context context;
        unsigned char test[44];
        memset(test, 0, sizeof(test));

        std::stringstream ss_cmd;
        std::stringstream ss_hfreq;
        std::stringstream ss_hamp;
        std::stringstream ss_lfreq;
        std::stringstream ss_lamp;
        std::stringstream ss_subcmd;
        std::stringstream ss_arguments;
        int i_cmd;
        int i_hfreq;
        int i_hamp;
        int i_lfreq;
        int i_lamp;
        int i_subcmd;
        char i_getarg;

        ss_cmd    << std::hex << context.marshal_as<std::string>(this->textBoxDbg_cmd->Text);
        ss_hfreq  << std::hex << context.marshal_as<std::string>(this->textBoxDbg_hfreq->Text);
        ss_hamp   << std::hex << context.marshal_as<std::string>(this->textBoxDbg_hamp->Text);
        ss_lfreq  << std::hex << context.marshal_as<std::string>(this->textBoxDbg_lfreq->Text);
        ss_lamp   << std::hex << context.marshal_as<std::string>(this->textBoxDbg_lfamp->Text);
        ss_subcmd << std::hex << context.marshal_as<std::string>(this->textBoxDbg_subcmd->Text);
        
        ss_cmd    >> i_cmd;
        ss_hfreq  >> i_hfreq;
        ss_hamp   >> i_hamp;
        ss_lfreq  >> i_lfreq;
        ss_lamp   >> i_lamp;
        ss_subcmd >> i_subcmd;
    
        test[0] = i_cmd;
        test[1] = i_hfreq;
        test[2] = i_hamp;
        test[3] = i_lfreq;
        test[4] = i_lamp;
        test[5] = i_subcmd;

        u8 get_i =0;
        ss_arguments << std::hex << context.marshal_as<std::string>(this->textBoxDbg_SubcmdArg->Text);

        //Get Low nibble if odd number of characters
        if (ss_arguments.tellp() % 2 ){
            ss_arguments.get(i_getarg);
            if (i_getarg >= 'A' && i_getarg <= 'F')
                test[6] = (i_getarg - '7') & 0xF;
            else if (i_getarg <= '9' && i_getarg >= '0')
                test[6] = (i_getarg - '0') & 0xF;
            get_i++;
        }

        while (ss_arguments.get(i_getarg)) {
            //Get High nibble
            if (i_getarg >= 'A' && i_getarg <= 'F')
                test[6 + get_i] = ((i_getarg - '7') << 4) & 0xF0;
            else if (i_getarg <= '9' && i_getarg > '0')
                test[6 + get_i] = ((i_getarg - '0') << 4) &0xF0;
            //Get Low nibble
            ss_arguments.get(i_getarg);
                if (i_getarg >= 'A' && i_getarg <= 'F')
                    test[6 + get_i] += (i_getarg - '7') & 0xF;
                else if (i_getarg <= '9' && i_getarg >= '0')
                    test[6 + get_i] += (i_getarg - '0') & 0xF;
            get_i++;
    
        }

        send_custom_command(test);
        this->textBoxDbg_sent->Visible      = true;
        this->textBoxDbg_reply->Visible     = true;
        this->textBoxDbg_reply_cmd->Visible = true;

        if (test[5] != 0x06) {
            update_battery();
            update_temperature();
        }
    }

    private: System::Void btn_loadSPIBackup_Click(System::Object^  sender, System::EventArgs^  e) {
        bool validation_check = true;
        bool mac_check = true;
        allow_full_restore = true;
        bool ota_exists = true;
        //Bootloader, device type, FW DS1, FW DS2
        array<byte>^ validation_magic = { 
            0x01, 0x08, 0x00, 0xF0, 0x00, 0x00, 0x62, 0x08, 0xC0, 0x5D, 0x89, 0xFD, 0x04, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x06,
            (u8)handle_ok, 0xA0,
            0x0A, 0xFB, 0x00, 0x00, 0x02, 0x0D,
            0xAA, 0x55, 0xF0, 0x0F, 0x68, 0xE5, 0x97, 0xD2 };
        Stream^ fileStream;
        String^ str_dev_type;
        String^ str_backup_dev_type;

        OpenFileDialog^ openFileDialog1 = gcnew OpenFileDialog;
        openFileDialog1->InitialDirectory =
            System::IO::Path::Combine(System::IO::Path::GetDirectoryName(Application::ExecutablePath), "BackupDirectory");
        openFileDialog1->Filter = "SPI Backup (*.bin)|*.bin";
        openFileDialog1->FilterIndex = 1;
        openFileDialog1->RestoreDirectory = true;
        if (openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK &&
            (fileStream = openFileDialog1->OpenFile()) != nullptr)
        {
            System::IO::MemoryStream^ ms = gcnew System::IO::MemoryStream(1048576);;
            fileStream->CopyTo(ms);
            this->backup_spi = ms->ToArray();
            fileStream->Close();

            if (this->backup_spi->Length != 524288) {
                MessageBox::Show(L"The file size must be 512KB (524288 Bytes)", L"Partial backup!",
                    MessageBoxButtons::OK, MessageBoxIcon::Stop);
                this->txtBox_fileLoaded->Text = L"No file loaded";
                this->comboBox_rstOption->Visible = false;
                this->lbl_rstDesc->Visible = false;
                this->btn_restore->Visible = false;
                this->grpBox_RstUser->Visible = false;
                this->lbl_rstDisclaimer->Visible = true;
                
                return;
            }

            if (handle_ok == 1)
                str_dev_type = L"Joy-Con (L)";
            else if (handle_ok == 2)
                str_dev_type = L"Joy-Con (R)";
            else if (handle_ok == 3)
                str_dev_type = L"Pro controller";

            if (this->backup_spi[0x6012] == 1)
                str_backup_dev_type = L"Joy-Con (L)";
            else if (this->backup_spi[0x6012] == 2)
                str_backup_dev_type = L"Joy-Con (R)";
            else if (this->backup_spi[0x6012] == 3)
                str_backup_dev_type = L"Pro controller";

            //Backup Validation
            for (int i = 0; i < 20; i++) {
                if (validation_magic[i] != this->backup_spi[i]) {
                    validation_check = false;
                    break;
                }
            }
            for (int i = 20; i < 22; i++) {
                if (validation_magic[i] != this->backup_spi[0x6012 + i - 20]) {
                    MessageBox::Show(L"The file is a \"" + str_backup_dev_type
                        + "\" backup but your device is a \"" + str_dev_type + "\"!\n\nPlease try with a \""
                        + str_dev_type + "\" SPI backup.", L"Wrong backup!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
                    this->txtBox_fileLoaded->Text = L"No file loaded";
                    this->lbl_rstDesc->Visible        = false;
                    this->comboBox_rstOption->Visible = false;
                    this->btn_restore->Visible        = false;
                    this->grpBox_RstUser->Visible     = false;
                    this->lbl_rstDisclaimer->Visible  = true;

                    return;
                }
            }
            for (int i = 28; i < 36; i++) {
                if (validation_magic[i] != this->backup_spi[0x1FF4 + i - 22])
                {
                    ota_exists = false;
                    break;
                }
            }

            if (ota_exists) {
                for (int i = 22; i < 28; i++) {
                    if (validation_magic[i] != this->backup_spi[0x10000 + i - 22] && i != 23) {
                        validation_check = false;
                        break;
                    }
                    if (validation_magic[i] != this->backup_spi[0x28000 + i - 22] && i != 23) {
                        validation_check = false;
                        break;
                    }
                }
            }
            else {
                for (int i = 22; i < 28; i++) {
                    if (validation_magic[i] != this->backup_spi[0x10000 + i - 22] && i != 23) {
                        validation_check = false;
                        break;
                    }
                }
            }
            if (!validation_check) {
                MessageBox::Show(L"The SPI backup is corrupted!\n\nPlease try another backup.",
                    L"Corrupt backup!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
                this->txtBox_fileLoaded->Text = L"No file loaded";
                this->lbl_rstDesc->Visible        = false;
                this->comboBox_rstOption->Visible = false;
                this->btn_restore->Visible        = false;
                this->grpBox_RstUser->Visible     = false;
                this->lbl_rstDisclaimer->Visible  = true;
                
                return;
            }

            unsigned char mac_addr[10];
            memset(mac_addr, 0, sizeof(mac_addr));
            get_device_info(mac_addr);

            for (int i = 4; i < 10; i++) {
                if (mac_addr[i] != this->backup_spi[0x1A - i + 4]) {
                    mac_check = false;
                }
            }
            if (!mac_check) {
                if (MessageBox::Show(L"The SPI backup is from another \"" + str_dev_type + "\".\n\nDo you want to continue?",
                    L"Different BT MAC address!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) ==
                    System::Windows::Forms::DialogResult::Yes)
                {
                    this->txtBox_fileLoaded->Text = String::Format("{0:x2}:{1:x2}:{2:x2}:{3:x2}:{4:x2}:{5:x2}",
                        this->backup_spi[0x1A], this->backup_spi[0x19], this->backup_spi[0x18],
                        this->backup_spi[0x17], this->backup_spi[0x16], this->backup_spi[0x15]);
                    this->comboBox_rstOption->SelectedItem = "Restore Color";
                    this->comboBox_rstOption->Visible = true;
                    this->btn_restore->Visible = true;
                    this->btn_restore->Enabled = true;
                    this->lbl_rstDisclaimer->Visible = false;
                    allow_full_restore = false;
                }
                else {
                    this->txtBox_fileLoaded->Text = L"No file loaded";
                    this->lbl_rstDesc->Visible        = false;
                    this->comboBox_rstOption->Visible = false;
                    this->btn_restore->Visible        = false;
                    this->grpBox_RstUser->Visible     = false;
                    this->lbl_rstDisclaimer->Visible  = true;
                }
            }
            else {
                this->txtBox_fileLoaded->Text = String::Format("{0:x2}:{1:x2}:{2:x2}:{3:x2}:{4:x2}:{5:x2}",
                    this->backup_spi[0x1A], this->backup_spi[0x19], this->backup_spi[0x18],
                    this->backup_spi[0x17], this->backup_spi[0x16], this->backup_spi[0x15]);
                this->comboBox_rstOption->SelectedItem = "Restore Color";
                this->comboBox_rstOption->Visible = true;
                this->btn_restore->Visible = true;
                this->btn_restore->Enabled = true;
                this->lbl_rstDisclaimer->Visible = false;
            }
        }
    }

    protected: System::Void comboBox_darkTheme_DrawItem(System::Object^ sender, DrawItemEventArgs^ e)
    {
        Brush^ brush = gcnew System::Drawing::SolidBrush(Color::FromArgb(251, 251, 251));;

        if (e->Index < 0)
            return;

        ComboBox^ combo = (ComboBox^)sender;
        if ((e->State & DrawItemState::Selected) == DrawItemState::Selected)
            e->Graphics->FillRectangle(gcnew SolidBrush(Color::FromArgb(105, 105, 105)), e->Bounds);
        else
            e->Graphics->FillRectangle(gcnew SolidBrush(combo->BackColor), e->Bounds);

        e->Graphics->DrawString(combo->Items[e->Index]->ToString(), e->Font, brush,
            e->Bounds, StringFormat::GenericDefault);

        e->DrawFocusRectangle();
    }

    private: System::Void comboBox_rstOption_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
        this->grpBox_RstUser->Visible = false;
        this->lbl_rstDesc->Visible = false;
        this->btn_restore->Text = L"Restore";
        this->lbl_rstDesc->Text = L"This allows for the device colors to be restored from the loaded backup.\n\n"+
            L"For more options click on the list above.";

        if (this->comboBox_rstOption->SelectedIndex == 0) {
            this->lbl_rstDesc->Visible = true;
        }
        if (this->comboBox_rstOption->SelectedIndex == 1) {
            this->lbl_rstDesc->Visible = true;
            this->lbl_rstDesc->Text = L"This will restore your S/N from the selected backup.\n" +
                L"*Make sure that this backup was your original one!\n\nIf you lost your S/N, " +
                L"check the plastic sliver it was wrapped inside the box.";
        }
        else if (this->comboBox_rstOption->SelectedIndex == 2) {
            this->lbl_rstDesc->Visible = true;
            this->lbl_rstDesc->Text =
                L"This lets you restore the chosen user calibrations from your SPI backup.";
            this->grpBox_RstUser->Visible = true;
            if (handle_ok == 1) {
                this->checkBox_rst_R_StickCal->Visible  = false;
                this->checkBox_rst_L_StickCal->Visible  = true;
                this->checkBox_rst_L_StickCal->Location = this->checkBox_rst_R_StickCal->Location;
            }
            else if (handle_ok == 2) {
                this->checkBox_rst_R_StickCal->Visible = true;
                this->checkBox_rst_L_StickCal->Visible = false;
            }
            else if (handle_ok == 3) {
                this->checkBox_rst_R_StickCal->Visible = true;
                this->checkBox_rst_L_StickCal->Visible = true;
            }

        }
        else if (this->comboBox_rstOption->SelectedIndex == 3) {
            this->lbl_rstDesc->Visible = true;
            this->lbl_rstDesc->Text =
                L"This option does the same factory reset with the option inside Switch's controller calibration menu.";
            this->btn_restore->Text = L"Reset";
            this->grpBox_RstUser->Visible = true;
            if (handle_ok == 1) {
                this->checkBox_rst_R_StickCal->Visible  = false;
                this->checkBox_rst_L_StickCal->Visible  = true;
                this->checkBox_rst_L_StickCal->Location = this->checkBox_rst_R_StickCal->Location;
            }
            else if (handle_ok == 2) {
                this->checkBox_rst_R_StickCal->Visible = true;
                this->checkBox_rst_L_StickCal->Visible = false;
            }
            else if (handle_ok == 3) {
                this->checkBox_rst_R_StickCal->Visible = true;
                this->checkBox_rst_L_StickCal->Visible = true;
            }
        }
        else if (this->comboBox_rstOption->SelectedIndex == 4) {
            this->lbl_rstDesc->Visible = true;
            this->lbl_rstDesc->Text = L"This restores the Factory and User configuration.\n\n" +
                L"To preserve factory configuration from accidental overwrite, " +
                L"the full restore is disabled if the backup does not match your device.";
            if (!allow_full_restore)
                this->btn_restore->Enabled = false;
        }
    }

    private: System::Void btn_restore_Click(System::Object^  sender, System::EventArgs^  e) {
        if (check_if_connected())
            return;
        this->menuStrip1->Enabled = false;
        this->toolStrip1->Enabled = false;
        set_led_busy();
        int error = 0;
        if (this->comboBox_rstOption->SelectedIndex == 0) {
            if (MessageBox::Show(L"The device color will be restored with the backup values!\n\nAre you sure you want to continue?",
                L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
            {
                unsigned char backupColor[12];
                memset(backupColor, 0, 12);

                for (int i = 0; i < 12; i++) {
                    backupColor[i] = this->backup_spi[0x6050 + i];
                }

                error = write_spi_data(0x6050, 12, backupColor);

                //Check that the colors were written
                update_colors_from_spi(true);
                send_rumble();
                if (error == 0)
                    MessageBox::Show(L"The colors were restored!", L"Restore Finished!", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
            }

        }
        else if (this->comboBox_rstOption->SelectedIndex == 1) {
            if (MessageBox::Show(L"The serial number will be restored with the backup values!\n\nAre you sure you want to continue?",
                L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
            {
                unsigned char sn[0x10];
                memset(sn, 0, 0x10);


                for (int i = 0; i < 0x10; i++) {
                    sn[i] = this->backup_spi[0x6000 + i];
                }

                error = write_spi_data(0x6000, 0x10, sn);
                send_rumble();
                if (error == 0) {
                    String^ new_sn;
                    if (handle_ok != 3) {
                        new_sn = gcnew String(get_sn(0x6001, 0xF).c_str());
                        MessageBox::Show(L"The serial number was restored and changed to \"" + new_sn + L"\"!",
                            L"Restore Finished!", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
                    }
                    else {
                        MessageBox::Show(L"The serial number was restored!", L"Restore Finished!",
                            MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
                    }
                }
            }
        }
        else if (this->comboBox_rstOption->SelectedIndex == 2) {
            if (MessageBox::Show(L"The selected user calibration will be restored from the backup!\n\n" +
                L"Are you sure you want to continue?", L"Warning!",
                MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
            {
                unsigned char l_stick[0xB];
                memset(l_stick, 0, 0xB);
                unsigned char r_stick[0xB];
                memset(r_stick, 0, 0xB);
                unsigned char sensor[0x1A];
                memset(sensor, 0, 0x1A);


                for (int i = 0; i < 0xB; i++) {
                    l_stick[i] = this->backup_spi[0x8010 + i];
                    r_stick[i] = this->backup_spi[0x801B + i];
                }
                for (int i = 0; i < 0x1A; i++) {
                    sensor[i] = this->backup_spi[0x8026 + i];
                }

                if (handle_ok != 2 && this->checkBox_rst_L_StickCal->Checked == true)
                    error = write_spi_data(0x8010, 0xB, l_stick);
                Sleep(100);
                if (handle_ok != 1 && this->checkBox_rst_R_StickCal->Checked == true && error == 0)
                    error = write_spi_data(0x801B, 0xB, r_stick);
                Sleep(100);
                if (this->checkBox_rst_accGyroCal->Checked == true && error == 0)
                    error = write_spi_data(0x8026, 0x1A, sensor);
                send_rumble();

                if (error == 0) {
                    MessageBox::Show(L"The user calibration was restored!", L"Calibration Restore Finished!",
                        MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
                }
            }

        }
        else if (this->comboBox_rstOption->SelectedIndex == 3) {
            if (MessageBox::Show(L"The selected user calibration will be factory resetted!\n\nAre you sure you want to continue?",
                L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
            {
                unsigned char l_stick[0xB];
                memset(l_stick, 0xFF, 0xB);
                unsigned char r_stick[0xB];
                memset(r_stick, 0xFF, 0xB);
                unsigned char sensor[0x1A];
                memset(sensor, 0xFF, 0x1A);

                if (handle_ok != 2 && this->checkBox_rst_L_StickCal->Checked == true)
                    error = write_spi_data(0x8010, 0xB, l_stick);
                Sleep(100);
                if (handle_ok != 1 && this->checkBox_rst_R_StickCal->Checked == true && error == 0)
                    error = write_spi_data(0x801B, 0xB, r_stick);
                Sleep(100);
                if (this->checkBox_rst_accGyroCal->Checked == true && error == 0)
                    error = write_spi_data(0x8026, 0x1A, sensor);
                send_rumble();

                if (error == 0) {
                    MessageBox::Show(L"The user calibration was factory resetted!", L"Calibration Reset Finished!",
                        MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
                }
            }

        }
        else if (this->comboBox_rstOption->SelectedIndex == 4) {
            if (MessageBox::Show(L"This will do a full restore of the Factory configuration and User calibration!\n\n" +
                L"Are you sure you want to continue?",
                L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
            {
                handler_close = 2;
                this->btn_loadSPIBackup->Enabled = false;
                this->btn_restore->Enabled = false;
                this->grpBox_Color->Visible = false;
                this->lbl_spiProggressDesc->Text = L"Restoring Factory Configuration and User Calibration...\n\nDon\'t disconnect your device!";
                unsigned char full_restore_data[0x10];
                unsigned char sn_backup_erase[0x10];
                memset(sn_backup_erase, 0xFF, sizeof(sn_backup_erase));

                // Factory Configuration Sector 0x6000
                for (int i = 0x00; i < 0x1000; i = i + 0x10) {
                    memset(full_restore_data, 0, 0x10);
                    for (int j = 0; j < 0x10; j++)
                        full_restore_data[j] = this->backup_spi[0x6000 + i + j];
                    if (error == 0)
                        error = write_spi_data(0x6000 + i, 0x10, full_restore_data);

                    std::stringstream offset_label;
                    offset_label << std::fixed << std::setprecision(2) << std::setfill(' ') << i / 1024.0f;
                    offset_label << "KB of 8KB";
                    FormJoy::myform1->label_progress->Text = gcnew String(offset_label.str().c_str());
                    Application::DoEvents();
                    Sleep(60);
                }
                // User Calibration Sector 0x8000
                for (int i = 0x00; i < 0x1000; i = i + 0x10) {
                    memset(full_restore_data, 0, 0x10);
                    for (int j = 0; j < 0x10; j++)
                        full_restore_data[j] = this->backup_spi[0x8000 + i + j];
                    if (error == 0)
                        error = write_spi_data(0x8000 + i, 0x10, full_restore_data);

                    std::stringstream offset_label;
                    offset_label << std::fixed << std::setprecision(2) << std::setfill(' ') << 4 + (i / 1024.0f);
                    offset_label << "KB of 8KB";
                    FormJoy::myform1->label_progress->Text = gcnew String(offset_label.str().c_str());
                    Application::DoEvents();
                    Sleep(60);
                }
                // Erase S/N backup storage
                if (error == 0)
                    error = write_spi_data(0xF000, 0x10, sn_backup_erase);

                std::stringstream offset_label;
                offset_label << std::fixed << std::setprecision(2) << std::setfill(' ') << 0x2000 / 1024.0f;
                offset_label << "KB of 8KB";
                FormJoy::myform1->label_progress->Text = gcnew String(offset_label.str().c_str());
                Application::DoEvents();

                if (error == 0) {
                    // Set shipment
                    unsigned char custom_cmd[7];
                    memset(custom_cmd, 0, 7);
                    custom_cmd[0] = 0x01;
                    custom_cmd[5] = 0x08;
                    custom_cmd[6] = 0x01;
                    send_custom_command(custom_cmd);
                    // Clear pairing info
                    memset(custom_cmd, 0, 7);
                    custom_cmd[0] = 0x01;
                    custom_cmd[5] = 0x07;
                    send_custom_command(custom_cmd);
                    // Reboot controller and go into pairing mode
                    memset(custom_cmd, 0, 7);
                    custom_cmd[0] = 0x01;
                    custom_cmd[5] = 0x06;
                    custom_cmd[6] = 0x02;
                    send_custom_command(custom_cmd);

                    send_rumble();

                    MessageBox::Show(L"The full restore was completed!\nThe controller was rebooted and it is now in pairing mode!\n\n" +
                        L"Exit Joy-Con Toolkit and pair with Switch or PC again.", L"Full Restore Finished!",
                        MessageBoxButtons::OK, MessageBoxIcon::Warning);
                    handler_close = 0;
                }
                this->grpBox_Color->Visible = true;
                this->btn_loadSPIBackup->Enabled = true;
                this->btn_restore->Enabled = true;
            }
        }
        if (error != 0) {
            MessageBox::Show(L"Failed to restore or restore incomplete!\n\nPlease try again..", L"CTCaer's Joy-Con Toolkit - Restore Failed!",
                MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
        }
        else {
            //Avoid sending commands with disconnected controller
            if (this->comboBox_rstOption->SelectedIndex != 4) {
                update_battery();
                update_temperature();
            }
        }
        this->menuStrip1->Enabled = true;
        this->toolStrip1->Enabled = true;
    }

    private: System::Void textBoxDbg_subcmd_Validating(System::Object^ sender, CancelEventArgs^ e)
    {
        bool cancel = false;
        int number = -1;
        if (Int32::TryParse(this->textBoxDbg_subcmd->Text, NumberStyles::HexNumber, CultureInfo::InvariantCulture, number))
        {
            if ((number == 0x10 || number == 0x11 || number == 0x12) && disable_expert_mode) {
                cancel = true;
                this->errorProvider2->SetError(this->textBoxDbg_subcmd,
                    L"The subcommands:\n0x10: SPI Read\n0x11: SPI Write\n0x12: SPI Sector Erase\nare disabled!");
            }else
                cancel = false;
        }
        else
        {
            //This control has failed validation: text box is not a number
            cancel = true;
            this->errorProvider2->SetError(this->textBoxDbg_subcmd, "The input must be a valid uint8 HEX!");
        }
        e->Cancel = cancel;
    }
    private: System::Void textBoxDbg_subcmd_Validated(System::Object^ sender, System::EventArgs^ e)
    {
        //Control has validated, clear any error message.
        this->errorProvider2->SetError(this->textBoxDbg_subcmd, String::Empty);
    }

    private: System::Void textBoxDbg_Validating(System::Object^ sender, CancelEventArgs^ e)
    {
        TextBox^ test = (TextBox^)sender;
        bool cancel = false;
        int number = -1;
        if (Int32::TryParse(test->Text, NumberStyles::HexNumber, CultureInfo::InvariantCulture, number))
        {
            cancel = false;
        }
        else
        {
            cancel = true;
            this->errorProvider2->SetError(test, "The input must be a valid uint8 HEX!");
        }
        e->Cancel = cancel;
    }
    private: System::Void textBoxDbg_Validated(System::Object^ sender, System::EventArgs^ e)
    {
        //Control has validated, clear any error message.
        TextBox^ test = (TextBox^)sender;
        this->errorProvider2->SetError(test, String::Empty);
    }

    private: System::Void textBox_loop_Validating(System::Object^ sender, CancelEventArgs^ e)
    {
        TextBox^ test = (TextBox^)sender;
        bool cancel = false;
        int number = -1;
        if (Int32::TryParse(test->Text, NumberStyles::Number, CultureInfo::InvariantCulture, number))
        {
            cancel = false;
        }
        else
        {
            //This control has failed validation: text box is not a number
            cancel = true;
            this->errorProvider2->SetError(test, "The input must be a number from 0-999!");
        }
        e->Cancel = cancel;
    }

    private: System::Void textBox_loop_Validated(System::Object^ sender, System::EventArgs^ e)
    {
        TextBox^ test = (TextBox^)sender;
        this->errorProvider2->SetError(test, String::Empty);
    }

    private: System::Void textBoxDbg_SubcmdArg_Validating(System::Object^ sender, CancelEventArgs^ e)
    {
        TextBox^ test = (TextBox^)sender;
        bool cancel = false;
        int number = -1;
        array<Char>^ mn_str_sn = test->Text->ToCharArray();

        for (int i = 0; i < mn_str_sn->Length; i++) {
            if ((u8)(mn_str_sn[i] >> 8) == 0x00)
            {
                if (('0' <= (u8)mn_str_sn[i] && (u8)mn_str_sn[i] <= '9') || ('A' <= (u8)mn_str_sn[i] && (u8)mn_str_sn[i] <= 'F')) {
                    cancel = false;
                }
                else {
                    cancel = true;
                    this->errorProvider1->SetError(test, "The input must be a valid HEX number!");
                }
            }
            else {
                cancel = true;
                this->errorProvider1->SetError(test, "The input must be a valid HEX number!");
            }
            if (cancel)
                break;
        }
        e->Cancel = cancel;
    }
    private: System::Void textBoxDbg_SubcmdArg_Validated(System::Object^ sender, System::EventArgs^ e)
    {
        //Control has validated, clear any error message.
        TextBox^ test = (TextBox^)sender;
        this->errorProvider1->SetError(test, String::Empty);
    }

    private: System::Void textBox_chg_sn_Validating(System::Object^ sender, CancelEventArgs^ e)
    {
        TextBox^ test = (TextBox^)sender;
        bool cancel = false;
        int number = -1;
        array<Char>^ mn_str_sn = test->Text->ToCharArray();

        for (int i = 0; i < mn_str_sn->Length; i++) {
            if ((u8)(mn_str_sn[i] >> 8)  == 0x00)
            {
                if (31 < (u8)mn_str_sn[i] && (u8)mn_str_sn[i] < 127) {
                    cancel = false;
                }
                else {
                    cancel = true;
                    this->errorProvider1->SetError(test, "Extended ASCII characters are not supported!");
                }
            }
            else
            {
                //This control has failed validation: text box is not a number
                cancel = true;
                this->errorProvider1->SetError(test, "Unicode characters are not supported!\n\n" +
                    L"Use non-extended ASCII characters only!");
            }
            if (cancel)
                break;
        }
        e->Cancel = cancel;
    }
    private: System::Void textBox_chg_sn_Validated(System::Object^ sender, System::EventArgs^ e)
    {
        //Control has validated, clear any error message.
        TextBox^ test = (TextBox^)sender;
        this->errorProvider1->SetError(test, String::Empty);
    }

    private: System::Void btn_changeSN_Click(System::Object^  sender, System::EventArgs^  e) {
        if (check_if_connected())
            return;

        if (handle_ok != 3) {
            if (MessageBox::Show(L"This will change your Serial Number!\n\nMake a backup first!\n\n" +
                L"Are you sure you want to continue?", L"Warning!",
                MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
            {
                if (MessageBox::Show(L"Did you make a backup?", L"Warning!",
                    MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
                {
                    int error = 0;
                    int sn_ok = 1;
                    unsigned char sn_magic[0x3] = { 0x00, 0x00, 0x58 };
                    unsigned char spi_sn[0x10];
                    unsigned char sn_backup[0x1];
                    memset(spi_sn, 0x11, sizeof(spi_sn));
                    memset(sn_backup, 0x00, sizeof(sn_backup));
                    
                    //Check if sn is original
                    get_spi_data(0x6000, 0x10, spi_sn);
                    for (int i = 0; i < 3; i++) {
                        if (spi_sn[i] != sn_magic[i]) {
                            sn_ok = 0;
                            break;
                        }
                    }
                    //Check if already made
                    get_spi_data(0xF000, 0x1, sn_backup);
                    if (sn_ok && sn_backup[0] == 0xFF)
                        error = write_spi_data(0xF000, 0x10, spi_sn);
                    Sleep(100);
                    array<Char>^ mn_str_sn = this->textBox_chg_sn->Text->ToCharArray();
                    unsigned char sn[32];

                    int length = 16 - mn_str_sn->Length;

                    for (int i = 0; i < length; i++) {
                        sn[i] = 0x00;
                    }
                    for (int i = 0; i < mn_str_sn->Length; i++) {
                        sn[length + i] = (unsigned char)(mn_str_sn[i] & 0xFF);
                    }
                    if (error == 0)
                        error = write_spi_data(0x6000, 0x10, sn);
                    update_battery();
                    update_temperature();
                    send_rumble();
                    if (error == 0) {
                        String^ new_sn = gcnew String(get_sn(0x6001, 0xF).c_str());
                        MessageBox::Show(L"The S/N was written to the device!\n\nThe new S/N is now \"" + new_sn +
                            L"\"!\n\nIf you still ignored the warnings about creating a backup, the S/N in the left of the main window will not change. " +
                            "Copy it somewhere safe!\n\nLastly, a backup of your S/N was created inside the SPI.");
                    }
                    else {
                        MessageBox::Show(L"Failed to write the S/N to the device!");
                    }
                }
            }
        }
        else
            MessageBox::Show(L"Changing S/N is not supported for Pro Controllers!", L"Error!", MessageBoxButtons::OK, MessageBoxIcon::Warning);
    }

    private: System::Void btn_restoreSN_Click(System::Object^  sender, System::EventArgs^  e) {
        if (check_if_connected())
            return;

        if (handle_ok != 3) {
            if (MessageBox::Show(L"Do you really want to restore it from the S/N backup inside your controller\'s SPI?\n\nYou can also choose to restore it from a SPI backup you previously made, through the main Restore option.",
                L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes) {
                int sn_ok = 1;
                int error = 0;
                unsigned char spi_sn[0x10];
                memset(spi_sn, 0x11, sizeof(spi_sn));

                //Check if there is an SN backup
                get_spi_data(0xF000, 0x10, spi_sn);
                Sleep(100);
                if (spi_sn[0] != 0x00) {
                        sn_ok = 0;
                    }
                if (sn_ok) {
                    error = write_spi_data(0x6000, 0x10, spi_sn);
                }
                else {
                    MessageBox::Show(L"No S/N backup found inside your controller\'s SPI.\n\nThis can happen if the first time you changed your S/N was with an older version of Joy-Con Toolkit.\nOtherwise, you never changed your S/N.",
                        L"Error!", MessageBoxButtons::OK, MessageBoxIcon::Error);
                    return;
                }

                update_battery();
                update_temperature();
                send_rumble();
                if (error == 0) {
                    String^ new_sn = gcnew String(get_sn(0x6001, 0xF).c_str());
                    MessageBox::Show(L"The S/N was restored to the device!\n\nThe new S/N is now \"" + new_sn + L"\"!");
                }
                else {
                    MessageBox::Show(L"Failed to restore the S/N!");
                }
            }
        }
        else
            MessageBox::Show(L"Restoring S/N is not supported for Pro Controllers!", L"Error!", MessageBoxButtons::OK, MessageBoxIcon::Warning);

    }

    private: System::Void pictureBoxBattery_Click(System::Object^  sender, System::EventArgs^  e) {
        if (check_if_connected())
            return;

        if (MessageBox::Show(L"HOORAY!!\n\nYou found the easter egg!\n\nMake sure you have a good signal and get the device near your ear.\n\nThen press OK to hear the tune!\n\nIf the tune is slow or choppy:\n1. Close the app\n2. Press the sync button once to turn off the device\n3. Get close to your BT adapter and maintain LoS\n4. Press any other button and run the app again.",
            L"Easter egg!", MessageBoxButtons::OKCancel, MessageBoxIcon::Information) == System::Windows::Forms::DialogResult::OK)
        {
            set_led_busy();
            play_tune(0);
            update_battery();
            update_temperature();
            send_rumble();
            MessageBox::Show(L"The HD Rumble music has ended.", L"Easter egg!");
        }
    }

    private: System::Void toolStripLabel_batt_Click(System::Object^  sender, System::EventArgs^  e) {
        if (check_if_connected())
            return;

        if (MessageBox::Show(L"HOORAY!!\n\nYou found another easter egg!\n\nMake sure you have a good signal and get the device near your ear.\n\nThen press OK to hear the tune!\n\nIf the tune is slow or choppy:\n1. Close the app\n2. Press the sync button once to turn off the device\n3. Get close to your BT adapter and maintain LoS\n4. Press any other button and run the app again.",
            L"Easter egg 2!", MessageBoxButtons::OKCancel, MessageBoxIcon::Information) == System::Windows::Forms::DialogResult::OK)
        {
            set_led_busy();
            play_tune(1);
            update_battery();
            update_temperature();
            send_rumble();
            MessageBox::Show(L"The HD Rumble music has ended.", L"Easter egg!");
        }
    }

    private: System::Void btn_loadVib_Click(System::Object^  sender, System::EventArgs^  e) {
        Stream^ fileStream;
        array<byte>^ file_magic = { 0x52, 0x52, 0x41, 0x57, 0x4, 0xC, 0x3, 0x10};

        OpenFileDialog^ openFileDialog1 = gcnew OpenFileDialog;
        openFileDialog1->InitialDirectory = System::IO::Path::Combine(System::IO::Path::GetDirectoryName(Application::ExecutablePath), "RumbleDirectory");
        openFileDialog1->Filter = "Binary HD Rumble (*.bnvib)|*.bnvib|Raw HD Rumble (*.jcvib)|*.jcvib";
        openFileDialog1->FilterIndex = 1;
        openFileDialog1->RestoreDirectory = true;
        if (openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK && (fileStream = openFileDialog1->OpenFile()) != nullptr)
        {    
            vib_converted = 0;
            this->label_hdrumble_filename->Text = openFileDialog1->SafeFileName;
            this->label_vib_loaded->Text = L"";
            this->label_samplerate->Text = L"";
            this->label_samples->Text = L"";
            this->textBox_vib_loop_times->Text = L"0";
            this->textBox_vib_loop_times->Visible = false;
            this->label_loop_times->Visible = false;
            System::IO::MemoryStream^ ms = gcnew System::IO::MemoryStream(fileStream->Length);;
            fileStream->CopyTo(ms);
            this->vib_loaded_file = ms->ToArray();
            this->vib_file_converted = ms->ToArray();
            fileStream->Close();

            //check for vib_file_type
            if (this->vib_loaded_file[0] == file_magic[0]) {
                for (int i = 1; i < 4; i++) {
                    if (this->vib_loaded_file[i] == file_magic[i]) {
                        vib_file_type = 1;
                        this->groupBox_vib_eq->Visible = true;
                        this->groupBox_vib_eq->Enabled = false;
                    }
                    else {
                        vib_file_type = 0;
                        this->groupBox_vib_eq->Visible = false;

                        break;
                    }
                }
                if (vib_file_type == 1) {
                    this->label_vib_loaded->Text = L"Type: Raw HD Rumble";
                    this->btn_vibPlay->Enabled = true;
                    vib_sample_rate = (this->vib_loaded_file[0x4] << 8) + this->vib_loaded_file[0x5];
                    vib_samples = (this->vib_loaded_file[0x6] << 24) + (this->vib_loaded_file[0x7] << 16) + (this->vib_loaded_file[0x8] << 8) + this->vib_loaded_file[0x9];
                    this->label_samplerate->Text = L"Sample rate: " + vib_sample_rate + L"ms";
                    this->label_samples->Text = L"Samples: " + vib_samples + L" (" + (vib_sample_rate * vib_samples) / 1000.0f + L"s)";
                }
                else {
                    this->label_vib_loaded->Text = L"Type: Unknown format";
                    this->btn_vibPlay->Enabled = false;
                }
            }
            else if (this->vib_loaded_file[4] == file_magic[6]) {
                if (this->vib_loaded_file[0] == file_magic[4]) {
                    vib_file_type = 2;
                    this->groupBox_vib_eq->Visible = true;
                    this->groupBox_vib_eq->Enabled = true;
                    this->label_vib_loaded->Text = L"Type: Binary HD Rumble";
                    u32 vib_size = this->vib_loaded_file[0x8] + (this->vib_loaded_file[0x9] << 8) + (this->vib_loaded_file[0xA] << 16) + (this->vib_loaded_file[0xB] << 24);
                    vib_sample_rate = 1000 / (this->vib_loaded_file[0x6] + (this->vib_loaded_file[0x7] << 8));
                    vib_samples = vib_size / 4;
                    this->label_samplerate->Text = L"Sample rate: " + vib_sample_rate + L"ms";
                    this->label_samples->Text = L"Samples: " + vib_samples + L" (" + (vib_sample_rate * vib_samples) / 1000.0f + L"s)";
                }
                else if (this->vib_file_converted[0] == file_magic[5]) {
                    vib_file_type = 3;
                    this->label_vib_loaded->Text = L"Type: Loop Binary HD Rumble";
                    this->groupBox_vib_eq->Visible = true;
                    this->groupBox_vib_eq->Enabled = true;
                    u32 vib_size = this->vib_loaded_file[0x10] + (this->vib_loaded_file[0x11] << 8) + (this->vib_loaded_file[0x12] << 16) + (this->vib_loaded_file[0x13] << 24);
                    vib_sample_rate = 1000 / (this->vib_loaded_file[0x6] + (this->vib_loaded_file[0x7] << 8));
                    vib_samples = vib_size / 4;
                    vib_loop_start = this->vib_loaded_file[0x8] + (this->vib_loaded_file[0x9] << 8) + (this->vib_loaded_file[0xA] << 16) + (this->vib_loaded_file[0xB] << 24);
                    vib_loop_end = this->vib_loaded_file[0xC] + (this->vib_loaded_file[0xD] << 8) + (this->vib_loaded_file[0xE] << 16) + (this->vib_loaded_file[0xF] << 24);
                    this->label_samplerate->Text = L"Sample rate: " + vib_sample_rate + L"ms";
                    this->label_samples->Text = L"Samples: " + vib_samples + L" (" + (vib_sample_rate * vib_samples) / 1000.0f + L"s)";
                    this->label_loop_times->Visible = true;
                    this->textBox_vib_loop_times->Visible = true;
                }
                else if (this->vib_file_converted[0] == file_magic[7]) {
                    vib_file_type = 4;
                    this->label_vib_loaded->Text = L"Type: Loop and Wait Binary";
                    this->groupBox_vib_eq->Visible = true;
                    this->groupBox_vib_eq->Enabled = true;
                    u32 vib_size = this->vib_loaded_file[0x14] + (this->vib_loaded_file[0x15] << 8) + (this->vib_loaded_file[0x16] << 16) + (this->vib_loaded_file[0x17] << 24);
                    vib_sample_rate = 1000 / (this->vib_loaded_file[0x6] + (this->vib_loaded_file[0x7] << 8));
                    vib_samples = vib_size / 4;
                    vib_loop_start = this->vib_loaded_file[0x8] + (this->vib_loaded_file[0x9] << 8) + (this->vib_loaded_file[0xA] << 16) + (this->vib_loaded_file[0xB] << 24);
                    vib_loop_end = this->vib_loaded_file[0xC] + (this->vib_loaded_file[0xD] << 8) + (this->vib_loaded_file[0xE] << 16) + (this->vib_loaded_file[0xF] << 24);
                    vib_loop_wait = this->vib_loaded_file[0x10] + (this->vib_loaded_file[0x11] << 8) + (this->vib_loaded_file[0x12] << 16) + (this->vib_loaded_file[0x13] << 24);
                    this->label_samplerate->Text = L"Sample rate: " + vib_sample_rate + L"ms";
                    this->label_samples->Text = L"Samples: " + vib_samples + L" (" + (vib_sample_rate * vib_samples) / 1000.0f + L"s)";
                    this->label_loop_times->Visible = true;
                    this->textBox_vib_loop_times->Visible = true;
                }
                this->btn_vibPlay->Enabled = true;
            }
            else {
                vib_file_type = 0;
                this->label_vib_loaded->Text = L"Type: Unknown format";
                this->btn_vibPlay->Enabled = false;
                this->groupBox_vib_eq->Visible = false;
            }
            this->btn_vibResetEQ->PerformClick();
        }
    }

    private: System::Void btn_vibPlay_Click(System::Object^  sender, System::EventArgs^  e) {
        if (check_if_connected())
            return;

        int vib_loop_times = 0;
        if ((vib_file_type == 2 || vib_file_type == 3 || vib_file_type == 4) && !vib_converted) {
            u8 vib_off = 0;
            if (vib_file_type == 3)
                vib_off = 8;
            if (vib_file_type == 4)
                vib_off = 12;
            //Convert to RAW vibration, apply EQ and clamp inside safe values
            this->btn_vibPlay->Text = L"Loading...";
            //vib_size = this->vib_loaded_file[0x8] + (this->vib_loaded_file[0x9] << 8) + (this->vib_loaded_file[0xA] << 16) + (this->vib_loaded_file[0xB] << 24);

            //Convert to raw
            for (u32 i = 0; i < (vib_samples * 4); i = i + 4)
            {
                //Apply amp eq
                u8 tempLA = (this->trackBar_lf_amp->Value == 10 ? this->vib_loaded_file[0xC + vib_off + i] : (u8)CLAMP((float)this->vib_loaded_file[0xC + vib_off + i] * lf_gain, 0.0f, 255.0f));
                u8 tempHA = (this->trackBar_hf_amp->Value == 10 ? this->vib_loaded_file[0xE + vib_off + i] : (u8)CLAMP((float)this->vib_loaded_file[0xE + vib_off + i] * hf_gain, 0.0f, 255.0f));

                //Apply safe limit. The sum of LF and HF amplitudes should be lower than 1.0
                float apply_safe_limit = (float)tempLA / 255.0f + tempHA / 255.0f;
                //u8 tempLA = (apply_safe_limit > 1.0f ? (u8)((float)this->vib_file_converted[0xC + i] * (0.55559999f / apply_safe_limit)) : (u8)((float)this->vib_file_converted[0xC + i] * 0.55559999f));
                tempLA = (apply_safe_limit > 1.0f ? (u8)((float)tempLA    * (1.0f / apply_safe_limit)) : tempLA);
                tempHA = (apply_safe_limit > 1.0f ? (u8)((float)tempHA * (1.0f / apply_safe_limit)) : tempHA);

                //Apply eq and convert frequencies to raw range
                u8 tempLF = (this->trackBar_lf_freq->Value == 10 ? this->vib_loaded_file[0xD + vib_off + i] : (u8)CLAMP((float)this->vib_loaded_file[0xD + vib_off + i] * lf_pitch, 0.0f, 191.0f)) - 0x40;
                u16 tempHF = ((this->trackBar_hf_freq->Value == 10 ? this->vib_loaded_file[0xF + vib_off + i] : (u8)CLAMP((float)this->vib_loaded_file[0xF + vib_off + i] * hf_pitch, 0.0f, 223.0f)) - 0x60) * 4;

                //Encode amplitudes with the look up table and direct encode frequencies
                int j;
                float temp = tempLA / 255.0f;
                for (j = 1; j < 101; j++) {
                    if (temp < lut_joy_amp.amp_float[j]) {
                        j--;
                        break;
                    }
                }
                this->vib_file_converted[0xE + vib_off + i] = ((lut_joy_amp.la[j] >> 8) & 0xFF) + tempLF;
                this->vib_file_converted[0xF + vib_off + i] = lut_joy_amp.la[j] & 0xFF;

                temp = tempHA / 255.0f;
                for (j = 1; j < 101; j++) {
                    if (temp < lut_joy_amp.amp_float[j]) {
                        j--;
                        break;
                    }
                }
                this->vib_file_converted[0xC + vib_off + i] = tempHF & 0xFF;
                this->vib_file_converted[0xD + vib_off + i] = ((tempHF >> 8) & 0xFF) + lut_joy_amp.ha[j];

            }
            vib_converted = 1;
        }

        msclr::interop::marshal_context context;
        std::stringstream ss_loops;
        ss_loops << std::dec << context.marshal_as<std::string>(this->textBox_vib_loop_times->Text);
        ss_loops >> vib_loop_times;

        this->btn_vibPlay->Enabled     = false;
        this->btn_loadVib->Enabled     = false;
        this->groupBox_vib_eq->Enabled = false;
        this->btn_vibPlay->Text = L"Playing...";


        play_hd_rumble_file(vib_file_type, vib_sample_rate, vib_samples, vib_loop_start, vib_loop_end, vib_loop_wait, vib_loop_times);

        this->btn_vibPlay->Text = L"Play";
        this->btn_vibPlay->Enabled = true;
        this->btn_loadVib->Enabled = true;
        if (vib_file_type == 2 || vib_file_type == 3 || vib_file_type == 4) {
            this->groupBox_vib_eq->Enabled = true;
        }
        update_battery();
        update_temperature();

    }
     
    private: System::Void btn_vibResetEQ_Click(System::Object^  sender, System::EventArgs^  e) {
        this->trackBar_lf_amp->Value = 10;
        this->trackBar_lf_freq->Value = 10;
        this->trackBar_hf_amp->Value = 10;
        this->trackBar_hf_freq->Value = 10;
    }

    private: System::Void TrackBar_ValueChanged(System::Object^ sender, System::EventArgs^ e)
    {
        lf_gain  = this->trackBar_lf_amp->Value / 10.0f;
        lf_pitch = this->trackBar_lf_freq->Value / 10.0f;
        hf_gain  = this->trackBar_hf_amp->Value / 10.0f;
        hf_pitch = this->trackBar_hf_freq->Value / 10.0f;
        this->toolTip1->SetToolTip(this->trackBar_lf_amp,  String::Format("{0:d}%", (int)(lf_gain * 100.01f)));
        this->toolTip1->SetToolTip(this->trackBar_lf_freq, String::Format("{0:d}%", (int)(lf_pitch * 100.01f)));
        this->toolTip1->SetToolTip(this->trackBar_hf_amp,  String::Format("{0:d}%", (int)(hf_gain * 100.01f)));
        this->toolTip1->SetToolTip(this->trackBar_hf_freq, String::Format("{0:d}%", (int)(hf_pitch * 100.01f)));
        vib_converted = 0;
    }

    private: System::Void btn_runBtnTest_Click(System::Object^  sender, System::EventArgs^  e) {
        if (check_if_connected())
            return;

        if (!enable_button_test) {
            this->btn_runBtnTest->Text = L"Turn off";
            enable_button_test = true;
            button_test();
        }
        else {
            this->btn_runBtnTest->Text = L"Turn on";
            enable_button_test = false;
        }

    }

    private: System::Void btn_spiCancel_Click(System::Object^  sender, System::EventArgs^  e) {
        cancel_spi_dump = true;
    }

    private: System::Void toolStripLabel_temp_Click(System::Object^  sender, System::EventArgs^  e) {
        if (temp_celsius)
            temp_celsius = false;
        else
            temp_celsius = true;

        update_temperature();
    }

    private: System::Void toolStripBtn_Disconnect_Click(System::Object^  sender, System::EventArgs^  e) {
        unsigned char custom_cmd[7];
        memset(custom_cmd, 0, 7);
        custom_cmd[0] = 0x01;
        custom_cmd[5] = 0x06;
        custom_cmd[6] = 0x00;
        send_custom_command(custom_cmd);
        this->toolStripBtn_Disconnect->Enabled = false;
        this->toolStripLabel_temp->Enabled     = false;
        this->toolStripLabel_batt->Enabled     = false;
        this->toolStripBtn_batt->Enabled       = false;
    }


    private: System::Void toolStripBtn_refresh_Click(System::Object^  sender, System::EventArgs^  e) {
        full_refresh(true);
    }
 

    private: System::Void btn_changeNormalColor_Click(System::Object^  sender, System::EventArgs^  e) {
        changeColorDialog(false);
    }


    private: System::Void btn_changeGripsColor_Click(System::Object^  sender, System::EventArgs^  e) {
        changeColorDialog(true);
    }


    private: System::Void changeColorDialog(bool gripsDialog) {
        if (gripsDialog)
            JCColorPicker = gcnew jcColor::JoyConColorPicker(this->jcGripLeftColor, this->jcGripRightColor, gripsDialog);
        else
            JCColorPicker = gcnew jcColor::JoyConColorPicker(this->jcBodyColor, this->jcButtonsColor, gripsDialog);

        System::Drawing::Rectangle screenRectangle = RectangleToScreen(this->ClientRectangle);
        int titleHeight = screenRectangle.Top - this->Top;

        JCColorPicker->TopLevel = false;
        JCColorPicker->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
        JCColorPicker->m_cmd_Cancel->Click += gcnew System::EventHandler(this, &FormJoy::Color_Picker_Cancel);
        JCColorPicker->m_cmd_OK->Click += gcnew System::EventHandler(this, &FormJoy::Color_Picker_OK);

        if (option_is_on == 5) {
            enable_button_test = false;
            enable_NFCScanning = false;
            this->Controls->Remove(this->grpBox_dev_param);
            this->Controls->Remove(this->grpBox_StickCal);
            this->Controls->Remove(this->grpBox_accGyroCal);
            this->Controls->Remove(this->grpBox_nfc);
        }

        JCColorPicker->Show();
        
        JCColorPicker->Location = System::Drawing::Point(0,25);
        JCColorPicker->AutoScaleDimensions = System::Drawing::SizeF(96, 96);

        this->panel_filler->Location = System::Drawing::Point(JCColorPicker->ClientSize.Width, 25);
        this->panel_filler->Size = System::Drawing::Size(3, JCColorPicker->ClientSize.Height);
        this->Controls->Add(JCColorPicker);

        this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);

        JCColorPicker->BringToFront();
        JCColorPicker->Padding = System::Windows::Forms::Padding(0, 0, 0, titleHeight);
        this->menuStrip1->BringToFront();
        this->toolStrip1->BringToFront();
        this->menuStrip1->Enabled = false;
        this->toolStrip1->Enabled = false;
        this->menuStrip1->Refresh();
        this->toolStrip1->Refresh();

        // Load grips color panel outside, so High DPI scaling can work
        JCColorPicker->loadGripsPanel();
    }

    private: System::Void Color_Picker_Cancel(System::Object^  sender, System::EventArgs^  e) {
        this->panel_filler->Location = System::Drawing::Point(0, 0);
        this->panel_filler->Size = System::Drawing::Size(0, 0);
        if (option_is_on == 5) {
            this->Controls->Add(this->grpBox_StickCal);
            this->Controls->Add(this->grpBox_dev_param);
            this->Controls->Add(this->grpBox_accGyroCal);
            this->Controls->Add(this->grpBox_nfc);
            this->lbl_nfcHelp->Size = System::Drawing::Size(203, 85);
            this->txtBox_nfcUid->Size = System::Drawing::Size(208, 53);
            this->textBox_lstick_fcal->Size   = System::Drawing::Size(207, 44);
            this->textBox_lstick_ucal->Size   = System::Drawing::Size(207, 44);
            this->textBox_rstick_fcal->Size   = System::Drawing::Size(207, 44);
            this->textBox_rstick_ucal->Size   = System::Drawing::Size(207, 44);
            this->textBox_6axis_cal->Size     = System::Drawing::Size(156, 88);
            this->textBox_6axis_ucal->Size    = System::Drawing::Size(156, 88);
            this->txtBox_devParameters->Size  = System::Drawing::Size(135, 185);
            this->txtBox_devParameters2->Size = System::Drawing::Size(140, 130);

            this->textBox_btn_test_reply->Size = System::Drawing::Size(205, 172);
            this->textBox_btn_test_subreply->Size = System::Drawing::Size(205, 140);

            this->btn_runBtnTest->Text = L"Turn on";
            option_is_on = 5;
            this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
        }
        this->menuStrip1->Enabled = true;
        this->toolStrip1->Enabled = true;
        this->menuStrip1->Refresh();
        this->toolStrip1->Refresh();
    }
    
    private: System::Void Color_Picker_OK(System::Object^  sender, System::EventArgs^  e) {
        this->panel_filler->Location = System::Drawing::Point(0, 0);
        this->panel_filler->Size = System::Drawing::Size(0, 0);
        if (option_is_on == 5) {
            this->Controls->Add(this->grpBox_StickCal);
            this->Controls->Add(this->grpBox_dev_param);
            this->Controls->Add(this->grpBox_accGyroCal);
            this->Controls->Add(this->grpBox_nfc);
            this->lbl_nfcHelp->Size = System::Drawing::Size(203, 85);
            this->txtBox_nfcUid->Size = System::Drawing::Size(208, 53);
            this->textBox_lstick_fcal->Size   = System::Drawing::Size(207, 44);
            this->textBox_lstick_ucal->Size   = System::Drawing::Size(207, 44);
            this->textBox_rstick_fcal->Size   = System::Drawing::Size(207, 44);
            this->textBox_rstick_ucal->Size   = System::Drawing::Size(207, 44);
            this->textBox_6axis_cal->Size     = System::Drawing::Size(156, 88);
            this->textBox_6axis_ucal->Size    = System::Drawing::Size(156, 88);
            this->txtBox_devParameters->Size  = System::Drawing::Size(135, 185);
            this->txtBox_devParameters2->Size = System::Drawing::Size(140, 130);

            this->textBox_btn_test_reply->Size = System::Drawing::Size(205, 172);
            this->textBox_btn_test_subreply->Size = System::Drawing::Size(205, 140);

            this->btn_runBtnTest->Text = L"Turn on";
            option_is_on = 5;
            this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
        }
        if (!JCColorPicker->GripsColorValue) {
            this->jcBodyColor = JCColorPicker->PrimaryColor;
            this->jcButtonsColor = JCColorPicker->SecondaryColor;
            this->lbl_Body_hex_txt->Text = L"Body: #" + String::Format("{0:X6}",
                (jcBodyColor.R << 16) + (jcBodyColor.G << 8) + (jcBodyColor.B));
            this->lbl_Body_hex_txt->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_Body_hex_txt->Size = System::Drawing::Size(128, 24);

            this->lbl_Buttons_hex_txt->Text = L"Buttons: #" + String::Format("{0:X6}",
                (jcButtonsColor.R << 16) + (jcButtonsColor.G << 8) + (jcButtonsColor.B));
            this->lbl_Buttons_hex_txt->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_Buttons_hex_txt->Size = System::Drawing::Size(128, 24);
        }
        else {
            this->jcGripLeftColor  = JCColorPicker->PrimaryColor;
            this->jcGripRightColor = JCColorPicker->SecondaryColor;
        }
        update_joycon_color(
            jcBodyColor.R,      jcBodyColor.G,      jcBodyColor.B,
            jcButtonsColor.R,   jcButtonsColor.G,   jcButtonsColor.B,
            jcGripLeftColor.R,  jcGripLeftColor.G,  jcGripLeftColor.B,
            jcGripRightColor.R, jcGripRightColor.G, jcGripRightColor.B
        );

        this->btn_writeColorsToSpi->Enabled = true;

        this->menuStrip1->Enabled = true;
        this->toolStrip1->Enabled = true;

        this->menuStrip1->Refresh();
        this->toolStrip1->Refresh();
    }


    private: System::Void fixToolstripOverlap(System::Object^  sender, System::EventArgs^  e) {
        this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
        System::Drawing::Rectangle screenRectangle = RectangleToScreen(this->ClientRectangle);
        int titleHeight = screenRectangle.Top - this->Top;
        
        this->grpBox_Color->Margin      = System::Windows::Forms::Padding(0, 0, 14, titleHeight);
        this->grpBox_StickCal->Margin   = System::Windows::Forms::Padding(0, 0, 0, titleHeight);
        this->grpBox_IRSettings->Margin = System::Windows::Forms::Padding(0, 0, 0, titleHeight);
    }


    private: System::Void TrackBarIR_ValueChanged(System::Object^ sender, System::EventArgs^ e) {
        this->toolTip1->SetToolTip(this->trackBar_IRGain, String::Format("{0:d}x", this->trackBar_IRGain->Value));
    }


    private: System::Void TrackBarIRLedsIntensity_ValueChanged(System::Object^ sender, System::EventArgs^ e) {
        TrackBar^ temp = (TrackBar^)sender;
        if (temp->Maximum == 15)
            this->toolTip1->SetToolTip((TrackBar^)sender, String::Format("{0:d}%", (temp->Value * 100) / 15));
        else
            this->toolTip1->SetToolTip((TrackBar^)sender, String::Format("{0:d}%", (temp->Value * 100) / 16));
    }


    private: System::Void IRFlashlight_checkedChanged(System::Object^ sender, System::EventArgs^ e) {
        if (this->chkBox_IRFlashlight->Checked) {
            this->chkBox_IRExFilter->Enabled     = false;
            this->chkBox_IRStrobe->Enabled       = false;
            this->trackBar_IRBrightLeds->Enabled = false;
            this->trackBar_IRDimLeds->Enabled    = false;
            this->lbl_IRLed1Int->Enabled         = false;
            this->lbl_IRLed2Int->Enabled         = false;
        }
        else {
            this->chkBox_IRExFilter->Enabled     = true;
            this->chkBox_IRStrobe->Enabled       = true;
            this->trackBar_IRBrightLeds->Enabled = true;
            this->trackBar_IRDimLeds->Enabled    = true;
            this->lbl_IRLed1Int->Enabled         = true;
            this->lbl_IRLed2Int->Enabled         = true;
        }
    }


    private: System::Void IRLeds_checkedChanged(System::Object^ sender, System::EventArgs^ e) {
        if (this->chkBox_IRBrightLeds->Checked) {
            this->trackBar_IRBrightLeds->Enabled = true;
            this->lbl_IRLed1Int->Enabled         = true;
        }
        else {
            this->trackBar_IRBrightLeds->Enabled = false;
            this->lbl_IRLed1Int->Enabled         = false;
        }
        if (this->chkBox_IRDimLeds->Checked) {
            this->trackBar_IRDimLeds->Enabled = true;
            this->lbl_IRLed2Int->Enabled      = true;
        }
        else {
            this->trackBar_IRDimLeds->Enabled = false;
            this->lbl_IRLed2Int->Enabled      = false;
        }
    }


    private: System::Void IRDenoise_checkedChanged(System::Object^ sender, System::EventArgs^ e) {
        if (this->chkBox_IRDenoise->Checked) {
            this->numeric_IRDenoiseEdgeSmoothing->Enabled = true;
            this->numeric_IRDenoiseColorInterpolation->Enabled = true;
            this->lbl_IRDenoise1->Enabled = true;
            this->lbl_IRDenoise2->Enabled = true;
        }
        else {
            this->numeric_IRDenoiseEdgeSmoothing->Enabled = false;
            this->numeric_IRDenoiseColorInterpolation->Enabled = false;
            this->lbl_IRDenoise1->Enabled = false;
            this->lbl_IRDenoise2->Enabled = false;
        }
    }


    private: System::Void IRAutoExposure_checkedChanged(System::Object^ sender, System::EventArgs^ e) {
        if (this->chkBox_IRAutoExposure->Checked) {
            if (this->radioBtn_IR30p->Checked)
                this->radioBtn_IR60p->Checked = true;
            this->radioBtn_IR30p->Enabled     = false;
            this->trackBar_IRGain->Enabled    = false;
            this->numeric_IRExposure->Enabled = false;
            this->lbl_digitalGain->Enabled    = false;
            this->lbl_exposure->Enabled       = false;
        }
        else {
            this->radioBtn_IR30p->Enabled     = true;
            this->trackBar_IRGain->Enabled    = true;
            this->numeric_IRExposure->Enabled = true;
            this->lbl_digitalGain->Enabled    = true;
            this->lbl_exposure->Enabled       = true;
        }
    }

 
    private: System::Int32 prepareSendIRConfig(bool startNewConfig) {
        String^ error_msg;
        ir_image_config ir_new_config = {0};
        int res = 0;

        this->lbl_IRStatus->Text = "Status: Configuring";
        Application::DoEvents();

        // The IR camera lens has a FoV of 123°. The IR filter is a NIR 850nm wavelength pass filter.

        // Resolution config register and no of packets expected
        // The sensor supports a max of Binning [4 x 2] and max Skipping [4 x 4]
        // The maximum reduction in resolution is a combined Binning/Skipping [16 x 8]
        // The bits control the matrices used. Skipping [Bits0,1 x Bits2,3], Binning [Bits4,5 x Bit6]. Bit7 is unused.
        if (startNewConfig) {
            if (this->radioBtn_IR240p->Checked) {
                ir_image_width  = 320;
                ir_image_height = 240;
                ir_new_config.ir_res_reg = 0b00000000; // Full pixel array
                ir_max_frag_no  = 0xff;
            }
            else if (this->radioBtn_IR120p->Checked) {
                ir_image_width  = 160;
                ir_image_height = 120;
                ir_new_config.ir_res_reg = 0b01010000; // Sensor Binning [2 X 2]
                ir_max_frag_no  = 0x3f;
            }
            else if (this->radioBtn_IR60p->Checked) {
                ir_image_width  = 80;
                ir_image_height = 60;
                ir_new_config.ir_res_reg = 0b01100100; // Sensor Binning [4 x 2] and Skipping [1 x 2]
                ir_max_frag_no  = 0x0f;
            }
            else if (this->radioBtn_IR30p->Checked) {
                ir_image_width  = 40;
                ir_image_height = 30;
                ir_new_config.ir_res_reg = 0b01101001; // Sensor Binning [4 x 2] and Skipping [2 x 4]
                ir_max_frag_no  = 0x03;
            }
            else {
                return 8;
            }
        }

        // Enable IR Leds. Only the following configurations are supported.
        if (this->chkBox_IRBrightLeds->Checked == true && this->chkBox_IRDimLeds->Checked == true)
            ir_new_config.ir_leds = 0b000000; // Both Far/Narrow 75° and Near/Wide 130° Led groups are enabled.
        else if (this->chkBox_IRBrightLeds->Checked == true && this->chkBox_IRDimLeds->Checked == false)
            ir_new_config.ir_leds = 0b100000; // Only Far/Narrow 75° Led group is enabled.
        else if (this->chkBox_IRBrightLeds->Checked == false && this->chkBox_IRDimLeds->Checked == true)
            ir_new_config.ir_leds = 0b010000; // Only Near/Wide 130° Led group is enabled.
        else if (this->chkBox_IRBrightLeds->Checked == false && this->chkBox_IRDimLeds->Checked == false)
            ir_new_config.ir_leds = 0b110000; // Both groups disabled

        // IR Leds Intensity
        ir_new_config.ir_leds_intensity = ((u8)this->trackBar_IRBrightLeds->Value << 8) | (u8)this->trackBar_IRDimLeds->Value;

        // IR Leds Effects
        if (this->chkBox_IRFlashlight->Checked)
            ir_new_config.ir_leds |= 0b01;
        if (this->chkBox_IRStrobe->Checked)
            ir_new_config.ir_leds |= 0b10000000;

        // External Light filter (Dark-frame subtraction). Additionally, disable if leds in flashlight mode.
        if ((this->chkBox_IRExFilter->Checked || this->chkBox_IRStrobe->Checked) && this->chkBox_IRExFilter->Enabled)
            ir_new_config.ir_ex_light_filter = 0x03;
        else
            ir_new_config.ir_ex_light_filter = 0x00;

        // Flip image. Food for tracking camera movement when taking selfies :P
        if (this->chkBox_IRSelfie->Checked)
            ir_new_config.ir_flip = 0x02;
        else
            ir_new_config.ir_flip = 0x00;

        // Exposure time (Shutter speed) is in us. Valid values are 0 to 600us or 0 - 1/1666.66s
        ir_new_config.ir_exposure = (u16)(this->numeric_IRExposure->Value * 31200 / 1000);
        if (!this->chkBox_IRAutoExposure->Checked && enable_IRVideoPhoto) {
            enable_IRAutoExposure = false;
            ir_new_config.ir_digital_gain = (u8)this->trackBar_IRGain->Value;
        }
        else {
            enable_IRAutoExposure = true;
            ir_new_config.ir_digital_gain = 1; // Disable digital gain for auto exposure
        }
        

        //De-noise algorithms
        if (this->chkBox_IRDenoise->Checked)
            ir_new_config.ir_denoise = 0x01 << 16;
        else
            ir_new_config.ir_denoise = 0x00 << 16;
        ir_new_config.ir_denoise |= ((u8)this->numeric_IRDenoiseEdgeSmoothing->Value & 0xFF) << 8;
        ir_new_config.ir_denoise |= (u8)this->numeric_IRDenoiseColorInterpolation->Value & 0xFF;

        // Initialize camera
        if (startNewConfig) {
            // Configure the IR camera and take a photo or stream.
            res = ir_sensor(ir_new_config);

            // Get error
            switch (res) {
            case 1:
                error_msg = "1ID31";
                break;
            case 2:
                error_msg = "2MCUON";
                break;
            case 3:
                error_msg = "3MCUONBUSY";
                break;
            case 4:
                error_msg = "4MCUMODESET";
                break;
            case 5:
                error_msg = "5MCUSETBUSY";
                break;
            case 6:
                error_msg = "6IRMODESET";
                break;
            case 7:
                error_msg = "7IRSETBUSY";
                break;
            case 8:
                error_msg = "8IRCFG";
                break;
            case 9:
                error_msg = "9IRFCFG";
                break;
            default:
                break;
            }
            if (res > 0)
                this->lbl_IRStatus->Text = "Status: Error " + error_msg + "!";
        }
        // Change camera configuration
        else {
            ir_new_config.ir_custom_register = ((u16)this->numeric_IRCustomRegAddr->Value) | ((u8)this->numeric_IRCustomRegVal->Value << 16);
            res = ir_sensor_config_live(ir_new_config);
        }

        return res;
    }
    
    public: System::Void setIRPictureWindow(u8* buf_image, bool ir_video_photo) {
        Bitmap^ MyImage = gcnew Bitmap(ir_image_width, ir_image_height, System::Drawing::Imaging::PixelFormat::Format24bppRgb);
        int buf_pos = 0;

        // Skip slow SetPixel(). Reduce latency pixel set latency from 842us -> 260ns.
        System::Drawing::Imaging::BitmapData^ bmd = MyImage->LockBits(System::Drawing::Rectangle(0, 0, ir_image_width, ir_image_height), System::Drawing::Imaging::ImageLockMode::ReadOnly, MyImage->PixelFormat);
        int PixelSize = 3;

        for (int y = 0; y < ir_image_height; y++) {
            byte* row = (byte *)bmd->Scan0.ToPointer() + (y * bmd->Stride);
            for (int x = 0; x < ir_image_width; x++) {
                // Ironbow Palette
                if (this->radioBtn_IRColorHeat->Checked) {
                    // Values are in BGR in memory. Here in RGB order.
                    row[x * PixelSize + 2] = (iron_palette[buf_image[x + buf_pos]] >> 16)&0xFF;
                    row[x * PixelSize + 1] = (iron_palette[buf_image[x + buf_pos]] >> 8) & 0xFF;
                    row[x * PixelSize]     =  iron_palette[buf_image[x + buf_pos]] & 0xFF;
                }
                // Greyscale
                else if (this->radioBtn_IRColorGrey->Checked) {
                    // Values are in BGR in memory. Here in RGB order.
                    row[x * PixelSize + 2] = buf_image[x + buf_pos];
                    row[x * PixelSize + 1] = buf_image[x + buf_pos];
                    row[x * PixelSize]     = buf_image[x + buf_pos];
                }
                // Night vision
                else if (this->radioBtn_IRColorGreen->Checked) {
                    // Values are in BGR in memory. Here in RGB order.
                    row[x * PixelSize + 2] = 0;
                    row[x * PixelSize + 1] = buf_image[x + buf_pos];
                    row[x * PixelSize]     = 0;
                }
                // Red vision
                else {
                    // Values are in BGR in memory. Here in RGB order.
                    row[x * PixelSize + 2] = buf_image[x + buf_pos];
                    row[x * PixelSize + 1] = 0;
                    row[x * PixelSize]     = 0;
                }
            }
            buf_pos += ir_image_width;
        }
        MyImage->UnlockBits(bmd);

        Image^ rotatedImage = dynamic_cast<Image^>(MyImage);
        rotatedImage->RotateFlip(RotateFlipType::Rotate90FlipNone);

        if (!enable_IRVideoPhoto)
            rotatedImage->Save("IRcamera.png", System::Drawing::Imaging::ImageFormat::Png);

        Image^ resizedImage = gcnew Bitmap(240, 320);
        Graphics^ graphicsHandle = Graphics::FromImage(resizedImage);
        graphicsHandle->InterpolationMode = System::Drawing::Drawing2D::InterpolationMode::HighQualityBicubic;
        graphicsHandle->DrawImage(rotatedImage, 0, 0, 240, 320);

        this->pictureBoxIR->Image = resizedImage;
        this->pictureBoxIR->ClientSize = System::Drawing::Size(240, 320);
        this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
    }

    private: System::Void btn_getImage_Click(System::Object^  sender, System::EventArgs^  e) {
        if (check_if_connected())
            return;

        int res;

        this->btn_getIRStream->Enabled = false;
        this->btn_getIRImage->Enabled  = false;
        enable_IRVideoPhoto = false;

        res = prepareSendIRConfig(true);
        
        if (res == 0)
            this->lbl_IRStatus->Text = "Status: Done! Saved to IRcamera.png";

        this->btn_getIRStream->Enabled = true;
        this->btn_getIRImage->Enabled  = true;
    }

    
    private: System::Void btn_getVideo_Click(System::Object^  sender, System::EventArgs^  e) {
        if (check_if_connected())
            return;

        int res;

        this->btn_getIRImage->Enabled = false;
        this->grpBox_IRRes->Enabled   = false;
        if (enable_IRVideoPhoto) {
            enable_IRVideoPhoto = false;
            this->btn_getIRStream->Enabled = false;
        }
        else {
            enable_IRVideoPhoto = true;
            this->btn_getIRStream->Text = L"Stop";
            this->btn_IRConfigLive->Enabled = true;
            res = prepareSendIRConfig(true);

            enable_IRVideoPhoto = false;
            if (res == 0)
                this->lbl_IRStatus->Text = L"Status: Standby";
            this->btn_getIRImage->Enabled  = true;
            this->grpBox_IRRes->Enabled    = true;
            this->btn_getIRStream->Enabled = true;
        }

        
        this->btn_IRConfigLive->Enabled = false;
        this->btn_getIRStream->Text = L"Stream";
    }


    private: System::Void btn_IRConfigLive_Click(System::Object^  sender, System::EventArgs^  e) {
        if (enable_IRVideoPhoto)
            int res = prepareSendIRConfig(false);
    }


    private: System::Void btn_NFC_Click(System::Object^  sender, System::EventArgs^  e) {
        if (check_if_connected())
            return;

        if (enable_NFCScanning) {
            this->btn_NFC->Text = "Scan";
            enable_NFCScanning  = false;
        }
        else {
            String^ error_msg;
            this->btn_NFC->Text = "Stop";
            enable_NFCScanning  = true;
            Application::DoEvents();
            int res = nfc_tag_info();

            // Get error
            switch (res) {
            case 1:
                error_msg = "1ID31";
                break;
            case 2:
                error_msg = "2MCUON";
                break;
            case 3:
                error_msg = "3MCUONBUSY";
                break;
            case 4:
                error_msg = "4MCUMODESET";
                break;
            case 5:
                error_msg = "5MCUSETBUSY";
                break;
            case 6:
                error_msg = "6NFCPOLL";
                break;
            default:
                break;
            }
            if (res > 0)
                this->txtBox_nfcUid->Text = "Error " + error_msg + "!";
        }
    }


    private: System::Void btn_refreshUserCal_Click(System::Object^  sender, System::EventArgs^  e) {
        if (check_if_connected())
            return;

        u8 user_stick_cal[22];
        u8 user_sensor_cal[26];
        u8 stick_model_main_left[3];
        u8 stick_model_pro_right[3];
        u16 decoded_stick_pair[16];
        memset(user_stick_cal,        0, sizeof(user_stick_cal));
        memset(user_sensor_cal,       0, sizeof(user_sensor_cal));
        memset(stick_model_main_left, 0, sizeof(stick_model_main_left));
        memset(stick_model_pro_right, 0, sizeof(stick_model_pro_right));
        memset(decoded_stick_pair,    0, sizeof(decoded_stick_pair));

        get_spi_data(0x8010, sizeof(user_stick_cal), user_stick_cal);
        Sleep(100);
        get_spi_data(0x8026, sizeof(user_sensor_cal), user_sensor_cal);
        Sleep(100);
        get_spi_data(0x6089, sizeof(stick_model_main_left), stick_model_main_left);
        if (handle_ok == 3) {
            Sleep(100);
            get_spi_data(0x609B, sizeof(stick_model_pro_right), stick_model_pro_right);
        }

        // Left stick user cal
        if (handle_ok != 2) {
            if (*(u16*)&user_stick_cal[0] == 0xA1B2) {
                // Center X,Y
                decode_stick_params(decoded_stick_pair + 2, user_stick_cal + 5);
                // -Axis X,Y Offset
                decode_stick_params(decoded_stick_pair,     user_stick_cal + 8);
                // +Axis X,Y Offset
                decode_stick_params(decoded_stick_pair + 4, user_stick_cal + 2);

                this->numeric_leftUserCal_x_minus->Value  = decoded_stick_pair[2] - decoded_stick_pair[0];
                this->numeric_leftUserCal_x_center->Value = decoded_stick_pair[2];
                this->numeric_leftUserCal_x_plus->Value   = decoded_stick_pair[2] + decoded_stick_pair[4];

                this->numeric_leftUserCal_y_minus->Value  = decoded_stick_pair[3] - decoded_stick_pair[1];
                this->numeric_leftUserCal_y_center->Value = decoded_stick_pair[3];
                this->numeric_leftUserCal_y_plus->Value   = decoded_stick_pair[3] + decoded_stick_pair[5];

                this->checkBox_enableLeftUserCal->Checked = true;
            }
            else
                this->checkBox_enableLeftUserCal->Checked = false;

            grpBox_leftStickUCal->Enabled = true;
        }
        else
            grpBox_leftStickUCal->Enabled = false;
        // Right stick user cal
        if (handle_ok != 1) {
            if (*(u16*)&user_stick_cal[0xB] == 0xA1B2) {
                // Center X,Y
                decode_stick_params(decoded_stick_pair + 8, user_stick_cal + 13);
                // -Axis X,Y Offset
                decode_stick_params(decoded_stick_pair + 6, user_stick_cal + 16);
                // +Axis X,Y Offset
                decode_stick_params(decoded_stick_pair + 10, user_stick_cal + 19);

                this->numeric_rightUserCal_x_minus->Value  = decoded_stick_pair[8] - decoded_stick_pair[6];
                this->numeric_rightUserCal_x_center->Value = decoded_stick_pair[8];
                this->numeric_rightUserCal_x_plus->Value   = decoded_stick_pair[8] + decoded_stick_pair[10];

                this->numeric_rightUserCal_y_minus->Value  = decoded_stick_pair[9] - decoded_stick_pair[7];
                this->numeric_rightUserCal_y_center->Value = decoded_stick_pair[9];
                this->numeric_rightUserCal_y_plus->Value   = decoded_stick_pair[9] + decoded_stick_pair[11];

                this->checkBox_enableRightUserCal->Checked = true;
            }
            else
                this->checkBox_enableRightUserCal->Checked = false;

            grpBox_rightStickUCal->Enabled = true;
        }
        else
            grpBox_rightStickUCal->Enabled = false;

        // Acc/Gyro user cal
        if (*(u16*)&user_sensor_cal[0] == 0xA1B2) {
            this->numeric_CalEditAccX->Value = *(u16*)&user_sensor_cal[2];
            this->numeric_CalEditAccY->Value = *(u16*)&user_sensor_cal[4];
            this->numeric_CalEditAccZ->Value = *(u16*)&user_sensor_cal[6];

            this->numeric_CalEditGyroX->Value = *(u16*)&user_sensor_cal[14];
            this->numeric_CalEditGyroY->Value = *(u16*)&user_sensor_cal[16];
            this->numeric_CalEditGyroZ->Value = *(u16*)&user_sensor_cal[18];

            this->checkBox_enableSensorUserCal->Checked = true;
        }
        else
            this->checkBox_enableSensorUserCal->Checked = false;
        grpBox_CalUserAcc->Enabled = true;

        // Stick device parameters. These come from factory.
        decode_stick_params(decoded_stick_pair + 12, stick_model_main_left);
        this->numeric_StickParamDeadzone->Value   = decoded_stick_pair[12];
        this->numeric_StickParamRangeRatio->Value = decoded_stick_pair[13];
        this->numeric_StickParamDeadzone2->Enabled   = false;
        this->numeric_StickParamRangeRatio2->Enabled = false;
        this->lbl_proStickHelp->Enabled              = false;
        if (handle_ok == 3) {
            decode_stick_params(decoded_stick_pair + 14, stick_model_pro_right);
            this->numeric_StickParamDeadzone2->Value   = decoded_stick_pair[14];
            this->numeric_StickParamRangeRatio2->Value = decoded_stick_pair[15];
            this->numeric_StickParamDeadzone2->Enabled   = true;
            this->numeric_StickParamRangeRatio2->Enabled = true;
            this->lbl_proStickHelp->Enabled              = true;
        }
        grpBox_StickDevParam->Enabled = true;
        
        // Enable write buttons
        this->btn_writeStickParams->Enabled = true;
        this->btn_writeUserCal->Enabled = true;
    }


    private: System::Void btn_writeUserCal_Click(System::Object^  sender, System::EventArgs^  e) {
        if (check_if_connected())
            return;

        if (MessageBox::Show(L"Are you sure you want to continue?",
            L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes) {
            u8 user_stick_cal[22];
            u8 user_sensor_cal[26];
            u16 decoded_stick_pair[2];
            memset(user_stick_cal,     0, sizeof(user_stick_cal));
            memset(user_sensor_cal,    0, sizeof(user_sensor_cal));
            memset(decoded_stick_pair, 0, sizeof(decoded_stick_pair));

            if (handle_ok != 2 && this->checkBox_enableLeftUserCal->Checked) {
                *(u16*)&user_stick_cal[0] = 0xA1B2;
                // Center X,Y
                decoded_stick_pair[0] = (u16)this->numeric_leftUserCal_x_center->Value;
                decoded_stick_pair[1] = (u16)this->numeric_leftUserCal_y_center->Value;
                encode_stick_params(user_stick_cal + 5, decoded_stick_pair);
                // +Axis X,Y
                decoded_stick_pair[0] = (u16)this->numeric_leftUserCal_x_plus->Value - (u16)this->numeric_leftUserCal_x_center->Value;
                decoded_stick_pair[1] = (u16)this->numeric_leftUserCal_y_plus->Value - (u16)this->numeric_leftUserCal_y_center->Value;
                encode_stick_params(user_stick_cal + 2, decoded_stick_pair);
                // -Axis X,Y
                decoded_stick_pair[0] = (u16)this->numeric_leftUserCal_x_center->Value - (u16)this->numeric_leftUserCal_x_minus->Value;
                decoded_stick_pair[1] = (u16)this->numeric_leftUserCal_y_center->Value - (u16)this->numeric_leftUserCal_y_minus->Value;
                encode_stick_params(user_stick_cal + 8, decoded_stick_pair);
            }
            else {
                // Erase left stick user cal
                memset(user_stick_cal, 0xFF, 11);
            }
            if (handle_ok != 1 && this->checkBox_enableRightUserCal->Checked) {
                *(u16*)&user_stick_cal[11] = 0xA1B2;
                // Center X,Y
                decoded_stick_pair[0] = (u16)this->numeric_rightUserCal_x_center->Value;
                decoded_stick_pair[1] = (u16)this->numeric_rightUserCal_y_center->Value;
                encode_stick_params(user_stick_cal + 13, decoded_stick_pair);
                // +Axis X,Y
                decoded_stick_pair[0] = (u16)this->numeric_rightUserCal_x_plus->Value - (u16)this->numeric_rightUserCal_x_center->Value;
                decoded_stick_pair[1] = (u16)this->numeric_rightUserCal_y_plus->Value - (u16)this->numeric_rightUserCal_y_center->Value;
                encode_stick_params(user_stick_cal + 19, decoded_stick_pair);
                // -Axis X,Y
                decoded_stick_pair[0] = (u16)this->numeric_rightUserCal_x_center->Value - (u16)this->numeric_rightUserCal_x_minus->Value;
                decoded_stick_pair[1] = (u16)this->numeric_rightUserCal_y_center->Value - (u16)this->numeric_rightUserCal_y_minus->Value;
                encode_stick_params(user_stick_cal + 16, decoded_stick_pair);
            }
            else {
                // Erase right stick user cal
                memset(&user_stick_cal[11], 0xFF, 11);
            }

            if (this->checkBox_enableSensorUserCal->Checked) {
                *(u16*)&user_sensor_cal[0] = 0xA1B2;
                *(u16*)&user_sensor_cal[2] = (u16)this->numeric_CalEditAccX->Value;
                *(u16*)&user_sensor_cal[4] = (u16)this->numeric_CalEditAccY->Value;
                *(u16*)&user_sensor_cal[6] = (u16)this->numeric_CalEditAccZ->Value;

                *(u16*)&user_sensor_cal[14] = (u16)this->numeric_CalEditGyroX->Value;
                *(u16*)&user_sensor_cal[16] = (u16)this->numeric_CalEditGyroY->Value;
                *(u16*)&user_sensor_cal[18] = (u16)this->numeric_CalEditGyroZ->Value;
            }
            else {
                // Erase user sensor cal
                memset(user_sensor_cal, 0xFF, sizeof(user_sensor_cal));
            }

            int res = write_spi_data(0x8010, sizeof(user_stick_cal), user_stick_cal);
            if (res == 0) {
                Sleep(100);
                res = write_spi_data(0x8026, sizeof(user_sensor_cal), user_sensor_cal);
            }
            if (res == 0)
                MessageBox::Show(L"The user calibration was written to SPI!", L"CTCaer's Joy-Con Toolkit - Write Success!", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
            else
                MessageBox::Show(L"Failed to write user calibration to SPI!\n\nPlease try again..", L"CTCaer's Joy-Con Toolkit - Write Failed!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
        }

    }


    private: System::Void btn_writeStickParams_Click(System::Object^  sender, System::EventArgs^  e) {
        if (check_if_connected())
            return;

        if (MessageBox::Show(L"Warning!\n\nThese are stick device parameters coming from factory.\n\nAre you sure you want to continue?",
            L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes) {
            u8 stick_model_main_left[3];
            u8 stick_model_pro_right[3];
            u16 decoded_stick_pair[4];

            memset(stick_model_main_left, 0, sizeof(stick_model_main_left));
            memset(stick_model_pro_right, 0, sizeof(stick_model_pro_right));
            memset(decoded_stick_pair, 0, sizeof(decoded_stick_pair));

            // Joy-Con's Main stick or Pro's Left Stick
            decoded_stick_pair[0] = (u16)this->numeric_StickParamDeadzone->Value;
            decoded_stick_pair[1] = (u16)this->numeric_StickParamRangeRatio->Value;
            encode_stick_params(stick_model_main_left, decoded_stick_pair);

            // Pro's Right Stick
            if (handle_ok == 3) {
                decoded_stick_pair[2] = (u16)this->numeric_StickParamDeadzone2->Value;
                decoded_stick_pair[3] = (u16)this->numeric_StickParamRangeRatio2->Value;
                encode_stick_params(stick_model_pro_right, decoded_stick_pair + 2);
            }

            int res = write_spi_data(0x6089, sizeof(stick_model_main_left), stick_model_main_left);
            if (res == 0 && handle_ok == 3) {
                Sleep(100);
                res = write_spi_data(0x609B, sizeof(stick_model_pro_right), stick_model_pro_right);
            }
            if (res == 0)
                MessageBox::Show(L"The Stick Device Parameters were written to SPI!", L"CTCaer's Joy-Con Toolkit - Write Success!", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
            else
                MessageBox::Show(L"Failed to Stick Device Parameters to SPI!\n\nPlease try again..", L"CTCaer's Joy-Con Toolkit - Write Failed!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
        }
    }


    private: System::Boolean check_if_connected() {
        if (!device_connection()) {
            MessageBox::Show(L"The device was disconnected!\n\n" +
                L"Press a button on the controller to re-connect\nand try again!",
                L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
            return true;
        }
        else
            return false;
    }
};
}


