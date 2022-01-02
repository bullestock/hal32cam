#include "defs.h"
#include "heartbeat.h"
#include "motion.h"
#include "upload.h"

#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ESP32Cam (AiThinker) PIN Map
#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1 //software reset will be performed
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sscb_sda = CAM_PIN_SIOD,
    .pin_sscb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 10000000, //20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE,

    .jpeg_quality = 12, //0-63 lower number means higher quality
    .fb_count = 1,       //if more than one, i2s runs in continuous mode. Use only with JPEG
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

static esp_err_t init_camera()
{
    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    return ESP_OK;
}

void camera_task(void*)
{
    if (init_camera() != ESP_OK)
        return;

    time_t last_heartbeat = 0;
    time_t last_pic = 0;
    
    while (1)
    {
        vTaskDelay(100 / portTICK_RATE_MS);

        // Get current time
        time_t current = 0;
        time(&current);
        struct tm timeinfo;
        gmtime_r(&current, &timeinfo);

        if (current - last_heartbeat > config_keepalive_secs)
        {
            heartbeat(timeinfo, last_pic);
            last_heartbeat = current;
        }

        if (config_active)
        {
            char ts[20];
            strftime(ts, sizeof(ts), "%Y%m%d%H%M%S", &timeinfo);
            printf("Taking picture: %s...", ts);
#if USE_FLASH
            gpio_set_level((gpio_num_t) 4, true);
            vTaskDelay(100 / portTICK_RATE_MS);
#endif
            auto pic = esp_camera_fb_get();
            gpio_set_level((gpio_num_t) 4, false);
            if (!pic)
            {
                ESP_LOGE(TAG, "No picture taken!");
                continue;
            }
            printf("size: %zu...", pic->len);

            if (motion_detect(pic, timeinfo))
                last_pic = current;
            
            // Release buffer
            esp_camera_fb_return(pic);
        }
    }
}
