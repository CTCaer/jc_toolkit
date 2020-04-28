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
#include <condition_variable>
#include <queue>
#include <string>
#include <codecvt>
#include <locale>
#include <thread>

#include "controller_session.hpp"

#include "jctool.h"

namespace ConSessManager {
    using SessionJobWrite = std::function<void(u8&)>;
    using SessionJobRead = std::function<int(void)>;
    using SessionJob = std::pair<SessionJobWrite, SessionJobRead>;
    using UpdateStatusCB = std::function<void(ConSess::Status)>;
    using GetStringStream = std::function<std::stringstream&(void)>;

    /**
     * Provides a context for an active controller session.
     */
    struct SessionContext {
        std::thread response_thread;

        std::queue<SessionJob> jobs;

        std::mutex jobs_mutex;
        std::condition_variable job_avail;

        u8 timming_byte;
    };

    struct SessionLink {
        controller_hid_handle_t handle;
        UpdateStatusCB update_status_cb;
        //GetStringStream msg_str;
        //GetStringStream err_str;
    };

    namespace {
        using SessionPair = std::pair<controller_hid_handle_t, SessionContext&>;
        static std::mutex cons_with_sessions_mutex;
        static std::map<controller_hid_handle_t, SessionContext&> cons_with_sessions;

        /**
         * Holds the session in a loop as it awaits any [expected] responses from the controller.
         */
        static int session_wait_on_rsp_loop(SessionLink& sess){

            sess.update_status_cb(ConSess::Status::SESS_OK); // TODO: do this better (perhaps receive an OK response from the controller?)
            //std::unique_lock<std::mutex> lock(sp.second.jobs_mutex);
            while(true){
                // hid_read_timeout

                // Parse response.
                
                // Send response to response queue.
            }

            return 0;
        }

        /**
         * Holds the session in a loop as it awaits any messages from session holder.
         */
        static int session_wait_on_jobs_loop(SessionLink sess){
            SessionContext sc;
            
            cons_with_sessions_mutex.lock();
            cons_with_sessions.insert({sess.handle, sc});
            cons_with_sessions_mutex.unlock();
            sc.response_thread = std::thread{
                session_wait_on_rsp_loop,
                std::ref(sess)
            };
            std::unique_lock<std::mutex> lock(sc.jobs_mutex);
            while(true){
                sc.job_avail.wait(lock, [&](){
                    return sc.jobs.size() > 0;
                });

                sc.jobs.front().first(sc.timming_byte);

                // Job done.
                sc.jobs.pop();
            }

            sc.response_thread.join();
            return 0;
        }

        static std::condition_variable start_session_avail;
        static std::mutex start_session_mutex;
        static std::queue<SessionLink> start_session_q;
        static std::thread session_dispatcher_thread;

        static void session_dispatcher(std::stringstream& output){
            output << "Session dispatcher started." << std::endl;
            std::unique_lock<std::mutex> lock(start_session_mutex);
            while(true){
                start_session_avail.wait(lock, [](){ return start_session_q.size() > 0; });
                auto new_sess = start_session_q.front();
                std::thread sess = std::thread(
                    session_wait_on_jobs_loop,
                    new_sess
                );
                sess.detach(); // TODO: keep track of instead of detaching.

                start_session_q.pop();

                output << new_sess.handle << " new session" << std::endl;
            }
            output << "Session dispatcher ended." << std::endl;
        }
    }

    void start_session_dispatcher(std::stringstream& session_dispatch_out_str){
        if(session_dispatcher_thread.joinable())
            return; // Already started.
        
        session_dispatcher_thread = std::thread(
            session_dispatcher,
            std::ref(session_dispatch_out_str)
        );
    }


    /**
     * Place it on a queue for the session dispatcher.
     */
    static void enqueue_session(controller_hid_handle_t handle, ConSessManager::UpdateStatusCB update_status){
        std::lock_guard<std::mutex> lock(start_session_mutex);
        start_session_q.push({handle, update_status});
        start_session_avail.notify_one();
    }

    /**
     * Return the end result of the session.
     */
    static ConSess::Status kill_session(controller_hid_handle_t handle){
        // Send a NULL command 0x00
        //if(no_response)
        //  return Session::NO_RESPONSE;
        return ConSess::NO_SESS;
    }

    static void add_job(controller_hid_handle_t handle, SessionJob job){
        std::lock_guard<std::mutex> lock_conswsess(cons_with_sessions_mutex);
        auto find_res = cons_with_sessions.find(handle);
        if(find_res == cons_with_sessions.end())
            return; // Nothing to do.
        
        // Add the job.
        SessionContext& ctx = find_res->second;
        std::lock_guard<std::mutex> lock(ctx.jobs_mutex);
        ctx.jobs.push(job);
        ctx.job_avail.notify_one();
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
 * Populate the controller,[TODO: and optionally start the session (See the move constructor as to why this is not impl as of now.)]
 */
ConSess::ConSess(const Con& con)://, bool start_session) :
ConSess()
{
    this->con = con;
}

/**
 * TODO: Prevent a move if the session was started, or create a workaround.
 *       The consensus for this is a reference to the status variable is passed to the session thread, and gets "lost"
 */
ConSess::ConSess(ConSess&& sess_move) :
handle(sess_move.handle),
status(sess_move.status),
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
void ConSess::startSession(){
    // TODO: start the session with the device path instead of the handle, and try to connect to the device using the session thread.
    this->handle = hid_open_path(this->con.dev_path.c_str());
    if(!this->handle)
        return; // Do nothing.
    this->status = WAITING_SESS;
    ConSessManager::enqueue_session(this->handle, [this](Status status_update){
        this->status = status_update;
    });
}

ConSess::Status ConSess::endSession(){
    switch(status = this->checkConnection()){ // Blocking call?
        case SESS_OK:
            switch(status = ConSessManager::kill_session(this->handle)){
                case NO_SESS:
                break;
                default:
                this->err_str << "Unhandled session_kill case." << std::endl;
                break;
            }
        break;
        default:
        this->err_str << "Unhandled checkConnection case." << std::endl;
        break;
    }

    return this->status;
}

/**
 * TODO: Provide a blocking, and a non-blocking call.
 * This is the blocking call.
 */
ConSess::Status ConSess::checkConnection(){
    // TODO: 
    return SESS_ERR; // Last resort.
}

void ConSess::testSetLedBusy(){
    ConSessManager::add_job(this->handle, {
        [handle = this->handle, con_prod_id = this->con.prod_id, &msg_str = this->msg_str](u8& timming_byte){
            // Convert ConHID::ProdID to Controller::Type for the sake of how set_led_busy works.
            Controller::Type con_type;
            switch(con_prod_id){
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
            
            msg_str << handle << " set_led_busy" << std::endl;
            
            set_led_busy(handle, timming_byte, con_type); // Always returns 0, so no error checking.
        }, // Write job: write and read happens, so no read job.
        nullptr // Read job: no.
    });
}
