#include "connect.h"
#include "console.h"
#include "defs.h"
#include "mqtt.h"
#include "nvs.h"
#include "otafwu.h"
#include "sntp.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp32/rom/ets_sys.h"
#include "esp_app_desc.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_system.h"
#include "esp_wifi.h"

extern void camera_task(void*);

static constexpr const char* TAG = "main";

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

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << EXT1_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    
    flash_led(1);

    init_nvs();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    bool debug = false;
    if (get_wifi_creds().empty())
        debug = true;

    printf("HAL32CAM v %s instance %d\n", esp_app_get_description()->version,
           (int) get_instance());
    printf("Press a key to enter console\n");
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

    // Connect to WiFi
    const auto creds = get_wifi_creds();
    if (connect(creds))
    {
        ESP_LOGI(TAG, "Connected to WiFi. Instance #%d", (int) get_instance());
        ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

        flash_led(3);

        initialize_sntp();
        bool do_ota_check = gpio_get_level(EXT1_PIN);
        if (!do_ota_check)
            ESP_LOGI(TAG, "OTA disabled");
        else
            if (!check_ota_update())
                ESP_LOGE(TAG, "OTA failed!");
    }
    xTaskCreate(&camera_task, "camera_task", 32768, nullptr, 5, nullptr);
    Mqtt::instance().start(get_mqtt_address());
}

// Local Variables:
// compile-command: "(cd ..; idf.py build)"
// End:
