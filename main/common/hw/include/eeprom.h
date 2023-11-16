/*
 * eeprom.h
 *
 *  Created on: 2019. 9. 24.
 *      Author: HanCheol Cho
 */

#ifndef SRC_COMMON_HW_INCLUDE_EEPROM_H_
#define SRC_COMMON_HW_INCLUDE_EEPROM_H_


#ifdef __cplusplus
 extern "C" {
#endif



#include "hw_def.h"

#ifdef _USE_HW_EEPROM



#define EEP_MAGIC     0x55AA


typedef struct
{
  data_t   value;
  uint16_t magic;
  uint16_t crc;
} eep_data_t;



bool     eepromInit();
bool     eepromIsInit(void);
bool     eepromValid(uint32_t addr);
uint8_t  eepromReadByte(uint32_t addr);
bool     eepromWriteByte(uint32_t addr, uint8_t data_in);
bool     eepromRead(uint32_t addr, uint8_t *p_data, uint32_t length);
bool     eepromWrite(uint32_t addr, uint8_t *p_data, uint32_t length);
uint32_t eepromGetLength(void);
bool     eepromFormat(void);

uint8_t  eepromGetError(void);
bool     eepromReadData(uint32_t addr, data_t *p_data);
bool     eepromWriteData(uint32_t addr, data_t *p_data);

#endif


#ifdef __cplusplus
}
#endif



#endif /* SRC_COMMON_HW_INCLUDE_EEPROM_H_ */
