/*

MIT License

Copyright (c) 2020 Jonathan Mendez

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#include <iostream>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <codecvt>
#include <locale>

#include "controller_session.hpp"

#include "TP/TP.hpp"

#include "jctool.h"
#include "jctool_helpers.hpp"

namespace ConSessManager {
    /**
     * Provides a context for a controller session.
     */
    struct SessionContext {
        controller_hid_handle_t con_handle = nullptr;
        u8 timming_byte = 0;
    };

    namespace {
        static std::mutex cons_with_sessions_mutex;
        static std::unordered_map<Con, SessionContext, ConHash, ConEqual> cons_with_sessions;

        static ConSess::Status session_dispatcher(const Con& con, std::stringstream& msg_str, std::stringstream& err_str){
            SessionContext sc;
            sc.con_handle = hid_open_path(con.dev_path.c_str());
            if(!sc.con_handle){
                err_str << con.hid_sn << " was not able to be used for a session." << std::endl;
                return ConSess::SESS_ERR;
            }

            //decltype(std::declval<decltype(cons_with_sessions)>().insert({con,sc})) insert_res;
            //{
                //std::lock_guard<std::mutex> lock(cons_with_sessions_mutex);
                cons_with_sessions_mutex.lock();
                auto insert_res = cons_with_sessions.insert({con, sc});
                cons_with_sessions_mutex.unlock();
            //}

            if(!insert_res.second){
                err_str << insert_res.first->first.hid_sn << " already has a session. [handle is " << insert_res.first->second.con_handle << "]" << std::endl;
                return ConSess::SESS_EXISTS_ERR;
            }

            msg_str << insert_res.first->first.hid_sn << " new session established. [handle is " << insert_res.first->second.con_handle << "]" << std::endl;
            return ConSess::SESS_OK;
        }
    }

    /**
     * Return the end result of the session.
     */
    static std::future<ConSess::Status> kill_session(const Con& con, std::stringstream& msg_str, std::stringstream& err_str){
        // TODO:
        return std::future<ConSess::Status>();
    }

    /**
     * Place it on a job queue for the session dispatcher.
     */
    static std::future<ConSess::Status> enqueue_session(const Con& con, std::stringstream& msg_str, std::stringstream& err_str){
        auto promise = std::shared_ptr<std::promise<ConSess::Status>>(new std::promise<ConSess::Status>());
        auto res = promise->get_future();
        TP::add_job(
            [&, promise] (){
                promise->set_value(session_dispatcher(con, msg_str, err_str));
            }
        );
        return res;
    }

    static std::future<ConSess::Status> add_job(const Con& con, std::function<ConSess::Status(controller_hid_handle_t, u8&)> job, std::promise<ConSess::Status>& promise){
        promise = std::promise<ConSess::Status>{};
        auto res = promise.get_future();
        TP::add_job(
            [&, job] (){
                cons_with_sessions_mutex.lock();
                auto find_res = cons_with_sessions.find(con);
                cons_with_sessions_mutex.unlock();
                if(find_res == cons_with_sessions.end())
                    return promise.set_value(ConSess::NO_SESS);

                SessionContext& sc = find_res->second;
                // Execute the job, the job returns the status of the controller.
                ConSess::Status con_status = job(sc.con_handle, sc.timming_byte);
                return promise.set_value(con_status);
            }
        );

        return res;
    }
}

// Convert wide char string to char string.
static std::string ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
};

Con::Con(const hid_device_info* dev):
prod_id((ConHID::ProdID)dev->product_id),
hid_sn(ws2s(dev->serial_number)),
manu_string(ws2s(dev->manufacturer_string)),
prod_string(ws2s(dev->product_string)),
dev_path(dev->path)
{}

/**
 * Populate the controller, and optionally start the session in conjuction with a status delay.
 */
ConSess::ConSess(const Con& con, bool start_session, StatusDelay status_delay) :
ConSess()
{
    *this->con = con; // The default constructor allocated space for a new con.
    if(start_session)
        this->startSession(status_delay);
}

/**
 * It is OK to move a session once it has been started.
 */
ConSess::ConSess(ConSess&& sess_move) :
status_promise(std::move(sess_move.status_promise)),
last_future_status(std::move(sess_move.last_future_status)),
last_status_get(sess_move.last_status_get),
con(std::move(sess_move.con)),
msg_str(std::move(sess_move.msg_str)),
err_str(std::move(sess_move.err_str))
{}

/**
 * Kill the session on destructor.
 */
ConSess::~ConSess() {
    this->endSession();
}

/**
 * Start the session.
 */
ConSess::Status ConSess::startSession(StatusDelay status_delay){
    this->last_future_status = ConSessManager::enqueue_session(*this->con, *this->msg_str, *this->err_str);
    return this->getStatus(status_delay);
}

ConSess::Status ConSess::endSession(StatusDelay status_delay){
    // TODO:
    this->last_future_status = this->getDummyStatus(NO_RESPONSE);// = ConSessManager::kill_session(this->con, this->msg_str, this->err_str);
    return this->getStatus(status_delay);
}

/**
 * TODO: Ping the controller.
 */
ConSess::Status ConSess::checkConnectionStatus(StatusDelay status_delay){
    // TODO: assign a new future to last_status.
    this->last_future_status = this->getDummyStatus(NO_RESPONSE);
    return this->getStatus(status_delay);
}

#define CON_JOB(lbda_captures...) [lbda_captures](controller_hid_handle_t handle, u8& timming_byte)

void ConSess::testSetLedBusy(){
    ConSessManager::add_job(*this->con,
        CON_JOB(this){
            *msg_str << this->con->hid_sn << " set_led_busy" << std::endl;
            
            set_led_busy(handle, timming_byte, this->getProdID()); // Always returns 0, so no error checking.
            return SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        },
        this->status_promise
    );
}

void ConSess::testIRCapture(IRSensor& ir){
    ConSessManager::add_job(*this->con,
        CON_JOB(&){
            *msg_str << this->con->hid_sn << " IRSensor::capture" << std::endl;
            ir.capture(handle, timming_byte);
            return SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        },
        this->status_promise
    );
}

void ConSess::testHDRumble(RumbleData& rumble_data, bool& is_active){
    is_active = true;
    ConSessManager::add_job(*this->con,
        CON_JOB(&){
            *msg_str << this->con->hid_sn << " play_hd_rumble_file" << std::endl;
            int res = play_hd_rumble_file(handle, timming_byte, rumble_data);
            is_active = false;
            return SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        },
        this->status_promise
    );
}

void ConSess::getTemperature(TemperatureData& fill_temp_data){
    ConSessManager::add_job(*this->con,
        CON_JOB(&){
            *msg_str << this->con->hid_sn << " get_temperature, parseTemperatureData" << std::endl;
            unsigned char temperature_data[2] = {};
            get_temperature(handle, timming_byte, temperature_data);
            fill_temp_data = parseTemperatureData(temperature_data);
            return SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        },
        this->status_promise
    );
}

void ConSess::getBattery(BatteryData& fill_batt_data){
    ConSessManager::add_job(*this->con,
        CON_JOB(&){
            *msg_str << this->con->hid_sn << " get_battery, parseBatteryData" << std::endl;
            unsigned char battery_data[3] = {};
            get_battery(handle, timming_byte, battery_data);
            fill_batt_data = parseBatteryData(battery_data);
            return SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        },
        this->status_promise
    );
}

void ConSess::getColors(SPIColors& fill_spi_colors){
    ConSessManager::add_job(*this->con,
        CON_JOB(&){
            *msg_str << this->con->hid_sn << " get_spi_colors" << std::endl;
            fill_spi_colors = get_spi_colors(handle, timming_byte);
            return SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        },
        this->status_promise
    );
}

void ConSess::dumpSPI(const std::string& to_file, bool& is_dumping, size_t& bytes_dumped, bool& cancel_spi_dump){
    is_dumping = true;
    cancel_spi_dump = false;
    ConSessManager::add_job(*this->con,
        CON_JOB(&, spi_dump_file = to_file){
            *msg_str << this->con->hid_sn << " dump_spi" << std::endl;
            DumpSPICTX ctx{
                cancel_spi_dump,
                bytes_dumped,
                spi_dump_file.c_str()
            };
            bytes_dumped = 0;
            int res = dump_spi(handle, timming_byte, ctx);
            if(res)
                *msg_str << this->con->hid_sn << " There was a problem backing up the SPI. Try again?" << std::endl;

            is_dumping = false;      
            return SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        },
        this->status_promise
    );
}

void ConSess::writeColorsToSPI(const SPIColors& colors){
    ConSessManager::add_job(*this->con,
        CON_JOB(&, colors = colors){
            *msg_str << this->con->hid_sn << " write_spi_colors" << std::endl;
            int res = write_spi_colors(handle, timming_byte, colors);
            if(res)
                *msg_str << this->con->hid_sn << " There was a problem writing the colors. Try again?" << std::endl;
            return SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        },
        this->status_promise
    );
}

void ConSess::testSendRumble(){
    ConSessManager::add_job(*this->con,
        CON_JOB(this){
            *msg_str << this->con->hid_sn << " send_rumble" << std::endl;
            send_rumble(handle, timming_byte, this->getProdID());
            return SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        },
        this->status_promise
    );
}
