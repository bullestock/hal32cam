#include "esp_system.h"
#include "esp_event.h"

#include <string>

#include "cJSON.h"
#include "esp_app_desc.h"
#include "esp_log.h"

#include "defs.h"
#include "format.h"
#include "mqtt.h"
#include "nvs.h"
#include "util.h"

static constexpr const char* TAG = "mqtt";

Mqtt& Mqtt::instance()
{
    static Mqtt the_instance;
    return the_instance;
}

void Mqtt::event_handler(void* handler_args,
                         esp_event_base_t base,
                         int32_t event_id,
                         void* event_data)
{
    Mqtt* self = reinterpret_cast<Mqtt*>(handler_args);
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    auto event = reinterpret_cast<esp_mqtt_event_handle_t>(event_data);
    switch ((esp_mqtt_event_id_t) event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Connected");
        self->connected = true;
        esp_mqtt_client_subscribe(event->client, "hal9k/acs/status/#", 1);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Disconnected");
        self->connected = false;
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "Pub %d", event->msg_id);
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "Sub %d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "Got data");
        self->handle_data(std::string(event->topic, event->topic_len),
                          std::string(event->data, event->data_len));
        break;
        
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "Error %d", event->msg_id);
        break;

    default:
        ESP_LOGI(TAG, "Other event: %d", event->event_id);
        break;
    }
}

void Mqtt::handle_data(const std::string& topic,
                       const std::string& data)
{
    // NOP for now
}

void Mqtt::log(const std::string& msg)
{
    const auto topic = format("hal9k/acs/log/cam%02d", get_instance());
    const auto msg_id = esp_mqtt_client_enqueue(client, topic.c_str(),
                                                msg.c_str(), 0, 1, 0, true);
    ESP_LOGI(TAG, "Q %d", msg_id);
}

void Mqtt::set_status(time_t last_pic)
{
    char timestamp[util::TIMESTAMP_SIZE];
    util::make_timestamp(timestamp);
    auto payload = cJSON_CreateObject();
    cJSON_wrapper jw(payload);
    auto jtimestamp = cJSON_CreateString(timestamp);
    cJSON_AddItemToObject(payload, "timestamp", jtimestamp);
    util::make_timestamp(last_pic, timestamp);
    auto jlast_pic = cJSON_CreateString(timestamp);
    cJSON_AddItemToObject(payload, "last_picture", jlast_pic);
    auto version = cJSON_CreateString(esp_app_get_description()->version);
    cJSON_AddItemToObject(payload, "version", version);

    const char* data = cJSON_PrintUnformatted(payload);
    if (!data)
    {
        ESP_LOGE(TAG, "cJSON_Print() returned nullptr");
        return;
    }
    
    const auto topic = format("hal9k/acs/status/cam%02d", get_instance());
    const auto msg_id = esp_mqtt_client_enqueue(client, topic.c_str(),
                                                data,
                                                0, 1, 1, true);
    ESP_LOGI(TAG, "Q %d", msg_id);
}

void Mqtt::start(const std::string& mqtt_address)
{
    std::string mqtt_url = std::string("mqtt://") + mqtt_address;
    esp_mqtt_client_config_t mqtt_cfg = {
    };
    ESP_LOGI(TAG, "URL %s", mqtt_url.c_str());
    mqtt_cfg.broker.address.uri = mqtt_url.c_str();
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client,
                                   static_cast<esp_mqtt_event_id_t>(ESP_EVENT_ANY_ID),
                                   &Mqtt::event_handler, this);
    esp_mqtt_client_start(client);
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
