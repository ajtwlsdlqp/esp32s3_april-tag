/*
 * ap_camera.h
 *
 *  Created on: Feb 7, 2023
 *      Author: LDH
 */

typedef enum
{
    COLOR_DETECTION_IDLE = 0,
    OPEN_REGISTER_COLOR_BOX,
    CLOSE_REGISTER_COLOR_BOX,
    REGISTER_COLOR,
    DELETE_COLOR,
    INCREASE_COLOR_AREA,
    DECREASE_COLOR_AREA,
    SWITCH_RESULT,

    COLOR_ERROR_DETECT,
} color_detection_state_t;

typedef enum
{
    IDLE = 0,
    DETECT,
    ENROLL,
    RECOGNIZE,
    DELETE,
} recognizer_state_t;

typedef enum
{
  MODEL_8bit = 0,
  MODEL_16bit,

  MODEL_NB
} recognizer_model_t;

typedef enum
{
  RECOG_ERR_NONE = 0,

  RECOG_ERR_EMPTY_INFO,
  RECOG_ERR_TOO_MANY_INFO,

} recognizer_error_t;


void apCameraInit(void);
void apCameraDeInit(void);

void apCameraLoop (void);
void apCameraEventLoop (void);
