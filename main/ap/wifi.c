/*
 * wifi_sta.c
 *
 *  Created on: Sep 28, 2021
 *      Author: Jason
 */



#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "wifi.h"
#include "ap.h"

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
static const char *TAG = "wifi";
static int s_retry_num = 0;

extern airb_info_t info;
extern esp_err_t http_server_init(httpd_handle_t *p_server, httpd_config_t *p_config);

void wifi_init(wifi_mode_t mode);


wifi_config_t wifi_config_ap ;
wifi_config_t wifi_config_sta;
bool is_user_request = false;


void wifiInit(wifi_mode_t mode, httpd_handle_t *p_server, httpd_config_t *p_config)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);


    if (mode == WIFI_MODE_APSTA)
    {
      cliPrintf("ESP_WIFI_MODE_APSTA\n");
    }
    else if (mode == WIFI_MODE_AP)
    {
      cliPrintf("ESP_WIFI_MODE_AP\n");
    }
    else if (mode == WIFI_MODE_STA)
    {
      cliPrintf("ESP_WIFI_MODE_STA\n");
    }

    wifi_init(mode);

    if (mode & WIFI_MODE_STA)
    {
      /* Use the URI wildcard matching function in order to
       * allow the same handler to respond to multiple different
       * target URIs which match the wildcard scheme */
      p_config->uri_match_fn = httpd_uri_match_wildcard;
    }
    if (mode & WIFI_MODE_AP)
    {
      delay(100);
      http_server_init(p_server, p_config);
    }

}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED && is_user_request == false) {
        if (s_retry_num < ESP_STA_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        info.is_sta_connect = false;
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;

        info.sta_addr[0] = esp_ip4_addr1_16(&event->ip_info.ip);
        info.sta_addr[1] = esp_ip4_addr2_16(&event->ip_info.ip);
        info.sta_addr[2] = esp_ip4_addr3_16(&event->ip_info.ip);
        info.sta_addr[3] = esp_ip4_addr4_16(&event->ip_info.ip);

        info.is_sta_connect = true;

        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
      wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
      logPrintf("station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
      wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
      logPrintf(TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

void wifi_init(wifi_mode_t mode)
{

    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());



    if (mode & WIFI_MODE_STA)
    {
      esp_netif_create_default_wifi_sta();
    }
    if (mode & WIFI_MODE_AP)
    {
      esp_netif_create_default_wifi_ap();
    }


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_sta.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config_sta.sta.pmf_cfg.capable = true;
    wifi_config_sta.sta.pmf_cfg.required = false;

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    if( info.sta_ssid[0] == 0xFF || info.sta_ssid[0] == 0x00)
      memcpy( info.sta_ssid, ESP_WIFI_STA_SSID, strlen(ESP_WIFI_STA_SSID));

    if( info.sta_pw[0] == 0xFF || info.sta_pw[0] == 0x00)
      memcpy( info.sta_pw, ESP_WIFI_STA_PASS, strlen(ESP_WIFI_STA_PASS));

    strcpy((char *)wifi_config_sta.sta.ssid,(char *)info.sta_ssid);
    strcpy((char *)wifi_config_sta.sta.password,(char *)info.sta_pw);

    // TODO : Load SSID PW Info
    if( info.ap_ssid[0] == 0xFF || info.ap_ssid[0] == 0x00)
    {
      uint8_t esp_mac[6];
      uint8_t ssid_ble_data = 0;

      memcpy( info.ap_ssid, ESP_WIFI_AP_SSID, strlen(ESP_WIFI_AP_SSID));
      esp_wifi_get_mac(ESP_IF_WIFI_STA, esp_mac);

      // AIRB_ABCD
      ssid_ble_data = ((esp_mac[4] & 0xF0) >> 4);
      if( ssid_ble_data < 10 ) info.ap_ssid[5] = (char)(ssid_ble_data + 48);
      else info.ap_ssid[5] = (char)((ssid_ble_data-10)+65);

      ssid_ble_data = ((esp_mac[4] & 0x0F) >> 0);
      if( ssid_ble_data < 10 ) info.ap_ssid[6] = (char)(ssid_ble_data + 48);
      else info.ap_ssid[6] = (char)((ssid_ble_data-10)+65);

      ssid_ble_data = ((esp_mac[5] & 0xF0) >> 4);
      if( ssid_ble_data < 10 ) info.ap_ssid[7] = (char)(ssid_ble_data + 48);
      else info.ap_ssid[7] = (char)((ssid_ble_data-10)+65);

      ssid_ble_data = ((esp_mac[5] & 0x0F) >> 0);
      if( ssid_ble_data < 10 ) info.ap_ssid[8] = (char)(ssid_ble_data + 48);
      else info.ap_ssid[8] = (char)((ssid_ble_data-10)+65);
    }

    if( info.ap_pw[0] == 0xFF || info.ap_pw[0] == 0x00)
    {
      memcpy( info.ap_pw, ESP_WIFI_AP_PASS, strlen(ESP_WIFI_AP_PASS));

      uint8_t esp_mac[6];
      uint8_t ssid_ble_data = 0;

      esp_wifi_get_mac(ESP_IF_WIFI_STA, esp_mac);

      // AIRB_ABCD
      ssid_ble_data = ((esp_mac[4] & 0xF0) >> 4);
      if( ssid_ble_data < 10 ) info.ap_pw[8] = (char)(ssid_ble_data + 48);
      else info.ap_pw[8] = (char)((ssid_ble_data-10)+65);

      ssid_ble_data = ((esp_mac[4] & 0x0F) >> 0);
      if( ssid_ble_data < 10 ) info.ap_pw[9] = (char)(ssid_ble_data + 48);
      else info.ap_pw[9] = (char)((ssid_ble_data-10)+65);

      ssid_ble_data = ((esp_mac[5] & 0xF0) >> 4);
      if( ssid_ble_data < 10 ) info.ap_pw[10] = (char)(ssid_ble_data + 48);
      else info.ap_pw[10] = (char)((ssid_ble_data-10)+65);

      ssid_ble_data = ((esp_mac[5] & 0x0F) >> 0);
      if( ssid_ble_data < 10 ) info.ap_pw[11] = (char)(ssid_ble_data + 48);
      else info.ap_pw[11] = (char)((ssid_ble_data-10)+65);
    }

    wifi_config_ap.ap.ssid_len = strlen(info.ap_ssid);
    wifi_config_ap.ap.channel = 2;
    wifi_config_ap.ap.max_connection = MAX_STA_CONN;
    wifi_config_ap.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    strcpy((char *)wifi_config_ap.ap.ssid,(char *)info.ap_ssid);
    strcpy((char *)wifi_config_ap.ap.password,(char *)info.ap_pw);

    if (strlen(info.ap_pw) == 0)
    {
      wifi_config_ap.ap.authmode = WIFI_AUTH_OPEN;
    }


    ESP_ERROR_CHECK(esp_wifi_set_mode(mode) );
    if (mode & WIFI_MODE_STA)
    {
      ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_sta));
    }
    if (mode & WIFI_MODE_AP)
    {
      ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap));
    }
    ESP_ERROR_CHECK(esp_wifi_start() );


    if (mode & WIFI_MODE_STA)
    {
      /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
       * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
      EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                             WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                             pdFALSE,
                                             pdFALSE,
                                             portMAX_DELAY);

      /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
       * happened. */
      if (bits & WIFI_CONNECTED_BIT)
      {
        uint32_t addr;
        cliPrintf("connected to ap SSID:%s password:%s\n",
            info.sta_ssid, info.sta_pw);

        tcpip_adapter_ip_info_t ip;
        tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip);
        addr = ip.ip.addr;
        cliPrintf("sta ip: %d.%d.%d.%d\n", (addr>>0)&0xFF, (addr>>8)&0xFF, (addr>>16)&0xFF, addr>>24);
        info.is_sta_connect = true;
      }
      else if (bits & WIFI_FAIL_BIT)
      {
        cliPrintf("Failed to connect to SSID:%s, password:%s\n",
            info.sta_ssid, info.sta_pw);
        info.is_sta_connect = false;
      }
      else
      {
        cliPrintf("UNEXPECTED EVENT\n");
      }
    }

    if (mode & WIFI_MODE_AP)
    {
      uint32_t addr;
      tcpip_adapter_ip_info_t ip;

      tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip);
      addr = ip.ip.addr;
      cliPrintf("ap ip: %d.%d.%d.%d\n", (addr>>0)&0xFF, (addr>>8)&0xFF, (addr>>16)&0xFF, addr>>24);
      info.ap_addr[0] = (addr>>0)&0xFF;
      info.ap_addr[1] = (addr>>8)&0xFF;
      info.ap_addr[2] = (addr>>16)&0xFF;
      info.ap_addr[3] = addr>>24;
    }

    /* The event will not be processed after unregister */
//    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
//    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
//    vEventGroupDelete(s_wifi_event_group);
}

//uint8_t airbApState(void)
//{
//
//}
//
//uint8_t airbStaState(void)
//{
//
//}

void disconnectAP(void)
{
  esp_err_t err = ESP_OK;
  is_user_request = true;
  info.is_sta_connect = false;
  err = esp_wifi_disconnect();
  ESP_LOGI("WiFi", "disconnectAP : %d", err);
}

void reconnectAP(void)
{
  esp_err_t err = ESP_OK;
  s_retry_num = 0; // clear
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_sta));

  is_user_request = false;
  err = esp_wifi_connect();
  ESP_LOGI("WiFi", "reconnectAP : %d", err);

  delay(100);

  /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
   * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE,
                                         pdFALSE,
                                         portMAX_DELAY);

  /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
   * happened. */
  if (bits & WIFI_CONNECTED_BIT)
  {
    uint32_t addr;
    cliPrintf("connected to ap SSID:%s password:%s\n", info.sta_ssid, info.sta_pw);

    tcpip_adapter_ip_info_t ip;
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip);
    addr = ip.ip.addr;
    cliPrintf("sta ip: %d.%d.%d.%d\n", (addr>>0)&0xFF, (addr>>8)&0xFF, (addr>>16)&0xFF, addr>>24);
    info.is_sta_connect = true;
  }
  else if (bits & WIFI_FAIL_BIT)
  {
    cliPrintf("Failed to connect to SSID:%s, password:%s\n", info.sta_ssid, info.sta_pw);
    info.is_sta_connect = false;
  }
  else
  {
    cliPrintf("UNEXPECTED EVENT\n");
  }
}

void setApSSID(void)
{
  strcpy((char *)wifi_config_ap.ap.ssid,(char *)info.ap_ssid);
  wifi_config_ap.ap.ssid_len = strlen(info.ap_ssid);

  ESP_LOGI("WiFi", "setApSSID : %s/%d", info.ap_ssid, wifi_config_ap.ap.ssid_len );
}

void setApPASS(void)
{
  strcpy((char *)wifi_config_ap.ap.password,(char *)info.ap_pw);
  ESP_LOGI("WiFi", "setApPASS : %s", info.ap_pw);
}

void resetApDefault(void)
{
  uint8_t esp_mac[6];
  uint8_t ssid_ble_data = 0;

  memcpy( info.ap_ssid, ESP_WIFI_AP_SSID, strlen(ESP_WIFI_AP_SSID));
  memcpy( info.ap_pw, ESP_WIFI_AP_PASS, strlen(ESP_WIFI_AP_PASS));

  esp_wifi_get_mac(ESP_IF_WIFI_STA, esp_mac);

  // AIRB_ABCD
  ssid_ble_data = ((esp_mac[4] & 0xF0) >> 4);
  if( ssid_ble_data < 10 ) info.ap_ssid[5] = (char)(ssid_ble_data + 48);
  else info.ap_ssid[5] = (char)((ssid_ble_data-10)+65);

  ssid_ble_data = ((esp_mac[4] & 0x0F) >> 0);
  if( ssid_ble_data < 10 ) info.ap_ssid[6] = (char)(ssid_ble_data + 48);
  else info.ap_ssid[6] = (char)((ssid_ble_data-10)+65);

  ssid_ble_data = ((esp_mac[5] & 0xF0) >> 4);
  if( ssid_ble_data < 10 ) info.ap_ssid[7] = (char)(ssid_ble_data + 48);
  else info.ap_ssid[7] = (char)((ssid_ble_data-10)+65);

  ssid_ble_data = ((esp_mac[5] & 0x0F) >> 0);
  if( ssid_ble_data < 10 ) info.ap_ssid[8] = (char)(ssid_ble_data + 48);
  else info.ap_ssid[8] = (char)((ssid_ble_data-10)+65);

  // AIRB_ABCD
  ssid_ble_data = ((esp_mac[4] & 0xF0) >> 4);
  if( ssid_ble_data < 10 ) info.ap_pw[8] = (char)(ssid_ble_data + 48);
  else info.ap_pw[8] = (char)((ssid_ble_data-10)+65);

  ssid_ble_data = ((esp_mac[4] & 0x0F) >> 0);
  if( ssid_ble_data < 10 ) info.ap_pw[9] = (char)(ssid_ble_data + 48);
  else info.ap_pw[9] = (char)((ssid_ble_data-10)+65);

  ssid_ble_data = ((esp_mac[5] & 0xF0) >> 4);
  if( ssid_ble_data < 10 ) info.ap_pw[10] = (char)(ssid_ble_data + 48);
  else info.ap_pw[10] = (char)((ssid_ble_data-10)+65);

  ssid_ble_data = ((esp_mac[5] & 0x0F) >> 0);
  if( ssid_ble_data < 10 ) info.ap_pw[11] = (char)(ssid_ble_data + 48);
  else info.ap_pw[11] = (char)((ssid_ble_data-10)+65);


  strcpy((char *)wifi_config_ap.ap.ssid,(char *)info.ap_ssid);
  wifi_config_ap.ap.ssid_len = strlen(info.ap_ssid);
  strcpy((char *)wifi_config_ap.ap.password,(char *)info.ap_pw);
}

void setStaSSID(void)
{
  strcpy((char *)wifi_config_sta.sta.ssid,(char *)info.sta_ssid);
  ESP_LOGI("WiFi", "setStaSSID : %s", info.sta_ssid);
}

void setStaPASS(void)
{
  strcpy((char *)wifi_config_sta.sta.password,(char *)info.sta_pw);
  ESP_LOGI("WiFi", "setStaPASS : %s", info.sta_pw);
}

void resetStaDefault(void)
{
  memcpy( info.sta_ssid, ESP_WIFI_STA_SSID, strlen(ESP_WIFI_STA_SSID));
  memcpy( info.sta_pw, ESP_WIFI_STA_PASS, strlen(ESP_WIFI_STA_PASS));

  strcpy((char *)wifi_config_sta.sta.ssid,(char *)info.sta_ssid);
  strcpy((char *)wifi_config_sta.sta.password,(char *)info.sta_pw);
}
