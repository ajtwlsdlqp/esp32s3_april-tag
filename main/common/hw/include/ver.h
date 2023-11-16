/*
 * ver.h
 *
 *  Created on: 2020. 11. 13.
 *      Author: Jason
 */

#ifndef SRC_COMMON_HW_INCLUDE_VER_H_
#define SRC_COMMON_HW_INCLUDE_VER_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "hw_def.h"


typedef enum
{
  PBA_VER_REVA = 1,

  PBA_VER_REVB = 2,  // mic 1ea -> 2ea,
                     // ir receiver resistor 10k -> 1k.

  PBA_VER_REVC = 3,  // voltage detection divider resistor 2.2k/3.9k -> 100k/47k,
                     // support wireless charging detection
                     // number of mcu pins used for charging detection: 1 -> 2
} PbaVer;

typedef enum
{
  PBA_VER_ERR_OK = 0,
  PBA_VER_ERR_MAGIC_CODE,
  PBA_VER_ERR_VALIDATION,
  PBA_VER_ERR_ERASED,
  PBA_VER_ERR_GARBAGE,
  PBA_VER_ERR_UNKNOWN_TAG_VERSION,
  PBA_VER_ERR_UNKNOWN_PBA_VERSION,
} PbaVerError;



bool verInit(void);
bool verIsInit(void);
uint8_t verGetPbaVersion(void);
uint8_t verGetPbaVersionChar(void);
uint8_t verGetIrType(void);
uint8_t verGetMicType(void);
uint8_t verGetPogoPinType(void);
PbaVerError verGetError(void);


#ifdef __cplusplus
}
#endif


#endif /* SRC_COMMON_HW_INCLUDE_VER_H_ */
