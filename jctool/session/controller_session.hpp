/**
 * Provides an easy way to interact with your controller through a connection session.
 * See controller_session.cpp for License information.
 */
#pragma once
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

    enum Status {
        NO_RESPONSE = -3,
        SESS_EXISTS_ERR = -2,
        SESS_ERR = -1,
        NO_SESS,
        WAITING_SESS,
        SESS_OK
    };

    inline ConSess() :
    handle(nullptr),
    status(NO_SESS)
    {}

    ConSess(const Con& con); //, bool start_session = false);
    ConSess(ConSess&& sess_move);
    ~ConSess();

    /**
     * Return the last known status.
     * This should not be relied on to provide an accurate status of the controller.
     * If the controller spontaneously disconnects there is no way of getting a
     * disconnected status without explicitly calling checkConnection().
     */
    inline controller_hid_handle_t getHandle() { return this->handle; }
    inline Status getLastStatus() { return this->status; }
    inline const std::stringstream& errorStream() const { return this->err_str; }
    inline const std::stringstream& messageStream() const { return this->msg_str; }

    inline ConHID::ProdID getProdID() { return this->con.prod_id; }
    inline const std::string& getHIDSN() { return this->con.hid_sn; }
    inline const std::string& getManuStr() { return this->con.manu_string; }
    inline const std::string& getProdStr() { return this->con.prod_string; }
    inline const std::string& getDevPath() { return this->con.dev_path; }

    void startSession();
    Status endSession();
    Status checkConnection();
    
    void testSetLedBusy();
private:
    controller_hid_handle_t handle; // TODO: let only the session thread know about the handle
    Status status;

    std::stringstream err_str;
    std::stringstream msg_str;

    Con con;
};
 

 namespace ConSessManager {
    void start_session_dispatcher(std::stringstream& session_dispatch_out_str);
 }
