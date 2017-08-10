#pragma once
#include <iomanip>
#include <sstream>

#include <msclr\marshal_cppstd.h>

#include "jctool.h"
#include "overrides.h"

namespace CppWinFormJoy {

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
			debug_is_on = 0;
			set_led_busy();

			InitializeComponent();

			myform1 = this;

			this->btnWriteBody->Enabled = false;

			if (handle_ok != 3) {
				this->textBoxSN->Text = gcnew String(get_sn(handle, 0x6001, 0xF).c_str());
				this->textBox_chg_sn->Text = this->textBoxSN->Text;
			}
			else {
				this->textBoxSN->Text = L"No S/N";
				this->textBox_chg_sn->Text = this->textBoxSN->Text;
				this->btnClrDlg2->Visible = false;
			}
			unsigned char device_info[10];
			memset(device_info, 0, sizeof(device_info));

			get_device_info(handle, device_info);
			
			this->textBoxFW->Text = String::Format("{0:X}.{1:X2}", device_info[0], device_info[1]);
			this->textBoxMAC->Text = String::Format("{0:X2}:{1:X2}:{2:X2}:{3:X2}:{4:X2}:{5:X2}", device_info[4], device_info[5], device_info[6], device_info[7], device_info[8], device_info[9]);

			if (handle_ok == 1)
				this->textBoxDev->Text = L"Joy-Con (L)";
			else if (handle_ok == 2)
				this->textBoxDev->Text = L"Joy-Con (R)";
			else if (handle_ok == 3)
				this->textBoxDev->Text = L"Pro Controller";

			update_battery();
			update_colors_from_spi(true);

			/*
			BOOL chk = AllocConsole();
			if (chk)
			{
				freopen("CONOUT$", "w", stdout);
				printf(" printing to console\n");
			}
			*/
		
			this->comboBox1->Items->Add("Restore Color");
			this->comboBox1->Items->Add("Restore S/N");
			this->comboBox1->Items->Add("Restore User Calibration");
			this->comboBox1->Items->Add("Factory Reset User Calibration");
			this->comboBox1->Items->Add("Full Restore");
			this->comboBox1->DrawItem += gcnew System::Windows::Forms::DrawItemEventHandler(this, &FormJoy::comboBox1_DrawItem);
			
			this->menuStrip1->Renderer = gcnew System::Windows::Forms::ToolStripProfessionalRenderer(gcnew Overrides::TestColorTable());

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

			this->toolTip1->SetToolTip(this->label_sn, "Click here to change your S/N");
			
			//Initialise locations on start for easy designing
			this->menuStrip1->Size = System::Drawing::Size(485, 24);
			this->groupRst->Location = System::Drawing::Point(494, 36);
			this->groupBox_chg_sn->Location = System::Drawing::Point(494, 36);
			this->ClientSize = System::Drawing::Size(485, 449);
			
			//Done drawing!
			send_rumble(handle);
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

	array<byte>^ backup_spi;
	private: System::Windows::Forms::GroupBox^  groupBoxColor;
	private: System::Windows::Forms::Button^ btnWriteBody;
	private: System::Windows::Forms::TextBox^ textBoxSN;
	private: System::Windows::Forms::Label^ label_sn;
	private: System::Windows::Forms::Button^  btnClrDlg1;
	private: System::Windows::Forms::ColorDialog^  colorDialog1;
	private: System::Windows::Forms::Button^  btnClrDlg2;
	private: System::Windows::Forms::ColorDialog^  colorDialog2;
	private: System::Windows::Forms::Label^  label_hint;
	private: System::Windows::Forms::Label^  label_mac;
	private: System::Windows::Forms::Label^  label_fw;
	private: System::Windows::Forms::TextBox^  textBoxMAC;
	private: System::Windows::Forms::TextBox^  textBoxFW;
	private: System::Windows::Forms::TextBox^  textBoxDev;
	private: System::Windows::Forms::Label^  label_dev;
	private: System::Windows::Forms::Button^  button3;
	private: System::Windows::Forms::GroupBox^  groupBoxSPI;
	private: System::Windows::Forms::Label^  label6;
	private: System::Windows::Forms::PictureBox^  pictureBoxPreview;
	public: System::Windows::Forms::Label^  label_progress;
	private: System::Windows::Forms::PictureBox^  pictureBoxBattery;
	private: System::Windows::Forms::MenuStrip^  menuStrip1;
	private: System::Windows::Forms::ToolStripMenuItem^  menuToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  debugToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  aboutToolStripMenuItem;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::TextBox^  textBoxDbg_SubcmdArg;
	private: System::Windows::Forms::Button^  btnDbg_send_cmd;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::TextBox^  textBoxDbg_cmd;
	private: System::Windows::Forms::TextBox^  textBoxDbg_subcmd;
	private: System::Windows::Forms::Label^  label4;
	private: System::Windows::Forms::GroupBox^  groupRst;
	private: System::Windows::Forms::Button^  btnLoadBackup;
	private: System::Windows::Forms::Label^  label_rst_mac;
	private: System::Windows::Forms::TextBox^  textBox2;
	private: System::Windows::Forms::ComboBox^  comboBox1;
	private: System::Windows::Forms::Button^  btn_restore;
	private: System::Windows::Forms::GroupBox^  grpRstUser;
	private: System::Windows::Forms::CheckBox^  checkBox3;
	private: System::Windows::Forms::CheckBox^  checkBox2;
	private: System::Windows::Forms::CheckBox^  checkBox1;
	private: System::Windows::Forms::Label^  label5;
	private: System::Windows::Forms::Label^  label7;
	private: System::Windows::Forms::Button^  btbRestoreEnable;
	private: System::Windows::Forms::ErrorProvider^  errorProvider1;
	private: System::Windows::Forms::ErrorProvider^  errorProvider2;
	private: System::Windows::Forms::Label^  label8;
	private: System::Windows::Forms::TextBox^  textBoxDbg_lfamp;
	private: System::Windows::Forms::TextBox^  textBoxDbg_lfreq;
	private: System::Windows::Forms::TextBox^  textBoxDbg_hamp;
	private: System::Windows::Forms::TextBox^  textBoxDbg_hfreq;
	private: System::Windows::Forms::Label^  label12;
	private: System::Windows::Forms::Label^  label11;
	private: System::Windows::Forms::Label^  label10;
	private: System::Windows::Forms::Label^  label9;
	private: System::Windows::Forms::GroupBox^  groupDbg;
	private: System::Windows::Forms::GroupBox^  groupBox_chg_sn;
	private: System::Windows::Forms::Button^  btnChangeSn;
	private: System::Windows::Forms::TextBox^  textBox_chg_sn;
	private: System::Windows::Forms::Label^  label13;
	private: System::Windows::Forms::Label^  label_sn_change_warning;
	private: System::Windows::Forms::ToolTip^  toolTip1;
	public: System::Windows::Forms::TextBox^  textBoxDbg_sent;
	public: System::Windows::Forms::TextBox^  textBoxDbg_reply;
	private: System::ComponentModel::IContainer^  components;


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
			this->btbRestoreEnable = (gcnew System::Windows::Forms::Button());
			this->pictureBoxBattery = (gcnew System::Windows::Forms::PictureBox());
			this->pictureBoxPreview = (gcnew System::Windows::Forms::PictureBox());
			this->button3 = (gcnew System::Windows::Forms::Button());
			this->btnClrDlg1 = (gcnew System::Windows::Forms::Button());
			this->btnClrDlg2 = (gcnew System::Windows::Forms::Button());
			this->label_hint = (gcnew System::Windows::Forms::Label());
			this->groupBoxSPI = (gcnew System::Windows::Forms::GroupBox());
			this->label_progress = (gcnew System::Windows::Forms::Label());
			this->label6 = (gcnew System::Windows::Forms::Label());
			this->textBoxSN = (gcnew System::Windows::Forms::TextBox());
			this->label_sn = (gcnew System::Windows::Forms::Label());
			this->colorDialog1 = (gcnew System::Windows::Forms::ColorDialog());
			this->colorDialog2 = (gcnew System::Windows::Forms::ColorDialog());
			this->label_mac = (gcnew System::Windows::Forms::Label());
			this->label_fw = (gcnew System::Windows::Forms::Label());
			this->textBoxMAC = (gcnew System::Windows::Forms::TextBox());
			this->textBoxFW = (gcnew System::Windows::Forms::TextBox());
			this->textBoxDev = (gcnew System::Windows::Forms::TextBox());
			this->label_dev = (gcnew System::Windows::Forms::Label());
			this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
			this->menuToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->debugToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->aboutToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->textBoxDbg_SubcmdArg = (gcnew System::Windows::Forms::TextBox());
			this->btnDbg_send_cmd = (gcnew System::Windows::Forms::Button());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->textBoxDbg_cmd = (gcnew System::Windows::Forms::TextBox());
			this->textBoxDbg_subcmd = (gcnew System::Windows::Forms::TextBox());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->groupDbg = (gcnew System::Windows::Forms::GroupBox());
			this->textBoxDbg_reply = (gcnew System::Windows::Forms::TextBox());
			this->textBoxDbg_sent = (gcnew System::Windows::Forms::TextBox());
			this->label12 = (gcnew System::Windows::Forms::Label());
			this->label11 = (gcnew System::Windows::Forms::Label());
			this->label10 = (gcnew System::Windows::Forms::Label());
			this->label9 = (gcnew System::Windows::Forms::Label());
			this->textBoxDbg_lfamp = (gcnew System::Windows::Forms::TextBox());
			this->textBoxDbg_lfreq = (gcnew System::Windows::Forms::TextBox());
			this->textBoxDbg_hamp = (gcnew System::Windows::Forms::TextBox());
			this->textBoxDbg_hfreq = (gcnew System::Windows::Forms::TextBox());
			this->label8 = (gcnew System::Windows::Forms::Label());
			this->groupRst = (gcnew System::Windows::Forms::GroupBox());
			this->comboBox1 = (gcnew System::Windows::Forms::ComboBox());
			this->label7 = (gcnew System::Windows::Forms::Label());
			this->grpRstUser = (gcnew System::Windows::Forms::GroupBox());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->checkBox3 = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox2 = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox1 = (gcnew System::Windows::Forms::CheckBox());
			this->btn_restore = (gcnew System::Windows::Forms::Button());
			this->textBox2 = (gcnew System::Windows::Forms::TextBox());
			this->label_rst_mac = (gcnew System::Windows::Forms::Label());
			this->btnLoadBackup = (gcnew System::Windows::Forms::Button());
			this->errorProvider1 = (gcnew System::Windows::Forms::ErrorProvider(this->components));
			this->errorProvider2 = (gcnew System::Windows::Forms::ErrorProvider(this->components));
			this->groupBox_chg_sn = (gcnew System::Windows::Forms::GroupBox());
			this->label13 = (gcnew System::Windows::Forms::Label());
			this->label_sn_change_warning = (gcnew System::Windows::Forms::Label());
			this->btnChangeSn = (gcnew System::Windows::Forms::Button());
			this->textBox_chg_sn = (gcnew System::Windows::Forms::TextBox());
			this->toolTip1 = (gcnew System::Windows::Forms::ToolTip(this->components));
			this->groupBoxColor->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxBattery))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxPreview))->BeginInit();
			this->groupBoxSPI->SuspendLayout();
			this->menuStrip1->SuspendLayout();
			this->groupDbg->SuspendLayout();
			this->groupRst->SuspendLayout();
			this->grpRstUser->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->errorProvider1))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->errorProvider2))->BeginInit();
			this->groupBox_chg_sn->SuspendLayout();
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
			this->btnWriteBody->Location = System::Drawing::Point(316, 253);
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
			this->groupBoxColor->Controls->Add(this->btbRestoreEnable);
			this->groupBoxColor->Controls->Add(this->pictureBoxBattery);
			this->groupBoxColor->Controls->Add(this->pictureBoxPreview);
			this->groupBoxColor->Controls->Add(this->button3);
			this->groupBoxColor->Controls->Add(this->btnClrDlg1);
			this->groupBoxColor->Controls->Add(this->btnWriteBody);
			this->groupBoxColor->Controls->Add(this->btnClrDlg2);
			this->groupBoxColor->Controls->Add(this->label_hint);
			this->groupBoxColor->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(161)));
			this->groupBoxColor->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
				static_cast<System::Int32>(static_cast<System::Byte>(206)));
			this->groupBoxColor->Location = System::Drawing::Point(14, 120);
			this->groupBoxColor->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
			this->groupBoxColor->Name = L"groupBoxColor";
			this->groupBoxColor->Padding = System::Windows::Forms::Padding(3, 4, 3, 4);
			this->groupBoxColor->Size = System::Drawing::Size(456, 315);
			this->groupBoxColor->TabIndex = 0;
			this->groupBoxColor->TabStop = false;
			this->groupBoxColor->Text = L"Device colors";
			// 
			// btbRestoreEnable
			// 
			this->btbRestoreEnable->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)));
			this->btbRestoreEnable->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
			this->btbRestoreEnable->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btbRestoreEnable->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 9.75F, System::Drawing::FontStyle::Bold));
			this->btbRestoreEnable->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)),
				static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)));
			this->btbRestoreEnable->Location = System::Drawing::Point(11, 25);
			this->btbRestoreEnable->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
			this->btbRestoreEnable->Name = L"btbRestoreEnable";
			this->btbRestoreEnable->Size = System::Drawing::Size(144, 36);
			this->btbRestoreEnable->TabIndex = 18;
			this->btbRestoreEnable->Text = L"Restore SPI";
			this->btbRestoreEnable->UseVisualStyleBackColor = false;
			this->btbRestoreEnable->Click += gcnew System::EventHandler(this, &FormJoy::btbRestoreEnable_Click);
			// 
			// pictureBoxBattery
			// 
			this->pictureBoxBattery->Cursor = System::Windows::Forms::Cursors::Arrow;
			this->pictureBoxBattery->Location = System::Drawing::Point(141, 266);
			this->pictureBoxBattery->Name = L"pictureBoxBattery";
			this->pictureBoxBattery->Size = System::Drawing::Size(48, 18);
			this->pictureBoxBattery->TabIndex = 17;
			this->pictureBoxBattery->TabStop = false;
			this->pictureBoxBattery->Click += gcnew System::EventHandler(this, &FormJoy::pictureBoxBattery_Click);
			// 
			// pictureBoxPreview
			// 
			this->pictureBoxPreview->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
				static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
			this->pictureBoxPreview->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Zoom;
			this->pictureBoxPreview->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
				static_cast<System::Int32>(static_cast<System::Byte>(206)));
			this->pictureBoxPreview->Location = System::Drawing::Point(1, 66);
			this->pictureBoxPreview->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
			this->pictureBoxPreview->Name = L"pictureBoxPreview";
			this->pictureBoxPreview->Size = System::Drawing::Size(312, 192);
			this->pictureBoxPreview->TabIndex = 15;
			this->pictureBoxPreview->TabStop = false;
			// 
			// button3
			// 
			this->button3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)));
			this->button3->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
			this->button3->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button3->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 9.75F, System::Drawing::FontStyle::Bold));
			this->button3->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
				static_cast<System::Int32>(static_cast<System::Byte>(251)));
			this->button3->Location = System::Drawing::Point(169, 25);
			this->button3->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
			this->button3->Name = L"button3";
			this->button3->Size = System::Drawing::Size(144, 36);
			this->button3->TabIndex = 1;
			this->button3->Text = L"Backup SPI";
			this->button3->UseVisualStyleBackColor = false;
			this->button3->Click += gcnew System::EventHandler(this, &FormJoy::button3_Click);
			// 
			// btnClrDlg1
			// 
			this->btnClrDlg1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)));
			this->btnClrDlg1->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
			this->btnClrDlg1->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnClrDlg1->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 9.75F, System::Drawing::FontStyle::Bold));
			this->btnClrDlg1->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
				static_cast<System::Int32>(static_cast<System::Byte>(206)));
			this->btnClrDlg1->Location = System::Drawing::Point(316, 66);
			this->btnClrDlg1->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
			this->btnClrDlg1->Name = L"btnClrDlg1";
			this->btnClrDlg1->Size = System::Drawing::Size(128, 60);
			this->btnClrDlg1->TabIndex = 4;
			this->btnClrDlg1->Text = L"Body Color";
			this->btnClrDlg1->UseVisualStyleBackColor = false;
			this->btnClrDlg1->Click += gcnew System::EventHandler(this, &FormJoy::btnClrDlg1_Click);
			// 
			// btnClrDlg2
			// 
			this->btnClrDlg2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)));
			this->btnClrDlg2->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
			this->btnClrDlg2->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->btnClrDlg2->Font = (gcnew System::Drawing::Font(L"Segoe UI Semibold", 9.75F, System::Drawing::FontStyle::Bold));
			this->btnClrDlg2->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
				static_cast<System::Int32>(static_cast<System::Byte>(206)));
			this->btnClrDlg2->Location = System::Drawing::Point(316, 126);
			this->btnClrDlg2->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
			this->btnClrDlg2->Name = L"btnClrDlg2";
			this->btnClrDlg2->Size = System::Drawing::Size(128, 60);
			this->btnClrDlg2->TabIndex = 5;
			this->btnClrDlg2->Text = L"Buttons Color";
			this->btnClrDlg2->UseVisualStyleBackColor = false;
			this->btnClrDlg2->Click += gcnew System::EventHandler(this, &FormJoy::btnClrDlg2_Click);
			// 
			// label_hint
			// 
			this->label_hint->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
				static_cast<System::Int32>(static_cast<System::Byte>(70)));
			this->label_hint->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(161)));
			this->label_hint->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
				static_cast<System::Int32>(static_cast<System::Byte>(251)));
			this->label_hint->Location = System::Drawing::Point(319, 202);
			this->label_hint->Name = L"label_hint";
			this->label_hint->Size = System::Drawing::Size(125, 36);
			this->label_hint->TabIndex = 7;
			this->label_hint->Text = L"Select colors,\nthen hit write.";
			// 
			// groupBoxSPI
			// 
			this->groupBoxSPI->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
				static_cast<System::Int32>(static_cast<System::Byte>(70)));
			this->groupBoxSPI->Controls->Add(this->label_progress);
			this->groupBoxSPI->Controls->Add(this->label6);
			this->groupBoxSPI->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
				static_cast<System::Int32>(static_cast<System::Byte>(206)));
			this->groupBoxSPI->Location = System::Drawing::Point(14, 120);
			this->groupBoxSPI->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
			this->groupBoxSPI->Name = L"groupBoxSPI";
			this->groupBoxSPI->Padding = System::Windows::Forms::Padding(3, 4, 3, 4);
			this->groupBoxSPI->Size = System::Drawing::Size(456, 315);
			this->groupBoxSPI->TabIndex = 14;
			this->groupBoxSPI->TabStop = false;
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
			// label6
			// 
			this->label6->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
				static_cast<System::Int32>(static_cast<System::Byte>(70)));
			this->label6->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(161)));
			this->label6->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
				static_cast<System::Int32>(static_cast<System::Byte>(251)));
			this->label6->Location = System::Drawing::Point(87, 87);
			this->label6->Name = L"label6";
			this->label6->Size = System::Drawing::Size(279, 97);
			this->label6->TabIndex = 0;
			this->label6->Text = L"Dumping SPI flash chip!\nThis will take around 10 minutes...\n\nDon\'t disconnect you"
				L"r device!";
			this->label6->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
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
			this->textBoxSN->Location = System::Drawing::Point(72, 36);
			this->textBoxSN->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
			this->textBoxSN->MaxLength = 16;
			this->textBoxSN->Name = L"textBoxSN";
			this->textBoxSN->ReadOnly = true;
			this->textBoxSN->Size = System::Drawing::Size(138, 20);
			this->textBoxSN->TabIndex = 2;
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
			this->label_sn->Location = System::Drawing::Point(10, 36);
			this->label_sn->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
			this->label_sn->Name = L"label_sn";
			this->label_sn->Size = System::Drawing::Size(39, 20);
			this->label_sn->TabIndex = 3;
			this->label_sn->Text = L"S/N:";
			this->label_sn->Click += gcnew System::EventHandler(this, &FormJoy::label_sn_Click);
			// 
			// colorDialog1
			// 
			this->colorDialog1->AnyColor = true;
			this->colorDialog1->FullOpen = true;
			// 
			// colorDialog2
			// 
			this->colorDialog2->AnyColor = true;
			this->colorDialog2->FullOpen = true;
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
			this->label_mac->Location = System::Drawing::Point(10, 77);
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
			this->label_fw->Location = System::Drawing::Point(234, 36);
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
			this->textBoxMAC->Location = System::Drawing::Point(72, 77);
			this->textBoxMAC->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
			this->textBoxMAC->Name = L"textBoxMAC";
			this->textBoxMAC->ReadOnly = true;
			this->textBoxMAC->Size = System::Drawing::Size(138, 20);
			this->textBoxMAC->TabIndex = 10;
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
			this->textBoxFW->Location = System::Drawing::Point(351, 36);
			this->textBoxFW->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
			this->textBoxFW->Name = L"textBoxFW";
			this->textBoxFW->ReadOnly = true;
			this->textBoxFW->Size = System::Drawing::Size(114, 20);
			this->textBoxFW->TabIndex = 11;
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
			this->textBoxDev->Location = System::Drawing::Point(351, 77);
			this->textBoxDev->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
			this->textBoxDev->Name = L"textBoxDev";
			this->textBoxDev->ReadOnly = true;
			this->textBoxDev->Size = System::Drawing::Size(114, 20);
			this->textBoxDev->TabIndex = 13;
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
			this->label_dev->Location = System::Drawing::Point(234, 77);
			this->label_dev->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
			this->label_dev->Name = L"label_dev";
			this->label_dev->Size = System::Drawing::Size(83, 20);
			this->label_dev->TabIndex = 12;
			this->label_dev->Text = L"Controller:";
			// 
			// menuStrip1
			// 
			this->menuStrip1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)));
			this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { this->menuToolStripMenuItem });
			this->menuStrip1->Location = System::Drawing::Point(0, 0);
			this->menuStrip1->Name = L"menuStrip1";
			this->menuStrip1->Size = System::Drawing::Size(1217, 24);
			this->menuStrip1->TabIndex = 0;
			this->menuStrip1->Text = L"menuStrip1";
			// 
			// menuToolStripMenuItem
			// 
			this->menuToolStripMenuItem->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
			this->menuToolStripMenuItem->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->menuToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {
				this->debugToolStripMenuItem,
					this->aboutToolStripMenuItem
			});
			this->menuToolStripMenuItem->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)),
				static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(206)));
			this->menuToolStripMenuItem->Name = L"menuToolStripMenuItem";
			this->menuToolStripMenuItem->Size = System::Drawing::Size(50, 20);
			this->menuToolStripMenuItem->Text = L"Menu";
			// 
			// debugToolStripMenuItem
			// 
			this->debugToolStripMenuItem->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
			this->debugToolStripMenuItem->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->debugToolStripMenuItem->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)),
				static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(206)));
			this->debugToolStripMenuItem->Name = L"debugToolStripMenuItem";
			this->debugToolStripMenuItem->Size = System::Drawing::Size(109, 22);
			this->debugToolStripMenuItem->Text = L"Debug";
			this->debugToolStripMenuItem->Click += gcnew System::EventHandler(this, &FormJoy::debugToolStripMenuItem_Click);
			// 
			// aboutToolStripMenuItem
			// 
			this->aboutToolStripMenuItem->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
			this->aboutToolStripMenuItem->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->aboutToolStripMenuItem->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)),
				static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(206)));
			this->aboutToolStripMenuItem->Name = L"aboutToolStripMenuItem";
			this->aboutToolStripMenuItem->Size = System::Drawing::Size(109, 22);
			this->aboutToolStripMenuItem->Text = L"About";
			this->aboutToolStripMenuItem->Click += gcnew System::EventHandler(this, &FormJoy::aboutToolStripMenuItem_Click);
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
				static_cast<System::Int32>(static_cast<System::Byte>(251)));
			this->label1->Location = System::Drawing::Point(13, 129);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(124, 17);
			this->label1->TabIndex = 15;
			this->label1->Text = L"Subcmd arguments:";
			// 
			// textBoxDbg_SubcmdArg
			// 
			this->textBoxDbg_SubcmdArg->AutoCompleteSource = System::Windows::Forms::AutoCompleteSource::HistoryList;
			this->textBoxDbg_SubcmdArg->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)));
			this->textBoxDbg_SubcmdArg->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->textBoxDbg_SubcmdArg->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
			this->textBoxDbg_SubcmdArg->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
				static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
			this->textBoxDbg_SubcmdArg->Location = System::Drawing::Point(16, 149);
			this->textBoxDbg_SubcmdArg->MaxLength = 50;
			this->textBoxDbg_SubcmdArg->Multiline = true;
			this->textBoxDbg_SubcmdArg->Name = L"textBoxDbg_SubcmdArg";
			this->textBoxDbg_SubcmdArg->Size = System::Drawing::Size(188, 40);
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
			this->btnDbg_send_cmd->Location = System::Drawing::Point(73, 202);
			this->btnDbg_send_cmd->Name = L"btnDbg_send_cmd";
			this->btnDbg_send_cmd->Size = System::Drawing::Size(75, 34);
			this->btnDbg_send_cmd->TabIndex = 17;
			this->btnDbg_send_cmd->Text = L"Send";
			this->btnDbg_send_cmd->UseVisualStyleBackColor = false;
			this->btnDbg_send_cmd->Click += gcnew System::EventHandler(this, &FormJoy::btnDbg_send_cmd_Click);
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
				static_cast<System::Int32>(static_cast<System::Byte>(251)));
			this->label2->Location = System::Drawing::Point(13, 98);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(38, 17);
			this->label2->TabIndex = 18;
			this->label2->Text = L"Cmd:";
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
				static_cast<System::Int32>(static_cast<System::Byte>(251)));
			this->label3->Location = System::Drawing::Point(106, 98);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(58, 17);
			this->label3->TabIndex = 19;
			this->label3->Text = L"Subcmd:";
			// 
			// textBoxDbg_cmd
			// 
			this->textBoxDbg_cmd->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)));
			this->textBoxDbg_cmd->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->textBoxDbg_cmd->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
			this->textBoxDbg_cmd->ForeColor = System::Drawing::Color::White;
			this->textBoxDbg_cmd->Location = System::Drawing::Point(64, 96);
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
			this->textBoxDbg_subcmd->Location = System::Drawing::Point(169, 96);
			this->textBoxDbg_subcmd->MaxLength = 2;
			this->textBoxDbg_subcmd->Name = L"textBoxDbg_subcmd";
			this->textBoxDbg_subcmd->Size = System::Drawing::Size(35, 25);
			this->textBoxDbg_subcmd->TabIndex = 24;
			this->textBoxDbg_subcmd->Text = L"00";
			this->textBoxDbg_subcmd->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label4
			// 
			this->label4->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
				static_cast<System::Int32>(static_cast<System::Byte>(0)));
			this->label4->Location = System::Drawing::Point(20, 275);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(184, 97);
			this->label4->TabIndex = 22;
			this->label4->Text = L"The debug feature is only for developer use!\r\n\r\nNo one is responsible for any com"
				L"mand sent.";
			// 
			// groupDbg
			// 
			this->groupDbg->Controls->Add(this->textBoxDbg_reply);
			this->groupDbg->Controls->Add(this->textBoxDbg_sent);
			this->groupDbg->Controls->Add(this->label12);
			this->groupDbg->Controls->Add(this->label11);
			this->groupDbg->Controls->Add(this->label10);
			this->groupDbg->Controls->Add(this->label9);
			this->groupDbg->Controls->Add(this->textBoxDbg_lfamp);
			this->groupDbg->Controls->Add(this->textBoxDbg_lfreq);
			this->groupDbg->Controls->Add(this->textBoxDbg_hamp);
			this->groupDbg->Controls->Add(this->textBoxDbg_hfreq);
			this->groupDbg->Controls->Add(this->label8);
			this->groupDbg->Controls->Add(this->label1);
			this->groupDbg->Controls->Add(this->label4);
			this->groupDbg->Controls->Add(this->textBoxDbg_SubcmdArg);
			this->groupDbg->Controls->Add(this->textBoxDbg_subcmd);
			this->groupDbg->Controls->Add(this->btnDbg_send_cmd);
			this->groupDbg->Controls->Add(this->textBoxDbg_cmd);
			this->groupDbg->Controls->Add(this->label2);
			this->groupDbg->Controls->Add(this->label3);
			this->groupDbg->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
				static_cast<System::Int32>(static_cast<System::Byte>(206)));
			this->groupDbg->Location = System::Drawing::Point(494, 36);
			this->groupDbg->Name = L"groupDbg";
			this->groupDbg->Size = System::Drawing::Size(220, 399);
			this->groupDbg->TabIndex = 23;
			this->groupDbg->TabStop = false;
			this->groupDbg->Text = L"Debug custom cmd";
			this->groupDbg->Visible = false;
			// 
			// textBoxDbg_reply
			// 
			this->textBoxDbg_reply->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
				static_cast<System::Int32>(static_cast<System::Byte>(70)));
			this->textBoxDbg_reply->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->textBoxDbg_reply->Font = (gcnew System::Drawing::Font(L"Lucida Console", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(161)));
			this->textBoxDbg_reply->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
				static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
			this->textBoxDbg_reply->Location = System::Drawing::Point(12, 320);
			this->textBoxDbg_reply->Multiline = true;
			this->textBoxDbg_reply->Name = L"textBoxDbg_reply";
			this->textBoxDbg_reply->ReadOnly = true;
			this->textBoxDbg_reply->Size = System::Drawing::Size(196, 69);
			this->textBoxDbg_reply->TabIndex = 33;
			this->textBoxDbg_reply->Text = L"Reply text";
			this->textBoxDbg_reply->Visible = false;
			// 
			// textBoxDbg_sent
			// 
			this->textBoxDbg_sent->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
				static_cast<System::Int32>(static_cast<System::Byte>(70)));
			this->textBoxDbg_sent->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->textBoxDbg_sent->Font = (gcnew System::Drawing::Font(L"Lucida Console", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(161)));
			this->textBoxDbg_sent->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)),
				static_cast<System::Int32>(static_cast<System::Byte>(0)));
			this->textBoxDbg_sent->Location = System::Drawing::Point(12, 247);
			this->textBoxDbg_sent->Multiline = true;
			this->textBoxDbg_sent->Name = L"textBoxDbg_sent";
			this->textBoxDbg_sent->ReadOnly = true;
			this->textBoxDbg_sent->Size = System::Drawing::Size(196, 62);
			this->textBoxDbg_sent->TabIndex = 32;
			this->textBoxDbg_sent->Text = L"Sent text";
			this->textBoxDbg_sent->Visible = false;
			// 
			// label12
			// 
			this->label12->AutoSize = true;
			this->label12->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(200)),
				static_cast<System::Int32>(static_cast<System::Byte>(200)));
			this->label12->Location = System::Drawing::Point(114, 66);
			this->label12->Name = L"label12";
			this->label12->Size = System::Drawing::Size(47, 17);
			this->label12->TabIndex = 31;
			this->label12->Text = L"L.Amp:";
			// 
			// label11
			// 
			this->label11->AutoSize = true;
			this->label11->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(200)),
				static_cast<System::Int32>(static_cast<System::Byte>(200)));
			this->label11->Location = System::Drawing::Point(114, 42);
			this->label11->Name = L"label11";
			this->label11->Size = System::Drawing::Size(50, 17);
			this->label11->TabIndex = 30;
			this->label11->Text = L"H.Amp:";
			// 
			// label10
			// 
			this->label10->AutoSize = true;
			this->label10->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(200)),
				static_cast<System::Int32>(static_cast<System::Byte>(200)));
			this->label10->Location = System::Drawing::Point(13, 66);
			this->label10->Name = L"label10";
			this->label10->Size = System::Drawing::Size(46, 17);
			this->label10->TabIndex = 29;
			this->label10->Text = L"L.Freq:";
			// 
			// label9
			// 
			this->label9->AutoSize = true;
			this->label9->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(200)),
				static_cast<System::Int32>(static_cast<System::Byte>(200)));
			this->label9->Location = System::Drawing::Point(13, 42);
			this->label9->Name = L"label9";
			this->label9->Size = System::Drawing::Size(49, 17);
			this->label9->TabIndex = 28;
			this->label9->Text = L"H.Freq:";
			// 
			// textBoxDbg_lfamp
			// 
			this->textBoxDbg_lfamp->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(85)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)));
			this->textBoxDbg_lfamp->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->textBoxDbg_lfamp->CharacterCasing = System::Windows::Forms::CharacterCasing::Upper;
			this->textBoxDbg_lfamp->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(250)),
				static_cast<System::Int32>(static_cast<System::Byte>(250)), static_cast<System::Int32>(static_cast<System::Byte>(250)));
			this->textBoxDbg_lfamp->Location = System::Drawing::Point(169, 66);
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
			this->textBoxDbg_lfreq->Location = System::Drawing::Point(64, 66);
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
			// label8
			// 
			this->label8->AutoSize = true;
			this->label8->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
				static_cast<System::Int32>(static_cast<System::Byte>(251)));
			this->label8->Location = System::Drawing::Point(13, 21);
			this->label8->Name = L"label8";
			this->label8->Size = System::Drawing::Size(64, 17);
			this->label8->TabIndex = 23;
			this->label8->Text = L"Vibration:";
			// 
			// groupRst
			// 
			this->groupRst->Controls->Add(this->comboBox1);
			this->groupRst->Controls->Add(this->label7);
			this->groupRst->Controls->Add(this->grpRstUser);
			this->groupRst->Controls->Add(this->btn_restore);
			this->groupRst->Controls->Add(this->textBox2);
			this->groupRst->Controls->Add(this->label_rst_mac);
			this->groupRst->Controls->Add(this->btnLoadBackup);
			this->groupRst->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
				static_cast<System::Int32>(static_cast<System::Byte>(206)));
			this->groupRst->Location = System::Drawing::Point(736, 38);
			this->groupRst->Name = L"groupRst";
			this->groupRst->Size = System::Drawing::Size(220, 399);
			this->groupRst->TabIndex = 24;
			this->groupRst->TabStop = false;
			this->groupRst->Text = L"Restore";
			this->groupRst->Visible = false;
			// 
			// comboBox1
			// 
			this->comboBox1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
				static_cast<System::Int32>(static_cast<System::Byte>(70)));
			this->comboBox1->DrawMode = System::Windows::Forms::DrawMode::OwnerDrawFixed;
			this->comboBox1->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->comboBox1->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
				static_cast<System::Int32>(static_cast<System::Byte>(206)));
			this->comboBox1->FormattingEnabled = true;
			this->comboBox1->Location = System::Drawing::Point(8, 69);
			this->comboBox1->Name = L"comboBox1";
			this->comboBox1->Size = System::Drawing::Size(203, 26);
			this->comboBox1->TabIndex = 5;
			this->comboBox1->Visible = false;
			this->comboBox1->SelectedIndexChanged += gcnew System::EventHandler(this, &FormJoy::comboBox1_SelectedIndexChanged);
			// 
			// label7
			// 
			this->label7->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
				static_cast<System::Int32>(static_cast<System::Byte>(251)));
			this->label7->Location = System::Drawing::Point(14, 101);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(188, 121);
			this->label7->TabIndex = 4;
			this->label7->Visible = false;
			// 
			// grpRstUser
			// 
			this->grpRstUser->Controls->Add(this->label5);
			this->grpRstUser->Controls->Add(this->checkBox3);
			this->grpRstUser->Controls->Add(this->checkBox2);
			this->grpRstUser->Controls->Add(this->checkBox1);
			this->grpRstUser->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->grpRstUser->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
				static_cast<System::Int32>(static_cast<System::Byte>(206)));
			this->grpRstUser->Location = System::Drawing::Point(8, 85);
			this->grpRstUser->Name = L"grpRstUser";
			this->grpRstUser->Size = System::Drawing::Size(203, 265);
			this->grpRstUser->TabIndex = 7;
			this->grpRstUser->TabStop = false;
			this->grpRstUser->Visible = false;
			// 
			// label5
			// 
			this->label5->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
				static_cast<System::Int32>(static_cast<System::Byte>(251)));
			this->label5->Location = System::Drawing::Point(6, 17);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(188, 153);
			this->label5->TabIndex = 3;
			// 
			// checkBox3
			// 
			this->checkBox3->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
			this->checkBox3->Location = System::Drawing::Point(6, 238);
			this->checkBox3->Name = L"checkBox3";
			this->checkBox3->RightToLeft = System::Windows::Forms::RightToLeft::No;
			this->checkBox3->Size = System::Drawing::Size(188, 21);
			this->checkBox3->TabIndex = 2;
			this->checkBox3->Text = L"6-Axis Sensor";
			// 
			// checkBox2
			// 
			this->checkBox2->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
			this->checkBox2->Location = System::Drawing::Point(6, 212);
			this->checkBox2->Name = L"checkBox2";
			this->checkBox2->RightToLeft = System::Windows::Forms::RightToLeft::No;
			this->checkBox2->Size = System::Drawing::Size(188, 21);
			this->checkBox2->TabIndex = 1;
			this->checkBox2->Text = L"Right Stick";
			// 
			// checkBox1
			// 
			this->checkBox1->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
			this->checkBox1->Location = System::Drawing::Point(6, 186);
			this->checkBox1->Name = L"checkBox1";
			this->checkBox1->RightToLeft = System::Windows::Forms::RightToLeft::No;
			this->checkBox1->Size = System::Drawing::Size(188, 21);
			this->checkBox1->TabIndex = 0;
			this->checkBox1->Text = L"Left Stick";
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
			// textBox2
			// 
			this->textBox2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
				static_cast<System::Int32>(static_cast<System::Byte>(70)));
			this->textBox2->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->textBox2->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
				static_cast<System::Int32>(static_cast<System::Byte>(206)));
			this->textBox2->Location = System::Drawing::Point(111, 38);
			this->textBox2->Name = L"textBox2";
			this->textBox2->Size = System::Drawing::Size(100, 18);
			this->textBox2->TabIndex = 2;
			this->textBox2->Text = L"No file loaded";
			// 
			// label_rst_mac
			// 
			this->label_rst_mac->AutoSize = true;
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
			// errorProvider1
			// 
			this->errorProvider1->ContainerControl = this;
			// 
			// errorProvider2
			// 
			this->errorProvider2->ContainerControl = this;
			// 
			// groupBox_chg_sn
			// 
			this->groupBox_chg_sn->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
				static_cast<System::Int32>(static_cast<System::Byte>(70)));
			this->groupBox_chg_sn->Controls->Add(this->label13);
			this->groupBox_chg_sn->Controls->Add(this->label_sn_change_warning);
			this->groupBox_chg_sn->Controls->Add(this->btnChangeSn);
			this->groupBox_chg_sn->Controls->Add(this->textBox_chg_sn);
			this->groupBox_chg_sn->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(9)), static_cast<System::Int32>(static_cast<System::Byte>(255)),
				static_cast<System::Int32>(static_cast<System::Byte>(206)));
			this->groupBox_chg_sn->Location = System::Drawing::Point(979, 38);
			this->groupBox_chg_sn->Name = L"groupBox_chg_sn";
			this->groupBox_chg_sn->Size = System::Drawing::Size(220, 399);
			this->groupBox_chg_sn->TabIndex = 25;
			this->groupBox_chg_sn->TabStop = false;
			this->groupBox_chg_sn->Text = L"Change S/N";
			this->groupBox_chg_sn->Visible = false;
			// 
			// label13
			// 
			this->label13->AutoSize = true;
			this->label13->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(251)), static_cast<System::Int32>(static_cast<System::Byte>(251)),
				static_cast<System::Int32>(static_cast<System::Byte>(251)));
			this->label13->Location = System::Drawing::Point(14, 167);
			this->label13->Name = L"label13";
			this->label13->Size = System::Drawing::Size(63, 17);
			this->label13->TabIndex = 33;
			this->label13->Text = L"New S/N:";
			// 
			// label_sn_change_warning
			// 
			this->label_sn_change_warning->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)),
				static_cast<System::Int32>(static_cast<System::Byte>(188)), static_cast<System::Int32>(static_cast<System::Byte>(0)));
			this->label_sn_change_warning->Location = System::Drawing::Point(14, 28);
			this->label_sn_change_warning->Name = L"label_sn_change_warning";
			this->label_sn_change_warning->Size = System::Drawing::Size(189, 124);
			this->label_sn_change_warning->TabIndex = 32;
			this->label_sn_change_warning->Text = L"Make a backup first!\r\n\r\nIf you lose your serial number, bad things will happen!\r\n"
				L"\r\nMaximum 15 non-extended ASCII characters.";
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
			this->btnChangeSn->Location = System::Drawing::Point(110, 232);
			this->btnChangeSn->Name = L"btnChangeSn";
			this->btnChangeSn->Size = System::Drawing::Size(93, 30);
			this->btnChangeSn->TabIndex = 1;
			this->btnChangeSn->Text = L"Change SN";
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
			this->textBox_chg_sn->Location = System::Drawing::Point(17, 191);
			this->textBox_chg_sn->MaxLength = 15;
			this->textBox_chg_sn->Name = L"textBox_chg_sn";
			this->textBox_chg_sn->Size = System::Drawing::Size(186, 25);
			this->textBox_chg_sn->TabIndex = 0;
			this->textBox_chg_sn->Text = L"";
			// 
			// FormJoy
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(7, 17);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)),
				static_cast<System::Int32>(static_cast<System::Byte>(70)));
			this->ClientSize = System::Drawing::Size(1217, 449);
			this->Controls->Add(this->groupBox_chg_sn);
			this->Controls->Add(this->groupRst);
			this->Controls->Add(this->groupDbg);
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
			this->Controls->Add(this->menuStrip1);
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
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Joy-Con Toolkit v1.5.2";
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &FormJoy::Form1_FormClosing);
			this->groupBoxColor->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxBattery))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxPreview))->EndInit();
			this->groupBoxSPI->ResumeLayout(false);
			this->menuStrip1->ResumeLayout(false);
			this->menuStrip1->PerformLayout();
			this->groupDbg->ResumeLayout(false);
			this->groupDbg->PerformLayout();
			this->groupRst->ResumeLayout(false);
			this->groupRst->PerformLayout();
			this->grpRstUser->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->errorProvider1))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->errorProvider2))->EndInit();
			this->groupBox_chg_sn->ResumeLayout(false);
			this->groupBox_chg_sn->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void btnWriteBody_Click(System::Object^ sender, System::EventArgs^ e) {
		if (!device_connection()) {
			MessageBox::Show(L"The device was disconnected!\n\nPress a button on the controller to connect\nand try to write the colors again!",L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
			return;
		}
		set_led_busy();
		if (MessageBox::Show(L"Don't forget to make a backup first!\nYou can also find retail colors at the bottom of the colors dialog for each type.\n\nNote: Pro controller buttons cannot be changed.\n\nAre you sure you want to continue?", L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
		{
			this->btnWriteBody->Enabled = false;

			unsigned char body_color[0x3];
			memset(body_color, 0, 0x3);
			unsigned char button_color[0x3];
			memset(button_color, 0, 0x3);

			body_color[0] = (u8)colorDialog1->Color.R;
			body_color[1] = (u8)colorDialog1->Color.G;
			body_color[2] = (u8)colorDialog1->Color.B;

			button_color[0] = (u8)colorDialog2->Color.R;
			button_color[1] = (u8)colorDialog2->Color.G;
			button_color[2] = (u8)colorDialog2->Color.B;

			write_spi_data(handle, 0x6050, 0x3, body_color);
			if (handle_ok != 3)
				write_spi_data(handle, 0x6053, 0x3, button_color);

			send_rumble(handle);
			MessageBox::Show(L"The colors were writen to the device!", L"Done!", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);

			//Check that the colors were written
			update_colors_from_spi(false);

			update_battery();
		}
	}

	private: System::Void btnClrDlg1_Click(System::Object^ sender, System::EventArgs^ e) {
		if(handle_ok !=3 )
			this->colorDialog1->CustomColors = getCustomColorFromConfig("bodycolors");
		else
			this->colorDialog1->CustomColors = getCustomColorFromConfig("bodycolors_pro");

		if (colorDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			this->btnClrDlg1->Text = L"Body Color\n#" + String::Format("{0:X6}", (colorDialog1->Color.R << 16) + (colorDialog1->Color.G << 8) + (colorDialog1->Color.B));

			update_joycon_color(colorDialog1->Color.R, colorDialog1->Color.G, colorDialog1->Color.B, colorDialog2->Color.R, colorDialog2->Color.G, colorDialog2->Color.B);

			this->btnWriteBody->Enabled = true;

			if (handle_ok != 3)
				saveCustomColorToConfig(this->colorDialog1, "bodycolors");
			else
				saveCustomColorToConfig(this->colorDialog1, "bodycolors_pro");
		}
	}

	private: System::Void btnClrDlg2_Click(System::Object^ sender, System::EventArgs^ e) {
		this->colorDialog2->CustomColors = getCustomColorFromConfig("buttoncolors");
		if (colorDialog2->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			this->btnClrDlg2->Text = L"Buttons Color\n#" + String::Format("{0:X6}", (colorDialog2->Color.R << 16) + (colorDialog2->Color.G << 8) + (colorDialog2->Color.B));

			update_joycon_color(colorDialog1->Color.R, colorDialog1->Color.G, colorDialog1->Color.B, colorDialog2->Color.R, 
				colorDialog2->Color.G, colorDialog2->Color.B);
			
			this->btnWriteBody->Enabled = true;
			
			saveCustomColorToConfig(this->colorDialog2, "buttoncolors");
		}
	}

	private: System::Void button3_Click(System::Object^  sender, System::EventArgs^  e) {
		if (!device_connection()) {
			MessageBox::Show(L"The device was disconnected!\n\nPress a button on the controller to connect\nand try to backup your SPI flash again!", L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
			return;
		}

		int do_backup = 0;
		unsigned char device_info[10];
		memset(device_info, 0, sizeof(device_info));
		get_device_info(handle, device_info);

		String^ filename = L"spi_";
		if (handle_ok == 1)
			filename += "left_";
		else if (handle_ok == 2)
			filename += "right_";
		else if (handle_ok == 3)
			filename += "pro_";

		filename += String::Format("{0:X2}{1:X2}{2:X2}{3:X2}{4:X2}{5:X2}", device_info[4], device_info[5], device_info[6], device_info[7], device_info[8], device_info[9]);
		filename += L".bin";

		String^ path = Path::Combine(Path::GetDirectoryName(Application::ExecutablePath), filename);
		if (File::Exists(path))
		{
			if (MessageBox::Show(L"The file " + filename + L" already exists!\n\nDo you want to overwrite it?", L"Warning", MessageBoxButtons::YesNo, MessageBoxIcon::Warning, MessageBoxDefaultButton::Button2) == System::Windows::Forms::DialogResult::Yes)
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
			Application::DoEvents();
			send_rumble(handle);
			set_led_busy();

			int error = dump_spi(handle, (context.marshal_as<std::string>(filename)).c_str());
			this->groupBoxColor->Visible = true;
			handler_close = 0;
			if (!error) {
				send_rumble(handle);
				MessageBox::Show(L"Done dumping SPI!", L"SPI Dumping", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
			}
		}
		
		update_battery();
	}

	private: System::Void update_joycon_color(u8 r, u8 g, u8 b, u8 rb, u8 gb, u8 bb) {
		// Stretches the image to fit the pictureBox.
		this->pictureBoxPreview->SizeMode = PictureBoxSizeMode::StretchImage;
		this->pictureBoxPreview->ClientSize = System::Drawing::Size(295, 295);
		
		cli::array<unsigned char>^ imagebytes;

		if (handle_ok == 1) {
			String^ base64_l_joy = "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";
			imagebytes = Convert::FromBase64String(base64_l_joy);
		}
		else if (handle_ok == 3) {
			String^ base64_pro = "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";
			imagebytes = Convert::FromBase64String(base64_pro);
		}
		else {
			String^ base64_r_joy = "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WvU5phx148aNE31ci4uLlBHgfRjZtq1EIiHbtt0lmKZ0UCtsjVwuVzUNqKurS1988YWmp6c1Ojqq0dFRt5Z7Eqanp927zGZt/ovc6Q0Gg+5JZWNjQ6urq4rFYopGo2dmyls0GlUymdTOzs6Rj+W9e/e0tramK1euHPiYqZMPDAzwYmwQluuekPn5efdFZGYYvPPOO0f+/7lcrqoe984778i2bVmWpYGBATmO444qG8W2bfX09LhLSj/44AP19fUd65Sajo4OxWIx9fb2KpvNqqenxz1+rWxsbEySnnlj7a2tLd29e7dmSSoQCJypqwPC9owyU27C4bAePnwon89X84ZYZ2enhoaGNDQ0dOAyMJPJuHU8c9ltWZY70mvktB7LstTT0yPLstTX16ehoSHP5i36fD719vZqaGhIu7u7isViNXeJaiWRSETRaFSrq6svfNMrn89re3ub/W0J27MTtqamatt2zaCV9iZaV/ZoqhW45lLbXB6ar1s5pcrroB0cHHSDr6ur60j/r1AouGUBSdrY2FA+n1ehUKh7M6iSGT2bCfytHriJREKBQED3798/0vGppVgsamVlRZFIROPj47wQG4iaLY4taIeGhuqeNIyNjY0Dc2uN/ZfIwWBQHR0d6uzsrLtyyufzaWhoSJlMRvF4XJJadraCWcIdj8eVyWTU39//1ONdaXNzU7lcToFA4FTvC0vY4pkvC2dnZ90X0fLyct2RSL2Rntk82nyetLcM09Rqu7u7PX0MZvbEUYLWjGC3t7fl8/nU2dmpUCjkjtwrH+/Ozo4KhYI2Nze1srKilZWVQ/dv3R+4jd6rtZHMicQEbm9v71OvJHZ2drS6uqr19XWFw2GlUin20SBszw4zk8C2bXV3dyuZTLpzTfePRmqNAn0+X9WepmZaUCQScVccef2CMpvmXLt2rW7QFotFLS0tqVgsyu/3a2BgwJ22VouZedDW1qa+vj53wvv6+rq761Wt6WMmcO/cuaNYLKbHjx83ZDerkwrcSCSieDyulZUVra6uuiev/W1xKrvVDg8Pu6UINB412xNiRl7pdNq9I/zFF18c+f/vH0l+8cUXCofDikQimp+f93y5azKZVDqdVldXV925s5ubm+52eX19fbpx48ahQVtLKBRybxB+//vf18LCglZWVuqWFK5du+auzGv1K6O1tTUlEgn98pe/VD6f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";
			imagebytes = Convert::FromBase64String(base64_r_joy);
		}

		System::IO::MemoryStream^ ms = gcnew System::IO::MemoryStream(10000000);
		ms->Write(imagebytes, 0, imagebytes->Length);
		Bitmap^ MyImage = dynamic_cast<Bitmap^>(Image::FromStream(ms));

		float color_coeff = 0.0;
		for (int x = 0; x < MyImage->Width; x++)
		{
			for (int y = 0; y < MyImage->Height; y++)
			{
				Color gotColor = MyImage->GetPixel(x, y);
				color_coeff = gotColor.R / 255.0f;

				if (gotColor.R <= 255 && gotColor.R > 100){
					gotColor = Color::FromArgb(gotColor.A, CLAMP(r*color_coeff, 0.0f, 255.0f), CLAMP(g*color_coeff, 0.0f, 255.0f), CLAMP(b*color_coeff, 0.0f, 255.0f));
				}
				else if (gotColor.R <= 100 && gotColor.R != 78 && gotColor.R > 30 && handle_ok !=3) {
					if (gotColor.R == 100)
						gotColor = Color::FromArgb(gotColor.A, rb, gb, bb);
					else {
						gotColor = Color::FromArgb(gotColor.A, CLAMP(rb*color_coeff, 0.0f, 255.0f), CLAMP(gb*color_coeff, 0.0f, 255.0f), CLAMP(bb*color_coeff, 0.0f, 255.0f));
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
		this->pictureBoxPreview->ClientSize = System::Drawing::Size(312, 192);
		this->pictureBoxPreview->Image = dynamic_cast<Image^>(MyImage);
	}

	private: System::Void update_colors_from_spi(bool update_color_dialog) {
		unsigned char body_color[0x3];
		memset(body_color, 0, 0x3);
		unsigned char button_color[0x3];
		memset(button_color, 0, 0x3);

		get_spi_data(handle, 0x6050, 0x3, body_color);
		if (handle_ok != 3)
			get_spi_data(handle, 0x6053, 0x3, button_color);

		update_joycon_color((u8)body_color[0], (u8)body_color[1], (u8)body_color[2], (u8)button_color[0], (u8)button_color[1], (u8)button_color[2]);

		if (update_color_dialog) {
			this->colorDialog1->Color = Color::FromArgb(0xFF, (u8)body_color[0], (u8)body_color[1], (u8)body_color[2]);
			this->btnClrDlg1->Text = L"Body Color\n#" + String::Format("{0:X6}", ((u8)body_color[0] << 16) + ((u8)body_color[1] << 8) + ((u8)body_color[2]));

			this->colorDialog2->Color = Color::FromArgb(0xFF, (u8)button_color[0], (u8)button_color[1], (u8)button_color[2]);
			this->btnClrDlg2->Text = L"Buttons Color\n#" + String::Format("{0:X6}", ((u8)button_color[0] << 16) + ((u8)button_color[1] << 8) + ((u8)button_color[2]));
		}

	}

	private: System::Void update_battery() {
			cli::array<unsigned char>^ imagebytes;

			unsigned char batt_info[1];
			memset(batt_info, 0, sizeof(batt_info));
			get_battery(handle, batt_info);

			int batt = ((u8)batt_info[0] & 0xF0) >> 4;

			//Debug icons
			//batt = 5;

			if (batt == 0) {
				String^ batt_0 = "iVBORw0KGgoAAAANSUhEUgAAADAAAAASCAYAAAAdZl26AAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAIGNIUk0AAHolAACAgwAA+f8AAIDpAAB1MAAA6mAAADqYAAAXb5JfxUYAAABlSURBVHja7JbBCgAhCEQ15v9/WME9LcTurSiZcq5B8Xgpo2YWkhcFMHVBE/L0+Lrx3VgBIF+d7k5l4H84+D93gh81AxmJ1xYAvdJAARRAbaHcVBcCW3U42kAwAtAbeAAAAP//AwDqjRMvk64R9QAAAABJRU5ErkJggg==";
				imagebytes = Convert::FromBase64String(batt_0);
				this->toolTip1->SetToolTip(this->pictureBoxBattery, "EMpty\n\nLol, how?");
			}
			else if (batt == 1) {
				String^ batt_0_chr = "iVBORw0KGgoAAAANSUhEUgAAADAAAAASCAYAAAAdZl26AAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAIGNIUk0AAHolAACAgwAA+f8AAIDpAAB1MAAA6mAAADqYAAAXb5JfxUYAAAJhSURBVHjazFaxahtBEH27s7cn2ZY4JGR8hTEYFbYEakwSAgoEUgRDviAQQ2pXqdKkzA+oTW/Sp0lpELiz3Rg1QRy5wpAqSFyMdN6bvTR35iwbYxIS33S7zO7Om/dmZ4QxJsXDmVBKXdvQWhMAEJG47cBoNHq/vr7+otls7hpjWKJkRkTCcRwJAMycSimvgHS73dbm5ubHIAi+zufzBACK8MV/jPNO1o0xFkBKRNIYY4lI1Go19+jo6BMA7O3tfSYiIaWUoiChG3QmSfKvAdwqIcdxpDGGiehKIdPp9IvWehfAL8/zWnEcJ8ycqrteWbz8vvY3wIlIWGvTYvDdbndVa/0cAKy151priuM4AYDS1QAzX5OX67rq+Pj4PF+fnJwMoiiKmTklInGnhP7U7sFAegvbAgCq1arKgRCRiKJoqJTaAaABYGtrqxWG4SQ/VzoGilIaDAYvlVJPASQALgCg1+utLi0tOa7rKsdxZGkZaLfbjbOzszDzrRZcJwAoSz6VDoDWWhKRnM1mUfbN50HNFoCg0+mslrGRSQBYW1treZ5Xq9fr7v7+/qMs6wBwCWASBMGH8Xj8s5QSygs48xO+79eCIPiRFzIzn66srDxZ7MSlmIWyDiyY2RKRZOZ0OBy+A8A5A77vP8vBlfEXEkUpVSoV8n3/Vab/i36/346iKM79rknoAWahGwwsSAiu69JkMpkDmFtrvy0vL+8U/UvZB4qj9MbGhgfg0lr7vdFoPL5RO/edEB9qrDg4OHgNQHc6nX4cx1yIVZSWASmlyGW0vb395vDw8G0YhtN8v5j03wMAWdMCIzqJz1wAAAAASUVORK5CYII=";
				imagebytes = Convert::FromBase64String(batt_0_chr);
				this->toolTip1->SetToolTip(this->pictureBoxBattery, "Empty, Charging.");
			}
			else if (batt == 2) {
				String^ batt_25 = "iVBORw0KGgoAAAANSUhEUgAAADAAAAASCAYAAAAdZl26AAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAIGNIUk0AAHolAACAgwAA+f8AAIDpAAB1MAAA6mAAADqYAAAXb5JfxUYAAABuSURBVHjaYvz9+/d/hoEDjCwsLBQZwMQwxAGy9xnpaC/VYn1YxQBEIFQAETrrv5AVK3/+/KGbB4ZfDNAZ/IfFFgsLC+OIjIFRD4x6YLQUGlgw2hZiRGqNDkRbaDQGWGjRQhwthUgAAAAAAP//AwBZsRUz1MC1oAAAAABJRU5ErkJggg==";
				imagebytes = Convert::FromBase64String(batt_25);
				this->toolTip1->SetToolTip(this->pictureBoxBattery, "Low\n\nPlease charge your device!");
			}
			else if (batt == 3) {
				String^ batt_25_chr = "iVBORw0KGgoAAAANSUhEUgAAADAAAAASCAYAAAAdZl26AAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAIGNIUk0AAHolAACAgwAA+f8AAIDpAAB1MAAA6mAAADqYAAAXb5JfxUYAAAJgSURBVHjazFYxaBRBFH0zf3Y2m3BhuZCQK0IgXJNbSCMKwgmChQRsbYQLWKeysrG0sbzWPlhYCDbXGQikS6qYRuLhFemC3HIJ3mbmz9rsxokXRBRyO90Ob5b//n//vy+MMTmmd4RS6tqF1poAgIjETQ+Oj49frqysPFpYWNg0xrBExQ4RiSAIJAAwcy6lvCKSJMni2tra636/3xuPxxYAfPriFuP8Y9WNMQ5ATkTSGOOISNRqtXB/f/8tAGxtbb0jIiGllMKT0DQI3CihIAikMYaJ6EohaZp+1FpvAjiP43gxyzLLzLn6/c/qafwrOx/O/4mUtfa/JOScy/3gkyRZ0lo/BADn3KnWmrIsswBQuR5g5mvyCsNQHRwcnJbfh4eH3dFolDFzTkRi2gRya21urc39CvhEzs7OPgG4BBABQKfTee/jKlcBn0i3232slLoPwAK4AICNjY2l2dnZIAxDFQTBZBPfUg9MTCGllACAKIoUADSbzfrR0dGgwEYedAiAiuSTqmAPOCKSRfDKG/U/CiJxiW21WktVNDIJAMvLy4txHNfm5+fD7e3tu0XWUfTDsN/vvzo5Ofmuqqh/Zs7TNB0X8hG9Xu+rbxXM/DlJkjdEJKZNYMLICgcWpZSYOd/b23sBgMsKNBqNByW5Kk4h4UtpZmaGGo3Gk0L/F+12uzkajbISV7lVIooiVZpUYWQ0HA7HAMbOuS9zc3N3fHwlfcBfpVdXV2MAl865b/V6/d7E+P3bDXFazbyzs/MMgG61Wu0sy9iLtbpOLKUUpYzW19c7u7u7zweDQVre+0n/OQBokvs34NxywAAAAABJRU5ErkJggg==";
				imagebytes = Convert::FromBase64String(batt_25_chr);
				this->toolTip1->SetToolTip(this->pictureBoxBattery, "Low\n\nCharging");
			}
			else if (batt == 4) {
				String^ batt_50 = "iVBORw0KGgoAAAANSUhEUgAAADAAAAASCAYAAAAdZl26AAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAIGNIUk0AAHolAACAgwAA+f8AAIDpAAB1MAAA6mAAADqYAAAXb5JfxUYAAABxSURBVHja7JbBCYBADAQ3sgXYodXZob8cnC/hHj5O1ATPnQISNgNhzd0r8jCStwZM+DhtfAvc+5j1oQwAABbO3ddZsZ1aK6WEBRjPQDD1sEXSfmlAARRAXygXdSFr2mhGF5IBvtEQ9YUusAMAAP//AwAn6RUz5VtjaQAAAABJRU5ErkJggg==";
				imagebytes = Convert::FromBase64String(batt_50);
				this->toolTip1->SetToolTip(this->pictureBoxBattery, "Medium");
			}
			else if (batt == 5) {
				String^ batt_50_chr = "iVBORw0KGgoAAAANSUhEUgAAADAAAAASCAYAAAAdZl26AAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAIGNIUk0AAHolAACAgwAA+f8AAIDpAAB1MAAA6mAAADqYAAAXb5JfxUYAAAJgSURBVHjazFYxSxxBFP5m3uysq5wsJ4pXiCDXeAs2IYHABQIpgpBfEDhDaqtUaVKmSXlt+iNFujTXRRDstDI2wRy5wk7CLafk1pk3m2ZXxpwEIeDtdDu83X3fe9/3vieMMTlmd4RS6saF1poAgIjEbS+cnJy8XVtbe7a0tLRtjGGJih0iEkEQSABg5lxKeQ0kSZLljY2N94PBoD+ZTCwA+PDFPeb5z64bYxyAnIikMcYRkajVauHBwcFHANjZ2flEREJKKYVHoVkAuJVCQRBIYwwT0TVD0jT9orXeBnARx/FylmWWmXP195dfqfjOmujh4lbQ1tr/opBzLveTT5JkRWv9FACcc2daa8qyzAJA5TTAzDcKGIahOjw8PCufj46OuuPxOGPmnIjErAHk1trcWpv7HfCBnJ+ffwVwBSACgE6n89mPq1wHfCDdbve5UuoxAAvgEgC2trZW5ufngzAMVRAE0yK+Jw1M/UMpJQAgiiIFAM1ms358fDwsYiMvdASAiuKTqqAGHBHJInnljfrfBZC4jG21WitVNDIJAKurq8txHNcWFxfD3d3dh0XVUehhNBgM3p2env5SVeQ/M+dpmk4K+oh+v//Dtwpm/pYkyQciErMGMGVkhQOLkkrMnO/v778BwGUHGo3GkxJcFaeQ8Kk0NzdHjUbjRcH/y3a73RyPx1kZV7lVIooiVZpUYWQ0Go0mACbOue8LCwsP/PhK+oC/Sq+vr8cArpxzP+v1+qOp8XvXDXFWYu71ei8B6Far1c6yjL1cq+vEUkpR0mhzc7Ozt7f3ejgcpuW9X/Q/AwAFAvs3nefMJAAAAABJRU5ErkJggg==";
				imagebytes = Convert::FromBase64String(batt_50_chr);
				this->toolTip1->SetToolTip(this->pictureBoxBattery, "Medium\n\nCharging");
			}
			else if (batt == 6) {
				String^ batt_75 = "iVBORw0KGgoAAAANSUhEUgAAADAAAAASCAYAAAAdZl26AAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAIGNIUk0AAHolAACAgwAA+f8AAIDpAAB1MAAA6mAAADqYAAAXb5JfxUYAAABtSURBVHja7JbBDYAwDAMd5AHYkNnYkH8ilQefflEhEcU3QFPnpMjm7g11GMmhBxZ8nD6+Jc59zPpUBgAAG9fh7exxpNmcz0AyLSKuj5D2SwMKoAC6QrWoC1nXRiu6kAzwjYaoK3SDEwAA//8DAJA+EzPzRHjGAAAAAElFTkSuQmCC";
				imagebytes = Convert::FromBase64String(batt_75);
				this->toolTip1->SetToolTip(this->pictureBoxBattery, "Good");
			}
			else if (batt == 7) {
				String^ batt_75_chr = "iVBORw0KGgoAAAANSUhEUgAAADAAAAASCAYAAAAdZl26AAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAIGNIUk0AAHolAACAgwAA+f8AAIDpAAB1MAAA6mAAADqYAAAXb5JfxUYAAAJeSURBVHjazFYxa9tAFP7u3ukUJTgIh4R4CAbjJRZkKS0UXCh0KIH+goJTOnvq1KVjl45eu3UIHbp18dZAIJs9pVlKauohWygWbqiV0zt1kcKlDqUQiHWbjifpfe993/ueMMZkWNwRSqlrF1prAgAiEje9cHJy8npra+vJ2trarjGGJUp2iEh4nicBgJkzKeUVkCiK1huNxtvRaNSfzWYpALjwxR3m+c+uG2MsgIyIpDHGEpGoVCr+0dHRewDY29v7SERCSimFQ6FFALiRQp7nSWMME9EVQ+I4/qy13gXwKwzD9SRJUmbO1N9ffqHCW2viQzoRt6GQtTZzk4+iaENr/RgArLVnWmtKkiQFgNJpgJmvFdD3fTUYDM6K5+Fw2JtOpwkzZ0QkFg0gS9M0S9M0czvgAjk/P/8C4BJAAACdTueTG1e6DrhAer3eU6XUQwApgAsA2NnZ2VheXvZ831ee582L+I40MPcPpZQAgCAIFAA0m83q8fHxOI8NnNAJAMqLT6qEGrBEJPPklTPqf+dAwiK21WptlNHIJABsbm6uh2FYWV1d9bvd7v286sj1MBmNRm9OT09/qjLyn5mzOI5nOX1Ev9//7loFM3+NougdEYlFA5gzstyBRUElZs4ODw9fAeCiA7Va7VEBroxTSLhUWlpaolqt9izn/0W73W5Op9OkiCvdKhEEgSpMKjcymkwmMwAza+23lZWVe258KX3AXaXr9XoI4NJa+6NarT6YG7//uyEuSsz7+/vPAehWq9VOkoSdXMvrxFJKUdBoe3u7c3Bw8HI8HsfFvVv0PwMA3gb3N+fZ5NkAAAAASUVORK5CYII=";
				imagebytes = Convert::FromBase64String(batt_75_chr);
				this->toolTip1->SetToolTip(this->pictureBoxBattery, "Good\n\nCharging");
			}
			else if (batt == 8) {
				String^ batt_100 = "iVBORw0KGgoAAAANSUhEUgAAADAAAAASCAYAAAAdZl26AAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAIGNIUk0AAHolAACAgwAA+f8AAIDpAAB1MAAA6mAAADqYAAAXb5JfxUYAAABuSURBVHja7JbLDYAwDEMd5AHYkNnYkHsilQOXXlE/EcVvgKbWUyKbuxfkYSSbHtjwcer4NnFuN+tLGQAAHNyH7cQZV3fL6xmYTImI5yOk/dKAAiiArlAu6kJWtdGMLiQDHNEQdYVecAMAAP//AwBS7hMzme/zjAAAAABJRU5ErkJggg==";
				imagebytes = Convert::FromBase64String(batt_100);
				this->toolTip1->SetToolTip(this->pictureBoxBattery, "Full");
			}
			else if (batt == 9) {
				String^ batt_100_chr = "iVBORw0KGgoAAAANSUhEUgAAADAAAAASCAYAAAAdZl26AAAACXBIWXMAAA7EAAAOxAGVKw4bAAAAIGNIUk0AAHolAACAgwAA+f8AAIDpAAB1MAAA6mAAADqYAAAXb5JfxUYAAAJeSURBVHjazFYxa9tAFP7u3ukUOTgIh4R4CIbgJRZkKS0UXCh0KIH+goJTOnvq1KVjl45eu3UIHbp18dZAIJs9pVlKauohWygWbqiV0zt1kcKlDiVLY92m40l633vf974njDEZFneEUurahdaaAICIxE0vnJycvN7c3Hyyurq6a4xhiZIdIhKe50kAYOZMSnkFJIqita2trbej0ag/m81SAHDhizvM859dN8ZYABkRSWOMJSJRrVb9o6Oj9wCwt7f3kYiElFIKh0KLAHAjhTzPk8YYJqIrhsRx/FlrvQvgVxiGa0mSpMycqb+//EKF/00TH9KJuA2FrLWZm3wUReta68cAYK0901pTkiQpAJROA8x8rYC+76vBYHBWPA+Hw950Ok2YOSMisWgAWZqmWZqmmdsBF8j5+fkXAJcAAgDodDqf3LjSdcAF0uv1niqlHgJIAVwAwM7OznqlUvF831ee582L+I40MPcPpZQAgCAIFAA0m83a8fHxOI8NnNAJAMqLT6qEGrBEJPPklTPqf+dAwiK21Wqtl9HIJABsbGyshWFYXVlZ8bvd7v286sj1MBmNRm9OT09/qjLyn5mzOI5nOX1Ev9//7loFM3+NougdEYlFA5gzstyBRUElZs4ODw9fAeCiA/V6/VEBroxTSLhUWlpaonq9/izn/0W73W5Op9OkiCvdKhEEgSpMKjcymkwmMwAza+235eXle258KX3AXaUbjUYI4NJa+6NWqz2YG7+33RAXJeb9/f3nAHSr1WonScJOruV1YimlKGi0vb3dOTg4eDkej+Pi3i36nwEAY2b3N3yqQBcAAAAASUVORK5CYII=";
				imagebytes = Convert::FromBase64String(batt_100_chr);
				this->toolTip1->SetToolTip(this->pictureBoxBattery, "Almost full\n\nCharging");
			}

			System::IO::MemoryStream^ ms = gcnew System::IO::MemoryStream(10000);
			ms->Write(imagebytes, 0, imagebytes->LongLength);

			this->pictureBoxBattery->Image = Image::FromStream(ms);

		}

	private: array<int>^ getCustomColorFromConfig(String^ custom_type)
	{
		System::Xml::XmlDocument^ doc = gcnew System::Xml::XmlDocument();
		doc->Load(Path::GetDirectoryName(Application::ExecutablePath) + "\\Colors.config");
		//get all named "custom_type" element
		System::Xml::XmlNodeList^ nodes = doc->GetElementsByTagName(custom_type + "_v");
		List<int>^ colors = gcnew List<int>();
		for (int i = 0; i < nodes->Count; i++)
		{
			//get key attribute
			XmlAttribute^ att = nodes[i]->Attributes["key"];
			if (att->Value == "CustomColor")
			{
				//get value attribute
				att = nodes[i]->Attributes["value"];
				colors->Add(Convert::ToInt32(att->Value, 16));
			}
		}
		return colors->ToArray();
	}

	private: System::Void saveCustomColorToConfig(ColorDialog^ dialog_type, String^ custom_type)
	{
		System::Xml::XmlDocument^ doc = gcnew System::Xml::XmlDocument();
		doc->Load(Path::GetDirectoryName(Application::ExecutablePath) + "\\Colors.config");
		System::Xml::XmlNode^ node = doc->SelectSingleNode("//" + custom_type);
		node->RemoveAll();
		//add custom colors
		array<int>^ colors = dialog_type->CustomColors;
		for (int i = 0; i < colors->Length; i++)
		{
			//if(colors[i]==)
			XmlElement^ xe = doc->CreateElement(custom_type + "_v");
			xe->SetAttribute("key", "CustomColor");
			xe->SetAttribute("value", String::Format("{0:X}", colors[i]));
			node->AppendChild(xe);

		}
		doc->Save(Path::GetDirectoryName(Application::ExecutablePath) + "\\Colors.config");

	}

	private: System::Void Form1_FormClosing(System::Object^  sender, FormClosingEventArgs^ e)
	{
		//check if spi dumping in progress
		if (handler_close == 1) {
			if (MessageBox::Show(L"SPI dumping in process!\n\nDo you really want to exit?", L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Stop) == System::Windows::Forms::DialogResult::Yes)
			{
				Environment::Exit(0);
			}
			else
			{
				e->Cancel = true;
			}
		}
		else if (handler_close == 2) {
			if (MessageBox::Show(L"Full restore in process!\n\nDo you really want to exit?", L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Stop) == System::Windows::Forms::DialogResult::Yes)
			{
				Environment::Exit(0);
			}
			else
			{
				e->Cancel = true;
			}
		}
		else if (handler_close == 0) {
			Environment::Exit(0);
		}
	}

	private: System::Void aboutToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
		if (MessageBox::Show(L"CTCaer's " + this->Text + L"\n\nDo you want to go to the official forum and check for the latest version?\n\n", L"About", MessageBoxButtons::YesNo, MessageBoxIcon::Asterisk) == System::Windows::Forms::DialogResult::Yes)
		{
			System::Diagnostics::Process::Start("http://gbatemp.net/threads/tool-joy-con-toolkit-v1-0.478560/");
		}
	}

	private: System::Void debugToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		if (debug_is_on == 0 || debug_is_on == 2 || debug_is_on == 3) {
			this->ClientSize = System::Drawing::Size(738, 449);
			this->groupRst->Visible = false;
			this->groupBox_chg_sn->Visible = false;
			this->groupDbg->Visible = true;
			debug_is_on = 1;
		}
		else {
			this->ClientSize = System::Drawing::Size(485, 449);
			this->groupRst->Visible = false;
			this->groupBox_chg_sn->Visible = false;
			this->textBoxDbg_sent->Visible = false;
			this->textBoxDbg_reply->Visible = false;
			this->groupDbg->Visible = false;
			debug_is_on = 0;
		}
	}

	private: System::Void btnDbg_send_cmd_Click(System::Object^  sender, System::EventArgs^  e) {
		if (!device_connection()) {
			MessageBox::Show(L"The device was disconnected!\n\nPress a button on the controller to connect\nand try to send the custom command again!", L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
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

		send_custom_command(handle, test);
		this->textBoxDbg_sent->Visible = true;
		this->textBoxDbg_reply->Visible = true;
	}

	private: System::Void btbRestoreEnable_Click(System::Object^  sender, System::EventArgs^  e) {
		if (debug_is_on == 0 || debug_is_on == 1 || debug_is_on == 3) {
			this->groupDbg->Visible = false;
			this->groupRst->Visible = true;
			this->groupBox_chg_sn->Visible = false;
			this->textBoxDbg_sent->Visible = false;
			this->textBoxDbg_reply->Visible = false;
			this->ClientSize = System::Drawing::Size(738, 449);
			debug_is_on = 2;
		}
		else {
			this->ClientSize = System::Drawing::Size(485, 449);
			this->groupDbg->Visible = false;
			this->groupRst->Visible = false;
			this->groupBox_chg_sn->Visible = false;
			debug_is_on = 0;
		}
	}

	private: System::Void btnLoadBackup_Click(System::Object^  sender, System::EventArgs^  e) {
		bool validation_check = true;
		bool mac_check = true;
		allow_full_restore = true;
		bool ota_exists = true;
		//Bootloader, device type, FW DS1, FW DS2
		array<byte>^ validation_magic = { 0x01, 0x08, 0x00, 0xF0, 0x00, 0x00, 0x62, 0x08, 0xC0, 0x5D, 0x89, 0xFD, 0x04, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x06,
			(u8)handle_ok, 0xA0,
			0x0A, 0xFB, 0x00, 0x00, 0x02, 0x0D,
			0xAA, 0x55, 0xF0, 0x0F, 0x68, 0xE5, 0x97, 0xD2 };
		Stream^ fileStream;
		String^ str_dev_type;
		String^ str_backup_dev_type;

		OpenFileDialog^ openFileDialog1 = gcnew OpenFileDialog;
		openFileDialog1->InitialDirectory = System::IO::Path::Combine(System::IO::Path::GetDirectoryName(Application::ExecutablePath), "YourSubDirectoryName");
		openFileDialog1->Filter = "SPI Backup (*.bin)|*.bin";
		openFileDialog1->FilterIndex = 1;
		openFileDialog1->RestoreDirectory = true;
		if (openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK && (fileStream = openFileDialog1->OpenFile()) != nullptr)
		{
			System::IO::MemoryStream^ ms = gcnew System::IO::MemoryStream(1048576);;
			fileStream->CopyTo(ms);
			this->backup_spi = ms->ToArray();

			if (this->backup_spi->Length != 524288) {
				MessageBox::Show(L"The file size must be 512KB (524288 Bytes)", L"Partial backup!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
				this->textBox2->Text = L"No file loaded";
				this->comboBox1->Visible = false;
				this->label7->Visible = false;
				this->btn_restore->Visible = false;
				this->grpRstUser->Visible = false;
				fileStream->Close();
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
					this->textBox2->Text = L"No file loaded";
					this->label7->Visible = false;
					this->comboBox1->Visible = false;
					this->btn_restore->Visible = false;
					this->grpRstUser->Visible = false;
					fileStream->Close();
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
				MessageBox::Show(L"The SPI backup is corrupted!\n\nPlease try another backup.", L"Corrupt backup!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
				this->textBox2->Text = L"No file loaded";
				this->label7->Visible = false;
				this->comboBox1->Visible = false;
				this->btn_restore->Visible = false;
				this->grpRstUser->Visible = false;
				fileStream->Close();
				return;
			}

			unsigned char mac_addr[10];
			memset(mac_addr, 0, sizeof(mac_addr));
			get_device_info(handle, mac_addr);

			for (int i = 4; i < 10; i++) {
				if (mac_addr[i] != this->backup_spi[0x1A - i + 4]) {
					mac_check = false;
				}
			}
			if (!mac_check) {
				if (MessageBox::Show(L"The SPI backup is from another \"" + str_dev_type + "\".\n\nDo you want to continue?", L"Different BT MAC address!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
				{
					this->textBox2->Text = String::Format("{0:x2}:{1:x2}:{2:x2}:{3:x2}:{4:x2}:{5:x2}", this->backup_spi[0x1A], this->backup_spi[0x19], this->backup_spi[0x18], this->backup_spi[0x17], this->backup_spi[0x16], this->backup_spi[0x15]);
					this->comboBox1->SelectedItem = "Restore Color";
					this->comboBox1->Visible = true;
					this->btn_restore->Visible = true;
					this->btn_restore->Enabled = true;
					allow_full_restore = false;
				}
				else {
					this->textBox2->Text = L"No file loaded";
					this->label7->Visible = false;
					this->comboBox1->Visible = false;
					this->btn_restore->Visible = false;
					this->grpRstUser->Visible = false;
				}
			}
			else {
				this->textBox2->Text = String::Format("{0:x2}:{1:x2}:{2:x2}:{3:x2}:{4:x2}:{5:x2}", this->backup_spi[0x1A], this->backup_spi[0x19], this->backup_spi[0x18], this->backup_spi[0x17], this->backup_spi[0x16], this->backup_spi[0x15]);
				this->comboBox1->SelectedItem = "Restore Color";
				this->comboBox1->Visible = true;
				this->btn_restore->Visible = true;
				this->btn_restore->Enabled = true;
			}
			fileStream->Close();
		}
	}

	protected: System::Void comboBox1_DrawItem(System::Object^ sender, DrawItemEventArgs^ e)
	{
		Brush^ brush = gcnew System::Drawing::SolidBrush(Color::FromArgb(9, 255, 206));;

		if (e->Index < 0)
			return;

		ComboBox^ combo = (ComboBox^)sender;
		if ((e->State & DrawItemState::Selected) == DrawItemState::Selected)
			e->Graphics->FillRectangle(gcnew SolidBrush(Color::FromArgb(105, 105, 105)), e->Bounds);
		else
			e->Graphics->FillRectangle(gcnew SolidBrush(combo->BackColor), e->Bounds);

		e->Graphics->DrawString(this->comboBox1->Items[e->Index]->ToString(), e->Font, brush, e->Bounds, StringFormat::GenericDefault);

		e->DrawFocusRectangle();
	}

	private: System::Void comboBox1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
		this->grpRstUser->Visible = false;
		this->label7->Visible = false;
		this->label7->Size = System::Drawing::Size(188, 121);
		this->btn_restore->Text = L"Restore";
		this->label5->Text = L"This will restore the user calibration from the selected backup.\n\nIf you have any problem with the analog stick or the 6-Axis sensor you can choose to factory reset user calibration.";
		this->label7->Text = L"This allows for the device colors to be restored from the loaded backup.\n\nFor mor"
			L"e options click on the list above.";

		if (this->comboBox1->SelectedIndex == 0) {
			this->label7->Visible = true;
		}
		if (this->comboBox1->SelectedIndex == 1) {
			this->label7->Visible = true;
			this->label7->Size = System::Drawing::Size(188, 230);
			this->label7->Text = L"This will restore your S/N from the selected backup.\n\nMake sure that this backup was your original one!";
		}
		else if (this->comboBox1->SelectedIndex == 2) {
			this->grpRstUser->Visible = true;
			if (handle_ok == 1) {
				this->checkBox2->Visible = false;
				this->checkBox1->Visible = true;
				this->checkBox1->Location = System::Drawing::Point(6, 212);
			}
			else if (handle_ok == 2) {
				this->checkBox2->Visible = true;
				this->checkBox1->Visible = false;
			}
			else if (handle_ok == 3) {
				this->checkBox2->Visible = true;
				this->checkBox1->Visible = true;
				this->checkBox1->Location = System::Drawing::Point(6, 186);
			}

		}
		else if (this->comboBox1->SelectedIndex == 3) {
			this->label5->Text = L"This option does the same factory reset with the option inside Switch's controller calibration menu.";
			this->btn_restore->Text = L"Reset";
			this->grpRstUser->Visible = true;
			if (handle_ok == 1) {
				this->checkBox2->Visible = false;
				this->checkBox1->Visible = true;
				this->checkBox1->Location = System::Drawing::Point(6, 212);
			}
			else if (handle_ok == 2) {
				this->checkBox2->Visible = true;
				this->checkBox1->Visible = false;
			}
			else if (handle_ok == 3) {
				this->checkBox2->Visible = true;
				this->checkBox1->Visible = true;
				this->checkBox1->Location = System::Drawing::Point(6, 186);
			}
		}
		else if (this->comboBox1->SelectedIndex == 4) {
			this->label7->Visible = true;
			this->label7->Size = System::Drawing::Size(188, 230);
			this->label7->Text = L"This basically restores the Factory Configuration\nand the User Calibration. It restores the only 2 4KB sectors that are usable in the writable 40KB region.\n\nTo preserve the factory configuration from accidental overwrite the full restore is disabled if the backup does not match your device.";
			if (!allow_full_restore)
				this->btn_restore->Enabled = false;
		}
	}

	private: System::Void btn_restore_Click(System::Object^  sender, System::EventArgs^  e) {
		if (!device_connection()) {
			MessageBox::Show(L"The device was disconnected!\n\nPress a button on the controller to connect\nand try to restore again!", L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
			return;
		}
		set_led_busy();
		if (this->comboBox1->SelectedIndex == 0) {
			if (MessageBox::Show(L"The device color will be restored with the backup values!\n\nAre you sure you want to continue?", L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
			{
				unsigned char body_color[0x3];
				memset(body_color, 0, 0x3);
				unsigned char button_color[0x3];
				memset(button_color, 0, 0x3);

				for (int i = 0; i < 3; i++) {
					body_color[i] = this->backup_spi[0x6050 + i];
					button_color[i] = this->backup_spi[0x6053 + i];
				}

				write_spi_data(handle, 0x6050, 0x3, body_color);
				write_spi_data(handle, 0x6053, 0x3, button_color);

				//Check that the colors were written
				update_colors_from_spi(true);
				update_battery();
				send_rumble(handle);

				MessageBox::Show(L"The colors were restored!", L"Restore Finished!", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);

			}

		}
		else if (this->comboBox1->SelectedIndex == 1) {
			if (MessageBox::Show(L"The serial number will be restored with the backup values!\n\nAre you sure you want to continue?", L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
			{
				unsigned char sn[0x10];
				memset(sn, 0, 0x10);


				for (int i = 0; i < 0x10; i++) {
					sn[i] = this->backup_spi[0x6000 + i];
				}

				write_spi_data(handle, 0x6000, 0x10, sn);

				String^ new_sn;
				if (handle_ok != 3) {
					new_sn = gcnew String(get_sn(handle, 0x6001, 0xF).c_str());
					MessageBox::Show(L"The serial number was restored and changed to \"" + new_sn + L"\"!", L"Restore Finished!", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
				}
				else {
					MessageBox::Show(L"The serial number was restored!", L"Restore Finished!", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
				}
				update_battery();
				send_rumble(handle);
			}

		}
		else if (this->comboBox1->SelectedIndex == 2) {
			if (MessageBox::Show(L"The selected user calibration will be restored from the backup!\n\nAre you sure you want to continue?", L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
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

				if (handle_ok != 2 && this->checkBox1->Checked == true)
					write_spi_data(handle, 0x8010, 0xB, l_stick);
				if (handle_ok != 1 && this->checkBox2->Checked == true)
					write_spi_data(handle, 0x801B, 0xB, r_stick);
				if (this->checkBox3->Checked == true)
					write_spi_data(handle, 0x8026, 0x1A, sensor);

				update_battery();
				send_rumble(handle);
				MessageBox::Show(L"The user calibration was restored!", L"Calibration Restore Finished!", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
			}

		}
		else if (this->comboBox1->SelectedIndex == 3) {
			if (MessageBox::Show(L"The selected user calibration will be factory resetted!\n\nAre you sure you want to continue?", L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
			{
				unsigned char l_stick[0xB];
				memset(l_stick, 0xFF, 0xB);
				unsigned char r_stick[0xB];
				memset(r_stick, 0xFF, 0xB);
				unsigned char sensor[0x1A];
				memset(sensor, 0xFF, 0x1A);

				if (handle_ok != 2 && this->checkBox1->Checked == true)
					write_spi_data(handle, 0x8010, 0xB, l_stick);
				if (handle_ok != 1 && this->checkBox2->Checked == true)
					write_spi_data(handle, 0x801B, 0xB, r_stick);
				if (this->checkBox3->Checked == true)
					write_spi_data(handle, 0x8026, 0x1A, sensor);

				update_battery();
				send_rumble(handle);
				MessageBox::Show(L"The user calibration was factory resetted!", L"Calibration Reset Finished!", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
			}

		}
		else if (this->comboBox1->SelectedIndex == 4) {
			if (MessageBox::Show(L"This will do a full restore of the Factory configuration and User calibration!\n\nAre you sure you want to continue?", L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
			{
				handler_close = 2;
				this->btnLoadBackup->Enabled = false;
				this->btn_restore->Enabled = false;
				this->groupBoxColor->Visible = false;
				this->label6->Text = L"Restoring Factory Configuration and User Calibration...\n\nDon\'t disconnect your device!";
				unsigned char full_restore_data[0x10];

				//Factory Configuration Sector 0x6000
				for (int i = 0x00; i < 0x1000; i = i + 0x10) {
					memset(full_restore_data, 0, 0x10);
					for (int j = 0; j < 0x10; j++)
						full_restore_data[j] = this->backup_spi[0x6000 + i + j];
					write_spi_data(handle, 0x6000 + i, 0x10, full_restore_data);

					std::stringstream offset_label;
					offset_label << std::fixed << std::setprecision(2) << std::setfill(' ') << i / 1024.0f;
					offset_label << "KB of 8KB";
					FormJoy::myform1->label_progress->Text = gcnew String(offset_label.str().c_str());
					Application::DoEvents();
				}
				//User Calibration Sector 0x8000
				for (int i = 0x00; i < 0x1000; i = i + 0x10) {
					memset(full_restore_data, 0, 0x10);
					for (int j = 0; j < 0x10; j++)
						full_restore_data[j] = this->backup_spi[0x8000 + i + j];
					write_spi_data(handle, 0x8000 + i, 0x10, full_restore_data);

					std::stringstream offset_label;
					offset_label << std::fixed << std::setprecision(2) << std::setfill(' ') << 4 + (i / 1024.0f);
					offset_label << "KB of 8KB";
					FormJoy::myform1->label_progress->Text = gcnew String(offset_label.str().c_str());
					Application::DoEvents();
				}
				std::stringstream offset_label;
				offset_label << std::fixed << std::setprecision(2) << std::setfill(' ') << 0x2000 / 1024.0f;
				offset_label << "KB of 8KB";
				FormJoy::myform1->label_progress->Text = gcnew String(offset_label.str().c_str());
				Application::DoEvents();

				unsigned char custom_cmd[3];
				memset(custom_cmd, 0, 3);
				custom_cmd[0] = 0x01;
				//Set shipment. This will force a full pair with Switch
				custom_cmd[1] = 0x08;
				custom_cmd[2] = 0x01;
				send_custom_command(handle, custom_cmd);
				update_battery();
				send_rumble(handle);

				MessageBox::Show(L"The full restore was completed!\n\nIt is recommended to turn off the controller and exit Joy-Con Toolkit.\nAfter that press any button on the controller and run the app again.", L"Full Restore Finished!", MessageBoxButtons::OK, MessageBoxIcon::Warning);
				this->groupBoxColor->Visible = true;
				this->btnLoadBackup->Enabled = true;
				this->btn_restore->Enabled = true;

				update_colors_from_spi(true);

				handler_close = 0;
			}

		}
	}

	private: System::Void textBoxDbg_subcmd_Validating(System::Object^ sender, CancelEventArgs^ e)
	{
		bool cancel = false;
		int number = -1;
		if (Int32::TryParse(this->textBoxDbg_subcmd->Text, NumberStyles::HexNumber, CultureInfo::InvariantCulture, number))
		{
			if (number == 0x10 || number == 0x11 || number == 0x12) {
				cancel = true;
				this->errorProvider2->SetError(this->textBoxDbg_subcmd, "The subcommands:\n0x10: SPI Read\n0x11: SPI Write\n0x12: SPI Sector Erase\nare disabled!");
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
			else
			{
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
				this->errorProvider1->SetError(test, "Unicode characters are not supported!\n\nUse non-extended ASCII characters only!");
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
			MessageBox::Show(L"The device was disconnected!\n\nPress a button on the controller to connect\nand try to change the S/N again!", L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Stop);
			return;
		}
		if (handle_ok != 3) {
			if (MessageBox::Show(L"This will change your Serial Number!\n\nMake a backup first!\n\nAre you sure you want to continue?", L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
			{
				if (MessageBox::Show(L"Did you make a backup?", L"Warning!", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == System::Windows::Forms::DialogResult::Yes)
				{
					array<Char>^ mn_str_sn = this->textBox_chg_sn->Text->ToCharArray();
					unsigned char sn[32];

					int length = 16 - mn_str_sn->Length;

					for (int i = 0; i < length; i++) {
						sn[i] = 0x00;
					}
					for (int i = 0; i < mn_str_sn->Length; i++) {
						sn[length + i] = (unsigned char)(mn_str_sn[i] & 0xFF);
					}

					write_spi_data(handle, 0x6000, 0x10, sn);
					update_battery();
					send_rumble(handle);
					String^ new_sn = gcnew String(get_sn(handle, 0x6001, 0xF).c_str());
					MessageBox::Show(L"The S/N was written to the device!\n\nThe new S/N is now \"" + new_sn + L"\"!\n\nIf you still ignored the warnings about creating a backup, the S/N in the left of the main window will not change. Copy it somewhere safe!");
				}
			}
		}
		else
			MessageBox::Show(L"Changing S/N is not supported for Pro Controllers!", L"Error!", MessageBoxButtons::OK, MessageBoxIcon::Warning);
	}


	private: System::Void label_sn_Click(System::Object^  sender, System::EventArgs^  e) {
		if (debug_is_on == 0 || debug_is_on == 1 || debug_is_on == 2) {
			this->ClientSize = System::Drawing::Size(738, 449);
			this->textBoxDbg_sent->Visible = false;
			this->textBoxDbg_reply->Visible = false;
			this->groupRst->Visible = false;
			this->groupBox_chg_sn->Visible = true;
			this->groupDbg->Visible = false;
			debug_is_on = 3;
		}
		else {
			this->ClientSize = System::Drawing::Size(485, 449);
			this->groupRst->Visible = false;
			this->groupBox_chg_sn->Visible = false;
			this->groupDbg->Visible = false;
			debug_is_on = 0;
		}
	}

	private: System::Void pictureBoxBattery_Click(System::Object^  sender, System::EventArgs^  e) {
		if (!device_connection()) {
			MessageBox::Show(L"The device was disconnected!\n\nPress a button on the controller to connect\nand try again!", L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
			return;
		}
		if (MessageBox::Show(L"HOORAY!!\n\nYou found the easter egg!\n\nMake sure you have a good signal and get the device near your ear.\n\nThen press OK to hear the tune!\n\nIf the tune is slow or choppy:\n1. Close the app\n2. Press the sync button once to turn off the device\n3. Get close to your BT adapter and maintain LoS\n4. Press any other button and run the app again.", L"Easter egg!", MessageBoxButtons::OKCancel, MessageBoxIcon::Information) == System::Windows::Forms::DialogResult::OK)
		{
			set_led_busy();
			play_tune(handle);
			update_battery();
			send_rumble(handle);
			MessageBox::Show(L"The HD Rumble music has ended.", L"Easter egg!");
		}
	}

};
}


