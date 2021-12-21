#pragma once

#include <stddef.h>

#include "esp_camera.h"

/// Return true if changes are found
bool motion_detect(bool force, const camera_fb_t* fb, const struct tm* cur_tm);
