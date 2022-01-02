#include "defs.h"
#include "eventhandler.h"
#include "heartbeat.h"

#include <string>

#include "cJSON.h"

#include "esp_log.h"
#include "esp_http_client.h"

extern const char howsmyssl_com_root_cert_pem_start[] asm("_binary_howsmyssl_com_root_cert_pem_start");
extern const char howsmyssl_com_root_cert_pem_end[]   asm("_binary_howsmyssl_com_root_cert_pem_end");

void heartbeat(const struct tm& current,
               time_t last_pic)
{
    struct tm timeinfo;
    gmtime_r(&last_pic, &timeinfo);
    char ts[20];
    strftime(ts, sizeof(ts), "%Y%m%d%H%M%S", &timeinfo);
    char resource[80];
    sprintf(resource, "/camera/%d?active=%d&continuous=%d&version=%s&last_pic=%s",
            (int) config_instance_number,
            (int) config_active,
            (int) config_continuous,
            VERSION,
            ts);
    char buffer[256];
    esp_http_client_config_t config {
        .host = "acsgateway.hal9k.dk",
        .path = resource,
        .cert_pem = howsmyssl_com_root_cert_pem_start,
        .event_handler = http_event_handler,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .user_data = buffer
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_GET);

    char bearer[80];
    snprintf(bearer, sizeof(bearer), "Bearer %s", config_gateway_token);
    esp_http_client_set_header(client, "Authentication", bearer);
    const char* content_type = "application/json";
    esp_http_client_set_header(client, "Content-Type", content_type);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Heartbeat status = %d", esp_http_client_get_status_code(client));
        auto root = cJSON_Parse(buffer);
        if (root)
        {
            auto keepalive_node = cJSON_GetObjectItem(root, "keepalive");
            if (keepalive_node)
            {
                auto keepalive = keepalive_node->valueint;
                if (keepalive != config_keepalive_secs)
                {
                    printf("New keepalive %d\n", keepalive);
                    config_keepalive_secs = keepalive;
                }
            }
            auto percent_node = cJSON_GetObjectItem(root, "percent");
            if (percent_node)
            {
                auto percent = percent_node->valueint;
                if (percent != config_percent_threshold)
                {
                    printf("New percent threshold %d\n", percent);
                    config_percent_threshold = percent;
                }
            }
            auto pixel_node = cJSON_GetObjectItem(root, "pixel");
            if (pixel_node)
            {
                auto pixel = pixel_node->valueint;
                if (pixel != config_pixel_threshold)
                {
                    printf("New pixel threshold %d\n", pixel);
                    config_pixel_threshold = pixel;
                }
            }
            auto action_node = cJSON_GetObjectItem(root, "action");
            if (action_node && action_node->type == cJSON_String)
            {
                const std::string action = action_node->valuestring;
                if (action == "on" && !config_active)
                {
                    printf("Going active\n");
                    config_active = true;
                }
                else if (action == "off" && config_active)
                {
                    printf("Going inactive\n");
                    config_active = false;
                }
                else if (action == "continuous" && !config_continuous)
                {
                    printf("Switching to continuous mode\n");
                    config_continuous = true;
                }
                else if (action == "motion" && config_continuous)
                {
                    printf("Switching to motion detect mode\n");
                    config_continuous = false;
                }
            }
            cJSON_Delete(root);
        }
    }
    else
        ESP_LOGE(TAG, "Error performing http request %s", esp_err_to_name(err));
    
    esp_http_client_cleanup(client);
}
