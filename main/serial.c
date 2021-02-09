#include "serial.h"

#include "utils.h"
#include "loki.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "driver/uart.h"

#include <string.h>

static const char *TAG = "serial";
const int uart_buffer_size = (RD_BUF_SIZE * 2);

static void uart_event_task(void *pvParameters) {
  uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE + 1);
  char* ctmp = (char*) malloc(LOG_LINE_SIZE + 1);
  char* ctmp2 = (char*) malloc(LOG_LINE_SIZE + 1);
  log_data_t out_line;
  for(;;) {
    bzero(dtmp, RD_BUF_SIZE);
    memset(&out_line, 0, sizeof(log_data_t));
    int len = uart_read_bytes(EX_UART_NUM, dtmp, RD_BUF_SIZE, 20 / portTICK_RATE_MS);
    if (!len) continue;
    ESP_LOGI(TAG, "[UART DATA]: %d", len);
    ESP_LOGV(TAG, "data: %s", dtmp);
    gettimeofday(&out_line.tv, NULL);
    char delim[] = "\r\n";
    char *ptr = strtok((char *)dtmp, delim);
    bzero(ctmp, LOG_LINE_SIZE + 1);
    bzero(ctmp2, LOG_LINE_SIZE + 1);
    while(ptr != NULL) {
      int chunk_len = strlen(ptr);
      if (chunk_len) {
        remove_vt100(chunk_len, ptr, LOG_LINE_SIZE, ctmp);
        replace_tabs(strlen(ctmp), ctmp, LOG_LINE_SIZE, ctmp2);
        ESP_LOGD(TAG, "%s", ctmp2);
        strcpy(out_line.log_line, ctmp2);
        if (strchr("VDIWE", ctmp2[0]) && ctmp2[1] == ' ' && ctmp2[2] == '(') {
          strcpy(out_line.labels[0], "level");
          if (ctmp2[0] == 'E') strcpy(out_line.labels[0 + LABELS_NUM], "error");
          else if (ctmp2[0] == 'W') strcpy(out_line.labels[0 + LABELS_NUM], "warning");
          else if (ctmp2[0] == 'I') strcpy(out_line.labels[0 + LABELS_NUM], "info");
          else if (ctmp2[0] == 'D') strcpy(out_line.labels[0 + LABELS_NUM], "debug");
          else if (ctmp2[0] == 'V') strcpy(out_line.labels[0 + LABELS_NUM], "verbose");
        }
        xQueueSendToBack(data0_queue, &out_line, 0);
      }
      ptr = strtok(NULL, delim);
      bzero(ctmp, LOG_LINE_SIZE + 1);
      bzero(ctmp2, LOG_LINE_SIZE + 1);
    }
  }
  free(dtmp);
  free(ctmp);
  free(ctmp2);
  dtmp = NULL;
  ctmp = NULL;
  ctmp2 = NULL;
  vTaskDelete(NULL);
}

void init_serial() {
  // configure UART
  uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
  };
  ESP_ERROR_CHECK(uart_param_config(EX_UART_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(EX_UART_NUM, UART_PIN_NO_CHANGE, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  ESP_ERROR_CHECK(uart_driver_install(EX_UART_NUM, uart_buffer_size, 0, 0, NULL, 0));

  xTaskCreate(uart_event_task, "uart_event_task", 4096, NULL, 12, NULL);
}
