#include "defs.h"
#include "upload.h"

#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"

#include "cJSON.h"

#include "mbedtls/base64.h"
#include "mbedtls/md.h"

#include "esp_http_client.h"

/* Root cert for howsmyssl.com, taken from howsmyssl_com_root_cert.pem

   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.howsmyssl.com:443 </dev/null

   The CA root cert is the last cert given in the chain of certs.

   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
extern const char howsmyssl_com_root_cert_pem_start[] asm("_binary_howsmyssl_com_root_cert_pem_start");
extern const char howsmyssl_com_root_cert_pem_end[]   asm("_binary_howsmyssl_com_root_cert_pem_end");

esp_err_t _http_event_handler(esp_http_client_event_t* evt)
{
    switch(evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        if (evt->user_data)
        {
            auto p = reinterpret_cast<char*>(evt->user_data);
            memcpy(p, evt->data, evt->data_len);
            p[evt->data_len] = 0;
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        int mbedtls_err = 0;
        esp_err_t err = esp_tls_get_and_clear_last_error(reinterpret_cast<esp_tls_error_handle_t>(evt->data), &mbedtls_err, nullptr);
        if (err != 0) {
            ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
            ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
        }
        break;
    }
    return ESP_OK;
}

void upload(const unsigned char* data, size_t size,
            const struct tm& current,
            const char* ext)
{
    char ts[20];
    strftime(ts, sizeof(ts), "%Y%m%d%H%M%S", &current);
    char resource[40];
    sprintf(resource, "/hal9kcam/%d-%s.%s", (int) config_instance_number, ts, ext);
    
    esp_http_client_config_t config {
        .host = "minio.hal9k.dk",
        .path = resource,
        .cert_pem = howsmyssl_com_root_cert_pem_start,
        .event_handler = _http_event_handler,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_PUT);

    esp_http_client_set_post_field(client, reinterpret_cast<const char*>(data), size);

    char date[40];
    strftime(date, sizeof(date), "%a, %d %b %Y %T %z", &current);
    esp_http_client_set_header(client, "Date", date);
    const char* content_type = "application/octet-stream";
    esp_http_client_set_header(client, "Content-Type", content_type);
    char signature[128];
    snprintf(signature, sizeof(signature), "PUT\n\n%s\n%s\n%s", content_type, date, resource);
    const char* secret = config_s3_secret_key;
    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
    unsigned char hmac[20]; // SHA1 HMAC is always 20 bytes
    mbedtls_md_hmac(md_info, (unsigned char*) secret, strlen(secret),
                    (unsigned char*) signature, strlen(signature), hmac);
    unsigned char b64hmac[29]; // 20 binary bytes -> 28 Base64 characters
    b64hmac[28] = 0;
    size_t written = 0;
    mbedtls_base64_encode(b64hmac, sizeof(b64hmac), &written, hmac, sizeof(hmac));
    char auth[80];
    sprintf(auth, "AWS %s:%s", config_s3_access_key, b64hmac);
    esp_http_client_set_header(client, "Authorization", auth);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Uploaded %s, HTTPS Status = %d",
                 resource,
                 esp_http_client_get_status_code(client));
    }
    else
        ESP_LOGE(TAG, "Error performing http request %s", esp_err_to_name(err));
    
    esp_http_client_cleanup(client);
}

void upload(const camera_fb_t* fb, const struct tm& current)
{
    // Picture
    
    const char* ext = "cam";
    if (fb->format == PIXFORMAT_JPEG)
        ext = "jpg";
    upload(fb->buf, fb->len, current, ext);
}

void heartbeat(const struct tm& current)
{
    char resource[20];
    sprintf(resource, "/camera/%d", (int) config_instance_number);
    char buffer[256];
    esp_http_client_config_t config {
        .host = "acsgateway.hal9k.dk",
        .path = resource,
        .cert_pem = howsmyssl_com_root_cert_pem_start,
        .event_handler = _http_event_handler,
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
            cJSON_Delete(root);
        }
    }
    else
        ESP_LOGE(TAG, "Error performing http request %s", esp_err_to_name(err));
    
    esp_http_client_cleanup(client);
}
