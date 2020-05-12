#include "jctool_input.hpp"
#include "jctool_helpers.hpp"

#ifdef WIN32
#include <cstdio>
#include <Windows.h>
#else
#include <stdio.h>
#include <unistd.h>
inline int Sleep(uint64_t ms){
    return usleep(ms*1000);
};
#include <math.h> // for sqrt
const auto min = [](auto a, auto b){
    return (a < b) ? a : b;
};
#endif

namespace Input {
    void decode_stick_params(u16 *decoded_stick_params, u8 *encoded_stick_params) {
        decoded_stick_params[0] = (encoded_stick_params[1] << 8) & 0xF00 | encoded_stick_params[0];
        decoded_stick_params[1] = (encoded_stick_params[2] << 4) | (encoded_stick_params[1] >> 4);
    }


    void encode_stick_params(u8 *encoded_stick_params, u16 *decoded_stick_params) {
        encoded_stick_params[0] =  decoded_stick_params[0] & 0xFF;
        encoded_stick_params[1] = (decoded_stick_params[0] & 0xF00) >> 8 | (decoded_stick_params[1] & 0xF) << 4;
        encoded_stick_params[2] = (decoded_stick_params[1] & 0xFF0) >> 4;
    }


    // Credit to Hypersect (Ryan Juckett)
    // http://blog.hypersect.com/interpreting-analog-sticks/
    void AnalogStickCalc(
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
            pOutX[0] = x_f * scale;
            pOutY[0] = y_f * scale;
        }
        else
        {
            // stick is in the inner dead zone
            pOutX[0] = 0.0f;
            pOutY[0] = 0.0f;
        }
    }

    #ifndef __jctool_disable_legacy_ui__
    int button_test() {
        int res;
        int limit_output = 0;
        String^ input_report_cmd;
        String^ input_report_sys;
        u8 buf_cmd[49];
        u8 buf_reply[0x170];
        float acc_cal_coeff[3];
        float gyro_cal_coeff[3];
        float cal_x[1] = { 0.0f };
        float cal_y[1] = { 0.0f };

        bool has_user_cal_stick_l = false;
        bool has_user_cal_stick_r = false;
        bool has_user_cal_sensor = false;

        u8 factory_stick_cal[0x12];
        u8 user_stick_cal[0x16];
        u8 sensor_model[0x6];
        u8 stick_model[0x24];
        u8 factory_sensor_cal[0x18];
        u8 user_sensor_cal[0x1A];
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
        FormJoy::myform1->txtBox_devParameters->Text = String::Format(L"Flat surface ACC Offset:\r\n{0:X4} {1:X4} {2:X4}\r\n\r\n\r\nStick Parameters:\r\n{3:X3} {4:X3}\r\n{5:X2} (Deadzone)\r\n{6:X3} (Range ratio)",
            sensor_model[0] | sensor_model[1] << 8,
            sensor_model[2] | sensor_model[3] << 8,
            sensor_model[4] | sensor_model[5] << 8,
            (stick_model[1] << 8) & 0xF00 | stick_model[0], (stick_model[2] << 4) | (stick_model[1] >> 4),
            (stick_model[4] << 8) & 0xF00 | stick_model[3],
            ((stick_model[5] << 4) | (stick_model[4] >> 4)));

        for (int i = 0; i < 10; i = i + 3) {
            FormJoy::myform1->txtBox_devParameters->Text += String::Format(L"\r\n{0:X3} {1:X3}",
                (stick_model[7 + i] << 8) & 0xF00 | stick_model[6 + i],
                (stick_model[8 + i] << 4) | (stick_model[7 + i] >> 4));
        }

        FormJoy::myform1->txtBox_devParameters2->Text = String::Format(L"Stick Parameters 2:\r\n{0:X3} {1:X3}\r\n{2:X2} (Deadzone)\r\n{3:X3} (Range ratio)",
            (stick_model[19] << 8) & 0xF00 | stick_model[18], (stick_model[20] << 4) | (stick_model[19] >> 4),
            (stick_model[22] << 8) & 0xF00 | stick_model[21],
            ((stick_model[23] << 4) | (stick_model[22] >> 4)));

        for (int i = 0; i < 10; i = i + 3) {
            FormJoy::myform1->txtBox_devParameters2->Text += String::Format(L"\r\n{0:X3} {1:X3}",
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
                stick_cal_x_r[1], stick_cal_y_r[1],    stick_cal_x_r[0], stick_cal_y_r[0],    stick_cal_x_r[2], stick_cal_y_r[2]);
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
        res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
        res = hid_read_timeout(handle, buf_cmd, 0, 120);

        // Enable IMU
        memset(buf_cmd, 0, sizeof(buf_cmd));
        hdr = (brcm_hdr *)buf_cmd;
        pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x01;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x40;
        pkt->subcmd_arg.arg1 = 0x01;
        res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
        res = hid_read_timeout(handle, buf_cmd, 0, 120);

        // Use SPI calibration and convert them to SI acc unit (m/s^2)
        acc_cal_coeff[0] = (float)(1.0 / (float)(16384 - uint16_to_int16(sensor_cal[0][0]))) * 4.0f  * 9.8f;
        acc_cal_coeff[1] = (float)(1.0 / (float)(16384 - uint16_to_int16(sensor_cal[0][1]))) * 4.0f  * 9.8f;
        acc_cal_coeff[2] = (float)(1.0 / (float)(16384 - uint16_to_int16(sensor_cal[0][2]))) * 4.0f  * 9.8f;

        // Use SPI calibration and convert them to SI gyro unit (rad/s)
        gyro_cal_coeff[0] = (float)(936.0 / (float)(13371 - uint16_to_int16(sensor_cal[1][0])) * 0.01745329251994);
        gyro_cal_coeff[1] = (float)(936.0 / (float)(13371 - uint16_to_int16(sensor_cal[1][1])) * 0.01745329251994);
        gyro_cal_coeff[2] = (float)(936.0 / (float)(13371 - uint16_to_int16(sensor_cal[1][2])) * 0.01745329251994);

        // Input report loop
        while (enable_button_test) {
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
                            cal_x[0], cal_y[0]);
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
                            cal_x[0], cal_y[0]);
                    }

                    input_report_sys = String::Format(L"Acc/meter (Raw/Cal):\r\n");
                    //The controller sends the sensor data 3 times with a little bit different values. Skip them
                    input_report_sys += String::Format(L"X: {0:X4}  {1,7:F2} m/s\u00B2\r\n", buf_reply[13] | (buf_reply[14] << 8) & 0xFF00,
                        (float)(uint16_to_int16(buf_reply[13] | (buf_reply[14] << 8) & 0xFF00)) * acc_cal_coeff[0]);
                    input_report_sys += String::Format(L"Y: {0:X4}  {1,7:F2} m/s\u00B2\r\n", buf_reply[15] | (buf_reply[16] << 8) & 0xFF00,
                        (float)(uint16_to_int16(buf_reply[15] | (buf_reply[16] << 8) & 0xFF00)) * acc_cal_coeff[1]);
                    input_report_sys += String::Format(L"Z: {0:X4}  {1,7:F2} m/s\u00B2\r\n", buf_reply[17] | (buf_reply[18] << 8) & 0xFF00,
                        (float)(uint16_to_int16(buf_reply[17] | (buf_reply[18] << 8) & 0xFF00))  * acc_cal_coeff[2]);

                    input_report_sys += String::Format(L"\r\nGyroscope (Raw/Cal):\r\n");
                    input_report_sys += String::Format(L"X: {0:X4}  {1,7:F2} rad/s\r\n", buf_reply[19] | (buf_reply[20] << 8) & 0xFF00,
                        (float)(uint16_to_int16(buf_reply[19] | (buf_reply[20] << 8) & 0xFF00) - uint16_to_int16(sensor_cal[1][0])) * gyro_cal_coeff[0]);
                    input_report_sys += String::Format(L"Y: {0:X4}  {1,7:F2} rad/s\r\n", buf_reply[21] | (buf_reply[22] << 8) & 0xFF00,
                        (float)(uint16_to_int16(buf_reply[21] | (buf_reply[22] << 8) & 0xFF00) - uint16_to_int16(sensor_cal[1][1])) * gyro_cal_coeff[1]);
                    input_report_sys += String::Format(L"Z: {0:X4}  {1,7:F2} rad/s\r\n", buf_reply[23] | (buf_reply[24] << 8) & 0xFF00,
                        (float)(uint16_to_int16(buf_reply[23] | (buf_reply[24] << 8) & 0xFF00) - uint16_to_int16(sensor_cal[1][2])) * gyro_cal_coeff[2]);
                }
                else if (buf_reply[0] == 0x3F) {
                    input_report_cmd = L"";
                    for (int i = 0; i < 17; i++)
                        input_report_cmd += String::Format(L"{0:X2} ", buf_reply[i]);
                }

                if (limit_output == 1) {
                    FormJoy::myform1->textBox_btn_test_reply->Text    = input_report_cmd;
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
        res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
        res = hid_read_timeout(handle, buf_cmd, 0, 64);

        memset(buf_cmd, 0, sizeof(buf_cmd));
        hdr = (brcm_hdr *)buf_cmd;
        pkt = (brcm_cmd_01 *)(hdr + 1);
        hdr->cmd = 0x01;
        hdr->timer = timming_byte & 0xF;
        timming_byte++;
        pkt->subcmd = 0x40;
        pkt->subcmd_arg.arg1 = 0x00;
        res = hid_write(handle, buf_cmd, sizeof(buf_cmd));
        res = hid_read_timeout(handle, buf_cmd, 0, 64);

        return 0;
    }
    #else
    // TODO: Implement else
    #endif

}