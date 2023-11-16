/*
 * camera.h
 *
 *  Created on: 2021. 1. 13.
 *      Author: HanCheol Cho
 */

#ifndef MAIN_COMMON_HW_INCLUDE_CAMERA_H_
#define MAIN_COMMON_HW_INCLUDE_CAMERA_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"

#ifdef _USE_HW_CAMERA


#include "esp_camera.h"



typedef struct
{
  bool     is_valid;

  camera_fb_t *fb;

  uint8_t *cam_buf;
  uint32_t length;

} camera_info_t;


bool cameraInit(void);
bool cameraIsInit(void);
bool cameraTake(camera_info_t *p_info);
bool cameraRelease(camera_info_t *p_info);

bool registerCamera(const pixformat_t pixel_fromat,
                const framesize_t frame_size,
                const uint8_t fb_count,
                const QueueHandle_t frame_o);

esp_err_t getCameraError(void);

#endif


#ifdef __cplusplus
}
#endif



#endif /* MAIN_COMMON_HW_INCLUDE_CAMERA_H_ */
