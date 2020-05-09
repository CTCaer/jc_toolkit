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

    inline ConHID::ProdID getProdID() { return this->con.prod_id; }
    inline const std::string& getHIDSN() const { return this->con.hid_sn; }
    inline const std::string& getManuStr() const { return this->con.manu_string; }
    inline const std::string& getProdStr() const { return this->con.prod_string; }
    inline const std::string& getDevPath() const { return this->con.dev_path; }

    void startSession(std::function<void(SessionStatus)> status_cb);
    void endSession();
    //SessionStatus checkConnectionStatus();
    
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
