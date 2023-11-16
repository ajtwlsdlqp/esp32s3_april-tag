/*
 * ap.cpp
 *
 *  Created on: 2021. 1. 8.
 *      Author: HanCheol Cho
 */

#include "ap.h"
#include "wifi_ap.h"

#include "wifi.h"
#include "esp_http_server.h"

#include "hw.h"
#include "ap_camera.h"


extern "C"
{
  void init_spiffs();
  esp_err_t start_file_server(const char *base_path, httpd_handle_t *p_server, httpd_config_t *p_config);

  bool drvEepromInit();

  uint16_t drvEepromReadByte(uint32_t addr);
  bool     drvEepromWriteByte(uint32_t index, uint8_t data_in);
  uint32_t drvEepromGetLength(void);
  bool     drvEepromFormat(void);
}


airb_info_t info;
ai_face_info_t face;
ai_color_info_t color_info;

static void threadsInit();
static void threadSlave(void const *arg);
static void threadCamera(void const *arg);
//static void threadEvent(void const *arg);

static httpd_handle_t server = NULL;
static httpd_config_t config = HTTPD_DEFAULT_CONFIG();

void apInit(void)
{
  bool ret;

  config.max_open_sockets = 3;  // change only avavilable one socket
  config.server_port = 5050;    // change default port 80 -> 5050


  wifiInit(WIFI_MODE_APSTA, &server, &config);

  init_spiffs();
  /* Start the file server */
  ESP_ERROR_CHECK(start_file_server("/spiffs", &server, &config));

  drvEepromInit();

  info.socket_stream = false;
  info.webpage_stream = false;

  info.xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
  info.xQueueAOFrame = xQueueCreate(2, sizeof(camera_fb_t *));

  info.cam_ap_select = CAM_AP_APRIL_TAG;
  info.ap_mode_change = CAM_AP_APRIL_TAG;
  ret = registerCamera(PIXFORMAT_RGB565, FRAMESIZE_240X240, 1, info.xQueueAIFrame);

//  info.cam_ap_select = CAM_AP_JEPG;
//  ret = registerCamera(PIXFORMAT_JPEG, FRAMESIZE_VGA, 2, info.xQueueAIFrame);

  info.cam_state |= PIXFORMAT_JPEG << 2;
  info.cam_state |= FRAMESIZE_VGA;

  cliPrintf("reg Cam[%d] !! \n", ret);

  info.xQueueFrameI = info.xQueueAIFrame;
  info.xQueueFrameO = info.xQueueAOFrame;

  threadsInit();
}

void apMain(void)
{
  uint32_t pre_time;

  pre_time = millis();
  while(1)
  {
    if (millis()-pre_time >= 500)
    {
      pre_time = millis();
      ledToggle(_DEF_LED1);
    }

    taskYIELD();
    // delay(1);

    cliMain();

//    if (uartAvailable(_DEF_UART2) > 0)
//    {
//      uint8_t rx_data;
//
//      rx_data = uartRead(_DEF_UART2);
//
//      //uartPrintf(_DEF_UART2, "Rx : 0x%X\n", uartRead(_DEF_UART2));
//      for (int i=0; i<50; i++)
//      {
//        uartPrintf(_DEF_UART2, "0x%X 00000000000000000000011111111111111111111112222222222222222222222\n", rx_data);
//      }
//    }
  }
}


static void threadsInit()
{
  /* To functions which background operation is needed */
  xTaskCreate((TaskFunction_t)threadCamera, "threadCamera", _HW_DEF_RTOS_THREAD_MEM_DETECTOR, NULL, _HW_DEF_RTOS_THREAD_PRI_DETECTOR, NULL);
}

static void threadCamera(void const *arg)
{
  (void)arg;

  apCameraInit();

  for(;;)
  {
    apCameraLoop();

    taskYIELD();
  }
}

airb_info_t* getAirbInfo (void)
{
  return &info;
}

/* 230728 Push Test @@ */
