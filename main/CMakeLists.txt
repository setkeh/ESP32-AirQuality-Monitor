set(COMPONENT_SRCS "task.c" "ota.c" "wifi_functions.c" "main.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")


# Embed the server root certificate into the final binary
set(COMPONENT_EMBED_TXTFILES ../server_certs/ca_cert.pem)

register_component()
