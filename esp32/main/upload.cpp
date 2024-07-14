#include "defs.h"
#include "eventhandler.h"
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

#include "mbedtls/base64.h"
#include "mbedtls/md.h"


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
        .event_handler = http_event_handler,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .crt_bundle_attach = esp_crt_bundle_attach,
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
