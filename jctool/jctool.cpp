#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <Windows.h>
#define NOMINMAX

#include "jctool.h"
#include "FormJoy.h"
#include "tune.h"

using namespace CppWinFormJoy;

#pragma comment(lib, "SetupAPI")

#pragma pack(push, 1)

struct brcm_hdr {
	u8 cmd;
	u8 rumble[9];
};

struct brcm_cmd_01 {
	u8 subcmd;
	union {
		
		struct {
			u32 offset;
			u8 size;
		} spi_read;
		
		struct {
			u32 address;
		} hax_read;
	};
};

int timming_byte = 0x0;

#pragma pack(pop)

int set_led_busy() {
	int res;
	u8 buf[0x100];
	memset(buf, 0, sizeof(buf));
	auto hdr = (brcm_hdr *)buf;
	auto pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->rumble[0] = timming_byte;
	timming_byte++;
	if (timming_byte > 0xF)
		timming_byte = 0x0;
	pkt->subcmd = 0x30;
	pkt->spi_read.offset = 0x81;
	pkt->spi_read.size = 0x00;
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf, 0);

	return 0;
}

std::string get_sn(hid_device *handle, u32 offset, const u16 read_len) {
	int res;
	u8 buf[0x100];
	std::string test = "";
	while (1) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brcm_hdr *)buf;
		auto pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 1;
		hdr->rumble[0] = timming_byte;
		timming_byte++;
		if (timming_byte > 0xF)
			timming_byte = 0x0;
		pkt->subcmd = 0x10;
		pkt->spi_read.offset = offset;
		pkt->spi_read.size = read_len;
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));

		res = hid_read(handle, buf, sizeof(buf));

		if ((*(uint16_t*)&buf[0xD] == 0x1090) && (*(uint32_t*)&buf[0xF] == offset))
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

int get_spi_data(hid_device *handle, u32 offset, const u16 read_len, u8 *test_buf) {
	int res;
	u8 buf[0x100];
	while (1) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brcm_hdr *)buf;
		auto pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 1;
		hdr->rumble[0] = timming_byte;
		timming_byte++;
		if (timming_byte > 0xF)
			timming_byte = 0x0;
		pkt->subcmd = 0x10;
		pkt->spi_read.offset = offset;
		pkt->spi_read.size = read_len;
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));

		res = hid_read(handle, buf, sizeof(buf));

		if ((*(uint16_t*)&buf[0xD] == 0x1090) && (*(uint32_t*)&buf[0xF] == offset))
			break;
	}
	if (res >= 0x14 + read_len) {
			for (int i = 0; i < read_len; i++) {
				test_buf[i] = buf[0x14 + i];
			}
	}
	
	return 0;
}

int write_spi_data(hid_device *handle, u32 offset, const u16 write_len, u8* test_buf) {
	int res;
	u8 buf[0x100];
	int error_writing = 0;
	while (1) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brcm_hdr *)buf;
		auto pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 1;
		hdr->rumble[0] = timming_byte;
		timming_byte++;
		if (timming_byte > 0xF)
			timming_byte = 0x0;
		pkt->subcmd = 0x11;
		pkt->spi_read.offset = offset;
		pkt->spi_read.size = write_len;
		for (int i = 0; i < write_len; i++) {
			buf[0x10 + i] = test_buf[i];
		}
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt) + write_len);

		res = hid_read(handle, buf, sizeof(buf));

		if (*(uint16_t*)&buf[0xD] == 0x1180)
			break;

		error_writing++;
		if (error_writing == 125)
			return 1;
	}

	return 0;

}

int get_device_info(hid_device *handle, u8* test_buf) {
	int res;
	u8 buf[0x100];
	int error_reading = 0;
	while (1) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brcm_hdr *)buf;
		auto pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 1;
		hdr->rumble[0] = timming_byte;
		timming_byte++;
		if (timming_byte > 0xF)
			timming_byte = 0x0;
		pkt->subcmd = 0x02;
		pkt->spi_read.offset = 0x00;
		pkt->spi_read.size = 0x00;

		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
		//printf("write %d\n", res);
		res = hid_read(handle, buf, sizeof(buf));
		
		if (*(uint16_t*)&buf[0xD] == 0x0282)
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

int get_battery(hid_device *handle, u8* test_buf) {
	int res;
	u8 buf[0x100];
	int error_reading = 0;
	while (1) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brcm_hdr *)buf;
		auto pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 1;
		hdr->rumble[0] = timming_byte;
		timming_byte++;
		if (timming_byte > 0xF)
			timming_byte = 0x0;
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
		//printf("write %d\n", res);
		res = hid_read(handle, buf, sizeof(buf));

		if (*(uint8_t*)&buf[0x0] == 0x21)
			break;
		error_reading++;
		if (error_reading == 125)
			break;
	}
		test_buf[0] = buf[0x2];

	return 0;

}

int dump_spi(hid_device *handle, const char *dev_name) {
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
	while (offset < 0x80000) {
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
			hdr->rumble[0] = timming_byte;
			timming_byte++;
			if (timming_byte > 0xF)
				timming_byte = 0x0;
			pkt->subcmd = 0x10;
			pkt->spi_read.offset = offset;
			pkt->spi_read.size = read_len;
			res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
			res = hid_read(handle, buf, sizeof(buf));

			if ((*(uint16_t*)&buf[0xD] == 0x1090) && (*(uint32_t*)&buf[0xF] == offset))
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

int send_rumble(hid_device *handle) {
	int res;
	u8 buf[0x100];
	u8 buf2[0x100];
	
	//Enable Vibration
	memset(buf, 0, sizeof(buf));
	auto hdr = (brcm_hdr *)buf;
	auto pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->rumble[0] = timming_byte;
	timming_byte++;
	if (timming_byte > 0xF)
		timming_byte = 0x0;
	pkt->subcmd = 0x48;
	pkt->spi_read.offset = 0x01;
	pkt->spi_read.size = 0x00;
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf2, 0);

	//New vibration like switch
	if (1) {
		Sleep(16);
		//Send confirmation 
		memset(buf, 0, sizeof(buf));
		hdr->cmd = 0x01;
		hdr->rumble[0] = timming_byte;
		timming_byte++;
		if (timming_byte > 0xF)
			timming_byte = 0x0;
		hdr->rumble[1] = 0xc2; hdr->rumble[2] = 0xc8; hdr->rumble[3] = 0x03; hdr->rumble[4] = 0x72;
		hdr->rumble[5] = 0xc2; hdr->rumble[6] = 0xc8; hdr->rumble[7] = 0x03; hdr->rumble[8] = 0x72;
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));

		res = hid_read(handle, buf2, 0);

		Sleep(81);
		hdr->rumble[0] = timming_byte;
		timming_byte++;
		if (timming_byte > 0xF)
			timming_byte = 0x0;
		hdr->rumble[1] = 0x00; hdr->rumble[2] = 0x01; hdr->rumble[3] = 0x40; hdr->rumble[4] = 0x40;
		hdr->rumble[5] = 0x00; hdr->rumble[6] = 0x01; hdr->rumble[7] = 0x40; hdr->rumble[8] = 0x40;
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));

		res = hid_read(handle, buf2, 0);

		Sleep(5);
		hdr->rumble[0] = timming_byte;
		timming_byte++;
		if (timming_byte > 0xF)
			timming_byte = 0x0;
		hdr->rumble[1] = 0xc3; hdr->rumble[2] = 0xc8; hdr->rumble[3] = 0x60; hdr->rumble[4] = 0x64;
		hdr->rumble[5] = 0xc3; hdr->rumble[6] = 0xc8; hdr->rumble[7] = 0x60; hdr->rumble[8] = 0x64;
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));

		res = hid_read(handle, buf2, 0);
		Sleep(5);
	}

	//old style vibration
	if (0) {
		//Send confirmation 
		memset(buf, 0, sizeof(buf));
		hdr->cmd = 0x01;
		hdr->rumble[0] = timming_byte;
		timming_byte++;
		if (timming_byte > 0xF)
			timming_byte = 0x0;
		hdr->rumble[1] = 0x20; hdr->rumble[2] = 0xb1; hdr->rumble[3] = 0x40; hdr->rumble[4] = 0x40;
		hdr->rumble[5] = 0x20; hdr->rumble[6] = 0xb1; hdr->rumble[7] = 0x40; hdr->rumble[8] = 0x40;
		//hid_set_nonblocking(handle, 1);
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
		//hid_set_nonblocking(handle, 1);
		res = hid_read(handle, buf2, 0);
		Sleep(64);
		//Stop vibration
		hdr->rumble[0] = timming_byte;
		timming_byte++;
		if (timming_byte > 0xF)
			timming_byte = 0x0;
		hdr->rumble[1] = 0x00; hdr->rumble[2] = 0x01; hdr->rumble[3] = 0x40; hdr->rumble[4] = 0x54;
		hdr->rumble[5] = 0x00; hdr->rumble[6] = 0x01; hdr->rumble[7] = 0x40; hdr->rumble[8] = 0x54;
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
		//hid_set_nonblocking(handle, 1);
		res = hid_read(handle, buf2, 0);
		Sleep(64);
	}
	//Disable vibration
	memset(buf, 0, sizeof(buf));
	hdr = (brcm_hdr *)buf;
	pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->rumble[0] = timming_byte;
	timming_byte++;
	if (timming_byte > 0xF)
		timming_byte = 0x0;
	hdr->rumble[1] = 0x00; hdr->rumble[2] = 0x01; hdr->rumble[3] = 0x40; hdr->rumble[4] = 0x40;
	hdr->rumble[5] = 0x00; hdr->rumble[6] = 0x01; hdr->rumble[7] = 0x40; hdr->rumble[8] = 0x40;
	pkt->subcmd = 0x48;
	pkt->spi_read.offset = 0x00;
	pkt->spi_read.size = 0x00;
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf, 0);

	memset(buf, 0, sizeof(buf));
	hdr = (brcm_hdr *)buf;
	pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->rumble[0] = timming_byte;
	timming_byte++;
	if (timming_byte > 0xF)
		timming_byte = 0x0;
	pkt->subcmd = 0x30;
	pkt->spi_read.offset = 0x01;
	pkt->spi_read.size = 0x00;
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf, 0);

	return 0;

}

int send_custom_command(hid_device *handle, u8* arg) {
	int res;
	u8 buf_cmd[36];
	u8 buf_reply[0x170];
		memset(buf_cmd, 0, sizeof(buf_cmd));
		memset(buf_reply, 0, sizeof(buf_reply));

		buf_cmd[0] = arg[0];
		buf_cmd[1] = timming_byte;
		buf_cmd[2] = buf_cmd[6] = arg[1];
		buf_cmd[3] = buf_cmd[7] = arg[2];
		buf_cmd[4] = buf_cmd[8] = arg[3];
		buf_cmd[5] = buf_cmd[9] = arg[4];
		timming_byte++;
		if (timming_byte > 0xF)
			timming_byte = 0x0;

		buf_cmd[10] = arg[5];

		String^ output_report_sys = String::Format(L"Cmd:  {0:X2}   Subcmd: {1:X2}\r\n", buf_cmd[0], buf_cmd[10]);
		int byte8_2 = 1;
		for (int i = 6; i < 31; i++) {
			buf_cmd[5 + i] = arg[i];
			output_report_sys += String::Format(L"{0:X2} ", buf_cmd[5 + i]);
			if (byte8_2 == 4)
				output_report_sys += L" ";
			if (byte8_2 == 8)
			{

				byte8_2 = 0;
				output_report_sys += L"\r\n";
			}
			byte8_2++;
		}
		FormJoy::myform1->textBoxDbg_sent->Text = output_report_sys;
	

		//Packet size header + subcommand and uint8 argument
		res = hid_write(handle, buf_cmd, sizeof(buf_cmd));

		res = hid_read_timeout(handle, buf_reply, sizeof(buf_reply), 200);

		String^ input_report_cmd;
		String^ input_report_sys;

		int byte8 = 2;

		if (res > 12) {
			if (buf_reply[0] == 0x21 || buf_reply[0] == 0x30 || buf_reply[0] == 0x33 || buf_reply[0] == 0x31 || buf_reply[0] == 0x3F) {
				input_report_cmd += String::Format(L"\r\nInput report: 0x{0:X2}\r\n", buf_reply[0]);
				input_report_sys += String::Format(L"Subcmd Reply:\r\n", buf_reply[0]);
				int len = 49;
				if (buf_reply[0] == 0x33 || buf_reply[0] == 0x31)
					len = 362;
				byte8 = 1;
				for (int i = 1; i < 13; i++)
					input_report_cmd += String::Format(L"{0:X2} ", buf_reply[i]);
				for (int i = 13; i < len; i++) {
					input_report_sys += String::Format(L"{0:X2} ", buf_reply[i]);
					if (byte8 == 4)
						input_report_sys += L" ";
					if (byte8 == 8)
					{
						
						byte8 = 0;
						input_report_sys += L"\r\n";
					}
					byte8++;
				}
			}
			else {
				input_report_sys += String::Format(L"ID: {0:X2} Subcmd reply:\r\n", buf_reply[0]);

				for (int i = 13; i < res; i++)
					input_report_sys += String::Format(L"{0:X2} ", buf_reply[i]);
			}
		}
		else if (res > 0){
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

int play_tune(hid_device *handle) {

	int res;
	u8 buf[0x100];
	u8 buf2[0x100];

	//Enable Vibration
	memset(buf, 0, sizeof(buf));
	auto hdr = (brcm_hdr *)buf;
	auto pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->rumble[0] = timming_byte;
	timming_byte++;
	if (timming_byte > 0xF)
		timming_byte = 0x0;
	pkt->subcmd = 0x48;
	pkt->spi_read.offset = 0x01;
	pkt->spi_read.size = 0x00;
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf2, 0);

	for (int i = 0; i < 2740; i++) {
		Sleep(15);
		memset(buf, 0, sizeof(buf));
		hdr = (brcm_hdr *)buf;
		pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 0x10;
		hdr->rumble[0] = timming_byte;
		timming_byte++;
		if (timming_byte > 0xF)
			timming_byte = 0x0;
		hdr->rumble[1] = (tune[i] >> 24) & 0xFF; hdr->rumble[2] = (tune[i] >> 16) & 0xFF; hdr->rumble[3] = (tune[i] >> 8) & 0xFF; hdr->rumble[4] = tune[i] & 0xFF;
		hdr->rumble[5] = (tune[i] >> 24) & 0xFF; hdr->rumble[6] = (tune[i] >> 16) & 0xFF; hdr->rumble[7] = (tune[i] >> 8) & 0xFF; hdr->rumble[8] = (tune[i] & 0xFF);
		res = hid_write(handle, buf, sizeof(*hdr));

		//Joy-con does not reply when Output Report is 0x10
		//res = hid_read_timeout(handle, buf2, 0, 0);
		Application::DoEvents();
	}

	//Disable vibration
	memset(buf, 0, sizeof(buf));
	hdr = (brcm_hdr *)buf;
	pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->rumble[0] = timming_byte;
	timming_byte++;
	if (timming_byte > 0xF)
		timming_byte = 0x0;
	hdr->rumble[1] = 0x00; hdr->rumble[2] = 0x01; hdr->rumble[3] = 0x40; hdr->rumble[4] = 0x40;
	hdr->rumble[5] = 0x00; hdr->rumble[6] = 0x01; hdr->rumble[7] = 0x40; hdr->rumble[8] = 0x40;
	pkt->subcmd = 0x48;
	pkt->spi_read.offset = 0x00;
	pkt->spi_read.size = 0x00;
	res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
	res = hid_read(handle, buf, 0);

	memset(buf, 0, sizeof(buf));
	hdr = (brcm_hdr *)buf;
	pkt = (brcm_cmd_01 *)(hdr + 1);
	hdr->cmd = 0x01;
	hdr->rumble[0] = timming_byte;
	timming_byte++;
	if (timming_byte > 0xF)
		timming_byte = 0x0;
	pkt->subcmd = 0x30;
	pkt->spi_read.offset = 0x01;
	pkt->spi_read.size = 0x00;
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

	buf_cmd[9] = timming_byte;
	timming_byte++;
	if (timming_byte > 0xF)
		timming_byte = 0x0;
	buf_cmd[18] = 0x40;
	buf_cmd[19] = 0x01;
	res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
	res = hid_read(handle, buf_cmd2, 0);
	Sleep(16);

	buf_cmd[9] = timming_byte;
	timming_byte++;
	if (timming_byte > 0xF)
		timming_byte = 0x0;
	buf_cmd[18] = 0x30;
	buf_cmd[19] = 0x1;
	res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
	res = hid_read(handle, buf_cmd2, 0);

	Sleep(16);

	buf_cmd[9] = timming_byte;
	timming_byte++;
	if (timming_byte > 0xF)
		timming_byte = 0x0;
	buf_cmd[18] = 0x38;
	buf_cmd[19] = 0x22;
	buf_cmd[9] = timming_byte;
	timming_byte++;
	if (timming_byte > 0xF)
		timming_byte = 0x0;
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
int device_connection(){
	handle_ok = 0;
	handle = hid_open(0x57e, 0x2006, nullptr);
	if (handle)
		handle_ok = 1;
	if (!handle_ok) {
		handle = hid_open(0x57e, 0x2007, nullptr);
		if (handle)
			handle_ok = 2;
	}
	if (!handle_ok) {
		handle = hid_open(0x57e, 0x2009, nullptr);
		if (handle)
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
int Main(array<String^>^ args)
{
	while (!device_connection()) {
		if (MessageBox::Show(L"The device is not paired or the device was disconnected!\n\nTo pair:\n1. Press and hold the sync button until the leds are on\n2. Pair the Bluetooth controller in Windows\n\n To connect again:\n1. Press a button on the controller", L"CTCaer's Joy-Con Toolkit - Connection Error!", MessageBoxButtons::RetryCancel, MessageBoxIcon::Stop) == System::Windows::Forms::DialogResult::Cancel)
			return 1;
	}

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
