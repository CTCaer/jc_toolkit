// Copyright (c) 2018 CTCaer. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <cstdio>
#include <functional>
#include <memory>
#include <string>
// #define NOMINMAX
#include <Windows.h>

#include "jctool.h"
#include "FormJoy.h"
#include "tune.h"

using namespace CppWinFormJoy;

#pragma comment(lib, "SetupAPI")

#pragma pack(push, 1)

struct brcm_hdr {
	u8 cmd;
	u8 timer;
	u8 rumble_l[4];
	u8 rumble_r[4];
};

struct brcm_cmd_01 {
	u8 subcmd;
	union {
		struct {
			u32 offset;
			u8 size;
		} spi_read;
		
		struct {
			u8 arg1;
			u8 arg2;
		} subcmd_arg;
	};
};

// Used to order the packets received in Joy-Con internally. Range 0x0-0xF.
u8 timming_byte = 0x0;

#pragma pack(pop)

s16 uint16_to_int16(u16 a) {
	s16 b;
	char* aPointer = (char*)&a, *bPointer = (char*)&b;
	memcpy(bPointer, aPointer, sizeof(a));
	return b;
}

// Credit to Hypersect (Ryan Juckett)
// http://blog.hypersect.com/interpreting-analog-sticks/
void AnalogStickCalc
(
	float *pOutX,       // out: resulting stick X value
	float *pOutY,       // out: resulting stick Y value
	u16 x,              // in: initial stick X value
	u16 y,              // in: initial stick Y value
	u16 x_calc[3],      // calc -X, CenterX, +X
	u16 y_calc[3]       // calc -Y, CenterY, +Y
)
{
	float x_f, y_f;
	// Apply Joy-Con center deadzone. 0xAE translates approx to 15%. Pro controller has a 10% deadzone.
	float deadZoneCenter = 0.15f;
	// Add a small ammount of outer deadzone to avoid edge cases or machine variety.
	float deadZoneOuter = 0.10f;

	// convert to float based on calibration and valid ranges per +/-axis
	x = CLAMP(x, x_calc[0], x_calc[2]);
	y = CLAMP(y, y_calc[0], y_calc[2]);
	if (x >= x_calc[1])
		x_f = (float)(x - x_calc[1]) / (float)(x_calc[2] - x_calc[1]);
	else
		x_f = -((float)(x - x_calc[1]) / (float)(x_calc[0] - x_calc[1]));
	if (y >= y_calc[1])
		y_f = (float)(y - y_calc[1]) / (float)(y_calc[2] - y_calc[1]);
	else
		y_f = -((float)(y - y_calc[1]) / (float)(y_calc[0] - y_calc[1]));

	// Interpolate zone between deadzones
	float mag = sqrtf(x_f*x_f + y_f*y_f);
	if (mag > deadZoneCenter) {
		// scale such that output magnitude is in the range [0.0f, 1.0f]
		float legalRange = 1.0f - deadZoneOuter - deadZoneCenter;
		float normalizedMag = min(1.0f, (mag - deadZoneCenter) / legalRange);
		float scale = normalizedMag / mag;
		pOutX[1] = x_f * scale;
		pOutY[1] = y_f * scale;
	}
	else
	{
		// stick is in the inner dead zone
		pOutX[1] = 0.0f;
		pOutY[1] = 0.0f;
	}
}

int set_led_busy() {
	int res;
	u8 buf[0x100];
	memset(buf, 0, sizeof(buf));
	auto hdr = (brcm_hdr *)buf;
	auto pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	pkt->subcmd = 0x30;
	pkt->subcmd_arg.arg1 = 0x81;
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf, 0);

	//Set breathing HOME Led
	if (handle_ok != 1) {
		memset(buf, 0, sizeof(buf));
		hdr = (brcm_hdr *)buf;
		pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 0x01;
		hdr->timer = timming_byte & 0xF;
		timming_byte++;
		pkt->subcmd = 0x38;
		pkt->subcmd_arg.arg1 = 0x28;
		pkt->subcmd_arg.arg2 = 0x20;
		buf[13] = 0xF2;
		buf[14] = buf[15] = 0xF0;
		res = hid_write(handle, buf, 16);
		res = hid_read(handle, buf, 0);
	}

	return 0;
}

std::string get_sn(u32 offset, const u16 read_len) {
	int res;
	u8 buf[0x100];
	std::string test = "";
	while (1) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brcm_hdr *)buf;
		auto pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 1;
		hdr->timer = timming_byte & 0xF;
		timming_byte++;
		pkt->subcmd = 0x10;
		pkt->spi_read.offset = offset;
		pkt->spi_read.size = read_len;
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));

		res = hid_read(handle, buf, sizeof(buf));
		if ((*(u16*)&buf[0xD] == 0x1090) && (*(uint32_t*)&buf[0xF] == offset))
			break;
	}

	if (res >= 0x14 + read_len) {
		for (int i = 0; i < read_len; i++) {
			if (buf[0x14 + i] != 0x000) {
				test += buf[0x14 + i];
			}else
				test += "";
			}
	}

	return test;
}

int get_spi_data(u32 offset, const u16 read_len, u8 *test_buf) {
	int res;
	u8 buf[0x100];
	while (1) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brcm_hdr *)buf;
		auto pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 1;
		hdr->timer = timming_byte & 0xF;
		timming_byte++;
		pkt->subcmd = 0x10;
		pkt->spi_read.offset = offset;
		pkt->spi_read.size = read_len;
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));

		res = hid_read(handle, buf, sizeof(buf));
		if ((*(u16*)&buf[0xD] == 0x1090) && (*(uint32_t*)&buf[0xF] == offset))
			break;
	}
	if (res >= 0x14 + read_len) {
			for (int i = 0; i < read_len; i++) {
				test_buf[i] = buf[0x14 + i];
			}
	}
	
	return 0;
}

int write_spi_data(u32 offset, const u16 write_len, u8* test_buf) {
	int res;
	u8 buf[0x100];
	int error_writing = 0;
	while (1) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brcm_hdr *)buf;
		auto pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 1;
		hdr->timer = timming_byte & 0xF;
		timming_byte++;
		pkt->subcmd = 0x11;
		pkt->spi_read.offset = offset;
		pkt->spi_read.size = write_len;
		for (int i = 0; i < write_len; i++)
			buf[0x10 + i] = test_buf[i];

		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt) + write_len);

		res = hid_read(handle, buf, sizeof(buf));
		if (*(u16*)&buf[0xD] == 0x1180)
			break;
		error_writing++;
		if (error_writing == 125)
			return 1;
	}

	return 0;
}

int get_device_info(u8* test_buf) {
	int res;
	u8 buf[0x100];
	int error_reading = 0;
	while (1) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brcm_hdr *)buf;
		auto pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 1;
		hdr->timer = timming_byte & 0xF;
		timming_byte++;
		pkt->subcmd = 0x02;
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));

		res = hid_read(handle, buf, sizeof(buf));		
		if (*(u16*)&buf[0xD] == 0x0282)
			break;
		error_reading++;
		if (error_reading == 125)
			break;
	}
	for (int i = 0; i < 0xA; i++) {
		test_buf[i] = buf[0xF + i];
	}

	return 0;
}

int get_battery(u8* test_buf) {
	int res;
	u8 buf[0x100];
	int error_reading = 0;
	while (1) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brcm_hdr *)buf;
		auto pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 1;
		hdr->timer = timming_byte & 0xF;
		timming_byte++;
		pkt->subcmd = 0x50;
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));

		res = hid_read(handle, buf, sizeof(buf));
		if (*(u16*)&buf[0xD] == 0x50D0)
			break;
		error_reading++;
		if (error_reading == 125)
			break;
	}
	test_buf[0] = buf[0x2];
	test_buf[1] = buf[0xF];
	test_buf[2] = buf[0x10];

	return 0;
}

int get_temperature(u8* test_buf) {
	int res;
	u8 buf[0x100];
	int error_reading = 0;
	bool imu_changed = false;

	while (1) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brcm_hdr *)buf;
		auto pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 1;
		hdr->timer = timming_byte & 0xF;
		timming_byte++;
		pkt->subcmd = 0x43;
		pkt->subcmd_arg.arg1 = 0x10;
		pkt->subcmd_arg.arg2 = 0x01;
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));

		res = hid_read(handle, buf, sizeof(buf));
		if (*(u16*)&buf[0xD] == 0x43C0)
			break;
		error_reading++;
		if (error_reading == 125)
			break;
	}

	if ((buf[0x11] >> 4) == 0x0) {

		memset(buf, 0, sizeof(buf));
		auto hdr = (brcm_hdr *)buf;
		auto pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 0x01;
		hdr->timer = timming_byte & 0xF;
		timming_byte++;
		pkt->subcmd = 0x40;
		pkt->subcmd_arg.arg1 = 0x01;
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
		res = hid_read(handle, buf, 0);

		imu_changed = true;

		// Let temperature sensor stabilize for a little bit.
		Sleep(64);
	}

	while (1) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brcm_hdr *)buf;
		auto pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 1;
		hdr->timer = timming_byte & 0xF;
		timming_byte++;
		pkt->subcmd = 0x43;
		pkt->subcmd_arg.arg1 = 0x20;
		pkt->subcmd_arg.arg2 = 0x02;
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));

		res = hid_read(handle, buf, sizeof(buf));
		if (*(u16*)&buf[0xD] == 0x43C0)
			break;
		error_reading++;
		if (error_reading == 125)
			break;
	}
	test_buf[0] = buf[0x11];
	test_buf[1] = buf[0x12];

	if (imu_changed) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brcm_hdr *)buf;
		auto pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 0x01;
		hdr->timer = timming_byte & 0xF;
		timming_byte++;
		pkt->subcmd = 0x40;
		pkt->subcmd_arg.arg1 = 0x00;
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
		res = hid_read(handle, buf, 0);
	}

	return 0;
}

int dump_spi(const char *dev_name) {
	std::string file_dev_name = dev_name;
	int i=0;

	String^ filename_sys = gcnew String(file_dev_name.c_str());
	file_dev_name = "./" + file_dev_name;

	FILE *f;
	errno_t err;

	if ((err = fopen_s(&f, file_dev_name.c_str(), "wb")) != 0) {
		MessageBox::Show(L"Cannot open file " + filename_sys + L" for writing!\n\nError: " + err, L"Error opening file!", MessageBoxButtons::OK ,MessageBoxIcon::Exclamation);
		
		return 1;
	}

	int res;
	u8 buf[0x100];
	
	u16 read_len = 0x1d;
	u32 offset = 0x0;
	while (offset < 0x80000 && !cancel_spi_dump) {
		std::stringstream offset_label;
		offset_label << std::fixed << std::setprecision(2) << std::setfill(' ') << offset/1024.0f;
		offset_label << "KB of 512KB";
		FormJoy::myform1->label_progress->Text = gcnew String(offset_label.str().c_str());
		Application::DoEvents();

		while(1) {
			memset(buf, 0, sizeof(buf));
			auto hdr = (brcm_hdr *)buf;
			auto pkt = (brcm_cmd_01 *)(hdr + 1);
			hdr->cmd = 1;
			hdr->timer = timming_byte & 0xF;
			timming_byte++;
			pkt->subcmd = 0x10;
			pkt->spi_read.offset = offset;
			pkt->spi_read.size = read_len;
			res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
			res = hid_read(handle, buf, sizeof(buf));

			if ((*(u16*)&buf[0xD] == 0x1090) && (*(uint32_t*)&buf[0xF] == offset))
				break;
		}
		fwrite(buf + 0x14, read_len, 1, f);
		offset += read_len;
		if (offset == 0x7FFE6)
			read_len = 0x1A;
	}
	fclose(f);

	return 0;
}

int send_rumble() {
	int res;
	u8 buf[0x100];
	u8 buf2[0x100];
	
	//Enable Vibration
	memset(buf, 0, sizeof(buf));
	auto hdr = (brcm_hdr *)buf;
	auto pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	pkt->subcmd = 0x48;
	pkt->subcmd_arg.arg1 = 0x01;
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf2, 0);

	//New vibration like switch
	Sleep(16);
	//Send confirmation 
	memset(buf, 0, sizeof(buf));
	hdr->cmd = 0x01;
	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	hdr->rumble_l[0] = 0xc2;
	hdr->rumble_l[1] = 0xc8;
	hdr->rumble_l[2] = 0x03;
	hdr->rumble_l[3] = 0x72;
	memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf2, 0);

	Sleep(81);

	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	hdr->rumble_l[0] = 0x00;
	hdr->rumble_l[1] = 0x01;
	hdr->rumble_l[2] = 0x40;
	hdr->rumble_l[3] = 0x40;
	memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf2, 0);

	Sleep(5);

	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	hdr->rumble_l[0] = 0xc3;
	hdr->rumble_l[1] = 0xc8;
	hdr->rumble_l[2] = 0x60;
	hdr->rumble_l[3] = 0x64;
	memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf2, 0);

	Sleep(5);

	//Disable vibration
	memset(buf, 0, sizeof(buf));
	hdr = (brcm_hdr *)buf;
	pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	hdr->rumble_l[0] = 0x00;
	hdr->rumble_l[1] = 0x01;
	hdr->rumble_l[2] = 0x40;
	hdr->rumble_l[3] = 0x40;
	memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
	pkt->subcmd = 0x48;
	pkt->subcmd_arg.arg1 = 0x00;
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf, 0);

	memset(buf, 0, sizeof(buf));
	hdr = (brcm_hdr *)buf;
	pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	pkt->subcmd = 0x30;
	pkt->subcmd_arg.arg1 = 0x01;
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf, 0);

	// Set HOME Led
	if (handle_ok != 1) {
		memset(buf, 0, sizeof(buf));
		hdr = (brcm_hdr *)buf;
		pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 0x01;
		hdr->timer = timming_byte & 0xF;
		timming_byte++;
		pkt->subcmd = 0x38;
		// Heartbeat style configuration
		buf[11] = 0xF1;
		buf[12] = 0x00;
		buf[13] = buf[14] = buf[15] = buf[16] = buf[17] = buf[18] = 0xF0;
		buf[19] = buf[22] = buf[25] = buf[28] = buf[31] = 0x00;
		buf[20] = buf[21] = buf[23] = buf[24] = buf[26] = buf[27] = buf[29] = buf[30] = buf[32] = buf[33] = 0xFF;
		res = hid_write(handle, buf, 34);
		res = hid_read(handle, buf, 0);
	}

	return 0;
}

int send_custom_command(u8* arg) {
	int res_write;
	int res;
	int byte_seperator = 1;
	String^ input_report_cmd;
	String^ input_report_sys;
	String^ output_report_sys;
	u8 buf_cmd[36];
	u8 buf_reply[0x170];
	memset(buf_cmd, 0, sizeof(buf_cmd));
	memset(buf_reply, 0, sizeof(buf_reply));

	buf_cmd[0] = arg[0];
	buf_cmd[1] = timming_byte & 0xF;
	timming_byte++;
	buf_cmd[2] = buf_cmd[6] = arg[1];
	buf_cmd[3] = buf_cmd[7] = arg[2];
	buf_cmd[4] = buf_cmd[8] = arg[3];
	buf_cmd[5] = buf_cmd[9] = arg[4];

	buf_cmd[10] = arg[5];

	output_report_sys = String::Format(L"Cmd:  {0:X2}   Subcmd: {1:X2}\r\n", buf_cmd[0], buf_cmd[10]);
	if (buf_cmd[0] == 0x01 || buf_cmd[0] == 0x10 || buf_cmd[0] == 0x11) {
		for (int i = 6; i < 31; i++) {
			buf_cmd[5 + i] = arg[i];
			output_report_sys += String::Format(L"{0:X2} ", buf_cmd[5 + i]);
			if (byte_seperator == 4)
				output_report_sys += L" ";
			if (byte_seperator == 8) {
				byte_seperator = 0;
				output_report_sys += L"\r\n";
			}
			byte_seperator++;
		}
	}
	//Use subcmd after command
	else {
		for (int i = 6; i < 31; i++) {
			buf_cmd[i - 5] = arg[i];
			output_report_sys += String::Format(L"{0:X2} ", buf_cmd[i - 5]);
			if (byte_seperator == 4)
				output_report_sys += L" ";
			if (byte_seperator == 8) {
				byte_seperator = 0;
				output_report_sys += L"\r\n";
			}
			byte_seperator++;
		}
	}
	FormJoy::myform1->textBoxDbg_sent->Text = output_report_sys;

	//Packet size header + subcommand and uint8 argument
	res_write = hid_write(handle, buf_cmd, sizeof(buf_cmd));

	if (res_write < 0)
		input_report_sys += L"hid_write failed!\r\n\r\n";

	res = hid_read_timeout(handle, buf_reply, sizeof(buf_reply), 200);

	byte_seperator = 1;
	if (res > 12) {
		if (buf_reply[0] == 0x21 || buf_reply[0] == 0x30 || buf_reply[0] == 0x33 || buf_reply[0] == 0x31 || buf_reply[0] == 0x3F) {
			input_report_cmd += String::Format(L"\r\nInput report: 0x{0:X2}\r\n", buf_reply[0]);
			input_report_sys += String::Format(L"Subcmd Reply:\r\n", buf_reply[0]);
			int len = 49;
			if (buf_reply[0] == 0x33 || buf_reply[0] == 0x31)
				len = 362;
			for (int i = 1; i < 13; i++) {
				input_report_cmd += String::Format(L"{0:X2} ", buf_reply[i]);
				if (byte_seperator == 4)
					input_report_cmd += L" ";
				if (byte_seperator == 8) {
					byte_seperator = 0;
					input_report_cmd += L"\r\n";
				}
				byte_seperator++;
			}
			byte_seperator = 1;
			for (int i = 13; i < len; i++) {
				input_report_sys += String::Format(L"{0:X2} ", buf_reply[i]);
				if (byte_seperator == 4)
					input_report_sys += L" ";
				if (byte_seperator == 8) {
					byte_seperator = 0;
					input_report_sys += L"\r\n";
				}
				byte_seperator++;
			}
		}
		else {
			input_report_sys += String::Format(L"ID: {0:X2} Subcmd reply:\r\n", buf_reply[0]);
			for (int i = 13; i < res; i++) {
				input_report_sys += String::Format(L"{0:X2} ", buf_reply[i]);
				if (byte_seperator == 4)
					input_report_sys += L" ";
				if (byte_seperator == 8) {
					byte_seperator = 0;
					input_report_sys += L"\r\n";
				}
				byte_seperator++;
			}
		}
	}
	else if (res > 0 && res <= 12) {
		for (int i = 0; i < res; i++)
			input_report_sys += String::Format(L"{0:X2} ", buf_reply[i]);
	}
	else {
		input_report_sys += L"No reply";
	}
	FormJoy::myform1->textBoxDbg_reply->Text = input_report_sys;
	FormJoy::myform1->textBoxDbg_reply_cmd->Text = input_report_cmd;

	return 0;
}

int button_test() {
	int res;
	int limit_output = 0;
	String^ input_report_cmd;
	String^ input_report_sys;
	u8 buf_cmd[36];
	u8 buf_reply[0x170];
	float acc_cal_coeff[3];
	float gyro_cal_coeff[3];
	float cal_x[1] = { 0.0f };
	float cal_y[1] = { 0.0f };

	bool has_user_cal_stick_l = false;
	bool has_user_cal_stick_r = false;
	bool has_user_cal_sensor = false;

	unsigned char factory_stick_cal[0x12];
	unsigned char user_stick_cal[0x16];
	unsigned char sensor_model[0x6];
	unsigned char stick_model[0x24];
	unsigned char factory_sensor_cal[0x18];
	unsigned char user_sensor_cal[0x1A];
	u16 factory_sensor_cal_calm[0xC];
	u16 user_sensor_cal_calm[0xC];
	s16 sensor_cal[0x2][0x3];
	u16 stick_cal_x_l[0x3];
	u16 stick_cal_y_l[0x3];
	u16 stick_cal_x_r[0x3];
	u16 stick_cal_y_r[0x3];
	memset(factory_stick_cal, 0, 0x12);
	memset(user_stick_cal, 0, 0x16);
	memset(sensor_model, 0, 0x6);
	memset(stick_model, 0, 0x12);
	memset(factory_sensor_cal, 0, 0x18);
	memset(user_sensor_cal, 0, 0x1A);
	memset(factory_sensor_cal_calm, 0, 0xC);
	memset(user_sensor_cal_calm, 0, 0xC);
	memset(sensor_cal, 0, sizeof(sensor_cal));
	memset(stick_cal_x_l, 0, sizeof(stick_cal_x_l));
	memset(stick_cal_y_l, 0, sizeof(stick_cal_y_l));
	memset(stick_cal_x_r, 0, sizeof(stick_cal_x_r));
	memset(stick_cal_y_r, 0, sizeof(stick_cal_y_r));
	get_spi_data(0x6020, 0x18, factory_sensor_cal);
	get_spi_data(0x603D, 0x12, factory_stick_cal);
	get_spi_data(0x6080, 0x6, sensor_model);
	get_spi_data(0x6086, 0x12, stick_model);
	get_spi_data(0x6098, 0x12, &stick_model[0x12]);
	get_spi_data(0x8010, 0x16, user_stick_cal);
	get_spi_data(0x8026, 0x1A, user_sensor_cal);

	// Analog Stick device parameters
	FormJoy::myform1->textBox_device_parameters->Text = String::Format(L"Flat surface ACC Offset:\r\n{0:X4} {1:X4} {2:X4}\r\n\r\n\r\nStick Parameters:\r\n{3:X3} {4:X3}\r\n{5:X2} (Deadzone)\r\n{6:X3} (Range ratio)",
		sensor_model[0] | sensor_model[1] << 8,
		sensor_model[2] | sensor_model[3] << 8,
		sensor_model[4] | sensor_model[5] << 8,
		(stick_model[1] << 8) & 0xF00 | stick_model[0], (stick_model[2] << 4) | (stick_model[1] >> 4),
		(stick_model[4] << 8) & 0xF00 | stick_model[3],
		((stick_model[5] << 4) | (stick_model[4] >> 4)));

	for (int i = 0; i < 10; i = i + 3) {
		FormJoy::myform1->textBox_device_parameters->Text += String::Format(L"\r\n{0:X3} {1:X3}",
			(stick_model[7 + i] << 8) & 0xF00 | stick_model[6 + i],
			(stick_model[8 + i] << 4) | (stick_model[7 + i] >> 4));
	}

	FormJoy::myform1->textBox_device_parameters2->Text = String::Format(L"Stick Parameters 2:\r\n{0:X3} {1:X3}\r\n{2:X2} (Deadzone)\r\n{3:X3} (Range ratio)",
		(stick_model[19] << 8) & 0xF00 | stick_model[18], (stick_model[20] << 4) | (stick_model[19] >> 4),
		(stick_model[22] << 8) & 0xF00 | stick_model[21],
		((stick_model[23] << 4) | (stick_model[22] >> 4)));

	for (int i = 0; i < 10; i = i + 3) {
		FormJoy::myform1->textBox_device_parameters2->Text += String::Format(L"\r\n{0:X3} {1:X3}",
			(stick_model[25 + i] << 8) & 0xF00 | stick_model[24 + i],
			(stick_model[26 + i] << 4) | (stick_model[25 + i] >> 4));
	}

	// Stick calibration
	if (handle_ok != 2) {
		stick_cal_x_l[1] = (factory_stick_cal[4] << 8) & 0xF00 | factory_stick_cal[3];
		stick_cal_y_l[1] = (factory_stick_cal[5] << 4) | (factory_stick_cal[4] >> 4);
		stick_cal_x_l[0] = stick_cal_x_l[1] - ((factory_stick_cal[7] << 8) & 0xF00 | factory_stick_cal[6]);
		stick_cal_y_l[0] = stick_cal_y_l[1] - ((factory_stick_cal[8] << 4) | (factory_stick_cal[7] >> 4));
		stick_cal_x_l[2] = stick_cal_x_l[1] + ((factory_stick_cal[1] << 8) & 0xF00 | factory_stick_cal[0]);
		stick_cal_y_l[2] = stick_cal_y_l[1] + ((factory_stick_cal[2] << 4) | (factory_stick_cal[2] >> 4));
		FormJoy::myform1->textBox_lstick_fcal->Text = String::Format(L"L Stick Factory:\r\nCenter X,Y: ({0:X3}, {1:X3})\r\nX: [{2:X3} - {4:X3}] Y: [{3:X3} - {5:X3}]",
			stick_cal_x_l[1], stick_cal_y_l[1], stick_cal_x_l[0], stick_cal_y_l[0], stick_cal_x_l[2], stick_cal_y_l[2]);
	}
	else {
		FormJoy::myform1->textBox_lstick_fcal->Text = L"L Stick Factory:\r\nNo calibration";
	}
	if (handle_ok != 1) {
		stick_cal_x_r[1] = (factory_stick_cal[10] << 8) & 0xF00 | factory_stick_cal[9];
		stick_cal_y_r[1] = (factory_stick_cal[11] << 4) | (factory_stick_cal[10] >> 4);
		stick_cal_x_r[0] = stick_cal_x_r[1] - ((factory_stick_cal[13] << 8) & 0xF00 | factory_stick_cal[12]);
		stick_cal_y_r[0] = stick_cal_y_r[1] - ((factory_stick_cal[14] << 4) | (factory_stick_cal[13] >> 4));
		stick_cal_x_r[2] = stick_cal_x_r[1] + ((factory_stick_cal[16] << 8) & 0xF00 | factory_stick_cal[15]);
		stick_cal_y_r[2] = stick_cal_y_r[1] + ((factory_stick_cal[17] << 4) | (factory_stick_cal[16] >> 4));
		FormJoy::myform1->textBox_rstick_fcal->Text = String::Format(L"R Stick Factory:\r\nCenter X,Y: ({0:X3}, {1:X3})\r\nX: [{2:X3} - {4:X3}] Y: [{3:X3} - {5:X3}]",
			stick_cal_x_r[1], stick_cal_y_r[1],	stick_cal_x_r[0], stick_cal_y_r[0],	stick_cal_x_r[2], stick_cal_y_r[2]);
	}
	else {
		FormJoy::myform1->textBox_rstick_fcal->Text = L"R Stick Factory:\r\nNo calibration";
	}

	if ((user_stick_cal[0] | user_stick_cal[1] << 8) == 0xA1B2) {
		stick_cal_x_l[1] = (user_stick_cal[6] << 8) & 0xF00 | user_stick_cal[5];
		stick_cal_y_l[1] = (user_stick_cal[7] << 4) | (user_stick_cal[6] >> 4);
		stick_cal_x_l[0] = stick_cal_x_l[1] - ((user_stick_cal[9] << 8) & 0xF00 | user_stick_cal[8]);
		stick_cal_y_l[0] = stick_cal_y_l[1] - ((user_stick_cal[10] << 4) | (user_stick_cal[9] >> 4));
		stick_cal_x_l[2] = stick_cal_x_l[1] + ((user_stick_cal[3] << 8) & 0xF00 | user_stick_cal[2]);
		stick_cal_y_l[2] = stick_cal_y_l[1] + ((user_stick_cal[4] << 4) | (user_stick_cal[3] >> 4));
		FormJoy::myform1->textBox_lstick_ucal->Text = String::Format(L"L Stick User:\r\nCenter X,Y: ({0:X3}, {1:X3})\r\nX: [{2:X3} - {4:X3}] Y: [{3:X3} - {5:X3}]",
			stick_cal_x_l[1], stick_cal_y_l[1], stick_cal_x_l[0], stick_cal_y_l[0], stick_cal_x_l[2], stick_cal_y_l[2]);
	}
	else {
		FormJoy::myform1->textBox_lstick_ucal->Text = L"L Stick User:\r\nNo calibration";
	}
	if ((user_stick_cal[0xB] | user_stick_cal[0xC] << 8) == 0xA1B2) {
		stick_cal_x_r[1] = (user_stick_cal[14] << 8) & 0xF00 | user_stick_cal[13];
		stick_cal_y_r[1] = (user_stick_cal[15] << 4) | (user_stick_cal[14] >> 4);
		stick_cal_x_r[0] = stick_cal_x_r[1] - ((user_stick_cal[17] << 8) & 0xF00 | user_stick_cal[16]);
		stick_cal_y_r[0] = stick_cal_y_r[1] - ((user_stick_cal[18] << 4) | (user_stick_cal[17] >> 4));
		stick_cal_x_r[2] = stick_cal_x_r[1] + ((user_stick_cal[20] << 8) & 0xF00 | user_stick_cal[19]);
		stick_cal_y_r[2] = stick_cal_y_r[1] + ((user_stick_cal[21] << 4) | (user_stick_cal[20] >> 4));
		FormJoy::myform1->textBox_rstick_ucal->Text = String::Format(L"R Stick User:\r\nCenter X,Y: ({0:X3}, {1:X3})\r\nX: [{2:X3} - {4:X3}] Y: [{3:X3} - {5:X3}]",
			stick_cal_x_r[1], stick_cal_y_r[1], stick_cal_x_r[0], stick_cal_y_r[0], stick_cal_x_r[2], stick_cal_y_r[2]);
	}
	else {
		FormJoy::myform1->textBox_rstick_ucal->Text = L"R Stick User:\r\nNo calibration";
	}

	// Sensor calibration
	FormJoy::myform1->textBox_6axis_cal->Text = L"6-Axis Factory (XYZ):\r\nAcc:  ";
	for (int i = 0; i < 0xC; i = i + 6) {
		FormJoy::myform1->textBox_6axis_cal->Text += String::Format(L"{0:X4} {1:X4} {2:X4}\r\n      ",
			factory_sensor_cal[i + 0] | factory_sensor_cal[i + 1] << 8,
			factory_sensor_cal[i + 2] | factory_sensor_cal[i + 3] << 8,
			factory_sensor_cal[i + 4] | factory_sensor_cal[i + 5] << 8);
	}
	// Acc cal origin position
	sensor_cal[0][0] = uint16_to_int16(factory_sensor_cal[0] | factory_sensor_cal[1] << 8);
	sensor_cal[0][1] = uint16_to_int16(factory_sensor_cal[2] | factory_sensor_cal[3] << 8);
	sensor_cal[0][2] = uint16_to_int16(factory_sensor_cal[4] | factory_sensor_cal[5] << 8);

	FormJoy::myform1->textBox_6axis_cal->Text += L"\r\nGyro: ";
	for (int i = 0xC; i < 0x18; i = i + 6) {
		FormJoy::myform1->textBox_6axis_cal->Text += String::Format(L"{0:X4} {1:X4} {2:X4}\r\n      ",
			factory_sensor_cal[i + 0] | factory_sensor_cal[i + 1] << 8,
			factory_sensor_cal[i + 2] | factory_sensor_cal[i + 3] << 8,
			factory_sensor_cal[i + 4] | factory_sensor_cal[i + 5] << 8);
	}
	// Gyro cal origin position
	sensor_cal[1][0] = uint16_to_int16(factory_sensor_cal[0xC] | factory_sensor_cal[0xD] << 8);
	sensor_cal[1][1] = uint16_to_int16(factory_sensor_cal[0xE] | factory_sensor_cal[0xF] << 8);
	sensor_cal[1][2] = uint16_to_int16(factory_sensor_cal[0x10] | factory_sensor_cal[0x11] << 8);

	if ((user_sensor_cal[0x0] | user_sensor_cal[0x1] << 8) == 0xA1B2) {
		FormJoy::myform1->textBox_6axis_ucal->Text = L"6-Axis User (XYZ):\r\nAcc:  ";
		for (int i = 0; i < 0xC; i = i + 6) {
			FormJoy::myform1->textBox_6axis_ucal->Text += String::Format(L"{0:X4} {1:X4} {2:X4}\r\n      ",
				user_sensor_cal[i + 2] | user_sensor_cal[i + 3] << 8,
				user_sensor_cal[i + 4] | user_sensor_cal[i + 5] << 8,
				user_sensor_cal[i + 6] | user_sensor_cal[i + 7] << 8);
		}
		// Acc cal origin position
		sensor_cal[0][0] = uint16_to_int16(user_sensor_cal[2] | user_sensor_cal[3] << 8);
		sensor_cal[0][1] = uint16_to_int16(user_sensor_cal[4] | user_sensor_cal[5] << 8);
		sensor_cal[0][2] = uint16_to_int16(user_sensor_cal[6] | user_sensor_cal[7] << 8);
		FormJoy::myform1->textBox_6axis_ucal->Text += L"\r\nGyro: ";
		for (int i = 0xC; i < 0x18; i = i + 6) {
			FormJoy::myform1->textBox_6axis_ucal->Text += String::Format(L"{0:X4} {1:X4} {2:X4}\r\n      ",
				user_sensor_cal[i + 2] | user_sensor_cal[i + 3] << 8,
				user_sensor_cal[i + 4] | user_sensor_cal[i + 5] << 8,
				user_sensor_cal[i + 6] | user_sensor_cal[i + 7] << 8);
		}
		// Gyro cal origin position
		sensor_cal[1][0] = uint16_to_int16(user_sensor_cal[0xE] | user_sensor_cal[0xF] << 8);
		sensor_cal[1][1] = uint16_to_int16(user_sensor_cal[0x10] | user_sensor_cal[0x11] << 8);
		sensor_cal[1][2] = uint16_to_int16(user_sensor_cal[0x12] | user_sensor_cal[0x13] << 8);
	}
	else {
		FormJoy::myform1->textBox_6axis_ucal->Text = L"\r\n\r\nUser:\r\nNo calibration";
	}


	// Enable nxpad standard input report
	memset(buf_cmd, 0, sizeof(buf_cmd));
	auto hdr = (brcm_hdr *)buf_cmd;
	auto pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	pkt->subcmd = 0x03;
	pkt->subcmd_arg.arg1 = 0x30;
	res = hid_write(handle, buf_cmd, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf_cmd, sizeof(*hdr) + sizeof(*pkt));

	// Enable IMU
	memset(buf_cmd, 0, sizeof(buf_cmd));
	hdr = (brcm_hdr *)buf_cmd;
	pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	pkt->subcmd = 0x40;
	pkt->subcmd_arg.arg1 = 0x01;
	res = hid_write(handle, buf_cmd, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf_cmd, sizeof(*hdr) + sizeof(*pkt));

	// Use SPI calibration and convert them to SI acc unit
	acc_cal_coeff[0] = (float)(1.0 / (float)(16384 - uint16_to_int16(sensor_cal[0][0]))) * 4.0f  * 9.8f;
	acc_cal_coeff[1] = (float)(1.0 / (float)(16384 - uint16_to_int16(sensor_cal[0][1]))) * 4.0f  * 9.8f;
	acc_cal_coeff[2] = (float)(1.0 / (float)(16384 - uint16_to_int16(sensor_cal[0][2]))) * 4.0f  * 9.8f;

	// Use SPI calibration and convert them to SI gyro unit
	gyro_cal_coeff[0] = (float)(936.0 / (float)(13371 - uint16_to_int16(sensor_cal[1][0])) * 0.01745329251994);
	gyro_cal_coeff[1] = (float)(936.0 / (float)(13371 - uint16_to_int16(sensor_cal[1][1])) * 0.01745329251994);
	gyro_cal_coeff[2] = (float)(936.0 / (float)(13371 - uint16_to_int16(sensor_cal[1][2])) * 0.01745329251994);

	// Input report loop
	while (enable_button_test) {
		memset(buf_cmd, 0, sizeof(buf_cmd));
		res = hid_read_timeout(handle, buf_reply, sizeof(buf_reply), 200);

		if (res > 12) {
			if (buf_reply[0] == 0x21 || buf_reply[0] == 0x30 || buf_reply[0] == 0x31 || buf_reply[0] == 0x32 || buf_reply[0] == 0x33) {
				if (((buf_reply[2] >> 1) & 0x3) == 3)
					input_report_cmd = String::Format(L"Conn: BT");
				else if (((buf_reply[2] >> 1) & 0x3) == 0)
					input_report_cmd = String::Format(L"Conn: USB");
				else
					input_report_cmd = String::Format(L"Conn: {0:X}?", (buf_reply[2] >> 1) & 0x3);
				input_report_cmd += String::Format(L"\r\nBatt: {0:X}/4   ", buf_reply[2] >> 5);
				if ((buf_reply[2] >> 4) & 0x1)
					input_report_cmd += L"Charging: Yes\r\n";
				else
					input_report_cmd += L"Charging: No\r\n";

				input_report_cmd += String::Format(L"Vibration decision: ");
				input_report_cmd += String::Format(L"{0:X}, {1:X}\r\n", (buf_reply[12] >> 7) & 1, (buf_reply[12] >> 4) & 7);
				input_report_cmd += String::Format(L"\r\nButtons: ");

				for (int i = 3; i < 6; i++)
					input_report_cmd += String::Format(L"{0:X2} ", buf_reply[i]);
			
				if (handle_ok != 2) {
					input_report_cmd += String::Format(L"\r\n\r\nL Stick (Raw/Cal):\r\nX:   {0:X3}   Y:   {1:X3}\r\n",
						buf_reply[6] | (u16)((buf_reply[7] & 0xF) << 8),
						(buf_reply[7] >> 4) | (buf_reply[8] << 4));

					AnalogStickCalc(
						cal_x, cal_y,
						buf_reply[6] | (u16)((buf_reply[7] & 0xF) << 8),
						(buf_reply[7] >> 4) | (buf_reply[8] << 4),
						stick_cal_x_l,
						stick_cal_y_l);

					input_report_cmd += String::Format(L"X: {0,5:f2}   Y: {1,5:f2}\r\n",
						cal_x[1], cal_y[1]);
				}
				if (handle_ok != 1) {
					input_report_cmd += String::Format(L"\r\n\r\nR Stick (Raw/Cal):\r\nX:   {0:X3}   Y:   {1:X3}\r\n",
						buf_reply[9] | (u16)((buf_reply[10] & 0xF) << 8),
						(buf_reply[10] >> 4) | (buf_reply[11] << 4));

					AnalogStickCalc(
						cal_x, cal_y,
						buf_reply[9] | (u16)((buf_reply[10] & 0xF) << 8),
						(buf_reply[10] >> 4) | (buf_reply[11] << 4),
						stick_cal_x_r,
						stick_cal_y_r);

					input_report_cmd += String::Format(L"X: {0,5:f2}   Y: {1,5:f2}\r\n",
						cal_x[1], cal_y[1]);

				}
				int len = 49;
				if (buf_reply[0] == 0x33 || buf_reply[0] == 0x31)
					len = 362;

				input_report_sys = String::Format(L"Acc/meter (Raw/Cal):\r\n");
				//The controller sends the sensor data 3 times with a little bit different values. Skip them
				input_report_sys += String::Format(L"X: {0:X4}  {1,6:F1} m/s\u00B2\r\n", buf_reply[13] | (buf_reply[14] << 8) & 0xFF00,
					(float)(uint16_to_int16(buf_reply[13] | (buf_reply[14] << 8) & 0xFF00)) * acc_cal_coeff[0]);
				input_report_sys += String::Format(L"Y: {0:X4}  {1,6:F1} m/s\u00B2\r\n", buf_reply[15] | (buf_reply[16] << 8) & 0xFF00,
					(float)(uint16_to_int16(buf_reply[15] | (buf_reply[16] << 8) & 0xFF00)) * acc_cal_coeff[1]);
				input_report_sys += String::Format(L"Z: {0:X4}  {1,6:F1} m/s\u00B2\r\n", buf_reply[17] | (buf_reply[18] << 8) & 0xFF00,
					(float)(uint16_to_int16(buf_reply[17] | (buf_reply[18] << 8) & 0xFF00))  * acc_cal_coeff[2]);

				input_report_sys += String::Format(L"\r\nGyroscope (Raw/Cal):\r\n");
				input_report_sys += String::Format(L"X: {0:X4}  {1,6:F1} rad/s\r\n", buf_reply[19] | (buf_reply[20] << 8) & 0xFF00,
					(float)(uint16_to_int16(buf_reply[19] | (buf_reply[20] << 8) & 0xFF00)) * gyro_cal_coeff[0]);
				input_report_sys += String::Format(L"Y: {0:X4}  {1,6:F1} rad/s\r\n", buf_reply[21] | (buf_reply[22] << 8) & 0xFF00,
					(float)(uint16_to_int16(buf_reply[21] | (buf_reply[22] << 8) & 0xFF00)) * gyro_cal_coeff[1]);
				input_report_sys += String::Format(L"Z: {0:X4}  {1,6:F1} rad/s\r\n", buf_reply[23] | (buf_reply[24] << 8) & 0xFF00,
					(float)(uint16_to_int16(buf_reply[23] | (buf_reply[24] << 8) & 0xFF00)) * gyro_cal_coeff[2]);
			}
			else if (buf_reply[0] == 0x3F) {
				input_report_cmd = L"";
				for (int i = 0; i < 17; i++)
					input_report_cmd += String::Format(L"{0:X2} ", buf_reply[i]);
			}

			if (limit_output == 1) {
				FormJoy::myform1->textBox_btn_test_reply->Text = input_report_cmd;
				FormJoy::myform1->textBox_btn_test_subreply->Text = input_report_sys;
			}
			//Only update every 75ms for better readability. No need for real time parsing.
			else if (limit_output > 4) {
				limit_output = 0;
			}
			limit_output++;
		}

		Application::DoEvents();
	}
	memset(buf_cmd, 0, sizeof(buf_cmd));
	hdr = (brcm_hdr *)buf_cmd;
	pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	pkt->subcmd = 0x03;
	pkt->subcmd_arg.arg1 = 0x3F;
	res = hid_write(handle, buf_cmd, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf_cmd, sizeof(*hdr) + sizeof(*pkt));

	memset(buf_cmd, 0, sizeof(buf_cmd));
	hdr = (brcm_hdr *)buf_cmd;
	pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	pkt->subcmd = 0x40;
	pkt->subcmd_arg.arg1 = 0x00;
	res = hid_write(handle, buf_cmd, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf_cmd, sizeof(*hdr) + sizeof(*pkt));

	return 0;
}

int play_tune(int tune_no) {
	int res;
	u8 buf[0x100];
	u8 buf2[0x100];

	//Enable Vibration
	memset(buf, 0, sizeof(buf));
	auto hdr = (brcm_hdr *)buf;
	auto pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	pkt->subcmd = 0x48;
	pkt->subcmd_arg.arg1 = 0x01;
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf2, 0);
	// This needs to be changed for new bigger tunes.
	u32 tune[6000];
	int tune_size;
	switch (tune_no) {
		case 0:
			memcpy(&tune, &tune_SMB, sizeof(tune_SMB));
			tune_size = sizeof(tune_SMB) / sizeof(u32);
			break;
		case 1:
			memcpy(&tune, &tune_SMO_OK, sizeof(tune_SMO_OK));
			tune_size = sizeof(tune_SMO_OK) / sizeof(u32);
			break;
	}

	for (int i = 0; i < tune_size; i++) {
		Sleep(15);
		memset(buf, 0, sizeof(buf));
		hdr = (brcm_hdr *)buf;
		pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 0x10;
		hdr->timer = timming_byte & 0xF;
		timming_byte++;
		hdr->rumble_l[0] = (tune[i] >> 24) & 0xFF;
		hdr->rumble_l[1] = (tune[i] >> 16) & 0xFF;
		hdr->rumble_l[2] = (tune[i] >> 8) & 0xFF;
		hdr->rumble_l[3] = tune[i] & 0xFF;
		memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
		res = hid_write(handle, buf, sizeof(*hdr));
		// Joy-con does not reply when Output Report is 0x10

		Application::DoEvents();
	}

	// Disable vibration
	Sleep(15);
	memset(buf, 0, sizeof(buf));
	hdr = (brcm_hdr *)buf;
	pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	hdr->rumble_l[0] = 0x00;
	hdr->rumble_l[1] = 0x01;
	hdr->rumble_l[2] = 0x40;
	hdr->rumble_l[3] = 0x40;
	memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
	pkt->subcmd = 0x48;
	pkt->subcmd_arg.arg1 = 0x00;
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf, 0);

	memset(buf, 0, sizeof(buf));
	hdr = (brcm_hdr *)buf;
	pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	pkt->subcmd = 0x30;
	pkt->subcmd_arg.arg1 = 0x01;
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf, 0);

	return 0;
}

int play_hd_rumble_file(int file_type, u16 sample_rate, int samples, int loop_start, int loop_end, int loop_wait, int loop_times) {
	int res;
	u8 buf[0x100];
	u8 buf2[0x100];

	//Enable Vibration
	memset(buf, 0, sizeof(buf));
	auto hdr = (brcm_hdr *)buf;
	auto pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	pkt->subcmd = 0x48;
	pkt->subcmd_arg.arg1 = 0x01;
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf2, 0);

	if (file_type == 1 || file_type == 2) {
		for (int i = 0; i < samples * 4; i = i + 4) {
			Sleep(sample_rate);
			memset(buf, 0, sizeof(buf));
			hdr = (brcm_hdr *)buf;
			pkt = (brcm_cmd_01 *)(hdr + 1);
			hdr->cmd = 0x10;
			hdr->timer = timming_byte & 0xF;
			timming_byte++;
			if (file_type == 1) {
				hdr->rumble_l[0] = FormJoy::myform1->vib_loaded_file[0x0A + i];
				hdr->rumble_l[1] = FormJoy::myform1->vib_loaded_file[0x0B + i];
				hdr->rumble_l[2] = FormJoy::myform1->vib_loaded_file[0x0C + i];
				hdr->rumble_l[3] = FormJoy::myform1->vib_loaded_file[0x0D + i];
			}
			//file_type is simple bnvib
			else {
				hdr->rumble_l[0] = FormJoy::myform1->vib_file_converted[0x0C + i];
				hdr->rumble_l[1] = FormJoy::myform1->vib_file_converted[0x0D + i];
				hdr->rumble_l[2] = FormJoy::myform1->vib_file_converted[0x0E + i];
				hdr->rumble_l[3] = FormJoy::myform1->vib_file_converted[0x0F + i];
			}
			memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));

			res = hid_write(handle, buf, sizeof(*hdr));
			Application::DoEvents();
		}
	}
	else if (file_type == 3 || file_type == 4) {
		u8 vib_off = 0;
		if (file_type == 3)
			vib_off = 8;
		else if (file_type == 4)
			vib_off = 12;

		for (int i = 0; i < loop_start * 4; i = i + 4) {
			Sleep(sample_rate);
			memset(buf, 0, sizeof(buf));
			hdr = (brcm_hdr *)buf;
			pkt = (brcm_cmd_01 *)(hdr + 1);
			hdr->cmd = 0x10;
			hdr->timer = timming_byte & 0xF;
			timming_byte++;
			
			hdr->rumble_l[0] = FormJoy::myform1->vib_loaded_file[0x0C + vib_off + i];
			hdr->rumble_l[1] = FormJoy::myform1->vib_loaded_file[0x0D + vib_off + i];
			hdr->rumble_l[2] = FormJoy::myform1->vib_loaded_file[0x0E + vib_off + i];
			hdr->rumble_l[3] = FormJoy::myform1->vib_loaded_file[0x0F + vib_off + i];
			memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
			
			res = hid_write(handle, buf, sizeof(*hdr));
			Application::DoEvents();
		}
		for (int j = 0; j < 1 + loop_times; j++) {
			for (int i = loop_start * 4; i < loop_end * 4; i = i + 4) {
				Sleep(sample_rate);
				memset(buf, 0, sizeof(buf));
				hdr = (brcm_hdr *)buf;
				pkt = (brcm_cmd_01 *)(hdr + 1);
				hdr->cmd = 0x10;
				hdr->timer = timming_byte & 0xF;
				timming_byte++;
				
				hdr->rumble_l[0] = FormJoy::myform1->vib_loaded_file[0x0C + vib_off + i];
				hdr->rumble_l[1] = FormJoy::myform1->vib_loaded_file[0x0D + vib_off + i];
				hdr->rumble_l[2] = FormJoy::myform1->vib_loaded_file[0x0E + vib_off + i];
				hdr->rumble_l[3] = FormJoy::myform1->vib_loaded_file[0x0F + vib_off + i];
				memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));

				res = hid_write(handle, buf, sizeof(*hdr));
				Application::DoEvents();
			}
			Sleep(sample_rate);
			// Disable vibration
			memset(buf, 0, sizeof(buf));
			hdr = (brcm_hdr *)buf;
			pkt = (brcm_cmd_01 *)(hdr + 1);
			hdr->cmd = 0x10;
			hdr->timer = timming_byte & 0xF;
			timming_byte++;
			hdr->rumble_l[0] = 0x00;
			hdr->rumble_l[1] = 0x01;
			hdr->rumble_l[2] = 0x40;
			hdr->rumble_l[3] = 0x40;
			memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
			res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
			Sleep(loop_wait * sample_rate);
		}
		for (int i = loop_end * 4; i < samples * 4; i = i + 4) {
			Sleep(sample_rate);
			memset(buf, 0, sizeof(buf));
			hdr = (brcm_hdr *)buf;
			pkt = (brcm_cmd_01 *)(hdr + 1);
			hdr->cmd = 0x10;
			hdr->timer = timming_byte & 0xF;
			timming_byte++;
			
			hdr->rumble_l[0] = FormJoy::myform1->vib_loaded_file[0x0C + vib_off + i];
			hdr->rumble_l[1] = FormJoy::myform1->vib_loaded_file[0x0D + vib_off + i];
			hdr->rumble_l[2] = FormJoy::myform1->vib_loaded_file[0x0E + vib_off + i];
			hdr->rumble_l[3] = FormJoy::myform1->vib_loaded_file[0x0F + vib_off + i];
			memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));

			res = hid_write(handle, buf, sizeof(*hdr));
			Application::DoEvents();
		}
	}

	Sleep(sample_rate);
	// Disable vibration
	memset(buf, 0, sizeof(buf));
	hdr = (brcm_hdr *)buf;
	pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x10;
	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	hdr->rumble_l[0] = 0x00;
	hdr->rumble_l[1] = 0x01;
	hdr->rumble_l[2] = 0x40;
	hdr->rumble_l[3] = 0x40;
	memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));

	Sleep(sample_rate + 120);
	memset(buf, 0, sizeof(buf));
	hdr = (brcm_hdr *)buf;
	pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	hdr->rumble_l[0] = 0x00;
	hdr->rumble_l[1] = 0x01;
	hdr->rumble_l[2] = 0x40;
	hdr->rumble_l[3] = 0x40;
	memcpy(hdr->rumble_r, hdr->rumble_l, sizeof(hdr->rumble_l));
	pkt->subcmd = 0x48;
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf, 0);

	memset(buf, 0, sizeof(buf));
	hdr = (brcm_hdr *)buf;
	pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->timer = timming_byte & 0xF;
	timming_byte++;
	pkt->subcmd = 0x30;
	pkt->subcmd_arg.arg1 = 0x01;
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf, 0);

	return 0;
}

/*
//usb test
int usb_init(hid_device *handle) {
	int res;
	u8 buf_cmd[2];
	u8 buf_cmd2[2];
	memset(buf_cmd, 0, sizeof(buf_cmd));
	memset(buf_cmd2, 0, sizeof(buf_cmd2));

	Sleep(16);
	buf_cmd[0] = 1;
	buf_cmd[0] = 0x80;
	Sleep(16);
	buf_cmd[1] = 0x01;
	res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
	res = hid_read(handle, buf_cmd2, 0);
	Sleep(16);
	buf_cmd[1] = 0x02;
	res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
	res = hid_read(handle, buf_cmd2, 0);
	Sleep(16);
	buf_cmd[1] = 0x03;
	res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
	res = hid_read(handle, buf_cmd2, 0);
	Sleep(16);
	buf_cmd[1] = 0x02;
	res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
	res = hid_read(handle, buf_cmd2, 0);
	Sleep(16);
	buf_cmd[1] = 0x04;
	res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
	res = hid_read(handle, buf_cmd2, 0);

	return 0;
}

int usb_deinit(hid_device *handle) {
	int res;
	u8 buf_cmd[2];
	u8 buf_cmd2[2];
	memset(buf_cmd, 0, sizeof(buf_cmd));
	memset(buf_cmd2, 0, sizeof(buf_cmd2));

	Sleep(16);
	buf_cmd[0] = 0x80;
	buf_cmd[1] = 0x05;
	res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
	res = hid_read(handle, buf_cmd2, 0);

	return 0;
}

int usb_command(hid_device *handle) {
	int res;
	u8 buf_cmd[20];
	u8 buf_cmd2[1];
	memset(buf_cmd, 0, sizeof(buf_cmd));
	memset(buf_cmd2, 0, sizeof(buf_cmd2));

	buf_cmd[0] = 0x80;
	buf_cmd[1] = 0x92;
	buf_cmd[3] = 0x31;
	buf_cmd[8] = 0x01;

	Sleep(16);

	buf_cmd[9] = timming_byte & 0xF;
	timming_byte++;
	buf_cmd[18] = 0x40;
	buf_cmd[19] = 0x01;
	res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
	res = hid_read(handle, buf_cmd2, 0);
	Sleep(16);

	buf_cmd[9] = timming_byte & 0xF;
	timming_byte++;
	buf_cmd[18] = 0x30;
	buf_cmd[19] = 0x1;
	res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
	res = hid_read(handle, buf_cmd2, 0);

	Sleep(16);

	buf_cmd[9] = timming_byte & 0xF;
	timming_byte++;
	buf_cmd[18] = 0x38;
	buf_cmd[19] = 0x22;
	buf_cmd[9] = timming_byte & 0xF;
	timming_byte++;
	res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
	res = hid_read(handle, buf_cmd2, 0);

	return 0;
}

void usb_device_print(struct hid_device_info *dev)
{
	const wchar_t *interface_name = L"";

	if (wcscmp(dev->serial_number, L"000000000001"))
		interface_name = L"BT-HID";
	else
		interface_name = L"USB-HID";

	printf("USB device info:\n  VID:          0x%04hX\n" \
		"  PID:          0x%04hX\n  Dev Path:     %s\n" \
		"  MAC:          %ls\n  Interface:    %ls (%d)\n  Manufacturer: %ls\n" \
		"  Product:      %ls\n\n", dev->vendor_id, dev->product_id,
		dev->path, dev->serial_number, interface_name, dev->interface_number,
		dev->manufacturer_string, dev->product_string);
}
*/

int test_chamber() {

	//Add your testing code.

	return 0;
	}

int device_connection(){
	handle_ok = 0;
	if (handle = hid_open(0x57e, 0x2006, nullptr))
		handle_ok = 1;
	if (!handle_ok) {
		if (handle = hid_open(0x57e, 0x2007, nullptr))
			handle_ok = 2;
	}
	if (!handle_ok) {
		if (handle = hid_open(0x57e, 0x2009, nullptr))
			handle_ok = 3;
	}
	/*
	//usb test
	if (!handle_ok) {
		hid_init();
		struct hid_device_info *devs = hid_enumerate(0x057E, 0x200e);
		if (devs){
			BOOL chk = AllocConsole();
			if (chk)
			{
				freopen("CONOUT$", "w", stdout);
				printf(" printing to console\n");
			}
			usb_device_print(devs);
			handle_l = hid_open_path(devs->path);
			devs = devs->next;
			handle = hid_open_path(devs->path);
			printf("\nlol\n");

			if (handle)
				handle_ok = 4;
		}
		hid_free_enumeration(devs);
	}
	*/
	if (handle_ok == 0)
		return 0;

	return 1;
}

[STAThread]
int Main(array<String^>^ args) {
	while (!device_connection()) {
		if (MessageBox::Show(L"The device is not paired or the device was disconnected!\n\nTo pair:\n1. Press and hold the sync button until the leds are on\n2. Pair the Bluetooth controller in Windows\n\n To connect again:\n1. Press a button on the controller", L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::RetryCancel, MessageBoxIcon::Stop) == System::Windows::Forms::DialogResult::Cancel)
			return 1;
	}

	//test_chamber();

	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	CppWinFormJoy::FormJoy^  myform1 = gcnew FormJoy();

	/*
	//usb test
	usb_init(handle);
	usb_init(handle_l);
	usb_command(handle);
	usb_command(handle);
	Sleep(2000);
	if (handle_ok) {
		usb_deinit(handle);
		hid_close(handle);
		usb_deinit(handle_l);
		hid_close(handle_l);
	}
	hid_exit();
	*/

	Application::Run(myform1);

	return 0;
}
