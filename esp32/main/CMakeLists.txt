# Embed the server root certificate into the final binary
idf_component_register(
  SRCS camera.cpp connect.cpp console.cpp format.cpp http.cpp
  main.cpp motion.cpp mqtt.cpp nvs.cpp otafwu.cpp sntp.cpp upload.cpp util.cpp JPEGDEC.cpp
  REQUIRES app_update components console esp_app_format esp_driver_gpio esp_driver_uart esp_http_client
  esp-tls esp_wifi mbedtls nvs_flash vfs
  INCLUDE_DIRS ".")
