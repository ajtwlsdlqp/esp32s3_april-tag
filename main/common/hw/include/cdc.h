/*
 * cdc.h
 *
 *  Created on: 2023. 9. 7.
 *      Author: Empe
 */

#ifndef MAIN_COMMON_HW_INCLUDE_CDC_H_
#define MAIN_COMMON_HW_INCLUDE_CDC_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"


#ifdef _USE_HW_UART


bool uartUsbCdcInit(void);
uint32_t uartUsbCdcAvailable(void);
uint8_t  uartUsbCdcRead(void);
void     uartUsbCdcDataIn(uint8_t rx_data);
uint32_t uartUsbCdcWrite(uint8_t *p_data, uint32_t length);


#endif

#ifdef __cplusplus
}
#endif




#endif /* MAIN_COMMON_HW_INCLUDE_CDC_H_ */
