#pragma once
#include <functional>
#include "jctool_types.h"

namespace IR {
    using StoreRawCaptureCB = std::function<void(const u8*, size_t)>;
    int ir_sensor(IRCaptureCTX& capture_context, StoreRawCaptureCB store_capture_cb);
    int ir_sensor_config_live(CT& ct, ir_image_config& ir_cfg);
}