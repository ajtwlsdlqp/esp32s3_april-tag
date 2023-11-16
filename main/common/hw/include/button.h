/*
 * button.h
 *
 *  Created on: 2020. 2. 1.
 *      Author: Baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_BUTTON_H_
#define SRC_COMMON_HW_INCLUDE_BUTTON_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"

#ifdef _USE_HW_BUTTON

#define BUTTON_MAX_CH       HW_BUTTON_MAX_CH



void     button_isr(void);
bool     buttonInit(void);
bool     buttonIsInit(void);
void     buttonEnable(bool enable);
uint8_t  buttonGetPressedCount(void);
void     buttonResetTime(uint8_t ch);
void     buttonSetRepeatTime(uint8_t ch, uint32_t detect_ms, uint32_t repeat_delay_ms, uint32_t repeat_ms);
bool     buttonGetPressed(uint8_t ch);
bool     buttonGetPressedEvent(uint8_t ch);
uint32_t buttonGetPressedTime(uint8_t ch);
uint32_t buttonGetRepeatEvent(uint8_t ch);

bool     buttonGetReleased(uint8_t ch);
bool     buttonGetReleasedEvent(uint8_t ch);
uint32_t buttonGetReleasedTime(uint8_t ch);

bool     buttonOsdGetPressed(uint8_t ch);

bool     buttonWasAnyInputForTimeMs(uint32_t time_ms);

#endif

#ifdef __cplusplus
}
#endif



#endif /* SRC_COMMON_HW_INCLUDE_BUTTON_H_ */
