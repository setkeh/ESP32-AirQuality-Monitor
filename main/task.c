#include "driver/gpio.h"
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "task.h"

// Blink task
void blink_task(void *pvParameter) {
	
    while(1) {
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(BLINK_INTERVAL / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(BLINK_INTERVAL / portTICK_PERIOD_MS);
    }
}