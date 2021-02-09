#include <string.h>

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

void app_main() {
  init_loki();
  init_serial();

	gpio_pad_select_gpio(BLINK_GPIO);
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

	printf("HTTPS OTA, firmware %.1f\n\n", FIRMWARE_VERSION);
	
	// start the blink task
	xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
	
	// connect to the wifi network
	wifi_initialise();
	wifi_wait_connected();
	printf("Connected to wifi network\n");

	// start the check update task
	xTaskCreate(&check_update_task, "check_update_task", 8192, NULL, 5, NULL);
}
