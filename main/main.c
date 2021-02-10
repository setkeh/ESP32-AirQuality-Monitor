#include <string.h>
#include "lwip/apps/sntp.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "sdkconfig.h"

#include "driver/gpio.h"
#include "wifi_functions.h"
#include "ota.h"
#include "task.h"

#include "loki.h"
#include "serial.h"

static const char *TAG = "ESP-Template-Project-MAIN";

void app_main() {
  time_t now = 0;
  struct tm timeinfo = { 0 };
  int retry = 0;
  const int retry_count = 100;
  char strftime_buf[64];
 	
  // connect to the wifi network
	wifi_initialise();
	wifi_wait_connected();
	printf("Connected to wifi network");
  init_loki();
  init_serial();

 
  ESP_LOGI(TAG, "Initializing SNTP");
  
//  setenv("TZ", "EST-10EDT,M10.1.0,M4.1.0", 1);
//  tzset();

  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "au.pool.ntp.org");
  sntp_init();

  while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
    ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    time(&now);
//    setenv("TZ", "EST-10EDT,M10.1.0,M4.1.0", 1);
//    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Sydney is: %s", strftime_buf);
  }

	gpio_pad_select_gpio(BLINK_GPIO);
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

	ESP_LOGI(TAG, "HTTPS OTA, firmware %.1f", FIRMWARE_VERSION);
	
	// start the blink task
	xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
	
	// start the check update task
	xTaskCreate(&check_update_task, "check_update_task", 8192, NULL, 5, NULL);
}
