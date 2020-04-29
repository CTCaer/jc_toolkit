/**
 * Provides an easy way to interact with your controller through a connection session.
 * See controller_session.cpp for License information.
 */
#pragma once
#include <future>
#include <sstream>
#include "jctool_types.h"

struct Con {
    // Information provided by hid_device_info
    ConHID::ProdID prod_id;
    std::string hid_sn;
    std::string manu_string;
    std::string prod_string;
    std::string dev_path;
    inline Con():
    prod_id(ConHID::NoCon)
    {}
    Con(const hid_device_info* dev);
};

class ConSess {
public:
    using StatusDelay = bool;
    static const StatusDelay STATUS_NOW = true;
    static const StatusDelay STATUS_DELAYED = false;

    enum Status {
        NO_RESPONSE = -3,
        SESS_EXISTS_ERR = -2,
        SESS_ERR = -1,
        NO_SESS,
        SESS_OK,
        DELAYED_STATUS, // Call getLastStatus()
    };

    inline ConSess():
    last_status{this->getDummyStatus(NO_SESS)}
    {}

    ConSess(const Con& con, bool start_session = false);
    ConSess(ConSess&& sess_move);
    ~ConSess();

    /**
     * Return the last known status.
     * Getting the value from the future may block if the status was not yet updated.
     */
    inline Status getLastStatus() {
        if (this->last_status.valid())
            this->last_got_status = this->last_status.get();
        return this->last_got_status;
    }
    inline const std::stringstream& errorStream() const { return this->err_str; }
    inline const std::stringstream& messageStream() const { return this->msg_str; }

    inline ConHID::ProdID getProdID() { return this->con.prod_id; }
    inline const std::string& getHIDSN() { return this->con.hid_sn; }
    inline const std::string& getManuStr() { return this->con.manu_string; }
    inline const std::string& getProdStr() { return this->con.prod_string; }
    inline const std::string& getDevPath() { return this->con.dev_path; }

    Status startSession(StatusDelay status_delay = STATUS_NOW);
    Status endSession(StatusDelay status_delay = STATUS_NOW);
    Status getConnectionStatus(StatusDelay status_delay = STATUS_NOW);
    
    void testSetLedBusy();
private:
    std::future<Status> last_status;
    Status last_got_status;

    std::stringstream err_str;
    std::stringstream msg_str;

    Con con;

    inline std::future<Status> getDummyStatus(Status dummy_status){
        std::promise<Status> p;
        p.set_value(NO_SESS);
        return p.get_future();
    }
    inline Status getStatus(StatusDelay status_delay){
        if(status_delay == STATUS_NOW)
            this->last_got_status = this->last_status.get();
        return this->last_got_status;
    }
};
 

 namespace ConSessManager {
    void start_session_dispatcher(std::stringstream& session_dispatch_out_str);
 }
