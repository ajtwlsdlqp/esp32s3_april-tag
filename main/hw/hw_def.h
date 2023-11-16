/*
 * hw_def.h
 *
 *  Created on: 2021. 1. 8.
 *      Author: HanCheol Cho
 */

#ifndef MAIN_HW_HW_DEF_H_
#define MAIN_HW_HW_DEF_H_


#include "def.h"
#include "bsp.h"



#define _DEF_MODEL_NUMBER                     402
#define _DEF_MODEL_INFO                       0x21091001
#define _DEF_FIRMWARE_VERSION                 ((1<<24)|(0<<16)|(0<<0)) //Major(2).Minor(2).Patch(4)
#define _DEF_FIRMWARE_VERSION_STR             "V210910R1"
#define _DEF_AIRB_ID                          202


#define _HW_DEF_RTOS_THREAD_PRI_SLAVE         ESP_TASK_PRIO_MAX - 3
#define _HW_DEF_RTOS_THREAD_PRI_DETECTOR      ESP_TASK_PRIO_MAX - 3


#define _HW_DEF_RTOS_THREAD_MEM_SLAVE        ( 6*1024 )
#define _HW_DEF_RTOS_THREAD_MEM_DETECTOR     ( 12*1024 )


#define _USE_HW_IR_REMOTE
#define _USE_HW_CTABLE


#define _USE_HW_LED
#define      HW_LED_MAX_CH          3

#define _USE_HW_BUTTON
#define      HW_BUTTON_MAX_CH       1
#define      HW_MODE_BUTTON_CH      0

#define _USE_HW_UART
#define      HW_UART_MAX_CH         4 // HW UART2 websocket 1

#define _USE_HW_CLI
#define      HW_CLI_CMD_LIST_MAX    16
#define      HW_CLI_CMD_NAME_MAX    16
#define      HW_CLI_LINE_HIS_MAX    4
#define      HW_CLI_LINE_BUF_MAX    64

#define _USE_HW_GPIO
#define      HW_GPIO_MAX_CH         1

#define _USE_HW_DXL
#define      HW_DXL_MAX_BUFFER              (1024+20)


#define _USE_HW_CAMERA
#define _USE_HW_OV2640

#define _USE_FACE_RECOGNITION       0

#endif /* MAIN_HW_HW_DEF_H_ */
