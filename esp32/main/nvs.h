#pragma once

#include "defs.h"

void init_nvs();

int get_instance();
wifi_creds_t get_wifi_creds();
std::string get_s3_access_key();
std::string get_s3_secret_key();
std::string get_mqtt_address();
bool get_camera_hmirror();

void clear_wifi_credentials();
void add_wifi_credentials(const char* ssid, const char* password);
void set_instance(int);
void set_s3_access_key(const char* key);
void set_s3_secret_key(const char* key);
void set_mqtt_address(const char* address);
void set_camera_hmirror(bool hmirror);

// Local Variables:
// compile-command: "(cd ..; idf.py build)"
// End:
