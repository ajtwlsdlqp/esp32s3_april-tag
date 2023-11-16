/*
 * hw.h
 *
 *  Created on: 2021. 1. 8.
 *      Author: HanCheol Cho
 */

#ifndef MAIN_HW_HW_H_
#define MAIN_HW_HW_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "hw_def.h"


#include "led.h"
#include "button.h"
#include "uart.h"
#include "cli.h"
#include "gpio.h"
#include "spi.h"
#include "camera.h"
#include "ir_remote.h"
#include "esp_port.h"
#include "cdc.h"

void hwInit(void);


#ifdef __cplusplus
}
#endif

#endif /* MAIN_HW_HW_H_ */
