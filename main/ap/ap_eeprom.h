/*
 * ap_eeprom.h
 *
 *  Created on: Jan 30, 2023
 *      Author: LDH
 */

#ifndef MAIN_AP_AP_EEPROM_H_
#define MAIN_AP_AP_EEPROM_H_

#include "hw.h"

#ifdef __cplusplus
 extern "C" {
#endif

/** @defgroup FLASH_Sectors FLASH Sectors
  * @{
  */
#define FLASH_SECTOR_0           ((uint32_t)0U) /*!< Sector Number 0   */
#define FLASH_SECTOR_1           ((uint32_t)1U) /*!< Sector Number 1   */
#define FLASH_SECTOR_2           ((uint32_t)2U) /*!< Sector Number 2   */
#define FLASH_SECTOR_3           ((uint32_t)3U) /*!< Sector Number 3   */
#define FLASH_SECTOR_4           ((uint32_t)4U) /*!< Sector Number 4   */
#define FLASH_SECTOR_5           ((uint32_t)5U) /*!< Sector Number 5   */
#define FLASH_SECTOR_6           ((uint32_t)6U) /*!< Sector Number 6   */
#define FLASH_SECTOR_7           ((uint32_t)7U) /*!< Sector Number 7   */


#define EE_ADDR_BLE_MAC0          (uint16_t)0xB0
#define EE_ADDR_BLE_MAC1          (uint16_t)0xB1
#define EE_ADDR_BLE_MAC2          (uint16_t)0xB2
#define EE_ADDR_BLE_MAC3          (uint16_t)0xB3
#define EE_ADDR_BLE_MAC4          (uint16_t)0xB4
#define EE_ADDR_BLE_MAC5          (uint16_t)0xB5

#define EE_ADDR_BOOT_STATE        (uint16_t)0x0A


typedef enum
{
  HAL_OK       = 0x00U,
  HAL_ERROR    = 0x01U,
  HAL_BUSY     = 0x02U,
  HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;


void saveIrCode (uint8_t index);
void loadIrCode (uint8_t index);

void loadAirbInfo (void);
void saveAirbInfo (void);

#ifdef __cplusplus
}
#endif




#endif /* MAIN_AP_AP_EEPROM_H_ */
