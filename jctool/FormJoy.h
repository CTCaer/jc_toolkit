// Copyright (c) 2018 CTCaer. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once
#include <iomanip>
#include <sstream>

#include <msclr\marshal_cppstd.h>

#include "jctool.h"
#include "overrides.h"
#include "luts.h"

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
    public:
        static FormJoy^ myform1;

    FormJoy(void)
    {
        handler_close = 0;
        option_is_on = 0;
        set_led_busy();

        InitializeComponent();

        myform1 = this;

        this->btnWriteBody->Enabled = false;
        temp_celsius = true;

        full_refresh(false);

        /*
        BOOL chk = AllocConsole();
        if (chk)
        {
            freopen("CONOUT$", "w", stdout);
            printf(" printing to console\n");
        }
        */
        
        this->comboBox_rstOption->Items->Add("Restore Color");
        this->comboBox_rstOption->Items->Add("Restore S/N");
        this->comboBox_rstOption->Items->Add("Restore User Calibration");
        this->comboBox_rstOption->Items->Add("Factory Reset User Calibration");
        this->comboBox_rstOption->Items->Add("Full Restore");
        this->comboBox_rstOption->DrawItem +=
            gcnew System::Windows::Forms::DrawItemEventHandler(this, &FormJoy::comboBox_rstOption_DrawItem);
            
        this->menuStrip1->Renderer =
            gcnew System::Windows::Forms::ToolStripProfessionalRenderer(gcnew Overrides::TestColorTable());

        this->textBoxDbg_cmd->Validating += gcnew CancelEventHandler(this, &FormJoy::textBoxDbg_Validating);
        this->textBoxDbg_cmd->Validated += gcnew EventHandler(this, &FormJoy::textBoxDbg_Validated);
            
        this->textBoxDbg_subcmd->Validating += gcnew CancelEventHandler(this, &FormJoy::textBoxDbg_subcmd_Validating);
        this->textBoxDbg_subcmd->Validated += gcnew EventHandler(this, &FormJoy::textBoxDbg_subcmd_Validated);

        this->textBoxDbg_SubcmdArg->Validating += gcnew CancelEventHandler(this, &FormJoy::textBoxDbg_SubcmdArg_Validating);
        this->textBoxDbg_SubcmdArg->Validated += gcnew EventHandler(this, &FormJoy::textBoxDbg_SubcmdArg_Validated);

        this->textBoxDbg_lfamp->Validating += gcnew CancelEventHandler(this, &FormJoy::textBoxDbg_Validating);
        this->textBoxDbg_lfamp->Validated += gcnew EventHandler(this, &FormJoy::textBoxDbg_Validated);
            
        this->textBoxDbg_lfreq->Validating += gcnew CancelEventHandler(this, &FormJoy::textBoxDbg_Validating);
        this->textBoxDbg_lfreq->Validated += gcnew EventHandler(this, &FormJoy::textBoxDbg_Validated);
            
        this->textBoxDbg_hamp->Validating += gcnew CancelEventHandler(this, &FormJoy::textBoxDbg_Validating);
        this->textBoxDbg_hamp->Validated += gcnew EventHandler(this, &FormJoy::textBoxDbg_Validated);
            
        this->textBoxDbg_hfreq->Validating += gcnew CancelEventHandler(this, &FormJoy::textBoxDbg_Validating);
        this->textBoxDbg_hfreq->Validated += gcnew EventHandler(this, &FormJoy::textBoxDbg_Validated);

        this->textBox_chg_sn->Validating += gcnew CancelEventHandler(this, &FormJoy::textBox_chg_sn_Validating);
        this->textBox_chg_sn->Validated += gcnew EventHandler(this, &FormJoy::textBox_chg_sn_Validated);

        this->textBox_vib_loop_times->Validating += gcnew CancelEventHandler(this, &FormJoy::textBox_loop_Validating);
        this->textBox_vib_loop_times->Validated += gcnew EventHandler(this, &FormJoy::textBox_loop_Validated);

        this->toolStrip1->Renderer = gcnew Overrides::OverrideTSSR();

        this->toolTip1->SetToolTip(this->label_sn, L"Click here to change your S/N");
        this->toolTip1->SetToolTip(this->textBox_vib_loop_times,
            L"Set how many additional times the loop will be played.\n\nChoose a number from 0 to 999");
        this->toolTip1->SetToolTip(this->label_loop_times,
            L"Set how many additional times the loop will be played.\n\nChoose a number from 0 to 999");
            
        //Initialise locations on start for easy designing
        this->grpBox_DebugCmd->Location = System::Drawing::Point(494, 36);
        this->grpBox_Restore->Location = System::Drawing::Point(494, 36);
        this->grpBox_ChangeSN->Location = System::Drawing::Point(494, 36);
        this->grpBox_VibPlayer->Location = System::Drawing::Point(494, 36);
        this->grpBox_ButtonTest->Location = System::Drawing::Point(494, 36);

        this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
        reset_window_option(true);

        vib_file_type = 0;
        vib_sample_rate = 0;
        vib_samples = 0;
        vib_loop_start = 0;
        vib_loop_end = 0;
        vib_loop_wait = 0;
        disable_expert_mode = true;

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

    private: array<byte>^ backup_spi;
    public: array<byte>^ vib_loaded_file;
    public: array<byte>^ vib_file_converted;
    private: u16 vib_sample_rate;
    private: u32 vib_samples;
    private: u32 vib_loop_start;
    private: u32 vib_loop_end;
    private: u32 vib_loop_wait;
    private: int vib_converted;
    private: bool temp_celsius;
    //file type: 1 = Raw, 2 = bnvib (0x4), 3 = bnvib loop (0xC), 4 = bnvib loop (0x10)
    private: int vib_file_type;
    private: bool disable_expert_mode;
    private: float lf_gain;
    private: float lf_pitch;
    private: float hf_gain;
    private: float hf_pitch;
    private: Color jcBodyColor;
    private: Color jcButtonsColor;
    private: System::Windows::Forms::GroupBox^  groupBoxColor;
    private: System::Windows::Forms::Button^ btnWriteBody;
    private: System::Windows::Forms::TextBox^ textBoxSN;
    private: System::Windows::Forms::Label^ label_sn;
    private: System::Windows::Forms::Label^  label_mac;
    private: System::Windows::Forms::Label^  label_fw;
    private: System::Windows::Forms::TextBox^  textBoxMAC;
    private: System::Windows::Forms::TextBox^  textBoxFW;
    private: System::Windows::Forms::TextBox^  textBoxDev;
    private: System::Windows::Forms::Label^  label_dev;
    private: System::Windows::Forms::Button^  btn_makeBackup;
    private: System::Windows::Forms::GroupBox^  groupBoxSPI;
    private: System::Windows::Forms::Label^  lbl_spiProggressDesc;
    private: System::Windows::Forms::PictureBox^  pictureBoxPreview;
    public: System::Windows::Forms::Label^  label_progress;
    private: System::Windows::Forms::MenuStrip^  menuStrip1;
    private: System::Windows::Forms::ToolStripMenuItem^  menuToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  debugToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  aboutToolStripMenuItem;
    private: System::Windows::Forms::Label^  lbl_subcmdArgs;
    private: System::Windows::Forms::TextBox^  textBoxDbg_SubcmdArg;
    private: System::Windows::Forms::Button^  btnDbg_send_cmd;
    private: System::Windows::Forms::Label^  lbl_cmd;
    private: System::Windows::Forms::Label^  lbl_subcmd;
    private: System::Windows::Forms::TextBox^  textBoxDbg_cmd;
    private: System::Windows::Forms::TextBox^  textBoxDbg_subcmd;
    private: System::Windows::Forms::Label^  lbl_dbgDisclaimer;
    private: System::Windows::Forms::GroupBox^  grpBox_Restore;
    private: System::Windows::Forms::Button^  btnLoadBackup;
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
    private: System::Windows::Forms::Button^  btnChangeSn;
    private: System::Windows::Forms::TextBox^  textBox_chg_sn;
    private: System::Windows::Forms::Label^  lbl_loadedMAC;
    private: System::Windows::Forms::Label^  label_sn_change_warning;
    private: System::Windows::Forms::ToolTip^  toolTip1;
    public: System::Windows::Forms::TextBox^  textBoxDbg_sent;
    public: System::Windows::Forms::TextBox^  textBoxDbg_reply;
    private: System::Windows::Forms::GroupBox^  grpBox_VibPlayer;
    private: System::Windows::Forms::Button^  btnVibPlay;
    private: System::Windows::Forms::Button^  btnLoadVib;
    private: System::Windows::Forms::Label^  label_vib_loaded;
    private: System::Windows::Forms::Label^  label_samplerate;
    public: System::Windows::Forms::Label^  label_samples;
    private: System::Windows::Forms::ToolStripMenuItem^  hDRumblePlayerToolStripMenuItem;
    private: System::Windows::Forms::Label^  label_hdrumble_filename;
    public: System::Windows::Forms::TextBox^  textBoxDbg_reply_cmd;
    private: System::ComponentModel::IContainer^  components;
    private: System::Windows::Forms::TrackBar^  trackBar_lf_amp;
    private: System::Windows::Forms::TrackBar^  trackBar_hf_freq;
    private: System::Windows::Forms::TrackBar^  trackBar_hf_amp;
    private: System::Windows::Forms::TrackBar^  trackBar_lf_freq;
    private: System::Windows::Forms::Label^  label_eq_info;
    private: System::Windows::Forms::Button^  btnVib_reset_eq;
    private: System::Windows::Forms::GroupBox^  groupBox_vib_eq;
    private: System::Windows::Forms::Button^  btn_enable_expert_mode;
    private: System::Windows::Forms::Label^  label_loop_times;
    private: System::Windows::Forms::Button^  btnRestore_SN;
    private: System::Windows::Forms::GroupBox^  groupBox_vib_info;
    private: System::Windows::Forms::TextBox^  textBox_vib_loop_times;
    private: System::Windows::Forms::GroupBox^  grpBox_ButtonTest;
    public: System::Windows::Forms::TextBox^  textBox_btn_test_reply;
    public: System::Windows::Forms::TextBox^  textBox_btn_test_subreply;
    private: System::Windows::Forms::ToolStripMenuItem^  buttonTestToolStripMenuItem;
    private: System::Windows::Forms::Button^  btn_run_btn_test;
    private: System::Windows::Forms::GroupBox^  grpBox_StickCal;
    public: System::Windows::Forms::TextBox^  textBox_lstick_ucal;
    public: System::Windows::Forms::TextBox^  textBox_rstick_ucal;
    public: System::Windows::Forms::TextBox^  textBox_lstick_fcal;
    public: System::Windows::Forms::TextBox^  textBox_rstick_fcal;
    public: System::Windows::Forms::TextBox^  textBox_device_parameters;
    public: System::Windows::Forms::TextBox^  textBox_6axis_cal;
    public: System::Windows::Forms::TextBox^  textBox_6axis_ucal;
    public: System::Windows::Forms::TextBox^  textBox_device_parameters2;
    private: System::Windows::Forms::GroupBox^  grpBox_dev_param;
    private: System::Windows::Forms::Button^  btn_spi_cancel;
    private: System::Windows::Forms::ToolStrip^  toolStrip1;
    private: System::Windows::Forms::ToolStripLabel^  toolStripLabel_temp;
    private: System::Windows::Forms::ToolStripButton^  toolStripBtn_refresh;
    private: System::Windows::Forms::ToolStripButton^  toolStripBtn_batt;
    private: System::Windows::Forms::ToolStripLabel^  toolStripLabel_batt;
    private: System::Windows::Forms::ToolStripButton^  toolStripBtn_Disconnect;
    private: System::Windows::Forms::Button^  btn_change_color;
    private: System::Windows::Forms::Panel^  panel_filler;
    private: jcColor::JoyConColorPicker^ JCColorPicker;
    private: System::Windows::Forms::Label^  lbl_Buttons_hex_txt;
    private: System::Windows::Forms::Label^  lbl_Body_hex_txt;
    private: System::Windows::Forms::GroupBox^  grpBox_accGyroCal;
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
            this->btnWriteBody = (gcnew System::Windows::Forms::Button());
            this->groupBoxColor = (gcnew System::Windows::Forms::GroupBox());
            this->lbl_Buttons_hex_txt = (gcnew System::Windows::Forms::Label());
            this->lbl_Body_hex_txt = (gcnew System::Windows::Forms::Label());
            this->btn_RestoreEnable = (gcnew System::Windows::Forms::Button());
            this->pictureBoxPreview = (gcnew System::Windows::Forms::PictureBox());
            this->btn_change_color = (gcnew System::Windows::Forms::Button());
            this->btn_makeBackup = (gcnew System::Windows::Forms::Button());
            this->groupBoxSPI = (gcnew System::Windows::Forms::GroupBox());
            this->btn_spi_cancel = (gcnew System::Windows::Forms::Button());
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
            this->menuToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->buttonTestToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->debugToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->aboutToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->lbl_subcmdArgs = (gcnew System::Windows::Forms::Label());
            this->textBoxDbg_SubcmdArg = (gcnew System::Windows::Forms::TextBox());
            this->btnDbg_send_cmd = (gcnew System::Windows::Forms::Button());
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
            this->btnLoadBackup = (gcnew System::Windows::Forms::Button());
            this->lbl_rstDisclaimer = (gcnew System::Windows::Forms::Label());
            this->errorProvider1 = (gcnew System::Windows::Forms::ErrorProvider(this->components));
            this->errorProvider2 = (gcnew System::Windows::Forms::ErrorProvider(this->components));
            this->grpBox_ChangeSN = (gcnew System::Windows::Forms::GroupBox());
            this->btnRestore_SN = (gcnew System::Windows::Forms::Button());
            this->lbl_loadedMAC = (gcnew System::Windows::Forms::Label());
            this->label_sn_change_warning = (gcnew System::Windows::Forms::Label());
            this->btnChangeSn = (gcnew System::Windows::Forms::Button());
            this->textBox_chg_sn = (gcnew System::Windows::Forms::TextBox());
            this->toolTip1 = (gcnew System::Windows::Forms::ToolTip(this->components));
            this->grpBox_VibPlayer = (gcnew System::Windows::Forms::GroupBox());
            this->groupBox_vib_info = (gcnew System::Windows::Forms::GroupBox());
            this->textBox_vib_loop_times = (gcnew System::Windows::Forms::TextBox());
            this->label_hdrumble_filename = (gcnew System::Windows::Forms::Label());
            this->label_loop_times = (gcnew System::Windows::Forms::Label());
            this->label_vib_loaded = (gcnew System::Windows::Forms::Label());
            this->label_samplerate = (gcnew System::Windows::Forms::Label());
            this->label_samples = (gcnew System::Windows::Forms::Label());
            this->groupBox_vib_eq = (gcnew System::Windows::Forms::GroupBox());
            this->btnVib_reset_eq = (gcnew System::Windows::Forms::Button());
            this->label_eq_info = (gcnew System::Windows::Forms::Label());
            this->trackBar_hf_amp = (gcnew System::Windows::Forms::TrackBar());
            this->trackBar_lf_amp = (gcnew System::Windows::Forms::TrackBar());
            this->trackBar_hf_freq = (gcnew System::Windows::Forms::TrackBar());
            this->trackBar_lf_freq = (gcnew System::Windows::Forms::TrackBar());
            this->btnVibPlay = (gcnew System::Windows::Forms::Button());
            this->btnLoadVib = (gcnew System::Windows::Forms::Button());
            this->btn_enable_expert_mode = (gcnew System::Windows::Forms::Button());
            this->textBox_btn_test_reply = (gcnew System::Windows::Forms::TextBox());
            this->textBox_btn_test_subreply = (gcnew System::Windows::Forms::TextBox());
            this->grpBox_ButtonTest = (gcnew System::Windows::Forms::GroupBox());
            this->btn_run_btn_test = (gcnew System::Windows::Forms::Button());
            this->grpBox_StickCal = (gcnew System::Windows::Forms::GroupBox());
            this->textBox_lstick_fcal = (gcnew System::Windows::Forms::TextBox());
            this->textBox_rstick_fcal = (gcnew System::Windows::Forms::TextBox());
            this->textBox_rstick_ucal = (gcnew System::Windows::Forms::TextBox());
            this->textBox_lstick_ucal = (gcnew System::Windows::Forms::TextBox());
            this->textBox_6axis_ucal = (gcnew System::Windows::Forms::TextBox());
            this->textBox_6axis_cal = (gcnew System::Windows::Forms::TextBox());
            this->textBox_device_parameters = (gcnew System::Windows::Forms::TextBox());
            this->textBox_device_parameters2 = (gcnew System::Windows::Forms::TextBox());
            this->grpBox_dev_param = (gcnew System::Windows::Forms::GroupBox());
            this->toolStrip1 = (gcnew System::Windows::Forms::ToolStrip());
            this->toolStripBtn_batt = (gcnew System::Windows::Forms::ToolStripButton());
            this->toolStripLabel_batt = (gcnew System::Windows::Forms::ToolStripLabel());
            this->toolStripLabel_temp = (gcnew System::Windows::Forms::ToolStripLabel());
            this->toolStripBtn_refresh = (gcnew System::Windows::Forms::ToolStripButton());
            this->toolStripBtn_Disconnect = (gcnew System::Windows::Forms::ToolStripButton());
            this->panel_filler = (gcnew System::Windows::Forms::Panel());
            this->grpBox_accGyroCal = (gcnew System::Windows::Forms::GroupBox());
            this->groupBoxColor->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxPreview))->BeginInit();
            this->groupBoxSPI->SuspendLayout();
            this->menuStrip1->SuspendLayout();
            this->grpBox_DebugCmd->SuspendLayout();
            this->grpBox_Restore->SuspendLayout();
            this->grpBox_RstUser->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->errorProvider1))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->errorProvider2))->BeginInit();
            this->grpBox_ChangeSN->SuspendLayout();
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
            this->SuspendLayout();
            // 
            // btnWriteBody
            // 
            this->btnWriteBody->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btnWriteBody->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btnWriteBody->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnWriteBody->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->btnWriteBody->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->btnWriteBody->Location = System::Drawing::Point(316, 252);
            this->btnWriteBody->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->btnWriteBody->Name = L"btnWriteBody";
            this->btnWriteBody->Size = System::Drawing::Size(128, 48);
            this->btnWriteBody->TabIndex = 1;
            this->btnWriteBody->Text = L"Write Colors";
            this->btnWriteBody->UseVisualStyleBackColor = false;
            this->btnWriteBody->Click += gcnew System::EventHandler(this, &FormJoy::btnWriteBody_Click);
            // 
            // groupBoxColor
            // 
            this->groupBoxColor->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->groupBoxColor->Controls->Add(this->lbl_Buttons_hex_txt);
            this->groupBoxColor->Controls->Add(this->lbl_Body_hex_txt);
            this->groupBoxColor->Controls->Add(this->btn_RestoreEnable);
            this->groupBoxColor->Controls->Add(this->pictureBoxPreview);
            this->groupBoxColor->Controls->Add(this->btn_change_color);
            this->groupBoxColor->Controls->Add(this->btn_makeBackup);
            this->groupBoxColor->Controls->Add(this->btnWriteBody);
            this->groupBoxColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->groupBoxColor->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->groupBoxColor->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->groupBoxColor->Location = System::Drawing::Point(14, 120);
            this->groupBoxColor->Margin = System::Windows::Forms::Padding(0, 0, 14, 39);
            this->groupBoxColor->Name = L"groupBoxColor";
            this->groupBoxColor->Padding = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->groupBoxColor->Size = System::Drawing::Size(456, 315);
            this->groupBoxColor->TabIndex = 0;
            this->groupBoxColor->TabStop = false;
            this->groupBoxColor->Text = L"Device colors";
            // 
            // lbl_Buttons_hex_txt
            // 
            this->lbl_Buttons_hex_txt->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->lbl_Buttons_hex_txt->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_Buttons_hex_txt->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(150)),
                static_cast<System::Int32>(static_cast<System::Byte>(150)), static_cast<System::Int32>(static_cast<System::Byte>(150)));
            this->lbl_Buttons_hex_txt->Location = System::Drawing::Point(316, 183);
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
            this->lbl_Body_hex_txt->Location = System::Drawing::Point(316, 159);
            this->lbl_Body_hex_txt->Margin = System::Windows::Forms::Padding(0);
            this->lbl_Body_hex_txt->Name = L"lbl_Body_hex_txt";
            this->lbl_Body_hex_txt->Size = System::Drawing::Size(128, 24);
            this->lbl_Body_hex_txt->TabIndex = 20;
            this->lbl_Body_hex_txt->Text = L"#Body Color";
            this->lbl_Body_hex_txt->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
            // 
            // btn_RestoreEnable
            // 
            this->btn_RestoreEnable->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
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
            // btn_change_color
            // 
            this->btn_change_color->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_change_color->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_change_color->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_change_color->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 9.75F));
            this->btn_change_color->Location = System::Drawing::Point(316, 66);
            this->btn_change_color->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->btn_change_color->Name = L"btn_change_color";
            this->btn_change_color->Size = System::Drawing::Size(128, 48);
            this->btn_change_color->TabIndex = 19;
            this->btn_change_color->Text = L"Change Color";
            this->btn_change_color->UseVisualStyleBackColor = false;
            this->btn_change_color->Click += gcnew System::EventHandler(this, &FormJoy::btn_change_color_Click);
            // 
            // btn_makeBackup
            // 
            this->btn_makeBackup->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_makeBackup->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_makeBackup->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_makeBackup->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 9.75F, System::Drawing::FontStyle::Bold));
            this->btn_makeBackup->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->btn_makeBackup->Location = System::Drawing::Point(169, 25);
            this->btn_makeBackup->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->btn_makeBackup->Name = L"btn_makeBackup";
            this->btn_makeBackup->Size = System::Drawing::Size(144, 36);
            this->btn_makeBackup->TabIndex = 1;
            this->btn_makeBackup->Text = L"Backup SPI";
            this->btn_makeBackup->UseVisualStyleBackColor = false;
            this->btn_makeBackup->Click += gcnew System::EventHandler(this, &FormJoy::btn_makeBackup_Click);
            // 
            // groupBoxSPI
            // 
            this->groupBoxSPI->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->groupBoxSPI->Controls->Add(this->btn_spi_cancel);
            this->groupBoxSPI->Controls->Add(this->label_progress);
            this->groupBoxSPI->Controls->Add(this->lbl_spiProggressDesc);
            this->groupBoxSPI->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->groupBoxSPI->Location = System::Drawing::Point(14, 120);
            this->groupBoxSPI->Margin = System::Windows::Forms::Padding(0, 0, 14, 39);
            this->groupBoxSPI->Name = L"groupBoxSPI";
            this->groupBoxSPI->Padding = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->groupBoxSPI->Size = System::Drawing::Size(456, 315);
            this->groupBoxSPI->TabIndex = 14;
            this->groupBoxSPI->TabStop = false;
            // 
            // btn_spi_cancel
            // 
            this->btn_spi_cancel->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_spi_cancel->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->btn_spi_cancel->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_spi_cancel->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btn_spi_cancel->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->btn_spi_cancel->Location = System::Drawing::Point(187, 250);
            this->btn_spi_cancel->Name = L"btn_spi_cancel";
            this->btn_spi_cancel->Size = System::Drawing::Size(75, 34);
            this->btn_spi_cancel->TabIndex = 35;
            this->btn_spi_cancel->Text = L"Cancel";
            this->btn_spi_cancel->UseVisualStyleBackColor = false;
            this->btn_spi_cancel->Click += gcnew System::EventHandler(this, &FormJoy::btn_spi_cancel_Click);
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
            this->lbl_spiProggressDesc->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->lbl_spiProggressDesc->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_spiProggressDesc->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)));
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
            this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {
                this->hDRumblePlayerToolStripMenuItem,
                    this->menuToolStripMenuItem
            });
            this->menuStrip1->Location = System::Drawing::Point(0, 0);
            this->menuStrip1->Name = L"menuStrip1";
            this->menuStrip1->Size = System::Drawing::Size(1662, 25);
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
            // menuToolStripMenuItem
            // 
            this->menuToolStripMenuItem->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(55)),
                static_cast<System::Int32>(static_cast<System::Byte>(55)), static_cast<System::Int32>(static_cast<System::Byte>(55)));
            this->menuToolStripMenuItem->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->menuToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {
                this->buttonTestToolStripMenuItem,
                    this->debugToolStripMenuItem, this->aboutToolStripMenuItem
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
            this->buttonTestToolStripMenuItem->Size = System::Drawing::Size(138, 22);
            this->buttonTestToolStripMenuItem->Text = L"Button test";
            this->buttonTestToolStripMenuItem->Click += gcnew System::EventHandler(this, &FormJoy::buttonTestToolStripMenuItem_Click);
            // 
            // debugToolStripMenuItem
            // 
            this->debugToolStripMenuItem->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->debugToolStripMenuItem->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->debugToolStripMenuItem->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->debugToolStripMenuItem->Name = L"debugToolStripMenuItem";
            this->debugToolStripMenuItem->Size = System::Drawing::Size(138, 22);
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
            this->aboutToolStripMenuItem->Size = System::Drawing::Size(138, 22);
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
            this->lbl_subcmdArgs->Location = System::Drawing::Point(13, 120);
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
            this->textBoxDbg_SubcmdArg->Location = System::Drawing::Point(16, 140);
            this->textBoxDbg_SubcmdArg->Margin = System::Windows::Forms::Padding(0);
            this->textBoxDbg_SubcmdArg->MaxLength = 50;
            this->textBoxDbg_SubcmdArg->Multiline = true;
            this->textBoxDbg_SubcmdArg->Name = L"textBoxDbg_SubcmdArg";
            this->textBoxDbg_SubcmdArg->Size = System::Drawing::Size(188, 38);
            this->textBoxDbg_SubcmdArg->TabIndex = 25;
            this->textBoxDbg_SubcmdArg->Text = L"00";
            this->textBoxDbg_SubcmdArg->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            // 
            // btnDbg_send_cmd
            // 
            this->btnDbg_send_cmd->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btnDbg_send_cmd->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->btnDbg_send_cmd->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnDbg_send_cmd->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btnDbg_send_cmd->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->btnDbg_send_cmd->Location = System::Drawing::Point(73, 194);
            this->btnDbg_send_cmd->Name = L"btnDbg_send_cmd";
            this->btnDbg_send_cmd->Size = System::Drawing::Size(75, 34);
            this->btnDbg_send_cmd->TabIndex = 17;
            this->btnDbg_send_cmd->Text = L"Send";
            this->btnDbg_send_cmd->UseVisualStyleBackColor = false;
            this->btnDbg_send_cmd->Click += gcnew System::EventHandler(this, &FormJoy::btnDbg_send_cmd_Click);
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
            this->textBoxDbg_cmd->ForeColor = System::Drawing::Color::White;
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
            this->lbl_dbgDisclaimer->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->lbl_dbgDisclaimer->Location = System::Drawing::Point(20, 275);
            this->lbl_dbgDisclaimer->Name = L"lbl_dbgDisclaimer";
            this->lbl_dbgDisclaimer->Size = System::Drawing::Size(178, 97);
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
            this->grpBox_DebugCmd->Controls->Add(this->btnDbg_send_cmd);
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
            this->textBoxDbg_reply_cmd->Location = System::Drawing::Point(20, 280);
            this->textBoxDbg_reply_cmd->Margin = System::Windows::Forms::Padding(0);
            this->textBoxDbg_reply_cmd->Multiline = true;
            this->textBoxDbg_reply_cmd->Name = L"textBoxDbg_reply_cmd";
            this->textBoxDbg_reply_cmd->ReadOnly = true;
            this->textBoxDbg_reply_cmd->Size = System::Drawing::Size(178, 45);
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
            this->textBoxDbg_reply->Location = System::Drawing::Point(20, 325);
            this->textBoxDbg_reply->Margin = System::Windows::Forms::Padding(0);
            this->textBoxDbg_reply->Multiline = true;
            this->textBoxDbg_reply->Name = L"textBoxDbg_reply";
            this->textBoxDbg_reply->ReadOnly = true;
            this->textBoxDbg_reply->Size = System::Drawing::Size(178, 69);
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
            this->textBoxDbg_sent->Location = System::Drawing::Point(20, 237);
            this->textBoxDbg_sent->Margin = System::Windows::Forms::Padding(0);
            this->textBoxDbg_sent->Multiline = true;
            this->textBoxDbg_sent->Name = L"textBoxDbg_sent";
            this->textBoxDbg_sent->ReadOnly = true;
            this->textBoxDbg_sent->Size = System::Drawing::Size(178, 47);
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
            this->grpBox_Restore->Controls->Add(this->btnLoadBackup);
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
            this->comboBox_rstOption->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->comboBox_rstOption->DrawMode = System::Windows::Forms::DrawMode::OwnerDrawFixed;
            this->comboBox_rstOption->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->comboBox_rstOption->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
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
            this->btn_restore->Location = System::Drawing::Point(136, 359);
            this->btn_restore->Name = L"btn_restore";
            this->btn_restore->Size = System::Drawing::Size(75, 30);
            this->btn_restore->TabIndex = 6;
            this->btn_restore->Text = L"Restore";
            this->btn_restore->UseVisualStyleBackColor = false;
            this->btn_restore->Visible = false;
            this->btn_restore->Click += gcnew System::EventHandler(this, &FormJoy::btn_restore_Click);
            // 
            // txtBox_fileLoaded
            // 
            this->txtBox_fileLoaded->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
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
            // btnLoadBackup
            // 
            this->btnLoadBackup->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btnLoadBackup->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btnLoadBackup->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnLoadBackup->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->btnLoadBackup->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->btnLoadBackup->Location = System::Drawing::Point(8, 22);
            this->btnLoadBackup->Name = L"btnLoadBackup";
            this->btnLoadBackup->Size = System::Drawing::Size(94, 32);
            this->btnLoadBackup->TabIndex = 0;
            this->btnLoadBackup->Text = L"Load Backup";
            this->btnLoadBackup->UseVisualStyleBackColor = false;
            this->btnLoadBackup->Click += gcnew System::EventHandler(this, &FormJoy::btnLoadBackup_Click);
            // 
            // lbl_rstDisclaimer
            // 
            this->lbl_rstDisclaimer->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_rstDisclaimer->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
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
            this->grpBox_ChangeSN->Controls->Add(this->btnRestore_SN);
            this->grpBox_ChangeSN->Controls->Add(this->lbl_loadedMAC);
            this->grpBox_ChangeSN->Controls->Add(this->label_sn_change_warning);
            this->grpBox_ChangeSN->Controls->Add(this->btnChangeSn);
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
            // btnRestore_SN
            // 
            this->btnRestore_SN->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btnRestore_SN->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btnRestore_SN->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnRestore_SN->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btnRestore_SN->Location = System::Drawing::Point(17, 359);
            this->btnRestore_SN->Name = L"btnRestore_SN";
            this->btnRestore_SN->Size = System::Drawing::Size(87, 30);
            this->btnRestore_SN->TabIndex = 34;
            this->btnRestore_SN->Text = L"Restore";
            this->btnRestore_SN->UseVisualStyleBackColor = false;
            this->btnRestore_SN->Click += gcnew System::EventHandler(this, &FormJoy::btnRestore_SN_Click);
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
            // btnChangeSn
            // 
            this->btnChangeSn->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btnChangeSn->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btnChangeSn->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnChangeSn->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btnChangeSn->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->btnChangeSn->Location = System::Drawing::Point(116, 359);
            this->btnChangeSn->Name = L"btnChangeSn";
            this->btnChangeSn->Size = System::Drawing::Size(87, 30);
            this->btnChangeSn->TabIndex = 1;
            this->btnChangeSn->Text = L"Change";
            this->btnChangeSn->UseVisualStyleBackColor = false;
            this->btnChangeSn->Click += gcnew System::EventHandler(this, &FormJoy::btnChangeSn_Click);
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
            // grpBox_VibPlayer
            // 
            this->grpBox_VibPlayer->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_VibPlayer->Controls->Add(this->groupBox_vib_info);
            this->grpBox_VibPlayer->Controls->Add(this->groupBox_vib_eq);
            this->grpBox_VibPlayer->Controls->Add(this->btnVibPlay);
            this->grpBox_VibPlayer->Controls->Add(this->btnLoadVib);
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
            this->groupBox_vib_eq->Controls->Add(this->btnVib_reset_eq);
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
            // btnVib_reset_eq
            // 
            this->btnVib_reset_eq->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btnVib_reset_eq->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btnVib_reset_eq->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnVib_reset_eq->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->btnVib_reset_eq->Location = System::Drawing::Point(66, 27);
            this->btnVib_reset_eq->Name = L"btnVib_reset_eq";
            this->btnVib_reset_eq->Size = System::Drawing::Size(75, 30);
            this->btnVib_reset_eq->TabIndex = 11;
            this->btnVib_reset_eq->Text = L"Reset EQ";
            this->btnVib_reset_eq->UseVisualStyleBackColor = false;
            this->btnVib_reset_eq->Click += gcnew System::EventHandler(this, &FormJoy::btnVib_reset_eq_Click);
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
            this->trackBar_hf_amp->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->trackBar_hf_amp->LargeChange = 1;
            this->trackBar_hf_amp->Location = System::Drawing::Point(113, 91);
            this->trackBar_hf_amp->Maximum = 20;
            this->trackBar_hf_amp->Name = L"trackBar_hf_amp";
            this->trackBar_hf_amp->Orientation = System::Windows::Forms::Orientation::Vertical;
            this->trackBar_hf_amp->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->trackBar_hf_amp->Size = System::Drawing::Size(45, 104);
            this->trackBar_hf_amp->TabIndex = 8;
            this->trackBar_hf_amp->TickStyle = System::Windows::Forms::TickStyle::TopLeft;
            this->trackBar_hf_amp->ValueChanged += gcnew System::EventHandler(this, &FormJoy::TrackBar_ValueChanged);
            // 
            // trackBar_lf_amp
            // 
            this->trackBar_lf_amp->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->trackBar_lf_amp->LargeChange = 1;
            this->trackBar_lf_amp->Location = System::Drawing::Point(15, 91);
            this->trackBar_lf_amp->Maximum = 20;
            this->trackBar_lf_amp->Name = L"trackBar_lf_amp";
            this->trackBar_lf_amp->Orientation = System::Windows::Forms::Orientation::Vertical;
            this->trackBar_lf_amp->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->trackBar_lf_amp->Size = System::Drawing::Size(45, 104);
            this->trackBar_lf_amp->TabIndex = 6;
            this->trackBar_lf_amp->TickStyle = System::Windows::Forms::TickStyle::TopLeft;
            this->trackBar_lf_amp->ValueChanged += gcnew System::EventHandler(this, &FormJoy::TrackBar_ValueChanged);
            // 
            // trackBar_hf_freq
            // 
            this->trackBar_hf_freq->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->trackBar_hf_freq->LargeChange = 1;
            this->trackBar_hf_freq->Location = System::Drawing::Point(162, 91);
            this->trackBar_hf_freq->Maximum = 20;
            this->trackBar_hf_freq->Name = L"trackBar_hf_freq";
            this->trackBar_hf_freq->Orientation = System::Windows::Forms::Orientation::Vertical;
            this->trackBar_hf_freq->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->trackBar_hf_freq->Size = System::Drawing::Size(45, 104);
            this->trackBar_hf_freq->TabIndex = 9;
            this->trackBar_hf_freq->TickStyle = System::Windows::Forms::TickStyle::TopLeft;
            this->trackBar_hf_freq->ValueChanged += gcnew System::EventHandler(this, &FormJoy::TrackBar_ValueChanged);
            // 
            // trackBar_lf_freq
            // 
            this->trackBar_lf_freq->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->trackBar_lf_freq->LargeChange = 1;
            this->trackBar_lf_freq->Location = System::Drawing::Point(64, 91);
            this->trackBar_lf_freq->Maximum = 20;
            this->trackBar_lf_freq->Name = L"trackBar_lf_freq";
            this->trackBar_lf_freq->Orientation = System::Windows::Forms::Orientation::Vertical;
            this->trackBar_lf_freq->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->trackBar_lf_freq->Size = System::Drawing::Size(45, 104);
            this->trackBar_lf_freq->TabIndex = 7;
            this->trackBar_lf_freq->TickStyle = System::Windows::Forms::TickStyle::TopLeft;
            this->trackBar_lf_freq->ValueChanged += gcnew System::EventHandler(this, &FormJoy::TrackBar_ValueChanged);
            // 
            // btnVibPlay
            // 
            this->btnVibPlay->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btnVibPlay->Enabled = false;
            this->btnVibPlay->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btnVibPlay->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnVibPlay->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 10));
            this->btnVibPlay->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
                static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->btnVibPlay->Location = System::Drawing::Point(113, 26);
            this->btnVibPlay->Name = L"btnVibPlay";
            this->btnVibPlay->Size = System::Drawing::Size(99, 32);
            this->btnVibPlay->TabIndex = 1;
            this->btnVibPlay->Text = L"Play";
            this->btnVibPlay->UseVisualStyleBackColor = false;
            this->btnVibPlay->Click += gcnew System::EventHandler(this, &FormJoy::btnVibPlay_Click);
            // 
            // btnLoadVib
            // 
            this->btnLoadVib->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btnLoadVib->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btnLoadVib->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btnLoadVib->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->btnLoadVib->Location = System::Drawing::Point(8, 26);
            this->btnLoadVib->Name = L"btnLoadVib";
            this->btnLoadVib->Size = System::Drawing::Size(99, 32);
            this->btnLoadVib->TabIndex = 0;
            this->btnLoadVib->Text = L"Load Rumble";
            this->btnLoadVib->UseVisualStyleBackColor = false;
            this->btnLoadVib->Click += gcnew System::EventHandler(this, &FormJoy::btnLoadVib_Click);
            // 
            // btn_enable_expert_mode
            // 
            this->btn_enable_expert_mode->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->btn_enable_expert_mode->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->btn_enable_expert_mode->FlatAppearance->BorderSize = 0;
            this->btn_enable_expert_mode->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_enable_expert_mode->Location = System::Drawing::Point(500, 438);
            this->btn_enable_expert_mode->Margin = System::Windows::Forms::Padding(0);
            this->btn_enable_expert_mode->Name = L"btn_enable_expert_mode";
            this->btn_enable_expert_mode->Size = System::Drawing::Size(10, 8);
            this->btn_enable_expert_mode->TabIndex = 27;
            this->btn_enable_expert_mode->UseVisualStyleBackColor = false;
            this->btn_enable_expert_mode->Click += gcnew System::EventHandler(this, &FormJoy::btn_enable_expert_mode_Click);
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
            this->textBox_btn_test_reply->Size = System::Drawing::Size(205, 160);
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
            this->textBox_btn_test_subreply->Size = System::Drawing::Size(205, 107);
            this->textBox_btn_test_subreply->TabIndex = 35;
            this->textBox_btn_test_subreply->TabStop = false;
            this->textBox_btn_test_subreply->Text = L"Acc/meter (Raw/Cal):\r\nX: \r\nY: \r\nX: \r\n\r\nGyroscope (Raw/Cal):\r\nX: \r\nY: \r\nZ: ";
            // 
            // grpBox_ButtonTest
            // 
            this->grpBox_ButtonTest->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_ButtonTest->Controls->Add(this->btn_run_btn_test);
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
            // btn_run_btn_test
            // 
            this->btn_run_btn_test->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_run_btn_test->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
                static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
            this->btn_run_btn_test->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->btn_run_btn_test->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->btn_run_btn_test->Location = System::Drawing::Point(77, 349);
            this->btn_run_btn_test->Name = L"btn_run_btn_test";
            this->btn_run_btn_test->Size = System::Drawing::Size(75, 27);
            this->btn_run_btn_test->TabIndex = 36;
            this->btn_run_btn_test->Text = L"Turn on";
            this->btn_run_btn_test->UseVisualStyleBackColor = false;
            this->btn_run_btn_test->Click += gcnew System::EventHandler(this, &FormJoy::btn_run_btn_test_Click);
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
            this->grpBox_StickCal->Size = System::Drawing::Size(218, 218);
            this->grpBox_StickCal->TabIndex = 37;
            this->grpBox_StickCal->TabStop = false;
            this->grpBox_StickCal->Text = L"Stick Calibration";
            // 
            // textBox_lstick_fcal
            // 
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
            this->textBox_lstick_fcal->Size = System::Drawing::Size(207, 35);
            this->textBox_lstick_fcal->TabIndex = 40;
            this->textBox_lstick_fcal->TabStop = false;
            this->textBox_lstick_fcal->Text = L"L Stick Factory:\r\nCenter X,Y: (   ,    )\r\nX: [    -    ] Y: [    -    ]";
            // 
            // textBox_rstick_fcal
            // 
            this->textBox_rstick_fcal->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBox_rstick_fcal->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBox_rstick_fcal->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F));
            this->textBox_rstick_fcal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->textBox_rstick_fcal->Location = System::Drawing::Point(6, 123);
            this->textBox_rstick_fcal->Margin = System::Windows::Forms::Padding(0);
            this->textBox_rstick_fcal->Multiline = true;
            this->textBox_rstick_fcal->Name = L"textBox_rstick_fcal";
            this->textBox_rstick_fcal->ReadOnly = true;
            this->textBox_rstick_fcal->Size = System::Drawing::Size(207, 35);
            this->textBox_rstick_fcal->TabIndex = 39;
            this->textBox_rstick_fcal->TabStop = false;
            this->textBox_rstick_fcal->Text = L"R Stick Factory:\r\nCenter X,Y: (   ,    )\r\nX: [    -    ] Y: [    -    ]";
            // 
            // textBox_rstick_ucal
            // 
            this->textBox_rstick_ucal->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBox_rstick_ucal->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBox_rstick_ucal->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F));
            this->textBox_rstick_ucal->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
            this->textBox_rstick_ucal->Location = System::Drawing::Point(6, 168);
            this->textBox_rstick_ucal->Margin = System::Windows::Forms::Padding(0);
            this->textBox_rstick_ucal->Multiline = true;
            this->textBox_rstick_ucal->Name = L"textBox_rstick_ucal";
            this->textBox_rstick_ucal->ReadOnly = true;
            this->textBox_rstick_ucal->Size = System::Drawing::Size(207, 35);
            this->textBox_rstick_ucal->TabIndex = 38;
            this->textBox_rstick_ucal->TabStop = false;
            this->textBox_rstick_ucal->Text = L"R Stick User:\r\nCenter X,Y: (   ,    )\r\nX: [    -    ] Y: [    -    ]";
            // 
            // textBox_lstick_ucal
            // 
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
            this->textBox_lstick_ucal->Size = System::Drawing::Size(207, 35);
            this->textBox_lstick_ucal->TabIndex = 37;
            this->textBox_lstick_ucal->TabStop = false;
            this->textBox_lstick_ucal->Text = L"L Stick User:\r\nCenter X,Y: (   ,    )\r\nX: [    -    ] Y: [    -    ]";
            // 
            // textBox_6axis_ucal
            // 
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
            this->textBox_6axis_ucal->Size = System::Drawing::Size(156, 75);
            this->textBox_6axis_ucal->TabIndex = 43;
            this->textBox_6axis_ucal->TabStop = false;
            this->textBox_6axis_ucal->Text = L"6-Axis User (XYZ):\r\nAcc:\r\n\r\n\r\nGyro:";
            // 
            // textBox_6axis_cal
            // 
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
            this->textBox_6axis_cal->Size = System::Drawing::Size(156, 75);
            this->textBox_6axis_cal->TabIndex = 42;
            this->textBox_6axis_cal->TabStop = false;
            this->textBox_6axis_cal->Text = L"6-Axis Factory (XYZ):\r\nAcc:\r\n\r\n\r\nGyro:";
            // 
            // textBox_device_parameters
            // 
            this->textBox_device_parameters->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBox_device_parameters->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBox_device_parameters->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F));
            this->textBox_device_parameters->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->textBox_device_parameters->Location = System::Drawing::Point(7, 22);
            this->textBox_device_parameters->Margin = System::Windows::Forms::Padding(0);
            this->textBox_device_parameters->Multiline = true;
            this->textBox_device_parameters->Name = L"textBox_device_parameters";
            this->textBox_device_parameters->ReadOnly = true;
            this->textBox_device_parameters->Size = System::Drawing::Size(124, 145);
            this->textBox_device_parameters->TabIndex = 41;
            this->textBox_device_parameters->TabStop = false;
            this->textBox_device_parameters->Text = L"Flat surface ACC Offsets:\r\n\r\n\r\n\r\nStick Parameters:";
            // 
            // textBox_device_parameters2
            // 
            this->textBox_device_parameters2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->textBox_device_parameters2->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBox_device_parameters2->Font = (gcnew System::Drawing::Font(L"Lucida Console", 8.25F));
            this->textBox_device_parameters2->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
                static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
            this->textBox_device_parameters2->Location = System::Drawing::Point(155, 78);
            this->textBox_device_parameters2->Margin = System::Windows::Forms::Padding(0);
            this->textBox_device_parameters2->Multiline = true;
            this->textBox_device_parameters2->Name = L"textBox_device_parameters2";
            this->textBox_device_parameters2->ReadOnly = true;
            this->textBox_device_parameters2->Size = System::Drawing::Size(135, 91);
            this->textBox_device_parameters2->TabIndex = 42;
            this->textBox_device_parameters2->TabStop = false;
            this->textBox_device_parameters2->Text = L"Stick Parameters 2:\r\n";
            // 
            // grpBox_dev_param
            // 
            this->grpBox_dev_param->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->grpBox_dev_param->Controls->Add(this->textBox_device_parameters);
            this->grpBox_dev_param->Controls->Add(this->textBox_device_parameters2);
            this->grpBox_dev_param->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->grpBox_dev_param->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
                static_cast<System::Int32>(static_cast<System::Byte>(206)));
            this->grpBox_dev_param->Location = System::Drawing::Point(408, 445);
            this->grpBox_dev_param->Margin = System::Windows::Forms::Padding(0);
            this->grpBox_dev_param->Name = L"grpBox_dev_param";
            this->grpBox_dev_param->Size = System::Drawing::Size(306, 218);
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
            this->toolStrip1->Location = System::Drawing::Point(0, 677);
            this->toolStrip1->Name = L"toolStrip1";
            this->toolStrip1->Padding = System::Windows::Forms::Padding(6, 0, 6, 0);
            this->toolStrip1->Size = System::Drawing::Size(1662, 25);
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
            this->toolStripBtn_batt->Name = L"toolStripBtn_batt";
            this->toolStripBtn_batt->Padding = System::Windows::Forms::Padding(0, 0, 4, 0);
            this->toolStripBtn_batt->Size = System::Drawing::Size(23, 22);
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
            this->toolStripLabel_batt->Name = L"toolStripLabel_batt";
            this->toolStripLabel_batt->Padding = System::Windows::Forms::Padding(4, 0, 0, 0);
            this->toolStripLabel_batt->Size = System::Drawing::Size(63, 22);
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
            this->toolStripLabel_temp->Name = L"toolStripLabel_temp";
            this->toolStripLabel_temp->Padding = System::Windows::Forms::Padding(4, 0, 4, 0);
            this->toolStripLabel_temp->Size = System::Drawing::Size(56, 22);
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
            this->toolStripBtn_refresh->Name = L"toolStripBtn_refresh";
            this->toolStripBtn_refresh->Padding = System::Windows::Forms::Padding(4, 0, 4, 0);
            this->toolStripBtn_refresh->Size = System::Drawing::Size(64, 22);
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
            this->toolStripBtn_Disconnect->Name = L"toolStripBtn_Disconnect";
            this->toolStripBtn_Disconnect->Padding = System::Windows::Forms::Padding(4, 0, 4, 0);
            this->toolStripBtn_Disconnect->Size = System::Drawing::Size(83, 22);
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
            this->grpBox_accGyroCal->Size = System::Drawing::Size(168, 218);
            this->grpBox_accGyroCal->TabIndex = 47;
            this->grpBox_accGyroCal->TabStop = false;
            this->grpBox_accGyroCal->Text = L"Acc/Gyro Calibration";
            // 
            // FormJoy
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Dpi;
            this->AutoSize = true;
            this->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
            this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
                static_cast<System::Int32>(static_cast<System::Byte>(70)));
            this->ClientSize = System::Drawing::Size(1662, 702);
            this->Controls->Add(this->menuStrip1);
            this->Controls->Add(this->toolStrip1);
            this->Controls->Add(this->panel_filler);
            this->Controls->Add(this->grpBox_dev_param);
            this->Controls->Add(this->grpBox_StickCal);
            this->Controls->Add(this->grpBox_accGyroCal);
            this->Controls->Add(this->grpBox_ButtonTest);
            this->Controls->Add(this->btn_enable_expert_mode);
            this->Controls->Add(this->grpBox_VibPlayer);
            this->Controls->Add(this->grpBox_ChangeSN);
            this->Controls->Add(this->grpBox_Restore);
            this->Controls->Add(this->grpBox_DebugCmd);
            this->Controls->Add(this->groupBoxColor);
            this->Controls->Add(this->textBoxDev);
            this->Controls->Add(this->label_dev);
            this->Controls->Add(this->textBoxFW);
            this->Controls->Add(this->groupBoxSPI);
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
			this->Text = L"Joy-Con Toolkit v2.8.2";
            this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &FormJoy::Form1_FormClosing);
            this->SizeChanged += gcnew System::EventHandler(this, &FormJoy::mainFormSizeChanged);
            this->groupBoxColor->ResumeLayout(false);
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxPreview))->EndInit();
            this->groupBoxSPI->ResumeLayout(false);
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
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion

    //////////////
    // Functions
    //////////////

    private: System::Void full_refresh(bool check_connection) {
        if (check_connection) {
            if (!device_connection()) {
                MessageBox::Show(L"The device was disconnected!\n\n" +
                    L"Press a button on the controller to connect\nand try to write the colors again!",
                    L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
                return;
            }
            this->toolStripBtn_Disconnect->Enabled = true;
        }

        if (check_connection)
            set_led_busy();

        this->btn_run_btn_test->Text = L"Turn on";
        enable_button_test = false;

        if (handle_ok != 3) {
            this->textBoxSN->Text = gcnew String(get_sn(0x6001, 0xF).c_str());
            this->textBox_chg_sn->Text = this->textBoxSN->Text;
        }
        else {
            this->textBoxSN->Text = L"No S/N";
            this->textBox_chg_sn->Text = this->textBoxSN->Text;
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

    private: System::Void btnWriteBody_Click(System::Object^ sender, System::EventArgs^ e) {
        if (!device_connection()) {
            MessageBox::Show(L"The device was disconnected!\n\n" +
                L"Press a button on the controller to connect\nand try to write the colors again!",
                L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
            return;
        }
        set_led_busy();
        if (MessageBox::Show(L"Don't forget to make a backup first!\n" +
            L"You can also find retail colors at the bottom of the colors dialog for each type.\n\n" +
            L"Are you sure you want to continue?",
            L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
        {
            this->btnWriteBody->Enabled = false;

            unsigned char body_color[0x3];
            memset(body_color, 0, 0x3);
            unsigned char button_color[0x3];
            memset(button_color, 0, 0x3);

            body_color[0] = (u8)jcBodyColor.R;
            body_color[1] = (u8)jcBodyColor.G;
            body_color[2] = (u8)jcBodyColor.B;

            button_color[0] = (u8)jcButtonsColor.R;
            button_color[1] = (u8)jcButtonsColor.G;
            button_color[2] = (u8)jcButtonsColor.B;

            write_spi_data(0x6050, 0x3, body_color);
            write_spi_data(0x6053, 0x3, button_color);

            send_rumble();
            MessageBox::Show(L"The colors were written to the device!", L"Done!",
                MessageBoxButtons::OK, MessageBoxIcon::Asterisk);

            //Check that the colors were written
            update_colors_from_spi(false);

            update_battery();
            update_temperature();
        }
    }

    private: System::Void btn_makeBackup_Click(System::Object^  sender, System::EventArgs^  e) {
        if (!device_connection()) {
            MessageBox::Show(L"The device was disconnected!\n\n" +
                L"Press a button on the controller to connect\nand try to backup your SPI flash again!",
                L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
            return;
        }
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
            this->groupBoxColor->Visible = false;
            reset_window_option(true);
            Application::DoEvents();
            send_rumble();
            set_led_busy();
            cancel_spi_dump = false;

            int error = dump_spi((context.marshal_as<std::string>(filename)).c_str());

            this->groupBoxColor->Visible = true;
            handler_close = 0;
            if (!error && !cancel_spi_dump) {
                send_rumble();
                MessageBox::Show(L"Done dumping SPI!", L"SPI Dumping", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
            }
        }
        this->menuStrip1->Enabled = true;
        this->toolStrip1->Enabled = true;
        update_battery();
        update_temperature();
    }

    private: System::Void update_joycon_color(u8 r, u8 g, u8 b, u8 rb, u8 gb, u8 bb) {
        Bitmap^ MyImage;
        if (handle_ok == 1) {
            MyImage = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"$this.base64_l_joy")));
        }
        else if (handle_ok == 3) {
            MyImage = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"$this.base64_pro")));
        }
        else {
            MyImage = (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"$this.base64_r_joy")));
        }

        float color_coeff = 0.0;
        for (int x = 0; x < MyImage->Width; x++)
        {
            for (int y = 0; y < MyImage->Height; y++)
            {
                Color gotColor = MyImage->GetPixel(x, y);
                color_coeff = gotColor.R / 255.0f;

                if (gotColor.R <= 255 && gotColor.R > 100){
                    gotColor = Color::FromArgb(gotColor.A, CLAMP(r*color_coeff, 0.0f, 255.0f),
                        CLAMP(g*color_coeff, 0.0f, 255.0f), CLAMP(b*color_coeff, 0.0f, 255.0f));
                }
                else if (handle_ok != 3 && gotColor.R <= 100 && gotColor.R != 78 && gotColor.R > 30) {
                    if (gotColor.R == 100)
                        gotColor = Color::FromArgb(gotColor.A, rb, gb, bb);
                    else {
                        gotColor = Color::FromArgb(gotColor.A, CLAMP(rb*color_coeff, 0.0f, 255.0f),
                            CLAMP(gb*color_coeff, 0.0f, 255.0f), CLAMP(bb*color_coeff, 0.0f, 255.0f));
                    }
                }
                else if (handle_ok == 3 && gotColor.R <= 100 && gotColor.R != 78 && gotColor.R > 30) {
                    if (rb == 255 && gb == 255 && bb == 255) {
                        //Do nothing
                    }
                    else {
                        if (gotColor.R == 100)
                            gotColor = Color::FromArgb(gotColor.A, rb, gb, bb);
                        else {
                            gotColor = Color::FromArgb(gotColor.A, CLAMP(rb*color_coeff, 0.0f, 255.0f),
                                CLAMP(gb*color_coeff, 0.0f, 255.0f), CLAMP(bb*color_coeff, 0.0f, 255.0f));
                        }
                    }
                }
                MyImage->SetPixel(x, y, gotColor);

            }
        }

        if (handle_ok == 3) {
            System::Drawing::Rectangle cloneRect = System::Drawing::Rectangle(0, 100, 520, 320);
            System::Drawing::Imaging::PixelFormat format = MyImage->PixelFormat;
            MyImage = MyImage->Clone(cloneRect, format);
        }
        else {
            System::Drawing::Rectangle cloneRect = System::Drawing::Rectangle(1, 66, 346, 213);
            System::Drawing::Imaging::PixelFormat format = MyImage->PixelFormat;
            MyImage = MyImage->Clone(cloneRect, format);
        }
        this->pictureBoxPreview->Image = dynamic_cast<Image^>(MyImage);
        this->pictureBoxPreview->ClientSize = System::Drawing::Size(312, 192);
        this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
    }

    private: System::Void update_colors_from_spi(bool update_color_dialog) {
        unsigned char body_color[0x3];
        memset(body_color, 0, 0x3);
        unsigned char button_color[0x3];
        memset(button_color, 0, 0x3);

        get_spi_data(0x6050, 0x3, body_color);
        get_spi_data(0x6053, 0x3, button_color);

        update_joycon_color((u8)body_color[0], (u8)body_color[1], (u8)body_color[2],
            (u8)button_color[0], (u8)button_color[1], (u8)button_color[2]);

        if (update_color_dialog) {
            this->jcBodyColor = Color::FromArgb(0xFF, (u8)body_color[0], (u8)body_color[1], (u8)body_color[2]);
            this->lbl_Body_hex_txt->Text = L"Body: #" + String::Format("{0:X6}",
                ((u8)body_color[0] << 16) + ((u8)body_color[1] << 8) + ((u8)body_color[2]));
            this->lbl_Body_hex_txt->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_Body_hex_txt->Size = System::Drawing::Size(128, 24);

            this->jcButtonsColor = Color::FromArgb(0xFF, (u8)button_color[0], (u8)button_color[1], (u8)button_color[2]);
            this->lbl_Buttons_hex_txt->Text = L"Buttons: #" + String::Format("{0:X6}",
                ((u8)button_color[0] << 16) + ((u8)button_color[1] << 8) + ((u8)button_color[2]));
            this->lbl_Buttons_hex_txt->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(161)));
            this->lbl_Buttons_hex_txt->Size = System::Drawing::Size(128, 24);
        }

    }

    private: System::Void update_battery() {
        unsigned char batt_info[3];
        memset(batt_info, 0, sizeof(batt_info));
        get_battery(batt_info);

        int batt_percent = 0;
        int batt = ((u8)batt_info[0] & 0xF0) >> 4;
        
        // Calculate aproximate battery percent from regulated voltage
        u16 batt_volt = (u8)batt_info[1] + ((u8)batt_info[2] << 8);
        if (batt_volt < 0x560)
            batt_percent = 1;
        else if (batt_volt > 0x55F && batt_volt < 0x5A0) {
            batt_percent = ((batt_volt - 0x60) & 0xFF) / 7.0f + 1;
        }
        else if (batt_volt > 0x59F && batt_volt < 0x5E0) {
            batt_percent = ((batt_volt - 0xA0) & 0xFF) / 2.625f + 11;
        }
        else if (batt_volt > 0x5DF && batt_volt < 0x618) {
            batt_percent = (batt_volt - 0x5E0) / 1.8965f + 36;
        }
        else if (batt_volt > 0x617 && batt_volt < 0x658) {
            batt_percent = ((batt_volt - 0x18) & 0xFF) / 1.8529f + 66;
        }
        else if (batt_volt > 0x657)
            batt_percent = 100;

        this->toolStripLabel_batt->Text = String::Format(" {0:f2}V - {1:D}%", (batt_volt * 2.5) / 1000, batt_percent);

        // Update Battery icon from input report value.
        switch (batt) {
            case 0:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"$this.batt_0")));
                this->toolStripBtn_batt->ToolTipText = L"Empty\n\nDisconnected?";
                break;
            case 1:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"$this.batt_0_chr")));
                this->toolStripBtn_batt->ToolTipText = L"Empty, Charging.";
                break;
            case 2:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"$this.batt_25")));
                this->toolStripBtn_batt->ToolTipText = L"Low\n\nPlease charge your device!";
                break;
            case 3:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"$this.batt_25_chr")));
                this->toolStripBtn_batt->ToolTipText = L"Low\n\nCharging";
                break;
            case 4:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"$this.batt_50")));
                this->toolStripBtn_batt->ToolTipText = L"Medium";
                break;
            case 5:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"$this.batt_50_chr")));
                this->toolStripBtn_batt->ToolTipText = L"Medium\n\nCharging";
                break;
            case 6:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"$this.batt_75")));
                this->toolStripBtn_batt->ToolTipText = L"Good";
                break;
            case 7:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"$this.batt_75_chr")));
                this->toolStripBtn_batt->ToolTipText = L"Good\n\nCharging";
                break;
            case 8:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"$this.batt_100")));
                this->toolStripBtn_batt->ToolTipText = L"Full";
                break;
            case 9:
                this->toolStripBtn_batt->Image =
                    (cli::safe_cast<System::Drawing::Bitmap^>(resources->GetObject(L"$this.batt_100_chr")));
                this->toolStripBtn_batt->ToolTipText = L"Almost full\n\nCharging";
                break;
        }
    
    }

    private: System::Void update_temperature() {
        unsigned char temp_info[2];
        memset(temp_info, 0, sizeof(temp_info));
        get_temperature(temp_info);
        // Convert reading to Celsius according to datasheet
        float temperature_c = 25.0f + uint16_to_int16(temp_info[1] << 8 | temp_info[0]) * 0.0625f;
        float temperature_f = temperature_c * 1.8f + 32;

        if (temp_celsius)
            this->toolStripLabel_temp->Text = String::Format(L"{0:f1}\u2103 ", temperature_c);
        else
            this->toolStripLabel_temp->Text = String::Format(L"{0:f1}\u2109 ", temperature_f);
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
            this->Controls->Add(this->btn_enable_expert_mode);
            option_is_on = 1;
            this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
        }
        else
            reset_window_option(true);
    }

    private: System::Void btn_enable_expert_mode_Click(System::Object^  sender, System::EventArgs^  e) {
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
            this->btn_run_btn_test->Text = L"Turn on";
            option_is_on = 5;
            this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
        }
        else {
            reset_window_option(true);
            this->btn_run_btn_test->Text = L"Turn on";
        }
    }

    private: System::Void reset_window_option(bool reset_all) {
        enable_button_test = false;
        this->Controls->Remove(this->grpBox_DebugCmd);
        this->Controls->Remove(this->grpBox_Restore);
        this->Controls->Remove(this->grpBox_ChangeSN);
        this->Controls->Remove(this->grpBox_VibPlayer);
        this->Controls->Remove(this->grpBox_ButtonTest);

        this->Controls->Remove(this->btn_enable_expert_mode);
        this->Controls->Remove(this->grpBox_StickCal);
        this->Controls->Remove(this->grpBox_dev_param);
        this->Controls->Remove(this->grpBox_accGyroCal);

        this->textBoxDbg_sent->Visible = false;
        this->textBoxDbg_reply->Visible = false;
        this->textBoxDbg_reply_cmd->Visible = false;

        this->menuStrip1->Refresh();
        this->toolStrip1->Refresh();

        if (reset_all) {
            option_is_on = 0;
        }
    }

    private: System::Void btnDbg_send_cmd_Click(System::Object^  sender, System::EventArgs^  e) {
        if (!device_connection()) {
            MessageBox::Show(L"The device was disconnected!\n\nPress a button on the controller to connect\n" +
                L"and try to send the custom command again!",
                L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
            return;
        }

        msclr::interop::marshal_context context;
        unsigned char test[31];
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

        ss_cmd << std::hex << context.marshal_as<std::string>(this->textBoxDbg_cmd->Text);
        ss_hfreq << std::hex << context.marshal_as<std::string>(this->textBoxDbg_hfreq->Text);
        ss_hamp << std::hex << context.marshal_as<std::string>(this->textBoxDbg_hamp->Text);
        ss_lfreq << std::hex << context.marshal_as<std::string>(this->textBoxDbg_lfreq->Text);
        ss_lamp << std::hex << context.marshal_as<std::string>(this->textBoxDbg_lfamp->Text);
        ss_subcmd << std::hex << context.marshal_as<std::string>(this->textBoxDbg_subcmd->Text);
        
        ss_cmd >> i_cmd;
        ss_hfreq >> i_hfreq;
        ss_hamp >> i_hamp;
        ss_lfreq >> i_lfreq;
        ss_lamp >> i_lamp;
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
        this->textBoxDbg_sent->Visible = true;
        this->textBoxDbg_reply->Visible = true;
        this->textBoxDbg_reply_cmd->Visible = true;

        if (test[5] != 0x06) {
            update_battery();
            update_temperature();
        }
    }

    private: System::Void btnLoadBackup_Click(System::Object^  sender, System::EventArgs^  e) {
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
                    this->lbl_rstDesc->Visible = false;
                    this->comboBox_rstOption->Visible = false;
                    this->btn_restore->Visible = false;
                    this->grpBox_RstUser->Visible = false;
                    this->lbl_rstDisclaimer->Visible = true;

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
                this->lbl_rstDesc->Visible = false;
                this->comboBox_rstOption->Visible = false;
                this->btn_restore->Visible = false;
                this->grpBox_RstUser->Visible = false;
                this->lbl_rstDisclaimer->Visible = true;
                
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
                    this->lbl_rstDesc->Visible = false;
                    this->comboBox_rstOption->Visible = false;
                    this->btn_restore->Visible = false;
                    this->grpBox_RstUser->Visible = false;
                    this->lbl_rstDisclaimer->Visible = true;
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

    protected: System::Void comboBox_rstOption_DrawItem(System::Object^ sender, DrawItemEventArgs^ e)
    {
        Brush^ brush = gcnew System::Drawing::SolidBrush(Color::FromArgb(9, 255, 206));;

        if (e->Index < 0)
            return;

        ComboBox^ combo = (ComboBox^)sender;
        if ((e->State & DrawItemState::Selected) == DrawItemState::Selected)
            e->Graphics->FillRectangle(gcnew SolidBrush(Color::FromArgb(105, 105, 105)), e->Bounds);
        else
            e->Graphics->FillRectangle(gcnew SolidBrush(combo->BackColor), e->Bounds);

        e->Graphics->DrawString(this->comboBox_rstOption->Items[e->Index]->ToString(), e->Font, brush,
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
            this->lbl_rstDesc->Text = L"This will restore your S/N from the selected backup.\n\n" +
                L"Make sure that this backup was your original one!\n\nIf you lost your S/N, " +
                L"check the plastic sliver it was wrapped inside the box.";
        }
        else if (this->comboBox_rstOption->SelectedIndex == 2) {
            this->lbl_rstDesc->Visible = true;
            this->lbl_rstDesc->Text =
                L"This lets you restore the chosen user calibrations from your SPI backup.";
            this->grpBox_RstUser->Visible = true;
            if (handle_ok == 1) {
                this->checkBox_rst_R_StickCal->Visible = false;
                this->checkBox_rst_L_StickCal->Visible = true;
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
                this->checkBox_rst_R_StickCal->Visible = false;
                this->checkBox_rst_L_StickCal->Visible = true;
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
        if (!device_connection()) {
            MessageBox::Show(L"The device was disconnected!\n\nPress a button on the controller to connect\n" +
                L"and try to restore again!", L"CTCaer's Joy-Con Toolkit - Connection Error!",
                MessageBoxButtons::OK, MessageBoxIcon::Stop);
            return;
        }
        this->menuStrip1->Enabled = false;
        this->toolStrip1->Enabled = false;
        set_led_busy();
        if (this->comboBox_rstOption->SelectedIndex == 0) {
            if (MessageBox::Show(L"The device color will be restored with the backup values!\n\nAre you sure you want to continue?",
                L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
            {
                unsigned char body_color[0x3];
                memset(body_color, 0, 0x3);
                unsigned char button_color[0x3];
                memset(button_color, 0, 0x3);

                for (int i = 0; i < 3; i++) {
                    body_color[i] = this->backup_spi[0x6050 + i];
                    button_color[i] = this->backup_spi[0x6053 + i];
                }

                write_spi_data(0x6050, 0x3, body_color);
                write_spi_data(0x6053, 0x3, button_color);

                //Check that the colors were written
                update_colors_from_spi(true);
                update_battery();
                update_temperature();
                send_rumble();

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

                write_spi_data(0x6000, 0x10, sn);

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
                update_battery();
                update_temperature();
                send_rumble();
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
                    write_spi_data(0x8010, 0xB, l_stick);
                if (handle_ok != 1 && this->checkBox_rst_R_StickCal->Checked == true)
                    write_spi_data(0x801B, 0xB, r_stick);
                if (this->checkBox_rst_accGyroCal->Checked == true)
                    write_spi_data(0x8026, 0x1A, sensor);

                update_battery();
                update_temperature();
                send_rumble();
                MessageBox::Show(L"The user calibration was restored!", L"Calibration Restore Finished!",
                    MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
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
                    write_spi_data(0x8010, 0xB, l_stick);
                if (handle_ok != 1 && this->checkBox_rst_R_StickCal->Checked == true)
                    write_spi_data(0x801B, 0xB, r_stick);
                if (this->checkBox_rst_accGyroCal->Checked == true)
                    write_spi_data(0x8026, 0x1A, sensor);

                update_battery();
                update_temperature();
                send_rumble();
                MessageBox::Show(L"The user calibration was factory resetted!", L"Calibration Reset Finished!",
                    MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
            }

        }
        else if (this->comboBox_rstOption->SelectedIndex == 4) {
            if (MessageBox::Show(L"This will do a full restore of the Factory configuration and User calibration!\n\n" +
                L"Are you sure you want to continue?",
                L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
            {
                handler_close = 2;
                this->btnLoadBackup->Enabled = false;
                this->btn_restore->Enabled = false;
                this->groupBoxColor->Visible = false;
                this->lbl_spiProggressDesc->Text = L"Restoring Factory Configuration and User Calibration...\n\nDon\'t disconnect your device!";
                unsigned char full_restore_data[0x10];
                unsigned char sn_backup_erase[0x10];
                memset(sn_backup_erase, 0xFF, sizeof(sn_backup_erase));

                // Factory Configuration Sector 0x6000
                for (int i = 0x00; i < 0x1000; i = i + 0x10) {
                    memset(full_restore_data, 0, 0x10);
                    for (int j = 0; j < 0x10; j++)
                        full_restore_data[j] = this->backup_spi[0x6000 + i + j];
                    write_spi_data(0x6000 + i, 0x10, full_restore_data);

                    std::stringstream offset_label;
                    offset_label << std::fixed << std::setprecision(2) << std::setfill(' ') << i / 1024.0f;
                    offset_label << "KB of 8KB";
                    FormJoy::myform1->label_progress->Text = gcnew String(offset_label.str().c_str());
                    Application::DoEvents();
                }
                // User Calibration Sector 0x8000
                for (int i = 0x00; i < 0x1000; i = i + 0x10) {
                    memset(full_restore_data, 0, 0x10);
                    for (int j = 0; j < 0x10; j++)
                        full_restore_data[j] = this->backup_spi[0x8000 + i + j];
                    write_spi_data(0x8000 + i, 0x10, full_restore_data);

                    std::stringstream offset_label;
                    offset_label << std::fixed << std::setprecision(2) << std::setfill(' ') << 4 + (i / 1024.0f);
                    offset_label << "KB of 8KB";
                    FormJoy::myform1->label_progress->Text = gcnew String(offset_label.str().c_str());
                    Application::DoEvents();
                }
                // Erase S/N backup storage
                write_spi_data(0xF000, 0x10, sn_backup_erase);

                std::stringstream offset_label;
                offset_label << std::fixed << std::setprecision(2) << std::setfill(' ') << 0x2000 / 1024.0f;
                offset_label << "KB of 8KB";
                FormJoy::myform1->label_progress->Text = gcnew String(offset_label.str().c_str());
                Application::DoEvents();

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

                update_battery();
                update_temperature();
                send_rumble();

                MessageBox::Show(L"The full restore was completed!\nThe controller was rebooted and it is now in pairing mode!\n\n" +
                    L"Exit Joy-Con Toolkit and pair with Switch or PC again.", L"Full Restore Finished!",
                    MessageBoxButtons::OK, MessageBoxIcon::Warning);
                this->groupBoxColor->Visible = true;
                this->btnLoadBackup->Enabled = true;
                this->btn_restore->Enabled = true;

                update_colors_from_spi(true);

                handler_close = 0;
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
            //This control has failed validation: text box is not a number
            //this->btnDbg_send_cmd->Enabled = false;
            cancel = true;
            this->errorProvider2->SetError(test, "The input must be a valid uint8 HEX!");
        }
        e->Cancel = cancel;
    }
    private: System::Void textBoxDbg_Validated(System::Object^ sender, System::EventArgs^ e)
    {
        //Control has validated, clear any error message.
        TextBox^ test = (TextBox^)sender;
        //this->btnDbg_send_cmd->Enabled = true;
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
        //this->btnChangeSn->Enabled = true;
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
                    //this->btnChangeSn->Enabled = false;
                    cancel = true;
                    this->errorProvider1->SetError(test, "Extended ASCII characters are not supported!");
                }
            }
            else
            {
                //This control has failed validation: text box is not a number
                //this->btnChangeSn->Enabled = false;
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
        //this->btnChangeSn->Enabled = true;
        this->errorProvider1->SetError(test, String::Empty);
    }

    private: System::Void btnChangeSn_Click(System::Object^  sender, System::EventArgs^  e) {
        if (!device_connection()) {
            MessageBox::Show(L"The device was disconnected!\n\nPress a button on the controller to connect\n" +
                L"and try to change the S/N again!",
                L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
            return;
        }
        if (handle_ok != 3) {
            if (MessageBox::Show(L"This will change your Serial Number!\n\nMake a backup first!\n\n" +
                L"Are you sure you want to continue?", L"Warning!",
                MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
            {
                if (MessageBox::Show(L"Did you make a backup?", L"Warning!",
                    MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
                {
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
                        write_spi_data(0xF000, 0x10, spi_sn);

                    array<Char>^ mn_str_sn = this->textBox_chg_sn->Text->ToCharArray();
                    unsigned char sn[32];

                    int length = 16 - mn_str_sn->Length;

                    for (int i = 0; i < length; i++) {
                        sn[i] = 0x00;
                    }
                    for (int i = 0; i < mn_str_sn->Length; i++) {
                        sn[length + i] = (unsigned char)(mn_str_sn[i] & 0xFF);
                    }

                    write_spi_data(0x6000, 0x10, sn);
                    update_battery();
                    update_temperature();
                    send_rumble();
                    String^ new_sn = gcnew String(get_sn(0x6001, 0xF).c_str());
                    MessageBox::Show(L"The S/N was written to the device!\n\nThe new S/N is now \"" + new_sn +
                        L"\"!\n\nIf you still ignored the warnings about creating a backup, the S/N in the left of the main window will not change. Copy it somewhere safe!\n\nLastly, a backup of your S/N was created inside the SPI.");
                }
            }
        }
        else
            MessageBox::Show(L"Changing S/N is not supported for Pro Controllers!", L"Error!", MessageBoxButtons::OK, MessageBoxIcon::Warning);
    }

    private: System::Void btnRestore_SN_Click(System::Object^  sender, System::EventArgs^  e) {
        if (!device_connection()) {
            MessageBox::Show(L"The device was disconnected!\n\nPress a button on the controller to connect\nand try again!",
                L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
            return;
        }
        if (handle_ok != 3) {
            if (MessageBox::Show(L"Do you really want to restore it from the S/N backup inside your controller\'s SPI?\n\nYou can also choose to restore it from a SPI backup you previously made, through the main Restore option.",
                L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes) {
                int sn_ok = 1;
                unsigned char spi_sn[0x10];
                memset(spi_sn, 0x11, sizeof(spi_sn));

                //Check if there is an SN backup
                get_spi_data(0xF000, 0x10, spi_sn);
                if (spi_sn[0] != 0x00) {
                        sn_ok = 0;
                    }
                if (sn_ok) {
                    write_spi_data(0x6000, 0x10, spi_sn);
                }
                else {
                    MessageBox::Show(L"No S/N backup found inside your controller\'s SPI.\n\nThis can happen if the first time you changed your S/N was with an older version of Joy-Con Toolkit.\nOtherwise, you never changed your S/N.",
                        L"Error!", MessageBoxButtons::OK, MessageBoxIcon::Error);
                    return;
                }

                update_battery();
                update_temperature();
                send_rumble();
                String^ new_sn = gcnew String(get_sn(0x6001, 0xF).c_str());
                MessageBox::Show(L"The S/N was restored to the device!\n\nThe new S/N is now \"" + new_sn + L"\"!");
            
            }
        }
        else
            MessageBox::Show(L"Restoring S/N is not supported for Pro Controllers!", L"Error!", MessageBoxButtons::OK, MessageBoxIcon::Warning);

    }

    private: System::Void pictureBoxBattery_Click(System::Object^  sender, System::EventArgs^  e) {
        if (!device_connection()) {
            MessageBox::Show(L"The device was disconnected!\n\nPress a button on the controller to connect\nand try again!",
                L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
            return;
        }
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
        if (!device_connection()) {
            MessageBox::Show(L"The device was disconnected!\n\nPress a button on the controller to connect\nand try again!",
                L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
            return;
        }
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

    private: System::Void btnLoadVib_Click(System::Object^  sender, System::EventArgs^  e) {
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
                    this->btnVibPlay->Enabled = true;
                    vib_sample_rate = (this->vib_loaded_file[0x4] << 8) + this->vib_loaded_file[0x5];
                    vib_samples = (this->vib_loaded_file[0x6] << 24) + (this->vib_loaded_file[0x7] << 16) + (this->vib_loaded_file[0x8] << 8) + this->vib_loaded_file[0x9];
                    this->label_samplerate->Text = L"Sample rate: " + vib_sample_rate + L"ms";
                    this->label_samples->Text = L"Samples: " + vib_samples + L" (" + (vib_sample_rate * vib_samples) / 1000.0f + L"s)";
                }
                else {
                    this->label_vib_loaded->Text = L"Type: Unknown format";
                    this->btnVibPlay->Enabled = false;
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
                this->btnVibPlay->Enabled = true;
            }
            else {
                vib_file_type = 0;
                this->label_vib_loaded->Text = L"Type: Unknown format";
                this->btnVibPlay->Enabled = false;
                this->groupBox_vib_eq->Visible = false;
            }
            this->btnVib_reset_eq->PerformClick();
        }
    }




    private: System::Void btnVibPlay_Click(System::Object^  sender, System::EventArgs^  e) {
        if (!device_connection()) {
            MessageBox::Show(L"The device was disconnected!\n\nPress a button on the controller to connect\nand try to replay the HD Rumble file!",
                L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
            return;
        }
        int vib_loop_times = 0;
        if ((vib_file_type == 2 || vib_file_type == 3 || vib_file_type == 4) && !vib_converted) {
            u8 vib_off = 0;
            if (vib_file_type == 3)
                vib_off = 8;
            if (vib_file_type == 4)
                vib_off = 12;
            //Convert to RAW vibration, apply EQ and clamp inside safe values
            this->btnVibPlay->Text = L"Loading...";
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

        this->btnVibPlay->Enabled = false;
        this->btnLoadVib->Enabled = false;
        this->groupBox_vib_eq->Enabled = false;
        this->btnVibPlay->Text = L"Playing...";


        play_hd_rumble_file(vib_file_type, vib_sample_rate, vib_samples, vib_loop_start, vib_loop_end, vib_loop_wait, vib_loop_times);

        this->btnVibPlay->Text = L"Play";
        this->btnVibPlay->Enabled = true;
        this->btnLoadVib->Enabled = true;
        if (vib_file_type == 2 || vib_file_type == 3 || vib_file_type == 4) {
            this->groupBox_vib_eq->Enabled = true;
        }
        update_battery();
        update_temperature();

    }
     
    private: System::Void btnVib_reset_eq_Click(System::Object^  sender, System::EventArgs^  e) {
        this->trackBar_lf_amp->Value = 10;
        this->trackBar_lf_freq->Value = 10;
        this->trackBar_hf_amp->Value = 10;
        this->trackBar_hf_freq->Value = 10;
    }

    private: System::Void TrackBar_ValueChanged(System::Object^ sender, System::EventArgs^ e)
    {
        lf_gain = this->trackBar_lf_amp->Value / 10.0f;
        lf_pitch = this->trackBar_lf_freq->Value / 10.0f;
        hf_gain = this->trackBar_hf_amp->Value / 10.0f;
        hf_pitch = this->trackBar_hf_freq->Value / 10.0f;
        this->toolTip1->SetToolTip(this->trackBar_lf_amp, String::Format("{0:d}%", (int)(lf_gain * 100.01f)));
        this->toolTip1->SetToolTip(this->trackBar_lf_freq, String::Format("{0:d}%", (int)(lf_pitch * 100.01f)));
        this->toolTip1->SetToolTip(this->trackBar_hf_amp, String::Format("{0:d}%", (int)(hf_gain * 100.01f)));
        this->toolTip1->SetToolTip(this->trackBar_hf_freq, String::Format("{0:d}%", (int)(hf_pitch * 100.01f)));
        vib_converted = 0;
    }

    private: System::Void btn_run_btn_test_Click(System::Object^  sender, System::EventArgs^  e) {
        if (!device_connection()) {
            MessageBox::Show(L"The device was disconnected!\n\nPress a button on the controller to connect\nand try again!",
                L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Stop);

            return;
        }

        if (!enable_button_test) {
            this->btn_run_btn_test->Text = L"Turn off";
            enable_button_test = true;
            button_test();
        }
        else {
            this->btn_run_btn_test->Text = L"Turn on";
            enable_button_test = false;
        }

    }

    private: System::Void btn_spi_cancel_Click(System::Object^  sender, System::EventArgs^  e) {
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
    }
    private: System::Void toolStripBtn_refresh_Click(System::Object^  sender, System::EventArgs^  e) {
        full_refresh(true);
    }

    private: System::Void btn_change_color_Click(System::Object^  sender, System::EventArgs^  e) {
        Color testcolor;
        JCColorPicker = gcnew jcColor::JoyConColorPicker(this->jcBodyColor, this->jcButtonsColor);
        System::Drawing::Rectangle screenRectangle = RectangleToScreen(this->ClientRectangle);
        int titleHeight = screenRectangle.Top - this->Top;

        JCColorPicker->TopLevel = false;
        JCColorPicker->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
        JCColorPicker->m_cmd_Cancel->Click += gcnew System::EventHandler(this, &FormJoy::Color_Picker_Cancel);
        JCColorPicker->m_cmd_OK->Click += gcnew System::EventHandler(this, &FormJoy::Color_Picker_OK);

        if (option_is_on == 5) {
            this->Controls->Remove(this->grpBox_dev_param);
            this->Controls->Remove(this->grpBox_StickCal);
            this->Controls->Remove(this->grpBox_accGyroCal);
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
        
    }

    private: System::Void Color_Picker_Cancel(System::Object^  sender, System::EventArgs^  e) {
        this->panel_filler->Location = System::Drawing::Point(0, 0);
        this->panel_filler->Size = System::Drawing::Size(0, 0);
        if (option_is_on == 5) {
            this->Controls->Add(this->grpBox_StickCal);
            this->Controls->Add(this->grpBox_dev_param);
            this->Controls->Add(this->grpBox_accGyroCal);
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
        }
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

        update_joycon_color(jcBodyColor.R, jcBodyColor.G, jcBodyColor.B,
            jcButtonsColor.R, jcButtonsColor.G, jcButtonsColor.B);

        this->btnWriteBody->Enabled = true;

        this->menuStrip1->Enabled = true;
        this->toolStrip1->Enabled = true;

        this->menuStrip1->Refresh();
        this->toolStrip1->Refresh();
    }
    
    private: System::Void mainFormSizeChanged(System::Object^  sender, System::EventArgs^  e) {
        this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
        System::Drawing::Rectangle screenRectangle = RectangleToScreen(this->ClientRectangle);
        int titleHeight = screenRectangle.Top - this->Top;
        
        this->groupBoxColor->Margin = System::Windows::Forms::Padding(0, 0, 14, titleHeight);
        this->grpBox_StickCal->Margin = System::Windows::Forms::Padding(0, 0, 0, titleHeight);
    }
};
}


