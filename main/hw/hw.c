/*
 * hw.c
 *
 *  Created on: 2021. 1. 8.
 *      Author: HanCheol Cho
 */




#include "hw.h"



void hwInit(void)
{
  bspInit();

  cliInit();
  ledInit();
  buttonInit();

//  cameraInit();
//  delay(500);

  uartInit();
  //gpioInit();

  cliOpen(_DEF_UART1, 115200);
  uartOpen(_DEF_UART2, 1000000);
  uartOpen(_DEF_UART4, 1000000);

//  irRemoteInit();
  //cameraInit();

  logPrintf("[ ] Free heap: %d\n", esp_get_free_heap_size());
}
