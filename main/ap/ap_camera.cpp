/*
 * ap_camera.c
 *
 *  Created on: Feb 7, 2023
 *      Author: LDH
 */
#include "ap_camera.h"

#include "./ap.h"
#include "../common/hw/include/camera.h"
#include "cli.h"
#include "esp_camera.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"



#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <list>
//#include <variant>
#include <vector>
#include <queue>
#include <algorithm>

#include "human_face_detect_mnp01.hpp"
#include "human_face_detect_msr01.hpp"

#if _USE_FACE_RECOGNITION
#include "face_recognition_tool.hpp"
#include "face_recognition_112_v1_s8.hpp"
#include "face_recognition_112_v1_s16.hpp"
#include "face_recognizer.hpp"
#endif

#include "color_detector.hpp"

#include "dl_tool.hpp"
#include "dl_detect_define.hpp"
#include "dl_image.hpp"
#include "fb_gfx.h"

#if NOT_YET
#include "quirc.h"
#include "quirc_internal.h"
#include "qrcode_classifier.h"

#endif

#include "apriltag/apriltag.h"
#include "apriltag/tagStandard41h12.h"
#include "apriltag/common/getopt.h"
#include "apriltag/common/image_u8.h"
#include "apriltag/common/pjpeg.h"
#include "apriltag/common/zarray.h"

using namespace std;
using namespace dl;

#ifdef __cplusplus
extern "C"
{
#endif


#ifdef __cplusplus
}
#endif


// Helper functions to convert an RGB565 image to grayscale
typedef union {
    uint16_t val;
    struct {
        uint16_t b: 5;
        uint16_t g: 6;
        uint16_t r: 5;
    };
} rgb565_t;
static uint8_t rgb565_to_grayscale(const uint8_t *img);
static void rgb565_to_grayscale_buf(const uint8_t *src, uint8_t *dst, int qr_width, int qr_height);


/* Minimum and Maximum macros */
#ifdef  min
/*! Definition for minimum. */
#define CRYS_MIN(a,b) min( a , b )
#else
/*! Definition for minimum. */
#define CRYS_MIN( a , b ) ( ( (a) < (b) ) ? (a) : (b) )
#endif

#ifdef max
/*! Definition for maximum. */
#define CRYS_MAX(a,b) max( a , b )
#else
/*! Definition for maximum. */
#define CRYS_MAX( a , b ) ( ( (a) > (b) ) ? (a) : (b) )
#endif

#define RGB565_LCD_RED 0x00F8
#define RGB565_LCD_ORANGE 0x20FD
#define RGB565_LCD_YELLOW 0xE0FF
#define RGB565_LCD_GREEN 0xE007
#define RGB565_LCD_CYAN 0xFF07
#define RGB565_LCD_BLUE 0x1F00
#define RGB565_LCD_PURPLE 0x1EA1
#define RGB565_LCD_WHITE 0xFFFF
#define RGB565_LCD_GRAY 0x1084
#define RGB565_LCD_BLACK 0x0000
#define RGB565_LCD_CUSTOM1 0x0000
#define RGB565_LCD_CUSTOM2 0x0000

uint8_t color_buff_idx;
uint16_t decrease_color;
std::vector<uint8_t> color_thresh;
std::vector<std::vector<int>> color_thresh_boxes = {{110, 110, 130, 130}, {100, 100, 140, 140}, {90, 90, 150, 150}, {80, 80, 160, 160}, {60, 60, 180, 180}, {40, 40, 200, 200}, {20, 20, 220, 220}};
int color_thresh_boxes_num;
int color_thresh_boxes_index;
std::vector<int> color_area_threshes = {1, 4, 16, 32, 64, 128, 256, 512, 1024};
int color_area_thresh_num;
int color_area_thresh_index;

#if _USE_FACE_RECOGNITION
static face_info_t recognize_result;

static recognizer_state_t mloop_recog_event = IDLE;
static int partition_result;
static int partition_error;

static   FaceRecognition112V1S8 *recognizer_8 = new FaceRecognition112V1S8();
static   FaceRecognition112V1S16 *recognizer_16 = new FaceRecognition112V1S16();
#endif

static bool return_frambuffer = true;

static void draw_detection_result(uint16_t *image_ptr, int image_height, int image_width, std::list<dl::detect::result_t> &results);
static void draw_detection_result(uint8_t *image_ptr, int image_height, int image_width, std::list<dl::detect::result_t> &results);
static void draw_color_detection_result(uint16_t *image_ptr, int image_height, int image_width, std::vector<color_detect_result_t> &results, uint16_t color, int idx);

static void *app_camera_decode(camera_fb_t *fb);

std::vector<color_info_t> detect_color_info =
{
  {{156, 10, 70, 255, 90, 255}, 32, "red"},
  {{11, 22, 70, 255, 90, 255}, 32, "orange"},
  {{23, 33, 70, 255, 90, 255}, 32, "yellow"},
  {{34, 75, 70, 255, 90, 255}, 32, "green"},
  {{76, 96, 70, 255, 90, 255}, 32, "cyan"},
  {{97, 124, 70, 255, 90, 255}, 32, "blue"},
  {{125, 155, 70, 255, 90, 255}, 32, "purple"},
  {{0, 180, 0, 40, 220, 255}, 32, "white"},
  {{0, 180, 0, 50, 50, 219}, 32, "gray"},
  {{0, 180, 0, 255, 0, 45}, 32, "black"},

  {{0, 180, 0, 255, 0, 45}, 32, "user1"},
  {{0, 180, 0, 255, 0, 45}, 32, "user2"},
};

std::vector<uint16_t> draw_lcd_color =
{
  RGB565_LCD_RED,
  RGB565_LCD_ORANGE,
  RGB565_LCD_YELLOW,
  RGB565_LCD_GREEN,
  RGB565_LCD_CYAN,
  RGB565_LCD_BLUE,
  RGB565_LCD_PURPLE,
  RGB565_LCD_WHITE,
  RGB565_LCD_GRAY,
  RGB565_LCD_BLACK,

  RGB565_LCD_CUSTOM1,
  RGB565_LCD_CUSTOM2
};


extern airb_info_t info;
extern ai_face_info_t face;
extern ai_color_info_t color_info;

static HumanFaceDetectMSR01 detector(0.3F, 0.3F, 10, 0.3F);
static HumanFaceDetectMNP01 detector2(0.4F, 0.3F, 10);

static ColorDetector detector_color;
static int draw_colors_num;

static dl::tool::Latency latency;

static camera_fb_t *frame = NULL;
static camera_fb_t *frame_2 = NULL;

apriltag_family_t *tf = NULL;
apriltag_detector_t *td = NULL;
int quiet = 0;
int maxiters = 0;
const int hamm_hist_max = 10;

void initFaceRecognition(void);
void initColorRecognition(void);
void initQrDecode(void);
void initAprilTagDecode(void);

void deInitFaceRecognition(void);
void deInitColorRecognition(void);
void deInitQrDecode(void);
void deInitAprilTagDecode(void);

void apCameraInit(void)
{
  switch(info.cam_ap_select)
  {
    case CAM_AP_JEPG :
    case CAM_AP_NB :
      break;  // Nothing

    case CAM_AP_FACE_DETECT :
    case CAM_AP_MOVE_DETECT :
      initFaceRecognition();
      break;  // use same detector


    case CAM_AP_COLOR_DETECT :
      initColorRecognition();
      break;

    case CAM_AP_QR_DECODE :
      initQrDecode();
      break;

    case CAM_AP_APRIL_TAG :
      initAprilTagDecode();
      break;
  }
}

void apCameraDeInit(void)
{
  switch(info.cam_ap_select)
  {
    case CAM_AP_JEPG :
    case CAM_AP_NB :
      break;  // Nothing

    case CAM_AP_FACE_DETECT :
    case CAM_AP_MOVE_DETECT :
      deInitFaceRecognition();
      break;  // use same detector

    case CAM_AP_COLOR_DETECT :
      deInitColorRecognition();
      break;

    case CAM_AP_QR_DECODE :
      deInitQrDecode();
      break;

    case CAM_AP_APRIL_TAG :
      deInitAprilTagDecode();
      break;
  }
}


void initFaceRecognition(void)
{
#if _USE_FACE_RECOGNITION
  if( face.dl_model_select == MODEL_8bit)
  {
    recognizer_8->set_partition(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "fr");
    partition_error = recognizer_8->check_partition();
    partition_result = recognizer_8->set_ids_from_flash();

    // dl_model_threshold 미입력시 모델 기본값 사용
    if( face.dl_model_threshold < 100 && face.dl_model_threshold != 0)
    {
      recognizer_8->set_thresh(static_cast<float>(face.dl_model_threshold) * 0.01f);
    }
  }
  else
  {
    recognizer_16->set_partition(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "fr");
    partition_error = recognizer_16->check_partition();
    partition_result = recognizer_16->set_ids_from_flash();

    if( face.dl_model_threshold < 100 && face.dl_model_threshold != 0)
    {
      recognizer_16->set_thresh(static_cast<float>(face.dl_model_threshold) * 0.01f);
    }
  }
#else
  return;
#endif
}

void deInitFaceRecognition(void)
{
#if _USE_FACE_RECOGNITION
  if( face.dl_model_select == MODEL_8bit)
  {
    // TODO : find or make deinit function
  }
  else
  {

  }
#else
  return;
#endif
}

void initColorRecognition(void)
{
  if( color_info.threshold_x == 0 || color_info.threshold_y == 0)
  {
    // set as default ...
    detector_color.set_detection_shape({32, 32, 1});
    color_info.threshold_x = 32;
    color_info.threshold_y = 32;
  }
  else
  {
    detector_color.set_detection_shape({color_info.threshold_x, color_info.threshold_y, 1});
  }

  for (int i = 0; i < detect_color_info.size(); ++i)
  {
    detector_color.register_color(detect_color_info[i].color_thresh, detect_color_info[i].area_thresh, detect_color_info[i].name);
  }

  color_thresh_boxes_num = color_thresh_boxes.size();
  color_thresh_boxes_index = color_thresh_boxes_num / 3;
  color_area_thresh_num = color_area_threshes.size();
  color_area_thresh_index = color_area_thresh_num / 3;

  detector_color.set_area_thresh({color_area_threshes[color_area_thresh_index]});
  draw_colors_num = draw_lcd_color.size();
}

void deInitColorRecognition(void)
{
  color_info.color_cmd = 0;  // clear

  detector_color.clear_color();
  // clear all detect color ...
}


void initQrDecode(void)
{
#if NOT_YET

#else
  return;
#endif
}

void deInitQrDecode(void)
{
#if NOT_YET

#else
  return;
#endif
}

void initAprilTagDecode(void)
{
//  tf = tagStandard52h13_create();
  tf = tagStandard41h12_create();

  td = apriltag_detector_create();

  apriltag_detector_add_family(td, tf);

  td->quad_decimate = 1.0;      // "Decimate input image by this factor"
  td->quad_sigma = 0.0;         // "Apply low-pass blur to input; negative sharpens"
  td->nthreads = 1;             // "Use this many CPU threads"
  td->debug = 0;                // "Enable debugging output (slow)"

  td->refine_edges = 1;         // "Spend more time trying to align edges of tags"
  td->decode_sharpening = 0;

  quiet = 0;                    // "Reduce output"
  maxiters = 1;                 // "Repeat processing on input set this many times"
}

void deInitAprilTagDecode(void)
{
//  tagStandard52h13_destroy(tf);
  tagStandard41h12_destroy(tf);

  apriltag_detector_destroy(td);
}

std::string converToString(char *a, int size);

void apCameraLoop (void)
{

  if (xQueueReceive(info.xQueueFrameI, &frame, portMAX_DELAY))
  {
    switch( info.cam_ap_select)
    {
      case CAM_AP_JEPG :

        if( info.socket_stream == true || info.webpage_stream == true) xQueueSend(info.xQueueFrameO, &frame, portMAX_DELAY);
        else if (return_frambuffer) esp_camera_fb_return(frame);
        else free(frame);
        break;

      /* Under Function Must Working on BMP */
      case CAM_AP_FACE_DETECT :
        {
          Tensor<uint8_t> rgby;

          latency.start();

          std::list<dl::detect::result_t> &detect_candidates = detector.infer((uint16_t *)frame, {(int)frame->height, (int)frame->width, 3});
          std::list<dl::detect::result_t> &detect_results = detector2.infer((uint16_t *)frame, {(int)frame->height, (int)frame->width, 3}, detect_candidates);

          latency.end();

          /* face Detect */
          if (detect_results.size() > 0)
          {
            std::list<dl::detect::result_t>::iterator prediction = detect_results.begin();

            if( face.dl_bmp_update)
            {
              draw_detection_result((uint8_t *)frame->buf, frame->height, frame->width, detect_results);
            }

            face.detect_score = (unsigned int)( prediction->score);

            face.eye_left[0] = (unsigned int)(prediction->keypoint[0]);
            face.eye_left[1] = (unsigned int)(prediction->keypoint[1]);

            face.eye_right[0] = (unsigned int)(prediction->keypoint[6]);
            face.eye_right[1] = (unsigned int)(prediction->keypoint[7]);

            face.nose[0] = (unsigned int)prediction->keypoint[4];
            face.nose[1] = (unsigned int)prediction->keypoint[5];

            face.mouth_left[0] = (unsigned int)prediction->keypoint[2];
            face.mouth_left[1] = (unsigned int)prediction->keypoint[3];

            face.mouth_right[0] = (unsigned int)prediction->keypoint[8];
            face.mouth_right[1] = (unsigned int)prediction->keypoint[9];
#if _USE_FACE_RECOGNITION
            switch(face.recog_cmd)
            {
              default : break;

              case ENROLL:
                  if( face.dl_model_select == MODEL_8bit)
                  {
                    recognizer_8->enroll_id((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3},
                        detect_results.front().keypoint, converToString(face.save_name, 32), true);
                  }
                  else
                  {
                    recognizer_16->enroll_id((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3},
                        detect_results.front().keypoint, converToString(face.save_name, 32), true);
                  }
                break;

              case RECOGNIZE:
                  if( face.dl_model_select == MODEL_8bit)
                  {
                    recognize_result = recognizer_8->recognize((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_results.front().keypoint);
                  }
                  else
                  {
                    recognize_result = recognizer_16->recognize((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_results.front().keypoint);
                  }

                  face.recog_result = (uint8_t)recognize_result.id;
                  face.recog_score = (uint16_t)recognize_result.similarity;

                  // strcpy((char *)face.load_name, recognize_result.name);
                break;

              case DELETE :
                vTaskDelay(10);

                if( face.dl_model_select == MODEL_8bit)
                  recognizer_8->delete_id(face.recog_option, true);
                else
                  recognizer_8->delete_id(face.recog_option, true);
                break;
            }
#endif
            face.is_detect = true;
            face.recog_error = RECOG_ERR_NONE;
          }
          else if (detect_results.size() > 1)
          {
            face.recog_error = RECOG_ERR_TOO_MANY_INFO;
            face.is_detect = false;
          }
          else
          {
            face.recog_error = RECOG_ERR_EMPTY_INFO;
            face.is_detect = false;
          }

        }

        if (return_frambuffer) esp_camera_fb_return(frame);
        else free(frame);

        break;

      case CAM_AP_MOVE_DETECT :
        {
          if (xQueueReceive(info.xQueueFrameI, &frame_2, portMAX_DELAY))
          {
            latency.start();

            unsigned long moving_point_number = dl::image::get_moving_point_number((uint16_t *)frame->buf, (uint16_t *)frame_2->buf, frame->height, frame->width, 2, 40);

            latency.end();
            if (moving_point_number > 50)
            {
                // dl::image::draw_filled_rectangle((uint16_t *)frame2->buf, frame2->height, frame2->width, 0, 0, 20, 20);
                //is_moved = true;
              info.move_state = 1;
            }
            else
            {
              info.move_state = 0;
            }

            if( info.enable_move_row_data == 1 || 1)  // test version
              info.move_row_data = (uint16_t)moving_point_number;

          }
        }
        if (return_frambuffer)
        {
          esp_camera_fb_return(frame);
          esp_camera_fb_return(frame_2);
        }
        break;

      case CAM_AP_COLOR_DETECT :
        {
          latency.start();

          std::vector<std::vector<color_detect_result_t>> &results = detector_color.detect((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3} );

          latency.end();

          color_buff_idx = 0;
          decrease_color = 0;

          // 10 * 1
          for( int m=0; m<12; m++)
            color_info.detect_cnt[m] = 0;

          for (int i = 0; i < results.size(); ++i)
          {
            draw_color_detection_result((uint16_t *)frame->buf, (int)frame->height, (int)frame->width, results[i], draw_lcd_color[i % draw_colors_num], i);
          }
        }
        if (return_frambuffer) esp_camera_fb_return(frame);
        else free(frame);

        break;
#if NOT_YET
      case CAM_AP_QR_DECODE :
        // Processing task: gets an image from the queue, runs QR code detection and recognition.
        // If a QR code is detected, classifies the QR code and displays the result (color fill or a picture) on the display.
        {
          struct quirc *qr = quirc_new();
          assert(qr);

          int qr_width = 80;
          int qr_height = 80;

          if (quirc_resize(qr, qr_height, qr_height) < 0)
          {
            ESP_LOGE("QR", "Failed to allocate QR buffer");
            return;
          }
          /*
          ESP_LOGI("QR", "Processing task ready");

          uint8_t *qr_buf = quirc_begin(qr, NULL, NULL);

          rgb565_to_grayscale_buf(frame->buf, qr_buf, qr_width, qr_height);

          // Process the frame. This step find the corners of the QR code (capstones)
          quirc_end(qr);
          int count = quirc_count(qr);

          quirc_decode_error_t err = QUIRC_ERROR_DATA_UNDERFLOW;

          // If a QR code was detected, try to decode it:
          for (int i = 0; i < count; i++)
          {
            struct quirc_code code = {};
            struct quirc_data qr_data = {};
            // Extract raw QR code binary data (values of black/white modules)
            quirc_extract(qr, i, &code);
            quirc_flip(&code);

            // Decode the raw data. This step also performs error correction.
            err = quirc_decode(&code, &qr_data);
            if (err != 0)
            {
              ESP_LOGE("QR", "QR err: %d", err);
            }
            else
            {
              // Indicate that we have successfully decoded something by blinking an LED
              ESP_LOGI("QR", "QR code: %d bytes: '%s'", qr_data.payload_len, qr_data.payload);
            }
          }
          */

          if (return_frambuffer) esp_camera_fb_return(frame);
          else free(frame);

        }
      break;        // Not Dev Yet
#endif
      case CAM_AP_APRIL_TAG :
      {
          image_u8_t im = {
              .width = (int32_t)frame->width,
              .height = (int32_t)frame->height,
              .stride = (int32_t)frame->width,
              .buf = frame->buf
          };


          ESP_LOGI("APRIL", "CAM_AP_APRIL_TAG");

          zarray_t *detections = apriltag_detector_detect(td, &im);

          for (int i = 0; i < zarray_size(detections); i++)
          {
              apriltag_detection_t *det;
              zarray_get(detections, i, &det);
              ESP_LOGE("TAG","%d, ",det->id);
          }

          apriltag_detections_destroy(detections);

          if (return_frambuffer) esp_camera_fb_return(frame);
          else free(frame);

      }
      break;

      case CAM_AP_CAT_FACE_DETECT : break;  // Not Dev Yet

      default : break;
    }
  }


  if( info.ap_mode_change != info.cam_ap_select)  // reinit
  {
    // change to new mode after return fb ...
    ESP_LOGE("CAM", "Mode Change[%d] -> [%d]", info.cam_ap_select, info.ap_mode_change);

    apCameraDeInit();

    info.cam_ap_select = info.ap_mode_change;

    apCameraInit();
  }
}

std::string converToString(char *a, int size)
{
  int i;
  std::string s ="";
  for (i=0; i<size; i++)
  {
    s += a[i];
  }

  return s;

}

void draw_detection_result(uint16_t *image_ptr, int image_height, int image_width, std::list<dl::detect::result_t> &results)
{
    int i = 0;
    for (std::list<dl::detect::result_t>::iterator prediction = results.begin(); prediction != results.end(); prediction++, i++)
    {
        dl::image::draw_hollow_rectangle(image_ptr, image_height, image_width,
                                         DL_MAX(prediction->box[0], 0),
                                         DL_MAX(prediction->box[1], 0),
                                         DL_MAX(prediction->box[2], 0),
                                         DL_MAX(prediction->box[3], 0),
                                         0b1110000000000111);

        if (prediction->keypoint.size() == 10)
        {
            dl::image::draw_point(image_ptr, image_height, image_width, DL_MAX(prediction->keypoint[0], 0), DL_MAX(prediction->keypoint[1], 0), 4, 0b0000000011111000); // left eye
            dl::image::draw_point(image_ptr, image_height, image_width, DL_MAX(prediction->keypoint[2], 0), DL_MAX(prediction->keypoint[3], 0), 4, 0b0000000011111000); // mouth left corner
            dl::image::draw_point(image_ptr, image_height, image_width, DL_MAX(prediction->keypoint[4], 0), DL_MAX(prediction->keypoint[5], 0), 4, 0b1110000000000111); // nose
            dl::image::draw_point(image_ptr, image_height, image_width, DL_MAX(prediction->keypoint[6], 0), DL_MAX(prediction->keypoint[7], 0), 4, 0b0001111100000000); // right eye
            dl::image::draw_point(image_ptr, image_height, image_width, DL_MAX(prediction->keypoint[8], 0), DL_MAX(prediction->keypoint[9], 0), 4, 0b0001111100000000); // mouth right corner
        }
    }
}

void draw_detection_result(uint8_t *image_ptr, int image_height, int image_width, std::list<dl::detect::result_t> &results)
{
    int i = 0;
    for (std::list<dl::detect::result_t>::iterator prediction = results.begin(); prediction != results.end(); prediction++, i++)
    {
        dl::image::draw_hollow_rectangle(image_ptr, image_height, image_width,
                                         DL_MAX(prediction->box[0], 0),
                                         DL_MAX(prediction->box[1], 0),
                                         DL_MAX(prediction->box[2], 0),
                                         DL_MAX(prediction->box[3], 0),
                                         0x00FF00);

        if (prediction->keypoint.size() == 10)
        {
            dl::image::draw_point(image_ptr, image_height, image_width, DL_MAX(prediction->keypoint[0], 0), DL_MAX(prediction->keypoint[1], 0), 4, 0x0000FF); // left eye
            dl::image::draw_point(image_ptr, image_height, image_width, DL_MAX(prediction->keypoint[2], 0), DL_MAX(prediction->keypoint[3], 0), 4, 0x0000FF); // mouth left corner
            dl::image::draw_point(image_ptr, image_height, image_width, DL_MAX(prediction->keypoint[4], 0), DL_MAX(prediction->keypoint[5], 0), 4, 0x00FF00); // nose
            dl::image::draw_point(image_ptr, image_height, image_width, DL_MAX(prediction->keypoint[6], 0), DL_MAX(prediction->keypoint[7], 0), 4, 0xFF0000); // right eye
            dl::image::draw_point(image_ptr, image_height, image_width, DL_MAX(prediction->keypoint[8], 0), DL_MAX(prediction->keypoint[9], 0), 4, 0xFF0000); // mouth right corner
        }
    }
}

void *app_camera_decode(camera_fb_t *fb)
{
    if (fb->format == PIXFORMAT_RGB565)
    {
        return (void *)fb->buf;
    }
    else
    {
        uint8_t *image_ptr = (uint8_t *)malloc(fb->height * fb->width * 3 * sizeof(uint8_t));
        if (image_ptr)
        {
            if (fmt2rgb888(fb->buf, fb->len, fb->format, image_ptr))
            {
                return (void *)image_ptr;
            }
            else
            {
                dl::tool::free_aligned(image_ptr);
            }
        }
    }
    return NULL;
}

static void draw_color_detection_result(uint16_t *image_ptr, int image_height, int image_width, std::vector<color_detect_result_t> &results, uint16_t color, int idx)
{
  if (results.size() == 0) return;

  vector<int> id(results.size(), 0);

  for (int i = 0; i < id.size(); i++)
  {
    id[i] = i;
  }

  // 면적이 작아지는 순으로의 index 획득
  sort(id.begin(), id.end(),
      [&](const int& a, const int& b) {
        return (results[a].box[2] * results[a].box[3] > results[b].box[2] * results[b].box[3]);
      }
  );


  for (int i = 0; i < results.size(); ++i)
  {
    if( color_info.dl_bmp_update == true)
    {
      dl::image::draw_hollow_rectangle(image_ptr, image_height, image_width,
                                       results[i].box[0],
                                       results[i].box[1],
                                       results[i].box[2],
                                       results[i].box[3],
                                       color);
    }

    if( idx < 12 && i < 10)
    {
      color_info.detect_cnt[idx] += 1; //1

      // color_info에 저장할때 id[i] 위치에 저장함으로써 면적이 작아지는 순으로 저장되도록 함
      color_info.c_data[idx][id[i]].x = results[i].box[0];
      color_info.c_data[idx][id[i]].y = results[i].box[1];
      color_info.c_data[idx][id[i]].w = results[i].box[2];
      color_info.c_data[idx][id[i]].h = results[i].box[3];

      //ESP_LOGE("C", "Detect[%d][%d]->[%d][%d][%d][%d]", idx, i, results[i].box[0], results[i].box[1], results[i].box[2], results[i].box[3]);
    }
  }


  /*
  // red인 경우만 출력 확인
  if (idx == 0)
  {
    for (int i = 0; i < results.size(); ++i)
    {
      ESP_LOGI("C", "Detect[%d][%d]->[%5d][%5d][%7d]", idx, i,
          color_info.c_data[idx][i].w,
          color_info.c_data[idx][i].h,
          color_info.c_data[idx][i].w * color_info.c_data[idx][i].h);
    }
  }
  */
}



static uint8_t rgb565_to_grayscale(const uint8_t *img)
{
    uint16_t *img_16 = (uint16_t *) img;
    rgb565_t rgb = {.val = __builtin_bswap16(*img_16)};
    uint16_t val = (rgb.r * 8 + rgb.g * 4 + rgb.b * 8) / 3;
    return (uint8_t) CRYS_MIN(255, val);
}

static void rgb565_to_grayscale_buf(const uint8_t *src, uint8_t *dst, int qr_width, int qr_height)
{
    for (size_t y = 0; y < qr_height; y++) {
        for (size_t x = 0; x < qr_width; x++) {
            dst[y * qr_width + x] = rgb565_to_grayscale(&src[(y * qr_width + x) * 2]);
        }
    }
}
