#pragma once
#include <string>
#include <mutex>
#include <tuple>

#include "jctool_types.h"

class IRSensor {
public:
    ir_image_config config{};


    bool capture_in_progress{};
    IRCaptureMode capture_mode{};
    IRCaptureStatus capture_status;

    int res_idx_selected{}; /** The index number of the resolution selected.
    * See ir_sensor.h.
    */
    IRColor colorize_with{};
    bool auto_exposure{};

    void capture(CT& ct);
    uintptr_t getCaptureTexID();

    IRSensor();
private:
    u8 ir_max_frag_no{};
    struct VideoStreamFrameData {
        uintptr_t textures[3]{};
        int idx_render;
        int idx_swap;
        int idx_display;
        bool updated{};
        std::mutex texture_mutex;
        
        inline VideoStreamFrameData():
        idx_render{0},
        idx_swap{1},
        idx_display{2}
        {}
    } vstream_frame_dat;
};
