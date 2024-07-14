#include "connect.h"
#include "console.h"
#include "defs.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp32/rom/ets_sys.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

extern void camera_task(void*);

void initialize_sntp()
{
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    esp_sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    esp_sntp_init();
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
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    printf("Restart in 5 seconds\n");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
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
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    esp_restart();
}

char config_s3_access_key[40];
char config_s3_secret_key[40];
char config_gateway_token[80];
int8_t config_instance_number = 0;
int config_keepalive_secs = DEFAULT_KEEPALIVE_SECS;
int config_pixel_threshold = DEFAULT_PIXEL_THRESHOLD;
int config_percent_threshold = DEFAULT_PERCENT_THRESHOLD;
bool config_active = true;
bool config_continuous = false;

void flash_led(int n)
{
    for (int i = 0; i < n; ++i)
    {
        gpio_set_level((gpio_num_t) 4, true);
        ets_delay_us(10);
        gpio_set_level((gpio_num_t) 4, false);
        if (i < n-1)
            vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

std::vector<std::pair<std::string, std::string>> get_wifi_credentials(char* buf)
{
    std::vector<std::pair<std::string, std::string>> v;
    bool is_ssid = true;
    std::string ssid;
    char* p = buf;
    while (1)
    {
        char* token = strsep(&p, ":");
        if (!token)
            break;
        if (is_ssid)
            ssid = std::string(token);
        else
            v.push_back(std::make_pair(ssid, std::string(token)));
        is_ssid = !is_ssid;
    }
    return v;
}

extern "C"
void app_main()
{
    // Configure flash control pin GPIO4
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << 4) | (1ULL << LED_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    flash_led(1);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    printf("HAL32CAM v %s\n", VERSION);
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

    flash_led(2);

    nvs_handle my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));
    char buf[256];
    get_nvs_string(my_handle, WIFI_KEY, buf, sizeof(buf));
    const auto creds = get_wifi_credentials(buf);
    get_nvs_string(my_handle, S3_ACCESS_KEY, config_s3_access_key, sizeof(config_s3_access_key));
    get_nvs_string(my_handle, S3_SECRET_KEY, config_s3_secret_key, sizeof(config_s3_secret_key));
    get_nvs_string(my_handle, GATEWAY_TOKEN_KEY, config_gateway_token, sizeof(config_gateway_token));
    get_nvs_i8(my_handle, INSTANCE_KEY, config_instance_number);
    nvs_close(my_handle);
    printf("Instance %d\n", config_instance_number);

    // Connect to WiFi
    ESP_ERROR_CHECK(connect(creds));
    ESP_LOGI(TAG, "Connected to WiFi. Instance #%d", (int) config_instance_number);
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    flash_led(3);

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
}
