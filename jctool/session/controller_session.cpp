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

namespace ConSessManager {
    /**
     * Provides a context for a controller session.
     */
    struct SessionContext {
        controller_hid_handle_t con_handle = nullptr;
        u8 timming_byte = 0;
    };

    struct compCon {
        bool operator()(const Con& l, const Con& r) const {
            return l.dev_path.compare(r.dev_path) < 0;
        }
    };
    namespace {
        static std::mutex cons_with_sessions_mutex;
        static std::map<Con, SessionContext, compCon> cons_with_sessions;

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

    static std::future<ConSess::Status> add_job(const Con& con, std::function<void(controller_hid_handle_t, u8&)> job){
        auto promise = std::shared_ptr<std::promise<ConSess::Status>>(new std::promise<ConSess::Status>());
        auto res = promise->get_future();
        TP::add_job(
            [&, promise, job] (){
                cons_with_sessions_mutex.lock();
                auto find_res = cons_with_sessions.find(con);
                cons_with_sessions_mutex.unlock();
                if(find_res == cons_with_sessions.end())
                    return promise->set_value(ConSess::NO_SESS);

                SessionContext& sc = find_res->second;
                job(sc.con_handle, sc.timming_byte);
                // TODO: Get connection status
                return promise->set_value(ConSess::SESS_OK);
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

void ConSess::testSetLedBusy(){
    ConSessManager::add_job(*this->con,
        [this](controller_hid_handle_t handle, u8& timming_byte){
            // Convert ConHID::ProdID to Controller::Type for the sake of how set_led_busy works.
            Controller::Type con_type;
            switch(this->con->prod_id){
                case ConHID::JoyConLeft:
                    con_type = Controller::Type::JoyConLeft;
                break;
                case ConHID::JoyConRight:
                    con_type = Controller::Type::JoyConRight;
                break;
                case ConHID::ProCon:
                    con_type = Controller::Type::ProCon;
                break;
                case ConHID::JoyConGrip:
                default:
                    con_type = Controller::Type::Undefined;
            }
            
            *msg_str << this->con->hid_sn << " set_led_busy" << std::endl;
            
            set_led_busy(handle, timming_byte, con_type); // Always returns 0, so no error checking.
        }
    );
}
