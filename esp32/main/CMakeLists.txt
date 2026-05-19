# Embed the server root certificate into the final binary
idf_component_register(
  SRCS camera.cpp connect.cpp console.cpp eventhandler.cpp
  heartbeat.cpp main.cpp motion.cpp nvs.cpp sntp.cpp upload.cpp cJSON.c JPEGDEC.cpp
  REQUIRES components console esp_driver_gpio esp_driver_uart esp_http_client
  esp-tls esp_wifi mbedtls nvs_flash vfs
  INCLUDE_DIRS ".")
