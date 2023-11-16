/*
 * ap_ir_remote.h
 *
 *  Created on: Feb 6, 2023
 *      Author: LDH
 */

#ifndef MAIN_AP_AP_IR_REMOTE_H_
#define MAIN_AP_AP_IR_REMOTE_H_

#include "hw.h"

#ifdef __cplusplus
 extern "C" {
#endif

 void clearIrBuffer (void);

 uint8_t GetIrRxData (void);
 uint8_t SendIrTxData (void);

 void irTestDefaultAc(uint8_t mode);

#ifdef __cplusplus
}
#endif




#endif /* MAIN_AP_AP_IR_REMOTE_H_ */
