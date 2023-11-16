/*
 * esp_port.c
 *
 *  Created on: Sep 15, 2021
 *      Author: Jason
 */

/* ----------------------------------------------------------------------
 * $Date:        5. February 2013
 * $Revision:    V1.02
 *
 * Project:      CMSIS-RTOS API
 * Title:        cmsis_os.c
 *
 * Version 0.02
 *    Initial Proposal Phase
 * Version 0.03
 *    osKernelStart added, optional feature: main started as thread
 *    osSemaphores have standard behavior
 *    osTimerCreate does not start the timer, added osTimerStart
 *    osThreadPass is renamed to osThreadYield
 * Version 1.01
 *    Support for C++ interface
 *     - const attribute removed from the osXxxxDef_t typedef's
 *     - const attribute added to the osXxxxDef macros
 *    Added: osTimerDelete, osMutexDelete, osSemaphoreDelete
 *    Added: osKernelInitialize
 * Version 1.02
 *    Control functions for short timeouts in microsecond resolution:
 *    Added: osKernelSysTick, osKernelSysTickFrequency, osKernelSysTickMicroSec
 *    Removed: osSignalGet
 *
 *
 *----------------------------------------------------------------------------
 *
 * Portions Copyright � 2016 STMicroelectronics International N.V. All rights reserved.
 * Portions Copyright (c) 2013 ARM LIMITED
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  - Neither the name of ARM  nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/


/**
* @brief Wait until a Mutex becomes available
* @param mutex_id      mutex ID obtained by \ref osMutexCreate.
* @param millisec      timeout value or 0 in case of no time-out.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMutexWait shall be consistent in every CMSIS-RTOS.
*/


#include "esp_port.h"

osStatus osMutexWait (osMutexId mutex_id, uint32_t millisec)
{
  TickType_t ticks;
  /*
  portBASE_TYPE taskWoken = pdFALSE;
  */


  if (mutex_id == NULL) {
    return osErrorParameter;
  }

  ticks = 0;
  if (millisec == osWaitForever) {
    ticks = portMAX_DELAY;
  }
  else if (millisec != 0) {
    ticks = millisec / portTICK_PERIOD_MS;
    if (ticks == 0) {
      ticks = 1;
    }
  }

  /*
  if (inHandlerMode()) {
    if (xSemaphoreTakeFromISR(mutex_id, &taskWoken) != pdTRUE) {
      return osErrorOS;
    }
  portEND_SWITCHING_ISR(taskWoken);
  }
  else */
  if (xSemaphoreTake(mutex_id, ticks) != pdTRUE) {
    return osErrorOS;
  }

  return osOK;
}

/**
* @brief Release a Mutex that was obtained by \ref osMutexWait
* @param mutex_id      mutex ID obtained by \ref osMutexCreate.
* @retval  status code that indicates the execution status of the function.
* @note   MUST REMAIN UNCHANGED: \b osMutexRelease shall be consistent in every CMSIS-RTOS.
*/
osStatus osMutexRelease (osMutexId mutex_id)
{
  osStatus result = osOK;
  /*
  portBASE_TYPE taskWoken = pdFALSE;


  if (inHandlerMode()) {
    if (xSemaphoreGiveFromISR(mutex_id, &taskWoken) != pdTRUE) {
      return osErrorOS;
    }
    portEND_SWITCHING_ISR(taskWoken);
  }
  else */
  if (xSemaphoreGive(mutex_id) != pdTRUE)
  {
    result = osErrorOS;
  }
  return result;
}
