#pragma once
#include "jctool_types.h"

namespace Input {
    void decode_stick_params(u16 *decoded_stick_params, u8 *encoded_stick_params);
    void encode_stick_params(u8 *encoded_stick_params, u16 *decoded_stick_params);
    int button_test(CT& ct);
}
