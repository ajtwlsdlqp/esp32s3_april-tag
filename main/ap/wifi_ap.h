/*
 * wifi_ap.h
 *
 *  Created on: 2022. 10. 5.
 *      Author: LDH
 */

#ifndef MAIN_AP_WIFI_AP_H_
#define MAIN_AP_WIFI_AP_H_

#include "hw.h"

#ifdef __cplusplus
 extern "C" {
#endif

 bool getStartStream(void);

 int getWebSocket(void);
 void setCloseSocket(void);

#ifdef __cplusplus
}
#endif


#endif /* MAIN_AP_WIFI_AP_H_ */





