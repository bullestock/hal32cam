#include "console.h"
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
#include "esp_sntp.h"

extern void camera_task(void*);

void initialize_sntp()
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    sntp_init();
}

void obtain_time()
{
    initialize_sntp();

    // wait for time to be set
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void watchdog_task(void*)
{
    int n = 0;
    while (1)
    {
        vTaskDelay(10000 / portTICK_RATE_MS);
        printf("WD %d\n", n);
        ++n;
    }
}

// Reboots if key not found
void get_nvs_string(nvs_handle my_handle, const char* key, char* buf, size_t buf_size)
{
    auto err = nvs_get_str(my_handle, key, buf, &buf_size);
    switch (err)
    {
    case ESP_OK:
        return;
    case ESP_ERR_NVS_NOT_FOUND:
        printf("%s: not found\n", key);
        break;
    default:
        printf("%s: NVS error %d\n", key, err);
        break;
    }
    printf("Restart in 10 seconds\n");
    vTaskDelay(10000 / portTICK_RATE_MS);
    esp_restart();
}

// Reboots if key not found
void get_nvs_i8(nvs_handle my_handle, const char* key, int8_t& value)
{
    auto err = nvs_get_i8(my_handle, key, &value);
    switch (err)
    {
    case ESP_OK:
        return;
    case ESP_ERR_NVS_NOT_FOUND:
        printf("%s: not found\n", key);
        break;
    default:
        printf("%s: NVS error %d\n", key, err);
        break;
    }
    printf("Restart in 10 seconds\n");
    vTaskDelay(10000 / portTICK_RATE_MS);
    esp_restart();
}

char config_wifi_ssid[80];
char config_wifi_password[80];
char config_s3_access_key[40];
char config_s3_secret_key[40];
int8_t config_instance_number = 0;

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

    printf("Press a key to enter console\n");
    bool debug = false;
    for (int i = 0; i < 20; ++i)
    {
        if (getchar() != EOF)
        {
            debug = true;
            break;
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    if (debug)
        run_console();        // never returns
    printf("\nStarting application\n");

    nvs_handle my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));
    get_nvs_string(my_handle, WIFI_SSID_KEY, config_wifi_ssid, sizeof(config_wifi_ssid));
    get_nvs_string(my_handle, WIFI_PASSWORD_KEY, config_wifi_password, sizeof(config_wifi_password));
    get_nvs_string(my_handle, S3_ACCESS_KEY, config_s3_access_key, sizeof(config_s3_access_key));
    get_nvs_string(my_handle, S3_SECRET_KEY, config_s3_secret_key, sizeof(config_s3_secret_key));
    get_nvs_i8(my_handle, INSTANCE_KEY, config_instance_number);
    nvs_close(my_handle);

    // Connect to WiFi
    ESP_ERROR_CHECK(example_connect());
    ESP_LOGI(TAG, "Connected to WiFi. Instance #%d", (int) config_instance_number);

    // Get current time
    time_t current = 0;
    time(&current);
    struct tm timeinfo;
    gmtime_r(&current, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900))
    {
        ESP_LOGI(TAG, "Getting time via NTP");
        obtain_time();
    }
    
    xTaskCreate(&camera_task, "camera_task", 32768, nullptr, 5, nullptr);
    xTaskCreate(&watchdog_task, "watchdog_task", 2048, nullptr, 5, nullptr);
}
