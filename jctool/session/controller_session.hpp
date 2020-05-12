/**
 * Provides an easy way to interact with your controller through a connection session.
 * See controller_session.cpp for License information.
 */
#pragma once
#include <functional>
#include <sstream>
#include "jctool_types.h"

#include "IRSensor.hpp"

enum SessionStatus {
    NO_RESPONSE = -2,
    SESS_ERR = -1,
    NO_SESS,
    SESS_OK
};

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

namespace ConSessManager{
    SessionStatus session_dispatcher(const Con& con);
}

/**
 * Provides a context for a controller session.
 */
class SessionContext {
private:
    controller_hid_handle_t con_handle;
    std::unique_ptr<std::mutex> ctx_mutex_p;
    u8 timming_byte;

    std::unique_ptr<std::mutex> log_mutex_p;
    std::unique_ptr<std::stringstream> err_log;
    std::unique_ptr<std::stringstream> msg_log;
public:
    friend SessionStatus ConSessManager::session_dispatcher(const Con&);
    SessionStatus status;

    SessionContext();
    
    SessionContext(SessionContext&& move_sess);

    inline const controller_hid_handle_t& getConHandle() const { return this->con_handle; }
    inline CT getCT() {
        return {this->con_handle, this->timming_byte};
    }

    inline std::mutex& ctx_mutex(){
        return *this->ctx_mutex_p;
    }

    inline void logMsg(const std::string& message){
        std::lock_guard lock{*this->log_mutex_p};
        *msg_log << message << std::endl;
    }

    inline void logErr(const std::string& message){
        std::lock_guard lock{*this->log_mutex_p};
        *err_log << message << std::endl;
    }

    inline const std::stringstream& getMsgLog() const {
        return *this->msg_log;
    }

    inline const std::stringstream& getErrLog() const {
        return *this->err_log;
    }
};

class ConSess {
    friend struct ConEqual;
    friend struct ConHash;
public:

    inline ConSess():
    con{}
    {}

    ConSess(const Con& con, bool start_session = false, std::function<void(SessionStatus)> = nullptr);
    ConSess(ConSess&& sess_move);
    ~ConSess();

    const std::stringstream* errorStream() const;
    const std::stringstream* messageStream() const;

    inline const Con& getCon() { return this->con; }

    void startSession(std::function<void(SessionStatus)> status_cb);
    void endSession();
    //SessionStatus checkConnectionStatus();
    
    void testSetLedBusy();
    void testIRCapture(IRSensor& ir);
    void testHDRumble(RumbleData& rumble_data, bool& is_active);
    void testSendRumble();

    void fetchTemperature(std::function<void(const TemperatureData&)> got_temp_data_cb);
    void fetchBattery(std::function<void(const BatteryData&)> got_batt_data_cb);
    void fetchColors(std::function<void(const SPIColors&)> got_colors_cb);
    void dumpSPI(const std::string& to_file, bool& is_dumping, size_t& bytes_dumped, bool& cancel_spi_dump);

    void writeColorsToSPI(const SPIColors& colors);
private:
    Con con;
};

struct ConHash {
    size_t operator()(const Con& c) const noexcept {
        return  std::hash<std::string>{}(c.dev_path);
    }
};

struct ConEqual {
    bool operator()(const Con& l, const Con& r) const noexcept {
        return l.dev_path == r.dev_path;
    }
};
