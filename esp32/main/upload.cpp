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
#include "esp_sntp.h"

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
    static char* output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
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
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        /*
         *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
         *  However, event handler can also be used in case chunked encoding is used.
         */
        if (!esp_http_client_is_chunked_response(evt->client)) {
            // If user_data buffer is configured, copy the response into the buffer
            if (evt->user_data)
                memcpy(reinterpret_cast<char*>(evt->user_data) + output_len, evt->data, evt->data_len);
            else {
                if (output_buffer == nullptr) {
                    output_buffer = (char* ) malloc(esp_http_client_get_content_length(evt->client));
                    output_len = 0;
                    if (output_buffer == nullptr) {
                        ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                        return ESP_FAIL;
                    }
                }
                memcpy(output_buffer + output_len, evt->data, evt->data_len);
            }
            output_len += evt->data_len;
        }

        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        if (output_buffer != nullptr) {
            // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
            // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
            free(output_buffer);
            output_buffer = nullptr;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        int mbedtls_err = 0;
        esp_err_t err = esp_tls_get_and_clear_last_error(reinterpret_cast<esp_tls_error_handle_t>(evt->data), &mbedtls_err, nullptr);
        if (err != 0) {
            if (output_buffer != nullptr) {
                free(output_buffer);
                output_buffer = nullptr;
            }
            output_len = 0;
            ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
            ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
        }
        break;
    }
    return ESP_OK;
}

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

void upload(const camera_fb_t* fb)
{
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
        time(&current);
        gmtime_r(&current, &timeinfo);
    }

    const char* ext = "cam";
    if (fb->format == PIXFORMAT_JPEG)
        ext = "jpg";
    char ts[20];
    strftime(ts, sizeof(ts), "%Y%m%d%T", &timeinfo);
    char resource[40];
    sprintf(resource, "/hal9kcam/%d-%s.%s", CONFIG_HAL32CAM_INSTANCE, ts, ext);
    
    esp_http_client_config_t config {
        .host = "minio.hal9k.dk",
        .path = resource,
        .cert_pem = howsmyssl_com_root_cert_pem_start,
        .event_handler = _http_event_handler,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_PUT);
    esp_http_client_set_post_field(client, reinterpret_cast<const char*>(fb->buf), fb->len);

    char date[40];
    strftime(date, sizeof(date), "%a, %d %b %Y %T %z", gmtime(&current));
    esp_http_client_set_header(client, "Date", date);
    const char* content_type = "application/octet-stream";
    esp_http_client_set_header(client, "Content-Type", content_type);
    char signature[128];
    snprintf(signature, sizeof(signature), "PUT\n\n%s\n%s\n%s", content_type, date, resource);
    const char* secret = CONFIG_HAL32CAM_SECRET_KEY;
    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
    unsigned char hmac[20]; // SHA1 HMAC is always 20 bytes
    mbedtls_md_hmac(md_info, (unsigned char*) secret, strlen(secret),
                    (unsigned char*) signature, strlen(signature), hmac);
    unsigned char b64hmac[29]; // 20 binary bytes -> 28 Base64 characters
    b64hmac[28] = 0;
    size_t written = 0;
    mbedtls_base64_encode(b64hmac, sizeof(b64hmac), &written, hmac, sizeof(hmac));
    char auth[80];
    sprintf(auth, "AWS %s:%s", CONFIG_HAL32CAM_ACCESS_KEY, b64hmac);
    esp_http_client_set_header(client, "Authorization", auth);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %d",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    else
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));

    esp_http_client_cleanup(client);
}
