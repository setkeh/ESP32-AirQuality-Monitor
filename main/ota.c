#include <string.h>
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "cJSON.h"
#include "esp_event.h"
#include "ota.h"
#include "sdkconfig.h"
#include "esp_log.h"

static const char *TAG = "OTA";

int OTA_HTTPS_ERR = 0;

// esp_http_client event handler
esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    
	switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
              ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            break;
        case HTTP_EVENT_HEADER_SENT:
            break;
        case HTTP_EVENT_ON_HEADER:
            break;
        case HTTP_EVENT_ON_DATA:
            if (!esp_http_client_is_chunked_response(evt->client)) {
				      strncpy(rcv_buffer, (char*)evt->data, evt->data_len);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            break;
        case HTTP_EVENT_DISCONNECTED:
            break;
    }
    return ESP_OK;
}

// Check update task
// downloads every 30sec the json file with the latest firmware
void check_update_task(void *pvParameter) {
	
	while(1) {
    if (OTA_HTTPS_ERR <=5) {
        
		  ESP_LOGI(TAG, "Looking for a new firmware...");
	
		  // configure the esp_http_client
		  esp_http_client_config_t config = {
          .url = UPDATE_JSON_URL,
          .event_handler = _http_event_handler,
          .cert_pem = (char *)server_cert_ota_pem_start,
		  };
		  esp_http_client_handle_t client = esp_http_client_init(&config);
	
		  // downloading the json file
		  esp_err_t err = esp_http_client_perform(client);
		  if(err == ESP_OK) {
			
			  // parse the json file	
			  cJSON *json = cJSON_Parse(rcv_buffer);
			  if(json == NULL) ESP_LOGI(TAG, "downloaded file is not a valid json, aborting...");
			  else {
          cJSON *project = cJSON_GetObjectItem(json, PROJECT_NAME);
				  cJSON *version = cJSON_GetObjectItemCaseSensitive(project, "version");
				  cJSON *file = cJSON_GetObjectItemCaseSensitive(project, "file");
          //cJSON_Print(project->valuestring);
          //printf(version->valuedouble);
          //printf(file->valuestring);
				
				// check the version
				  if(!cJSON_IsNumber(version)) ESP_LOGI(TAG, "unable to read new version, aborting...");
				  else {
					
					  double new_version = version->valuedouble;
					  if(new_version > FIRMWARE_VERSION) {
						
						  ESP_LOGI(TAG, "current firmware version (%.1f) is lower than the available one (%.1f), upgrading...", FIRMWARE_VERSION, new_version);
						  if(cJSON_IsString(file) && (file->valuestring != NULL)) {
							  ESP_LOGI(TAG, "downloading and installing new firmware (%s)...", file->valuestring);
							
							  esp_http_client_config_t ota_client_config = {
								  .url = file->valuestring,
								  .cert_pem = (char *)server_cert_ota_pem_start,
							  };
							  esp_err_t ret = esp_https_ota(&ota_client_config);
							  if (ret == ESP_OK) {
                  OTA_HTTPS_ERR = 0;
								  ESP_LOGI(TAG, "OTA OK, restarting...");
								  esp_restart();
							  } else {
								  ESP_LOGI(TAG, "OTA failed...");
							  }
						  }
						  else ESP_LOGI(TAG, "unable to read the new file name, aborting...");
					  }
					  else ESP_LOGI(TAG, "current firmware version (%.1f) is greater or equal to the available one (%.1f), nothing to do...", FIRMWARE_VERSION, new_version);
				  }
			  }
		  }
		  else {
        OTA_HTTPS_ERR++;
			  ESP_LOGI(TAG, "unable to download the json file, aborting...");
		  }
		
		  // cleanup
		  esp_http_client_cleanup(client);
		  printf("\n");
          vTaskDelay(120000 / portTICK_PERIOD_MS);
      }
      else {
        esp_restart();
      }
  }
}
