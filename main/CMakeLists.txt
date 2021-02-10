set(COMPONENT_SRCS "utils.c" "serial.c" "loki.c" "task.c" "ota.c" "wifi_functions.c" "main.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")


# Embed the server root certificate into the final binary
set(COMPONENT_EMBED_TXTFILES ../server_certs/ca_cert_ota.pem ../server_certs/ca_cert_loki.pem)

register_component()
