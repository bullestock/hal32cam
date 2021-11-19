#pragma once

#include "esp_camera.h"

constexpr const char* TAG = "HAL32CAM";

/// Frame size defs
constexpr const framesize_t FRAMESIZE = FRAMESIZE_UXGA;
constexpr const int FRAMESIZE_X = 1600;
constexpr const int FRAMESIZE_Y = 1200;

/// Minimum change in greyscale value (0-255) for pixel to be considered changed
constexpr const int PIXEL_THRESHOLD = 25;

/// Minimum percent of changed pixels for motion detection
constexpr const int PERCENT_THRESHOLD = 10;
