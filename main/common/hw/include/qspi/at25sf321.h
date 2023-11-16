/*
 * at25sf321.h
 *
 *  Created on: 2020. 4. 10.
 *      Author: HanCheol Cho
 */

#ifndef SRC_COMMON_HW_INCLUDE_QSPI_AT25SF321_H_
#define SRC_COMMON_HW_INCLUDE_QSPI_AT25SF321_H_


#ifdef __cplusplus
 extern "C" {
#endif


#include "hw_def.h"


#if defined(_USE_HW_QSPI) && HW_QSPI_DRIVER == AT25SF321

#include "qspi.h"


bool at25sf321InitDriver(qspi_driver_t *p_driver);


#endif

#ifdef __cplusplus
}
#endif


#endif /* SRC_COMMON_HW_INCLUDE_QSPI_AT25SF321_H_ */
