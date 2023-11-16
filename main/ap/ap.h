/*
 * ap.h
 *
 *  Created on: 2021. 1. 8.
 *      Author: HanCheol Cho
 */

#ifndef MAIN_AP_AP_H_
#define MAIN_AP_AP_H_



#include <string.h>


#include "esp_spiffs.h"
#include "esp_log.h"
#include "hw.h"

#include "hw.h"

#include "esp_efuse.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_partition.h"
#include "esp_flash_encrypt.h"
#include "esp_efuse_table.h"
#include "nvs_flash.h"

#include "esp_flash_spi_init.h"
#include "esp_flash.h"
#include "esp_flash_internal.h"

typedef enum
{
  IR_PROC_RX_WAITING = 0,

  IR_PROC_RX_START,
  IR_PROC_RX_SAVE_START,
  IR_PROC_RX_COMPLETE,

  IR_PROC_RX_TIMEOVER,

}ir_rx_proc_t;

typedef enum
{
  IR_PROC_TX_WAITING = 0,

  IR_PROC_TX_LOAD_START,
  IR_PROC_TX_START,
  IR_PROC_TX_COMPLETE,

  IR_PROC_TX_EMPTY,

}ir_tx_proc_t;

typedef enum
{
  CAM_AP_JEPG = 0,

  CAM_AP_FACE_DETECT,
  CAM_AP_MOVE_DETECT,
  CAM_AP_COLOR_DETECT,

  CAM_AP_APRIL_TAG,

  CAM_AP_NB,

  /* Not Dev Yet */
  CAM_AP_QR_DECODE,
  CAM_AP_CAT_FACE_DETECT,

}cam_ap_list;

typedef enum
{
  COORDINATE_X = 0,
  COORDINATE_Y,

  CORDINATE_NB,
}coordinat;

typedef struct
{
  const esp_partition_t* partition;

  esp_chip_info_t chip_info;
  esp_flash_t flash_info;
  // airb state var
  uint8_t ap_addr[4];
  char ap_ssid[32];
  char ap_pw[32];
  bool is_ap_conneect;

  uint8_t sta_addr[4];
  char sta_ssid[32];
  char sta_pw[32];
  bool is_sta_connect;

  // IR Global var
  uint8_t global_page_buff[4*1024];

  rmt_item32_t global_ir_buff[120];
  char ir_discrip[32];

  ir_rx_proc_t ir_rx_proc_state;
  ir_tx_proc_t ir_tx_proc_state;

  uint16_t ir_code_select_nb;

  // cam Blobal info
  QueueHandle_t xQueueFrameI;
  QueueHandle_t xQueueFrameO;

  QueueHandle_t xQueueAIFrame;
  QueueHandle_t xQueueAOFrame;

  bool     is_valid;
  bool socket_stream;
  bool webpage_stream;
  bool step_shutdown;

  camera_fb_t *fb;
  uint8_t *cam_buf;
  uint32_t length;

  uint8_t ap_mode_change;
  uint8_t cam_ap_select;
  uint8_t cam_state;

  uint8_t move_state;
  bool enable_move_row_data;
  uint16_t move_row_data;


}airb_info_t;

typedef struct
{
  // 감지된 얼굴이 있는가?
  bool is_detect;
  // 얼마나 완벽하게 인식했는가?
  uint16_t detect_score;
  // 감지된 얼굴의 좌표 정보
  uint16_t eye_left[CORDINATE_NB];
  uint16_t eye_right[CORDINATE_NB];
  uint16_t nose[CORDINATE_NB];
  uint16_t mouth_left[CORDINATE_NB];
  uint16_t mouth_right[CORDINATE_NB];

  uint8_t recog_error;
#if _USE_FACE_RECOGNITION
  // 얼굴인식
  char save_name[32];
  char load_name[32];

  uint8_t recog_cmd;
  int8_t recog_option;


  uint8_t recog_result;
  uint16_t recog_score;

  uint8_t dl_model_select;
  uint8_t dl_model_threshold;
#endif
  bool dl_bmp_update;

}ai_face_info_t;

typedef struct
{
  uint16_t x;     //2
  uint16_t y;     //2
  uint16_t w;     //2
  uint16_t h;     //2
} color_data_t;

typedef struct
{
  uint8_t h_min;
  uint8_t h_max;

  uint8_t s_min;
  uint8_t s_max;

  uint8_t v_min;
  uint8_t v_max;
} user_color_data_t;

typedef struct
{
  uint8_t color_cmd;
  int16_t color_error;

  // default 10 Color + Custom 2 Color
  uint8_t detect_cnt[12];
  color_data_t c_data[12][10];
  user_color_data_t custom_data[2];

  uint8_t threshold_x;
  uint8_t threshold_y;

  bool dl_bmp_update;

}ai_color_info_t;

void apInit(void);
void apMain(void);

airb_info_t* getAirbInfo (void);

#endif /* MAIN_AP_AP_H_ */
