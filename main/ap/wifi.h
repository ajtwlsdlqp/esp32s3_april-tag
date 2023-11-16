/*
 * wifi.h
 *
 *  Created on: Sep 29, 2021
 *      Author: Jason
 */

#ifndef MAIN_AP_WIFI_H_
#define MAIN_AP_WIFI_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "hw.h"
#include "esp_wifi.h"
#include "esp_http_server.h"


// AP for esp32 to connect with
#define ESP_WIFI_STA_SSID       "CONNECT"
#define ESP_WIFI_STA_PASS       "PASSWD"
#define ESP_STA_MAXIMUM_RETRY   3

// AP info when esp32 is AP
#define ESP_WIFI_AP_SSID        "AP"
#define ESP_WIFI_AP_PASS        "PASSWD"
#define MAX_STA_CONN            5


void wifiInit(wifi_mode_t mode, httpd_handle_t *p_server, httpd_config_t *p_config);

void resetStaDefault(void);
void setStaPASS(void);
void setStaSSID(void);

void resetApDefault(void);
void setApPASS(void);
void setApSSID(void);

void reconnectAP(void);
void disconnectAP(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_AP_WIFI_H_ */
