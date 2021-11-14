#include "defs.h"

#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "protocol_examples_common.h"

extern void camera_task(void*);

extern "C"
void app_main()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Connect to WiFi
    ESP_ERROR_CHECK(example_connect());
    ESP_LOGI(TAG, "Connected to WiFi. Instance #%d", CONFIG_HAL32CAM_INSTANCE);

    xTaskCreate(&camera_task, "camera_task", 8192, nullptr, 5, nullptr);
}
