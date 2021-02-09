#include "esp_http_client.h"

extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

#define PROJECT_NAME      CONFIG_PROJECT_NAME
#define FIRMWARE_VERSION	1.7
#define UPDATE_JSON_URL		CONFIG_FIRMWARE_UPGRADE_JSON_URL

// receive buffer
char rcv_buffer[200];

esp_err_t _http_event_handler(esp_http_client_event_t *evt);
void check_update_task(void *pvParameter);
