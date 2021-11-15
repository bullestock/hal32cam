#pragma once

#include <stddef.h>

#include "esp_camera.h"

/// Return true if changes are found
bool motion_detect(const camera_fb_t* fb);
