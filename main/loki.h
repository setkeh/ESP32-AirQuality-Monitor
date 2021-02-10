#ifndef __LOKI_H__
#define __LOKI_H__

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include <sys/time.h>

extern const uint8_t server_cert_loki_pem_start[] asm("_binary_ca_cert_loki_pem_start");
extern const uint8_t server_cert_loki_pem_end[] asm("_binary_ca_cert_loki_pem_end");

#define LOKI_PATH "/loki/api/v1/push"
#define EMITTER_LABEL "esploki"
#define JOB_LABEL "uarttail"

#define LOKI_PROJECT_NAME CONFIG_PROJECT_NAME
#define LOKI_USERNAME CONFIG_LOKI_USERNAME
#define LOKI_PASSWORD CONFIG_LOKI_PASSWORD
#define LOKI_HOST CONFIG_LOKI_HOST
#define LOKI_PORT CONFIG_LOKI_PORT
#define LOKI_TRANSPORT CONFIG_LOKI_TRANSPORT

#define LABELS_NUM 3
#define LABEL_SIZE 16 + 1
#define LOG_LINE_SIZE 1024 + 1
#define JSON_BUFF_SIZE 32768
#define ENTRY_BUFF_SIZE 128

typedef struct {
  struct timeval tv;
  char labels[LABELS_NUM * 2][LABEL_SIZE];
  char log_line[LOG_LINE_SIZE];
} log_data_t;

extern QueueHandle_t data0_queue;

void init_loki();

#endif
