#pragma once
#include <string>
#include <mutex>
#include <tuple>

#include "jctool_types.h"

class IRSensor {
public:
    ir_image_config config;


    bool capture_in_progress;
    IRCaptureMode capture_mode;
    IRCaptureStatus capture_status;

    int res_idx_selected; /** The index number of the resolution selected.
    * See ir_sensor.h.
    */
    IRColor colorize_with;
    bool auto_exposure;

    void capture(controller_hid_handle_t host_controller, u8& timming_byte);
    uintptr_t getCaptureTexID();

    IRSensor();
private:
    u8 ir_max_frag_no;
    struct VideoStreamFrameData {
        uintptr_t textures[3] = {};
        int idx_render = 0;
        int idx_swap = 1;
        int idx_display = 2;
        bool updated = false;
        std::mutex texture_mutex;
    } vstream_frame_dat;
};
