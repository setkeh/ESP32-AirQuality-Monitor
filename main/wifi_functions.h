#ifndef _WIFI_FUNCTIONS_H_
#define _WIFI_FUNCTIONS_H_

#define WIFI_SSID			CONFIG_WIFI_SSID
#define WIFI_PASS			CONFIG_WIFI_PASSWORD

void wifi_initialise(void);
void wifi_wait_connected();

#endif