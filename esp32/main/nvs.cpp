#include "nvs.h"

#include "defs.h"

#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_vfs.h>
#include <nvs_flash.h>

static int8_t instance = 0;
static wifi_creds_t wifi_creds;
static char s3_access_key[40];
static char s3_secret_key[40];
static char mqtt_address[80];
static uint8_t hmirror = false;

void clear_wifi_credentials()
{
    nvs_handle my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_set_str(my_handle, WIFI_KEY, ""));
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);
    wifi_creds.clear();
}

void add_wifi_credentials(const char* ssid, const char* password)
{
    nvs_handle my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));
    std::string creds;
    char buf[256];
    auto size = sizeof(buf);
    if (nvs_get_str(my_handle, WIFI_KEY, buf, &size) == ESP_OK)
        creds = std::string(buf);
    creds += std::string(ssid) + std::string(":") + std::string(password) + std::string(":");
    ESP_ERROR_CHECK(nvs_set_str(my_handle, WIFI_KEY, creds.c_str()));
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);
}

void set_instance(int instance_arg)
{
    nvs_handle my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_set_i8(my_handle, INSTANCE_KEY, (int8_t) instance_arg));
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);
    instance = instance_arg;
}

void set_camera_hmirror(bool hmirror_arg)
{
    nvs_handle my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_set_u8(my_handle, HMIRROR_KEY, hmirror_arg));
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);
    hmirror = hmirror_arg;
}

void set_s3_access_key(const char* key)
{
    nvs_handle my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_set_str(my_handle, S3_ACCESS_KEY, key));
    nvs_close(my_handle);
}

void set_s3_secret_key(const char* key)
{
    nvs_handle my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_set_str(my_handle, S3_SECRET_KEY, key));
    nvs_close(my_handle);
}

void set_mqtt_address(const char* address)
{
    nvs_handle my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_set_str(my_handle, MQTT_ADDRESS_KEY, address));
    nvs_close(my_handle);
}

bool get_nvs_string(nvs_handle my_handle, const char* key, char* buf, size_t buf_size)
{
    auto err = nvs_get_str(my_handle, key, buf, &buf_size);
    switch (err)
    {
    case ESP_OK:
        return true;
    case ESP_ERR_NVS_NOT_FOUND:
        printf("%s: not found\n", key);
        break;
    default:
        printf("%s: NVS error %d\n", key, err);
        break;
    }
    return false;
}

std::vector<std::pair<std::string, std::string>> parse_wifi_credentials(char* buf)
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

int get_instance()
{
    return instance;
}

bool get_camera_hmirror()
{
    return hmirror;
}

wifi_creds_t get_wifi_creds()
{
    return wifi_creds;
}

std::string get_s3_access_key()
{
    return s3_access_key;
}

std::string get_s3_secret_key()
{
    return s3_secret_key;
}

std::string get_mqtt_address()
{
    return mqtt_address;
}

void init_nvs()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    nvs_handle my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));
    char buf[256];
    if (nvs_get_i8(my_handle, INSTANCE_KEY, &instance) != ESP_OK)
        instance = 0;
    if (nvs_get_u8(my_handle, HMIRROR_KEY, &hmirror) != ESP_OK)
        hmirror = false;
    if (get_nvs_string(my_handle, WIFI_KEY, buf, sizeof(buf)))
        wifi_creds = parse_wifi_credentials(buf);
    if (!get_nvs_string(my_handle, S3_ACCESS_KEY, s3_access_key, sizeof(s3_access_key)))
        s3_access_key[0] = 0;
    if (!get_nvs_string(my_handle, S3_SECRET_KEY, s3_secret_key, sizeof(s3_secret_key)))
        s3_secret_key[0] = 0;
    if (!get_nvs_string(my_handle, MQTT_ADDRESS_KEY, mqtt_address, sizeof(mqtt_address)))
        strcpy(mqtt_address, "imqtt.hal9k.dk");
    nvs_close(my_handle);
}

// Local Variables:
// compile-command: "(cd ..; idf.py build)"
// End:
