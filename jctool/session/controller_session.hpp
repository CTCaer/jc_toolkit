/**
 * Provides an easy way to interact with your controller through a connection session.
 * See controller_session.cpp for License information.
 */
#pragma once
#include <future>
#include <sstream>
#include "jctool_types.h"

#include "IRSensor.hpp"


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
    friend struct ConEqual;
    friend struct ConHash;
public:
    enum class StatusDelay {
        None,
        DelayedManual,
        //DelayedAsync // TODO: Will spawn an asynchronous thread.
    };
    enum class LastStatusFrom {
        LastFuture,
        LastStatusGet,
    };

    enum Status {
        //DELAYED_STATUS_ASYNC_ERR // TODO: Something went wrong getting the status asynchronously
        NO_RESPONSE = -3,
        SESS_EXISTS_ERR = -2,
        SESS_ERR = -1,
        NO_SESS,
        SESS_OK,
        DELAYED_STATUS_MANUAL, // Will need to call getLastStatus() manually!
        //DELAYED_STATUS_ASYNC, // TODO: Do not need to call getLastStatus(), it is begin done asynchronously.
    };

    inline ConSess():
    last_future_status{this->getDummyStatus(NO_SESS)},
    con{new Con},
    err_str{new std::stringstream},
    msg_str{new std::stringstream}
    {}

    ConSess(const Con& con, bool start_session = false, StatusDelay status_delay = StatusDelay::None);
    ConSess(ConSess&& sess_move);
    ~ConSess();

    /**
     * Return the last known status.
     * Getting the value from the future may block if the status was not yet updated.
     */
    inline Status getLastStatus(LastStatusFrom from = LastStatusFrom::LastFuture) {
        if ((from == LastStatusFrom::LastFuture) && this->last_future_status.valid())
            this->last_status_get = this->last_future_status.get();
        return this->last_status_get;
    }
    inline const std::stringstream& errorStream() const { return *this->err_str; }
    inline const std::stringstream& messageStream() const { return *this->msg_str; }

    inline ConHID::ProdID getProdID() { return this->con->prod_id; }
    inline const std::string& getHIDSN() const { return this->con->hid_sn; }
    inline const std::string& getManuStr() const { return this->con->manu_string; }
    inline const std::string& getProdStr() const { return this->con->prod_string; }
    inline const std::string& getDevPath() const { return this->con->dev_path; }

    Status startSession(StatusDelay status_delay = StatusDelay::None);
    Status endSession(StatusDelay status_delay = StatusDelay::None);
    Status checkConnectionStatus(StatusDelay status_delay = StatusDelay::None);
    
    void testSetLedBusy();
    void testIRCapture(IRSensor& ir);
    void testHDRumble(RumbleData& rumble_data, bool& is_active);
    void testSendRumble();

    void getTemperature(TemperatureData& fill_temp_data);
    void getBattery(BatteryData& fill_batt_data);
    void getColors(SPIColors& fill_spi_colors);
    void dumpSPI(const std::string& to_file, bool& is_dumping, size_t& bytes_dumped, bool& cancel_spi_dump);

    void writeColorsToSPI(const SPIColors& colors);
private:
    std::promise<Status> status_promise;
    std::future<Status> last_future_status;
    Status last_status_get; // Last status got from calling last_future_status.get()

    std::unique_ptr<std::stringstream> err_str;
    std::unique_ptr<std::stringstream> msg_str;
    
    std::unique_ptr<Con> con;

    inline static std::future<Status> getDummyStatus(Status dummy_status){
        std::promise<Status> p;
        p.set_value(NO_SESS);
        return p.get_future();
    }
    inline Status getStatus(StatusDelay status_delay){
        if(status_delay == StatusDelay::None)
            this->last_status_get = this->last_future_status.get();
        else
            this->last_status_get = DELAYED_STATUS_MANUAL;
        return this->last_status_get;
    }
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
