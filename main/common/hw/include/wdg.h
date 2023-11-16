/*
 * wdg.h
 *
 *  Created on: 2020. 7. 6.
 *      Author: HanCheol Cho
 */

#ifndef SRC_COMMON_HW_INCLUDE_WDG_H_
#define SRC_COMMON_HW_INCLUDE_WDG_H_



#ifdef __cplusplus
extern "C" {
#endif


#include "hw_def.h"


#ifdef _USE_HW_WDG



bool wdgInit(void);
void wdgSetReload(uint32_t reload_value);
void wdgRefresh(void);

#endif

#ifdef __cplusplus
}
#endif


#endif /* SRC_COMMON_HW_INCLUDE_WDG_H_ */
