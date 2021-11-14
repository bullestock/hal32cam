# Embed the server root certificate into the final binary
idf_component_register(SRCS camera.cpp main.cpp upload.cpp
                       INCLUDE_DIRS "."
                       EMBED_TXTFILES howsmyssl_com_root_cert.pem)
