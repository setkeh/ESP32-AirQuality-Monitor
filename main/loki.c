#include "loki.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_system.h"

#include <string.h>
#include <time.h>

static const char *TAG = "loki";
static const char *stream_header = "{\"stream\": {\"emitter\": \"" EMITTER_LABEL "\", \"job\": \"" JOB_LABEL "\"";
static const char *stream_values_header = "}, \"values\":[[";
// static const char *stream_values_delimiter = "\"], [";
static const char *stream_footer = "\"]]}";
const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
static log_data_t in_frame;
static char post_buff[JSON_BUFF_SIZE];
static char entry_buff[ENTRY_BUFF_SIZE];

QueueHandle_t data0_queue;

esp_err_t _http_event_handle(esp_http_client_event_t *evt) {
  switch(evt->event_id) {
    case HTTP_EVENT_ERROR:
      ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
      break;
    case HTTP_EVENT_ON_CONNECTED:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
      break;
    case HTTP_EVENT_HEADER_SENT:
      ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
      break;
    case HTTP_EVENT_ON_HEADER:
      ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER");
      printf("%.*s", evt->data_len, (char*)evt->data);
      break;
    case HTTP_EVENT_ON_DATA:
      ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
      break;
    case HTTP_EVENT_ON_FINISH:
      ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
      break;
    case HTTP_EVENT_DISCONNECTED:
      ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
      break;
  }
  return ESP_OK;
}

void send_data(char *post_buff, esp_http_client_config_t *http_config) {
  int len, read_len, status = 0;
  
  ESP_LOGD(TAG, "POST body: %s", post_buff);
  
  esp_http_client_handle_t client = esp_http_client_init(http_config);
  esp_http_client_set_header(client, "Content-Type", "application/json");
  len = strlen(post_buff);
  esp_http_client_open(client, len);
  esp_http_client_write(client, post_buff, len);
  len = esp_http_client_fetch_headers(client);
  status = esp_http_client_get_status_code(client);
  if (status != 204) {
    ESP_LOGW(TAG, "POST body: %s", post_buff);
    read_len = esp_http_client_read(client, post_buff, len);
    post_buff[read_len] = '\0';
    ESP_LOGE(TAG, "%d: %s", status, read_len > 0 ? post_buff:"error");
  } else {
    ESP_LOGD(TAG, "Status = %d", status);
  }
  esp_http_client_cleanup(client);
}

void send_data_task(void *arg) {
  unsigned int log_line_cnt = 0;
  unsigned int log_time_shift = 0;
  unsigned long int prev_log_usec = 0;
  unsigned long int new_log_usec = 0;
  time_t now, prev_now;
  BaseType_t xStatus;
  char mac_id[13] = "";
  uint8_t mac[6] = {0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
  ESP_ERROR_CHECK(esp_read_mac(mac, 0));
  sprintf(mac_id, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  //loki_cfg_t _config = get_loki_config();
  sprintf(post_buff, "{\"streams\": [%s, \"hwid\": \"%s\", \"iname\": \"%s\"", stream_header, mac_id, LOKI_PROJECT_NAME);
  time(&prev_now);
  // Prepare client configuration
  if (!strcmp(LOKI_HOST, "")) {
    vTaskDelete(NULL);
    return;
  }
  esp_http_client_config_t http_config = {
    .event_handler = _http_event_handle,
    .method = HTTP_METHOD_POST,
    .host = LOKI_HOST,
    .port = LOKI_PORT,
    .path = LOKI_PATH,
    .transport_type = HTTP_TRANSPORT_OVER_TCP
  };
  if (strcmp(LOKI_USERNAME, "")) {
    http_config.auth_type = HTTP_AUTH_TYPE_BASIC;
    http_config.username = strdup(LOKI_USERNAME);
    http_config.password = strdup(LOKI_PASSWORD);
  }
  while(1) {
    xStatus = xQueueReceive(data0_queue, &in_frame, xTicksToWait);
    if(xStatus == pdPASS) {
      // -- Make POST body
      // {
      //   "streams": [
      //     {
      //       "stream": {
      //         "label": "value"
      //       },
      //       "values": [
      //           [ "<unix epoch in nanoseconds>", "<log line>" ],
      //           [ "<unix epoch in nanoseconds>", "<log line>" ]
      //       ]
      //     }
      //   ]
      // }
      if (log_line_cnt) {
        strcat(post_buff, ", ");
        strcat(post_buff, stream_header);
        sprintf(entry_buff, ", \"hwid\": \"%s\", \"iname\": \"%s\"", mac_id, LOKI_PROJECT_NAME);
        strcat(post_buff, entry_buff);
      }
      log_line_cnt++;
      for (int i = 0; i < LABELS_NUM; i++) {
        if (in_frame.labels[i][0] != '\0') {
          sprintf(entry_buff, ", \"%s\": \"%s\"", in_frame.labels[i], in_frame.labels[i + LABELS_NUM]);
          strcat(post_buff, entry_buff);
        }
      }
      strcat(post_buff, stream_values_header);
      new_log_usec = (unsigned long int)in_frame.tv.tv_sec * 1000000 + in_frame.tv.tv_usec;
      if (prev_log_usec < new_log_usec) {
        prev_log_usec = new_log_usec;
        log_time_shift = 0;
      }
      log_time_shift++;
      sprintf(entry_buff, "\"%ld%09ld\", \"", in_frame.tv.tv_sec, in_frame.tv.tv_usec * 1000 + log_time_shift);
      strcat(post_buff, entry_buff);
      strcat(post_buff, in_frame.log_line);
      strcat(post_buff, stream_footer);
    }

    time(&now);
    if (log_line_cnt && (strlen(post_buff) > JSON_BUFF_SIZE - LOG_LINE_SIZE * 2 || now - prev_now > 1)) {
      strcat(post_buff, "]}");
      send_data(post_buff, &http_config);
      sprintf(post_buff, "{\"streams\": [%s, \"hwid\": \"%s\", \"iname\": \"%s\"", stream_header, mac_id, LOKI_PROJECT_NAME);
      log_line_cnt = 0;
      prev_now = now;
    }
  }
}

void init_loki() {
  ESP_LOGI("Initializing Loki Logging");
  data0_queue = xQueueCreate(45, sizeof(log_data_t));
  xTaskCreate(send_data_task, "send_data_task", 8192, NULL, 10, NULL);
  ESP_LOGI("Loki Logging Initialized");
}
