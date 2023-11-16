/*
 * camera.c
 *
 *  Created on: 2021. 1. 13.
 *      Author: HanCheol Cho
 */




#include "camera.h"
#include "cli.h"
// #define dev_board



#define CAM_WIDTH   (320)
#define CAM_HIGH    (240)



#define CAM_PIN_PWDN		-1  //power down is not used
#define CAM_PIN_RESET		45 //software reset will be performed
#define CAM_PIN_XCLK		14
#define CAM_PIN_SIOD		18
#define CAM_PIN_SIOC		8

#ifdef dev_board
#define CAM_PIN_D7			11
#define CAM_PIN_D6			10
#define CAM_PIN_D5			7
#define CAM_PIN_D4			6
#define CAM_PIN_D3			5
#define CAM_PIN_D2			4
#define CAM_PIN_D1			13
#define CAM_PIN_D0			12
#else
#define CAM_PIN_D7			13
#define CAM_PIN_D6			12
#define CAM_PIN_D5			11
#define CAM_PIN_D4			10
#define CAM_PIN_D3			7
#define CAM_PIN_D2			6
#define CAM_PIN_D1			5
#define CAM_PIN_D0			4
#endif

#define CAM_PIN_VSYNC		17
#define CAM_PIN_HREF 		47
#define CAM_PIN_PCLK 		21


#define CONFIG_CAMERA_JPEG_MODE


static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sscb_sda = CAM_PIN_SIOD,
    .pin_sscb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 10000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_RGB565, //YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_QVGA,    //QQVGA-UXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 12,  //0-63 lower number means higher quality
    .fb_count = 2,       //if more than one, i2s runs in continuous mode. Use only with JPEG
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    //.grab_mode = CAMERA_GRAB_LATEST,
    //.fb_location = CAMERA_FB_IN_DRAM,
};

static bool is_init = false;
static bool is_connected = false;
static esp_err_t camera_error_statae;
static QueueHandle_t xQueueFrameO = NULL;

static void threadCameraHandler(void *arg);

#ifdef _USE_HW_CLI
static void cliCamera(cli_args_t *args);
#endif

static void threadCameraHandler(void *arg)
{
//  cliPrintf("threadCameraHandler \n");
  while (true)
  {
    camera_fb_t *frame = esp_camera_fb_get();
    if (frame)
    {
      xQueueSend(xQueueFrameO, &frame, 0xFFFFFFFF);
    }

    taskYIELD();
  }
}

bool registerCamera(const pixformat_t pixel_fromat,
                    const framesize_t frame_size,
                    const uint8_t fb_count,
                    const QueueHandle_t frame_o)
{
//  cliPrintf("register_camera \n");

  camera_config.pixel_format = pixel_fromat;
  camera_config.frame_size = frame_size;
  camera_config.fb_count = fb_count;
  // camera init
  esp_err_t err = esp_camera_init(&camera_config);
  camera_error_statae = err;

  if (err == ESP_OK)
  {
    is_init = true;
//    cliPrintf("[O] \tinit OK\n");
  }
  else
  {
//    cliPrintf("[X] \tinit fail\n");
    return err;
  }

  if( is_init)
  {
    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1); //flip it back
    s->set_framesize(s, frame_size);
    s->set_awb_gain(s, true);
    s->set_whitebal(s, true);

    //initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID)
    {
        s->set_brightness(s, 1);  //up the blightness just a bit
        s->set_saturation(s, -2); //lower the saturation
    }

    is_connected = true;
  }

  xQueueFrameO = frame_o;

  xTaskCreate((TaskFunction_t)threadCameraHandler, "threadCameraHandler", 2 * 1024, NULL, 5, NULL);
//  cliPrintf("[O] \tstart..\n");

#ifdef _USE_HW_CLI
  cliAdd("cam", cliCamera);
#endif

  err = is_init;

  return err;
}

bool cameraInit(void)
{
  bool ret = true;
  esp_err_t esp_ret;


//  cliPrintf("[ ] camera\n");


  esp_ret = esp_camera_init(&camera_config);
  camera_error_statae = esp_ret;

  if (esp_ret == ESP_OK)
  {
    is_init = true;
//    cliPrintf("[O] \tinit OK\n");
  }
  else
  {
//    cliPrintf("[X] \tinit fail\n");
  }


  if (is_init == true)
  {
    sensor_t * s = esp_camera_sensor_get();

    s->set_vflip(s, 1);//flip it back
    s->set_framesize(s, FRAMESIZE_QVGA);
    s->set_awb_gain(s, true);
    s->set_whitebal(s, true);


    cliPrintf("[O] \tstart..\n");

    is_connected = true;
  }

#ifdef _USE_HW_CLI
  cliAdd("cam", cliCamera);
#endif

  ret = is_init;

  return ret;
}

bool cameraIsInit(void)
{
  return is_init;
}

esp_err_t getCameraError(void)
{
	return camera_error_statae;
}

bool cameraTake(camera_info_t *p_info)
{
  bool ret = true;

  p_info->fb = esp_camera_fb_get();

  if (p_info->fb != NULL)
  {
    p_info->cam_buf = p_info->fb->buf;
    p_info->length = p_info->fb->len;
    p_info->is_valid = true;
  }
  else
  {
    ret = false;
    p_info->is_valid = false;
  }

  return ret;
}


bool cameraRelease(camera_info_t *p_info)
{
  bool ret = true;

  if (p_info->fb)
  {
    esp_camera_fb_return(p_info->fb);
    p_info->fb = NULL;
  }

  p_info->is_valid = false;
  return ret;
}

void cliCamera(cli_args_t *args)
{
#if 0
  camera_info_t cam_info_;
  if (args->argc == 1 && args->isStr(0, "conv"))
  {
    int64_t fr_start = esp_timer_get_time();

   // fb = esp_camera_fb_get();
    cliPrintf("bmp_httpd_handler \n!");
    delay(20);
    cameraTake(&cam_info_);

    cliPrintf("cameraTake \n!");
    delay(20);

    if (cam_info_.is_valid != true)
    {
      cliPrintf("Camera capture failed \n!");
      delay(20);
    }

    uint8_t * buf;
    size_t buf_len = 0;

    bool converted = frame2bmp( cam_info_.fb, &buf, &buf_len);

    cliPrintf("frame2bmp[%d] \n!", converted);
    delay(20);

    cameraRelease(&cam_info_);
    cliPrintf("cameraRelease \n!");
    delay(20);

    if( !converted)
    {
      cliPrintf("BMP conversion failed \n!");
      delay(20);
    }

    free(buf);
    cliPrintf("free \n!");
    delay(20);

    int64_t fr_end = esp_timer_get_time();
    cliPrintf("esp_timer_get_time \n!");
    delay(20);

    cliPrintf("BMP: %uKB %ums", (uint32_t)(buf_len/1024), (uint32_t)((fr_end - fr_start)/1000));
    delay(20);

    for(uint32_t i=0; i<buf_len; i++ )
    {
      if( i%40 == 0) cliPrintf("\n");
      cliPrintf("%02X ", *(buf+i) );
      delay(10);
    }
  }
  else if (args->argc == 1 && args->isStr(0, "buf"))
  {
    cameraTake(&cam_info_);

    cliPrintf("cameraTake \n!");
    delay(20);

    if (cam_info_.is_valid != true)
    {
      cliPrintf("Camera capture failed \n!");
      delay(20);
    }

    for(uint32_t i=0; i<cam_info_.length; i++ )
    {
      if( i%40 == 0) cliPrintf("\n");
      cliPrintf("%02X ", *(cam_info_.cam_buf+i) );
      delay(10);
    }
  }
#else
  return;
#endif
}
