/*
 * mx25l25645.h
 *
 *  Created on: 2020. 5. 19.
 *      Author: HanCheol Cho
 */

#ifndef SRC_COMMON_HW_INCLUDE_QSPI_MX25L25645_H_
#define SRC_COMMON_HW_INCLUDE_QSPI_MX25L25645_H_



#ifdef __cplusplus
 extern "C" {
#endif


#include "hw_def.h"


#if defined(_USE_HW_QSPI) && HW_QSPI_DRIVER == MX25L25645

#include "qspi.h"


bool mx25l25645InitDriver(qspi_driver_t *p_driver);


#endif

#ifdef __cplusplus
}
#endif



#endif /* SRC_COMMON_HW_INCLUDE_QSPI_MX25L25645_H_ */
