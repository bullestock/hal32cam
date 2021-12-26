#pragma once

#include <stddef.h>

#include "esp_camera.h"

void upload(const camera_fb_t* fb,
            const struct tm& current);

void heartbeat(const struct tm& current);
