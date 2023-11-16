/*
 * ir_remote.h
 *
 *  Created on: 2021. 1. 15.
 *      Author: HanCheol Cho
 */

#ifndef MAIN_COMMON_HW_INCLUDE_IR_REMOTE_H_
#define MAIN_COMMON_HW_INCLUDE_IR_REMOTE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "hw_def.h"
#include "driver/rmt.h"

#ifdef _USE_HW_IR_REMOTE



bool irRemoteInit(void);

RingbufHandle_t *pGetIrRingBuff (void);
rmt_channel_t GetIrRxChannel (void);
rmt_channel_t GetIrTxChannel (void);

#endif

#ifdef __cplusplus
}


#endif



#endif /* MAIN_COMMON_HW_INCLUDE_IR_REMOTE_H_ */
