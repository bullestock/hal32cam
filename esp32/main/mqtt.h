#pragma once

#include <string>
#include <time.h>

#include "mqtt_client.h"

/// MQTT singleton
class Mqtt
{
public:
    static Mqtt& instance();

    void start(const std::string& mqtt_address);

    void log(const std::string& msg);

    void set_status(time_t last_pic);

private:
    Mqtt() = default;

    ~Mqtt() = default;

    static void event_handler(void* handler_args,
                              esp_event_base_t base,
                              int32_t event_id,
                              void* event_data);

    void handle_data(const std::string& topic,
                     const std::string& data);
    
    bool connected = false;
    esp_mqtt_client_handle_t client = 0;
};

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:
