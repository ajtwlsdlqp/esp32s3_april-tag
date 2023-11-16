/*
 * bsp.h
 *
 *  Created on: 2021. 1. 8.
 *      Author: HanCheol Cho
 */

#ifndef MAIN_BSP_BSP_H_
#define MAIN_BSP_BSP_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "../common/def.h"


#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "soc/rtc.h"
#include "soc/io_mux_reg.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_task_wdt.h"
#include "esp_task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_intr_alloc.h"
#include "driver/gpio.h"




#define logPrintf(...) printf(__VA_ARGS__)


void bspInit(void);
void bspDeInit(void);
uint32_t bspGetCpuFreqMhz(void);

extern void delay(uint32_t delay_ms);
extern uint32_t IRAM_ATTR millis(void);
extern uint32_t IRAM_ATTR micros(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_BSP_BSP_H_ */
