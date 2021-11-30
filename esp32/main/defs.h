#pragma once

#include "esp_camera.h"

constexpr const char* VERSION = "0.2";

constexpr const char* WIFI_SSID_KEY = "ssid";
constexpr const char* WIFI_PASSWORD_KEY = "pass";
constexpr const char* S3_ACCESS_KEY = "s3a";
constexpr const char* S3_SECRET_KEY = "s3s";
constexpr const char* INSTANCE_KEY = "inst";

constexpr const int FLASH_ON_TIME_SECS = 60;

extern char config_wifi_ssid[];
extern char config_wifi_password[];
extern char config_s3_access_key[];
extern char config_s3_secret_key[];
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
