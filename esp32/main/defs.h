#pragma once

#include "esp_camera.h"

constexpr const char* VERSION = "0.6";

constexpr const char* WIFI_KEY = "wifi";
constexpr const char* S3_ACCESS_KEY = "s3a";
constexpr const char* S3_SECRET_KEY = "s3s";
constexpr const char* INSTANCE_KEY = "inst";
constexpr const char* GATEWAY_TOKEN_KEY = "gwt";

constexpr const int HEARTBEAT_RATE_SECS = 10*60;

constexpr const auto MODE_HIGH_PIN = (gpio_num_t) 14;
constexpr const auto MODE_SELECT_PIN = (gpio_num_t) 15;

extern char config_s3_access_key[];
extern char config_s3_secret_key[];
extern char config_gateway_token[];
extern int8_t config_instance_number;

constexpr const char* TAG = "HAL32CAM";

/// Frame size defs
constexpr const framesize_t FRAMESIZE = FRAMESIZE_UXGA;
constexpr const int FRAMESIZE_X = 1600;
constexpr const int FRAMESIZE_Y = 1200;

/// Minimum change in greyscale value (0-255) for pixel to be considered changed
constexpr const int PIXEL_THRESHOLD = 10;

/// Minimum percent of changed pixels for motion detection
constexpr const int PERCENT_THRESHOLD = 2;
