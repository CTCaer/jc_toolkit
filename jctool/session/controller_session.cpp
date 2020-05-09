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

/**
 * Provides a context for a controller session.
 */
struct SessionContext {
    controller_hid_handle_t con_handle;
    u8 timming_byte;
    SessionStatus status;
    std::unique_ptr<std::stringstream> err_str;
    std::unique_ptr<std::stringstream> msg_str;

    SessionContext():
    con_handle{nullptr},
    timming_byte{0},
    status{NO_SESS},
    err_str{new std::stringstream},
    msg_str{new std::stringstream}
    {}
    
    SessionContext(SessionContext&& move_sess):
    con_handle{move_sess.con_handle},
    timming_byte{move_sess.timming_byte},
    status{move_sess.status},
    err_str{std::move(move_sess.err_str)},
    msg_str{std::move(move_sess.msg_str)}
    {}
};
namespace ConSessManager {
    namespace {
        static std::mutex cons_with_sessions_mutex;
        static std::unordered_map<Con, SessionContext, ConHash, ConEqual> cons_with_sessions;
        static std::stringstream msg_stream;
        static std::stringstream err_stream;
        /**
         * Trys to connect to a controller device.
         * On success, dispatches a session.
         */
        static SessionStatus session_dispatcher(const Con& con){
            SessionContext sc;
            sc.con_handle = hid_open_path(con.dev_path.c_str());
            if(!sc.con_handle){
                err_stream << con.hid_sn << " was not able to be used for a session." << std::endl;
                return SESS_ERR;
            }

            cons_with_sessions_mutex.lock();
            auto insert_res = cons_with_sessions.insert({con, std::move(sc)});
            cons_with_sessions_mutex.unlock();

            if(!insert_res.second){
                msg_stream << insert_res.first->first.hid_sn << " already has a session. [handle is " << insert_res.first->second.con_handle << "]" << std::endl;
                return SESS_OK;
            }

            *insert_res.first->second.msg_str << insert_res.first->first.hid_sn << " new session established. [handle is " << insert_res.first->second.con_handle << "]" << std::endl;
            return SESS_OK;
        }
    }

    /**
     * Return the end result of the session.
     */
    static void kill_session(const Con& con, std::stringstream& msg_str, std::stringstream& err_str){
        // TODO:
        return;
    }


    using FindConSessRes = decltype(cons_with_sessions.end());
    static void find_con_sess_ctx(const Con& con, FindConSessRes& out_find_res){
        cons_with_sessions_mutex.lock();
        out_find_res = cons_with_sessions.find(con);
        cons_with_sessions_mutex.unlock();
    }
    static bool get_con_sess_ctx(const Con& con, SessionContext*& out_sess_ctx){
        FindConSessRes find_res;
        find_con_sess_ctx(con, find_res);
        if((find_res) == cons_with_sessions.end())
            return false;
        out_sess_ctx = &find_res->second;
        return true;
    }

    static void add_job(const Con& con, std::function<void(const Con&, SessionContext&)> job){
        TP::add_job(
            [con = con, job] (){
                SessionContext* sess_ctx_p;
                if(!get_con_sess_ctx(con, sess_ctx_p))
                    return;
                job(con, *sess_ctx_p);
                //return promise->set_value(con_status);
            }
        );
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
ConSess::ConSess(const Con& con, bool start_session, std::function<void(SessionStatus)> status_cb) :
ConSess()
{
    this->con = con; // The default constructor allocated space for a new con.
    if(start_session)
        this->startSession(status_cb);
}

/**
 * It is OK to move a session once it has been started.
 */
ConSess::ConSess(ConSess&& sess_move) :
con(std::move(sess_move.con))
{}

/**
 * Kill the session on destructor.
 */
ConSess::~ConSess() {
    this->endSession();
}

const std::stringstream* ConSess::errorStream() const {
    SessionContext* sess_ctx_p;
    if(!ConSessManager::get_con_sess_ctx(this->con, sess_ctx_p))
        return nullptr;
    return sess_ctx_p->err_str.get();
}

const std::stringstream* ConSess::messageStream() const {
    SessionContext* sess_ctx_p;
    if(!ConSessManager::get_con_sess_ctx(this->con, sess_ctx_p))
        return nullptr;
    return sess_ctx_p->msg_str.get();
}

/**
 * Start the session.
 */
void ConSess::startSession(std::function<void(SessionStatus)> status_cb){
    TP::add_job(
        [&con = this->con, status_cb](){
            SessionStatus s = ConSessManager::session_dispatcher(con);
            if(status_cb)
                status_cb(s);
        }
    );
}

void ConSess::endSession(){
    // TODO:
    // IF OWNS SESSION:
    //      kill session
}

/**
 * TODO: Ping the controller.
SessionStatus ConSess::checkConnectionStatus(StatusDelay status_delay){
    // TODO: assign a new future to last_status.
    this->last_future_status = this->getDummyStatus(NO_RESPONSE);
    return this->getStatus(status_delay);
}
*/

#define CON_JOB(...) \
            [__VA_ARGS__](const Con& con, SessionContext& sc)
#define CON_JOB_VARS \
            CT ct{sc.con_handle, sc.timming_byte};\
            std::stringstream& msg_str = *sc.msg_str;\
            std::stringstream& err_str = *sc.err_str;

void ConSess::testSetLedBusy(){
    ConSessManager::add_job(this->con,
        CON_JOB(){
            CON_JOB_VARS
            msg_str << "set_led_busy" << std::endl;
            
            set_led_busy(ct, con.prod_id); // Always returns 0, so no error checking.
            sc.status = SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        }
    );
}

void ConSess::testIRCapture(IRSensor& ir){
    ConSessManager::add_job(this->con,
        CON_JOB(&) {
            CON_JOB_VARS
            msg_str << "IRSensor::capture" << std::endl;
            ir.capture(ct);
            sc.status = SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        }
    );
}

void ConSess::testHDRumble(RumbleData& rumble_data, bool& is_active){
    is_active = true;
    ConSessManager::add_job(this->con,
        CON_JOB(&rumble_data, &is_active) {
            CON_JOB_VARS
            msg_str << " play_hd_rumble_file" << std::endl;
            int res = play_hd_rumble_file(ct, rumble_data);
            is_active = false;
            sc.status = SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        }
    );
}

/**
 * Fills in the temperature data asynchronously.
 */
void ConSess::getTemperature(TemperatureData& fill_temp_data){
    ConSessManager::add_job(this->con,
        CON_JOB(&fill_temp_data) {
            CON_JOB_VARS
            msg_str << "get_temperature, parseTemperatureData" << std::endl;
            unsigned char temperature_data[2] = {};
            get_temperature(ct, temperature_data);
            fill_temp_data = parseTemperatureData(temperature_data);
            sc.status = SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        }
    );
}

void ConSess::getBattery(BatteryData& fill_batt_data){
    ConSessManager::add_job(this->con,
        CON_JOB(&fill_batt_data) {
            CON_JOB_VARS
            msg_str << "get_battery, parseBatteryData" << std::endl;
            unsigned char battery_data[3] = {};
            get_battery(ct, battery_data);
            fill_batt_data = parseBatteryData(battery_data);
            sc.status = SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        }
    );
}

void ConSess::getColors(SPIColors& fill_spi_colors){
    ConSessManager::add_job(this->con,
        CON_JOB(&fill_spi_colors) {
            CON_JOB_VARS
            msg_str << "get_spi_colors" << std::endl;
            fill_spi_colors = get_spi_colors(ct);
            sc.status = SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        }
    );
}

void ConSess::dumpSPI(const std::string& to_file, bool& is_dumping, size_t& bytes_dumped, bool& cancel_spi_dump){
    is_dumping = true;
    cancel_spi_dump = false;
    ConSessManager::add_job(this->con,
        CON_JOB(spi_dump_file = to_file, &is_dumping, &bytes_dumped, &cancel_spi_dump) {
            CON_JOB_VARS
            msg_str << "dump_spi" << std::endl;
            DumpSPICTX ctx{
                cancel_spi_dump,
                bytes_dumped,
                spi_dump_file.c_str()
            };
            bytes_dumped = 0;
            int res = dump_spi(ct, ctx);
            if(res)
                msg_str << "There was a problem backing up the SPI. Try again?" << std::endl;

            is_dumping = false;      
            sc.status = SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        }
    );
}

void ConSess::writeColorsToSPI(const SPIColors& colors){
    ConSessManager::add_job(this->con,
        CON_JOB(colors = colors){
            CON_JOB_VARS
            msg_str << "write_spi_colors" << std::endl;
            int res = write_spi_colors(ct, colors);
            if(res)
                msg_str << "There was a problem writing the colors. Try again?" << std::endl;
            sc.status = SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        }
    );
}

void ConSess::testSendRumble(){
    ConSessManager::add_job(this->con,
        CON_JOB(){
            CON_JOB_VARS
            msg_str << "send_rumble" << std::endl;
            send_rumble(ct, con.prod_id);
            sc.status = SESS_OK; // TODO: Use the return value of the main job function to provide session status.
        }
    );
}
