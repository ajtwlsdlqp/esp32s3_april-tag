/*
 * ap_eeprom.c
 *
 *  Created on: Jan 30, 2023
 *      Author: LDH
 */

#include <string.h>
#include "ap.h"


#include "esp_spiffs.h"
#include "esp_log.h"
#include "hw.h"

#include "esp_efuse.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_partition.h"
#include "esp_flash_encrypt.h"
#include "esp_efuse_table.h"
#include "nvs_flash.h"

#include "esp_flash_spi_init.h"
#include "esp_flash.h"
#include "esp_flash_internal.h"

#include "ap_eeprom.h"
#include "ir_remote.h"
static bool IsInit = false;

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DEBUG_OUT 0

/* Define of the return value */
#define EE_OK      (uint32_t)HAL_OK
#define EE_ERROR   (uint32_t)HAL_ERROR
#define EE_BUSY    (uint32_t)HAL_BUSY
#define EE_TIMEOUT (uint32_t)HAL_TIMEOUT

/* Variables' number */
#define NB_OF_VAR  ((uint32_t)(PAGE_SIZE/4))

/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint16_t EE_Init(void);
uint16_t EE_ReadVariable(uint16_t VirtAddress, uint16_t* Data);
uint16_t EE_WriteVariable(uint16_t VirtAddress, uint16_t Data);

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Define the size of the sectors to be used */
#define PAGE_SIZE               (uint32_t)0x1000  /* Page size = 4KByte */

/* EEPROM start address in Flash */
// addr = page_size * pg_num 0x1000
#define EEPROM_START_ADDRESS  ((uint32_t)0x007B0000)

/* Pages 0 and 1 base and end addresses */
#define PAGE0_BASE_ADDRESS    ((uint32_t)(EEPROM_START_ADDRESS + 0x0000))
#define PAGE0_END_ADDRESS     ((uint32_t)(EEPROM_START_ADDRESS + (PAGE_SIZE - 1)))
#define PAGE0_ID               FLASH_SECTOR_2

#define PAGE1_BASE_ADDRESS    ((uint32_t)(EEPROM_START_ADDRESS + PAGE_SIZE))
#define PAGE1_END_ADDRESS     ((uint32_t)(EEPROM_START_ADDRESS + (2 * PAGE_SIZE - 1)))
#define PAGE1_ID               FLASH_SECTOR_3


/* Used Flash pages for EEPROM emulation */
#define PAGE0                 ((uint16_t)0x0000)
#define PAGE1                 ((uint16_t)0x0001) /* Page nb between PAGE0_BASE_ADDRESS & PAGE1_BASE_ADDRESS*/

/* No valid page define */
#define NO_VALID_PAGE         ((uint16_t)0x00AB)

/* Page status definitions */
#define ERASED                ((uint32_t)0xFFFFFFFF)     /* Page is empty */
#define RECEIVE_DATA          ((uint32_t)0xEEEEEEEE)     /* Page is marked to receive data */
#define VALID_PAGE            ((uint32_t)0x00000000)     /* Page containing valid data */

/* Valid pages in read and write defines */
#define READ_FROM_VALID_PAGE  ((uint16_t)0x0000)
#define WRITE_IN_VALID_PAGE   ((uint16_t)0x0001)

/* Page full define */
#define PAGE_FULL             ((uint32_t)0x00000080)


#define IR_DATA_START_ADDR    ((uint32_t)0x007B2000)

#define IR_CODE_BLOCK_LANGTH  IR_DISCRIPTION_LANGTH + IR_CODE_DATA_LANGTH
#define IR_DISCRIPTION_LANGTH 32
#define IR_CODE_DATA_LANGTH   480

#define AIRB_DATA_START_ADDR  ((uint32_t)0x007D2000)


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static HAL_StatusTypeDef EE_Format(void);
static uint16_t EE_FindValidPage(uint8_t Operation);
static uint16_t EE_VerifyPageFullWriteVariable(uint16_t VirtAddress, uint16_t Data);
static uint16_t EE_PageTransfer(uint16_t VirtAddress, uint16_t Data);
static uint16_t EE_VerifyPageFullyErased(uint32_t Address);

/* Virtual address defined by the user: 0xFFFF value is prohibited */
static uint16_t VirtAddVarTab[NB_OF_VAR];
uint16_t DataVar = 0;


static void eraseFlashPage (uint16_t page_no);
static void writeFlashData (uint32_t address, uint16_t table_addr, uint16_t data);

extern airb_info_t info;

void cliEeprom(cli_args_t *args);

bool drvEepromInit()
{
  uint16_t i;

  for( i=0; i<NB_OF_VAR; i++ )
  {
    VirtAddVarTab[i] = 0;
  }

  if( EE_Init() == EE_OK )
  {
    IsInit = 1;
  }

#ifdef _USE_HW_CLI
  cliAdd("eep", cliEeprom);
#endif

  return IsInit;
}



uint16_t drvEepromReadByte(uint32_t addr)
{
  uint16_t read_value;


  if( IsInit == false ) return 0;

  EE_ReadVariable((uint16_t) addr,  &read_value);

  return (uint16_t)read_value;
}


bool drvEepromWriteByte(uint32_t index, uint8_t data_in)
{
  if( IsInit == false ) return false;

  if (EE_WriteVariable((uint16_t) index, (uint16_t)data_in) == EE_OK)
  {
    return true;
  }
  else
  {
    return false;
  }

  return true;
}

uint32_t drvEepromGetLength(void)
{
  if( IsInit == false ) return 0;

  return NB_OF_VAR;
}


bool drvEepromFormat(void)
{
  if (EE_Format() == HAL_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}



/**
  * @brief  Restore the pages to a known good state in case of page's status
  *   corruption after a power loss.
  * @param  None.
  * @retval - Flash error code: on write Flash error
  *         - FLASH_COMPLETE: on success
  */
uint16_t EE_Init(void)
{
  uint32_t PageStatus0 = 6, PageStatus1 = 6;
  uint32_t VarIdx = 0;
  uint32_t EepromStatus = 0, ReadStatus = 0;
  int16_t x = -1;

  cliPrintf("EE_Init \n\n");

  /* Print chip information */

  esp_chip_info(&info.chip_info);
  cliPrintf("This is %s chip with %d CPU core(s), WiFi%s%s, \r\n",
          CONFIG_IDF_TARGET,
          info.chip_info.cores,
          (info.chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
          (info.chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

  cliPrintf("silicon revision %d, \r\n", info.chip_info.revision);

  cliPrintf("%dMB %s flash \r\n", spi_flash_get_chip_size() / (1024 * 1024),
          (info.chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");


  esp_flash_enc_mode_t mode = esp_get_flash_encryption_mode();
  if (mode == ESP_FLASH_ENC_MODE_DISABLED) {
    cliPrintf("Flash encryption feature is disabled\n");
  } else {
    cliPrintf("Flash encryption feature is enabled in %s mode\n",
          mode == ESP_FLASH_ENC_MODE_DEVELOPMENT ? "DEVELOPMENT" : "RELEASE");
  }

//  const esp_partition_t* partition = esp_partition_find_first(
//      ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
//  assert(partition != NULL);

  info.partition = esp_partition_find_first( ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
  assert(info.partition != NULL);

  // loadAirbInfo();

  /* Verify if Address and Address+2 contents are 0xFFFFFFFF */
  spi_flash_read( PAGE0_BASE_ADDRESS, &PageStatus0, 4);
//  (esp_partition_read(info.partition, 0, &PageStatus0, 4));

  spi_flash_read( PAGE1_BASE_ADDRESS, &PageStatus1, 4);
//  (esp_partition_read(info.partition, PAGE_SIZE, &PageStatus1, 4));

#if DEBUG_OUT
  cliPrintf("Pread -> p0[0x%08X], p1[0x%08X] \r\n", PageStatus0, PageStatus1);
#endif

  /* Check for invalid header states and repair if necessary */
  switch (PageStatus0)
  {
    case ERASED:
      if (PageStatus1 == VALID_PAGE) /* Page0 erased, Page1 valid */
      {
          /* Erase Page0 */
        if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
        {
          eraseFlashPage(PAGE0);
        }
      }
      else if (PageStatus1 == RECEIVE_DATA) /* Page0 erased, Page1 receive */
      {
        /* Erase Page0 */
        if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
        {
          eraseFlashPage(PAGE0);
        }

        /* Mark Page1 as valid */
        writeFlashData(PAGE1_BASE_ADDRESS, VALID_PAGE, VALID_PAGE);
      }
      else /* First EEPROM access (Page0&1 are erased) or invalid state -> format EEPROM */
      {
        /* Erase both Page0 and Page1 and set Page0 as valid page */
        EE_Format();
      }
      break;

    case RECEIVE_DATA:
      if (PageStatus1 == VALID_PAGE) /* Page0 receive, Page1 valid */
      {
        //uint16_t temp_virtaddvar = (esp_partition_read(info.partition, 4, &receive_data, 4)) >> 16;
        uint32_t receive_data = 0;
        uint16_t temp_virtaddvar = 0;
        spi_flash_read( (PAGE0_BASE_ADDRESS+4), &receive_data, 4);
        temp_virtaddvar = (uint16_t)(temp_virtaddvar >> 16);

        /* Transfer data from Page1 to Page0 */
        for (VarIdx = 0; VarIdx < NB_OF_VAR; VarIdx++)
        {
          if (temp_virtaddvar == VirtAddVarTab[VarIdx])
          {
            x = VarIdx;
          }
          if (VarIdx != x)
          {
            /* Read the last variables' updates */
            ReadStatus = EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar);
            /* In case variable corresponding to the virtual address was found */
            if (ReadStatus != 0x1)
            {
              /* Transfer the variable to the Page0 */
              EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar);
              /* If program operation was failed, a Flash error code is returned */
              if (EepromStatus != HAL_OK)
              {
                return EepromStatus;
              }
            }
          }
        }
        /* Mark Page0 as valid */
        writeFlashData(PAGE0_BASE_ADDRESS, VALID_PAGE, VALID_PAGE);

        /* Erase Page1 */
        if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS))
        {
          eraseFlashPage(PAGE1);
        }
      }
      else if (PageStatus1 == ERASED) /* Page0 receive, Page1 erased */
      {
        /* Erase Page1 */
        if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS))
        {
          eraseFlashPage(PAGE1);
        }

        /* Mark Page0 as valid */
        writeFlashData(PAGE0_BASE_ADDRESS, VALID_PAGE, VALID_PAGE);
      }
      else /* Invalid state -> format eeprom */
      {
        /* Erase both Page0 and Page1 and set Page0 as valid page */
        EE_Format();
      }
      break;

    case VALID_PAGE:
      if (PageStatus1 == VALID_PAGE) /* Invalid state -> format eeprom */
      {
        /* Erase both Page0 and Page1 and set Page0 as valid page */
        EE_Format();
      }
      else if (PageStatus1 == ERASED) /* Page0 valid, Page1 erased */
      {
        /* Erase Page1 */
        if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS))
        {
          eraseFlashPage(PAGE1);
        }
      }
      else /* Page0 valid, Page1 receive */
      {
        // uint16_t temp_virtaddvar = (esp_partition_read(info.partition, (4+PAGE_SIZE), &receive_data, 4)) >> 16;
        uint32_t receive_data = 0;
        uint16_t temp_virtaddvar = 0;
        spi_flash_read( (PAGE1_BASE_ADDRESS+4), &receive_data, 4);
        temp_virtaddvar = (uint16_t)(receive_data >> 16);

        /* Transfer data from Page0 to Page1 */
        for (VarIdx = 0; VarIdx < NB_OF_VAR; VarIdx++)
        {
          if ( temp_virtaddvar == VirtAddVarTab[VarIdx])
          {
            x = VarIdx;
          }
          if (VarIdx != x)
          {
            /* Read the last variables' updates */
            ReadStatus = EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar);
            /* In case variable corresponding to the virtual address was found */
            if (ReadStatus != 0x1)
            {
              /* Transfer the variable to the Page1 */
              EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar);
              /* If program operation was failed, a Flash error code is returned */
              if (EepromStatus != HAL_OK)
              {
                return EepromStatus;
              }
            }
          }
        }

        /* Mark Page1 as valid */
        writeFlashData(PAGE0_BASE_ADDRESS, VALID_PAGE, VALID_PAGE);

        /* Erase Page0 */
        if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
        {
          eraseFlashPage(PAGE0);
        }
      }
      break;

    default:  /* Any other state -> format eeprom */
      /* Erase both Page0 and Page1 and set Page0 as valid page */
      EE_Format();
      break;
  }

  return HAL_OK;
}


/**
  * @brief  Verify if specified page is fully erased.
  * @param  Address: page address
  *   This parameter can be one of the following values:
  *     @arg PAGE0_BASE_ADDRESS: Page0 base address
  *     @arg PAGE1_BASE_ADDRESS: Page1 base address
  * @retval page fully erased status:
  *           - 0: if Page not erased
  *           - 1: if Page erased
  */
uint16_t EE_VerifyPageFullyErased(uint32_t Address)
{
  uint32_t ReadStatus = 1;
  uint32_t AddressValue = 0x55555555;
  uint32_t PageEndAddress = PAGE0_END_ADDRESS;


  if (Address == PAGE1_BASE_ADDRESS)
  {
    PageEndAddress = PAGE1_END_ADDRESS;
  }

  /* Check each active page address starting from base address to end address */
  while (Address <= PageEndAddress)
  {
    /* Get the current location content to be compared with virtual address */
    // AddressValue = *( (uint32_t *)Address);

//    (esp_partition_read(info.partition, (Address - PAGE0_BASE_ADDRESS), &AddressValue, 4));
    spi_flash_read( Address, &AddressValue, 4);

    /* Compare the read address with the virtual address */
    if (AddressValue != ERASED)
    {
      /* In case variable value is read, reset ReadStatus flag */
      ReadStatus = 0;
      break;
    }
    /* Next address location */
    Address = Address + 4;
  }

  /* Return ReadStatus value: (0: Page not erased, 1: Sector erased) */
  return ReadStatus;
}

uint16_t EE_ReadVariable(uint16_t VirtAddress, uint16_t* Data)
{
  uint16_t ValidPage = PAGE0;
  uint32_t AddressValue = 0x55555555, ReadStatus = 1;
  uint16_t read_virtAddress = 0, read_virtdata = 0;
  uint32_t Address = EEPROM_START_ADDRESS, PageStartAddress = EEPROM_START_ADDRESS;

  /* Get active Page for read operation */
  ValidPage = EE_FindValidPage(READ_FROM_VALID_PAGE);

  /* Check if there is no valid page */
  if (ValidPage == NO_VALID_PAGE)
  {
    return  NO_VALID_PAGE;
  }

  /* Get the valid Page start Address */
  PageStartAddress = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * PAGE_SIZE));

  /* Get the valid Page end Address */
  Address = (uint32_t)( (EEPROM_START_ADDRESS-4) + (uint32_t)((1 + ValidPage) * PAGE_SIZE));

#if DEBUG_OUT
  cliPrintf("EE_ReadVariable Debug info \t\n");
  cliPrintf("0x%08X 0x%08X \t\n", Address, PageStartAddress );
#endif

  /* Check each active page address starting from end */
  while (Address > (PageStartAddress + 4))
  {
    /* Get the current location content to be compared with virtual address */
    // AddressValue = *( (uint32_t *)Address);  // read 32bit data
//    (esp_partition_read(info.partition, (Address - EEPROM_START_ADDRESS), &AddressValue, 4));
    spi_flash_read(Address, &AddressValue, 4);

    read_virtAddress = (uint16_t)(AddressValue >> 16);
    read_virtdata    = (uint16_t)(AddressValue & 0xFFFF);

    /* Compare the read address with the virtual address */
#if DEBUG_OUT
    cliPrintf("0x%08X -> 0x%04X 0x%04X \t\n", Address, read_virtAddress, read_virtdata );
#endif

    if (read_virtAddress  == VirtAddress)
    {
#if DEBUG_OUT
      cliPrintf("0x%08X -> 0x%04X 0x%04X \t\n", Address, read_virtAddress, read_virtdata );
#endif

      /* Get content of Address-2 which is variable value */
      *Data = read_virtdata;

      /* In case variable value is read, reset ReadStatus flag */
      ReadStatus = 0;

      break;
    }
    else
    {
      /* Next address location */
      Address = Address - 4;
    }
  }

  if (ReadStatus == 1)
  {
    *Data = 0xFF;
  }


  /* Return ReadStatus value: (0: variable exist, 1: variable doesn't exist) */
  return ReadStatus;
}

/**
  * @brief  Writes/upadtes variable data in EEPROM.
  * @param  VirtAddress: Variable virtual address
  * @param  Data: 16 bit data to be written
  * @retval Success or error status:
  *           - FLASH_COMPLETE: on success
  *           - PAGE_FULL: if valid page is full
  *           - NO_VALID_PAGE: if no valid page was found
  *           - Flash error code: on write Flash error
  */
uint16_t EE_WriteVariable(uint16_t VirtAddress, uint16_t Data)
{
  uint16_t Status = 0;

  /* Write the variable virtual address and value in the EEPROM */
  Status = EE_VerifyPageFullWriteVariable(VirtAddress, Data);

  /* In case the EEPROM active page is full */
  if (Status == PAGE_FULL)
  {
    /* Perform Page transfer */
    Status = EE_PageTransfer(VirtAddress, Data);
  }

  /* Return last operation status */
  return Status;
}

/**
  * @brief  Erases PAGE and PAGE1 and writes VALID_PAGE header to PAGE
  * @param  None
  * @retval Status of the last operation (Flash write or erase) done during
  *         EEPROM formating
  */
static HAL_StatusTypeDef EE_Format(void)
{
  /* Erase Page0 */
  if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
  {
    eraseFlashPage(PAGE0);
  }

  writeFlashData(PAGE0_BASE_ADDRESS, VALID_PAGE, VALID_PAGE);


  if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS))
  {
    eraseFlashPage(PAGE1);
  }

  return HAL_OK;
}

/**
  * @brief  Find valid Page for write or read operation
  * @param  Operation: operation to achieve on the valid page.
  *   This parameter can be one of the following values:
  *     @arg READ_FROM_VALID_PAGE: read operation from valid page
  *     @arg WRITE_IN_VALID_PAGE: write operation from valid page
  * @retval Valid page number (PAGE or PAGE1) or NO_VALID_PAGE in case
  *   of no valid page was found
  */
static uint16_t EE_FindValidPage(uint8_t Operation)
{
  uint32_t PageStatus0 = 6, PageStatus1 = 6;

  /* Get Page0 actual status */
//  PageStatus0 =  *( (uint32_t *)PAGE0_BASE_ADDRESS);
//  (esp_partition_read(info.partition, 0, &PageStatus0, 4));
  spi_flash_read(PAGE0_BASE_ADDRESS, &PageStatus0, 4);

  /* Get Page1 actual status */
//  PageStatus1 = *( (uint32_t *)PAGE1_BASE_ADDRESS);
//  (esp_partition_read(info.partition, PAGE_SIZE, &PageStatus1, 4));
  spi_flash_read(PAGE1_BASE_ADDRESS, &PageStatus1, 4);

#if DEBUG_OUT
  cliPrintf("EE_FindValidPage Debug info \t\n");
  cliPrintf("0x%08X 0x%08X \t\n", PageStatus0, PageStatus1 );
#endif

  /* Write or read operation */
  switch (Operation)
  {
    case WRITE_IN_VALID_PAGE:   /* ---- Write operation ---- */
      if (PageStatus1 == VALID_PAGE)
      {
        /* Page0 receiving data */
        if (PageStatus0 == RECEIVE_DATA)
        {
          return PAGE0;         /* Page0 valid */
        }
        else
        {
          return PAGE1;         /* Page1 valid */
        }
      }
      else if (PageStatus0 == VALID_PAGE)
      {
        /* Page1 receiving data */
        if (PageStatus1 == RECEIVE_DATA)
        {
          return PAGE1;         /* Page1 valid */
        }
        else
        {
          return PAGE0;         /* Page0 valid */
        }
      }
      else
      {
        return NO_VALID_PAGE;   /* No valid Page */
      }

    case READ_FROM_VALID_PAGE:  /* ---- Read operation ---- */
#if DEBUG_OUT
      cliPrintf("Debug -> 0x%08X, 0x%08X \n\n", PageStatus0, PageStatus1);
#endif

      if (PageStatus0 == VALID_PAGE)
      {
        return PAGE0;           /* Page0 valid */
      }
      else if (PageStatus1 == VALID_PAGE)
      {
        return PAGE1;           /* Page1 valid */
      }
      else
      {
        return NO_VALID_PAGE ;  /* No valid Page */
      }

    default:
      return PAGE0;             /* Page0 valid */
  }
}

/**
  * @brief  Verify if active page is full and Writes variable in EEPROM.
  * @param  VirtAddress: 16 bit virtual address of the variable
  * @param  Data: 16 bit data to be written as variable value
  * @retval Success or error status:
  *           - FLASH_COMPLETE: on success
  *           - PAGE_FULL: if valid page is full
  *           - NO_VALID_PAGE: if no valid page was found
  *           - Flash error code: on write Flash error
  */
static uint16_t EE_VerifyPageFullWriteVariable(uint16_t VirtAddress, uint16_t Data)
{
  uint16_t ValidPage = PAGE0;
  uint32_t Address = EEPROM_START_ADDRESS, PageEndAddress = EEPROM_START_ADDRESS+PAGE_SIZE;

  esp_err_t err;
  uint32_t read_data;

  /* Get valid Page for write operation */
  ValidPage = EE_FindValidPage(WRITE_IN_VALID_PAGE);

#if DEBUG_OUT
  cliPrintf("EE_Write Debug info : %d  \t\n", ValidPage);
#endif

  /* Check if there is no valid page */
  if (ValidPage == NO_VALID_PAGE)
  {
    return  NO_VALID_PAGE;
  }

  /* Get the valid Page start Address */
  Address = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * PAGE_SIZE));

  /* Get the valid Page end Address */
  PageEndAddress = (uint32_t)((EEPROM_START_ADDRESS - 1) + (uint32_t)((ValidPage + 1) * PAGE_SIZE));

  /* Check each active page address starting from begining */
  while (Address < PageEndAddress)
  {
    /* Verify if Address and Address+2 contents are 0xFFFFFFFF */
//    err = (esp_partition_read(info.partition, (Address - EEPROM_START_ADDRESS), &read_data, 4));
    err = spi_flash_read( Address, &read_data, 4);

#if DEBUG_OUT
    cliPrintf("Reading with esp_partition_read E[%d] \r\n", err);
#endif

//    if ( *( (uint32_t *)Address) == 0xFFFFFFFF)
    if( read_data == 0xFFFFFFFF)
    {
      writeFlashData (Address, VirtAddress, Data);
      /* Set variable data */
      return HAL_OK;
    }
    else
    {
      /* Next address location */
      Address = Address + 4;
    }
  }

  /* Return PAGE_FULL in case the valid page is full */
  return PAGE_FULL;
}

/**
  * @brief  Transfers last updated variables data from the full Page to
  *   an empty one.
  * @param  VirtAddress: 16 bit virtual address of the variable
  * @param  Data: 16 bit data to be written as variable value
  * @retval Success or error status:
  *           - FLASH_COMPLETE: on success
  *           - PAGE_FULL: if valid page is full
  *           - NO_VALID_PAGE: if no valid page was found
  *           - Flash error code: on write Flash error
  */
static uint16_t EE_PageTransfer(uint16_t VirtAddress, uint16_t Data)
{
  HAL_StatusTypeDef FlashStatus = HAL_OK;
  uint32_t NewPageAddress = EEPROM_START_ADDRESS;
  uint16_t OldPageId=0;
  uint16_t ValidPage = PAGE0, VarIdx = 0;
  uint16_t EepromStatus = 0, ReadStatus = 0;

  /* Get active Page for read operation */
  ValidPage = EE_FindValidPage(READ_FROM_VALID_PAGE);

  // printf("EE_PageTransfer : %d \n", ValidPage);
  if (ValidPage == PAGE1)       /* Page1 valid */
  {
    /* New page address where variable will be moved to */
    NewPageAddress = PAGE0_BASE_ADDRESS;

    /* Old page ID where variable will be taken from */
    OldPageId = PAGE1_ID;
  }
  else if (ValidPage == PAGE0)  /* Page0 valid */
  {
    /* New page address  where variable will be moved to */
    NewPageAddress = PAGE1_BASE_ADDRESS;

    /* Old page ID where variable will be taken from */
    OldPageId = PAGE0_ID;
  }
  else
  {
    return NO_VALID_PAGE;       /* No valid Page */
  }


  writeFlashData (NewPageAddress, (uint16_t)RECEIVE_DATA, (uint16_t)RECEIVE_DATA);

#if DEBUG_OUT
  cliPrintf("new addr 0x%08X, status 0x%08X \n\n", NewPageAddress);
#endif

  /* Write the variable passed as parameter in the new active page */
  EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddress, Data);
  /* If program operation was failed, a Flash error code is returned */
  if (EepromStatus != HAL_OK)
  {
    return EepromStatus;
  }

  /* Transfer process: transfer variables from old to the new active page */
  for (VarIdx = 0; VarIdx < NB_OF_VAR; VarIdx++)
  {
    if (VirtAddVarTab[VarIdx] != VirtAddress)  /* Check each variable except the one passed as parameter */
    {
      /* Read the other last variable updates */
      ReadStatus = EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar);
      /* In case variable corresponding to the virtual address was found */
      if (ReadStatus != 0x1)
      {
        /* Transfer the variable to the new active page */
        EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar);
        /* If program operation was failed, a Flash error code is returned */
        if (EepromStatus != HAL_OK)
        {
          return EepromStatus;
        }
      }
    }
  }

#if DEBUG_OUT
  cliPrintf("START CLEAN \n\n");
#endif

  if( OldPageId == PAGE0_ID) eraseFlashPage(PAGE0);
  else eraseFlashPage(PAGE1);

#if DEBUG_OUT
  if( OldPageId == PAGE0_ID) cliPrintf("OldPage 0x%08X, status 0x%08X\n", PAGE0_BASE_ADDRESS);
  else cliPrintf("OldPage 0x%08X, status 0x%08X\n", PAGE1_BASE_ADDRESS);
#endif

  writeFlashData (NewPageAddress, VALID_PAGE, VALID_PAGE);

#if DEBUG_OUT
  cliPrintf("Page0 0x%08X, status 0x%08X\n", PAGE0_BASE_ADDRESS);
  cliPrintf("Page1 0x%08X, status 0x%08X\n\n\n\n", PAGE1_BASE_ADDRESS);
#endif

  /* Return last operation flash status */
  return FlashStatus;
}

static void eraseFlashPage (uint16_t page_no)
{
  esp_err_t err;

  switch( page_no)
  {
    case PAGE0 :
#if DEBUG_OUT
      cliPrintf("Erasing page 0 \r\n", info.partition->label, info.partition->size);
      cliPrintf("P[0x%08X], O[0x%08X], S[0x%08X] \r\n", info.partition->address, 0, PAGE_SIZE);
#endif
//      err = (esp_partition_erase_range(info.partition, 0, PAGE_SIZE));
      err = spi_flash_erase_range(PAGE0_BASE_ADDRESS, PAGE_SIZE );

#if DEBUG_OUT
      cliPrintf("E[%d] \r\n", err);
#endif
      break;


    case PAGE1 :
#if DEBUG_OUT
      cliPrintf("Erasing page 1 \r\n", info.partition->label, info.partition->size);
      cliPrintf("P[0x%08X], O[0x%08X], S[0x%08X] \r\n", info.partition->address, PAGE_SIZE, PAGE_SIZE);
#endif
//      err = (esp_partition_erase_range(info.partition, PAGE_SIZE, PAGE_SIZE));
      err = spi_flash_erase_range(PAGE1_BASE_ADDRESS, PAGE_SIZE);

#if DEBUG_OUT
      cliPrintf("E[%d] \r\n", err);
#endif
      break;
  }
}

static void writeFlashData (uint32_t address, uint16_t table_addr, uint16_t data)
{
  uint32_t temp_data = 0;
  esp_err_t err;

  temp_data = table_addr << 16;
  temp_data |= data;

#if DEBUG_OUT
  uint32_t elapse_time = millis();
  cliPrintf("Writing data with esp_partition_write \r\n");
#endif
//  err = (esp_partition_write(info.partition, (address - PAGE0_BASE_ADDRESS), &temp_data, 4));
  err = spi_flash_write(address, &temp_data, 4);

#if DEBUG_OUT
  cliPrintf("E[%d] W[0x%08X] S[%d]ms ",err, temp_data, (millis() - elapse_time));
#endif
}


void loadIrCode (uint8_t index)
{
  uint8_t ir_sector= (uint8_t)(index /  8);
  uint8_t ir_index  = (uint8_t)(index % 8);
  uint16_t ir_offset = 0;

  uint32_t start_addr = IR_DATA_START_ADDR + (PAGE_SIZE * ir_sector);

  data_t t;

  // Read back the data, checking that read data and written data match
  ESP_ERROR_CHECK(esp_partition_read( info.partition,
                                      (start_addr - EEPROM_START_ADDRESS),  // offset
                                      info.global_page_buff, PAGE_SIZE));

  ir_offset = (IR_CODE_BLOCK_LANGTH * ir_index);

  memcpy(info.ir_discrip, &info.global_page_buff[ir_offset], 32);

  ir_offset += IR_DISCRIPTION_LANGTH;

  for( int i=0; i<IR_CODE_DATA_LANGTH; i+=4)
  {
    t.u8Data[0] = info.global_page_buff[ir_offset+i+0];
    t.u8Data[1] = info.global_page_buff[ir_offset+i+1];
    t.u8Data[2] = info.global_page_buff[ir_offset+i+2];
    t.u8Data[3] = info.global_page_buff[ir_offset+i+3];

    info.global_ir_buff[i/4].val = t.u32D;
  }


}

void saveIrCode (uint8_t index)
{
  uint8_t ir_sector = 8 / index;
  uint8_t ir_index  = 8 % index;
  uint32_t start_addr = IR_DATA_START_ADDR + (PAGE_SIZE * ir_sector);
  uint32_t modify_offest = (IR_CODE_BLOCK_LANGTH * ir_index);

  data_t t;

  // Read back the data, checking that read data and written data match
  ESP_ERROR_CHECK(esp_partition_read( info.partition,
                                      (start_addr - EEPROM_START_ADDRESS),
                                      info.global_page_buff, PAGE_SIZE));

  memcpy(&info.global_page_buff[modify_offest], info.ir_discrip, 32); // change

  modify_offest += IR_DISCRIPTION_LANGTH;
  for(int i=0; i<480; i+=4)
  {
    t.u32D = (uint32_t)info.global_ir_buff[i/4].val;

    info.global_page_buff[modify_offest+i+0] = t.u8Data[0];
    info.global_page_buff[modify_offest+i+1] = t.u8Data[1];
    info.global_page_buff[modify_offest+i+2] = t.u8Data[2];
    info.global_page_buff[modify_offest+i+3] = t.u8Data[3];
  }


  ESP_ERROR_CHECK(esp_partition_erase_range(  info.partition,
                                              (start_addr - EEPROM_START_ADDRESS), PAGE_SIZE));
  // Write the data, starting from the beginning of the partition
  ESP_ERROR_CHECK(esp_partition_write(  info.partition,
                                         (start_addr - EEPROM_START_ADDRESS),
                                         info.global_page_buff, PAGE_SIZE));
}

void saveAirbInfo (void)
{
  // Read back the data, checking that read data and written data match
  ESP_ERROR_CHECK(esp_partition_read( info.partition,
                                      (AIRB_DATA_START_ADDR - EEPROM_START_ADDRESS),
                                      info.global_page_buff, PAGE_SIZE));

  memcpy(&info.global_page_buff[0],   info.sta_ssid, 32); // change
  memcpy(&info.global_page_buff[32],  info.sta_pw, 32);   // change
  memcpy(&info.global_page_buff[64],  info.ap_ssid, 32);  // change
  memcpy(&info.global_page_buff[96],  info.ap_pw, 32);    // change

  // TODO : add more data

  ESP_ERROR_CHECK(esp_partition_erase_range(  info.partition,
                                              (AIRB_DATA_START_ADDR - EEPROM_START_ADDRESS), PAGE_SIZE));
  // Write the data, starting from the beginning of the partition
  ESP_ERROR_CHECK(esp_partition_write(  info.partition,
                                         (AIRB_DATA_START_ADDR - EEPROM_START_ADDRESS),
                                         info.global_page_buff, PAGE_SIZE));
}

void loadAirbInfo (void)
{
  // Read back the data, checking that read data and written data match
  ESP_ERROR_CHECK(esp_partition_read( info.partition,
                                      (AIRB_DATA_START_ADDR - EEPROM_START_ADDRESS),  // offset
                                      info.global_page_buff, PAGE_SIZE));

  memcpy(info.sta_ssid, &info.global_page_buff[0], 32);
  memcpy(info.sta_pw, &info.global_page_buff[32], 32);
  memcpy(info.ap_ssid, &info.global_page_buff[64], 32);
  memcpy(info.ap_pw, &info.global_page_buff[96], 32);
}


#ifdef _USE_HW_CLI
void cliEeprom(cli_args_t *args)
{
  if (args->argc == 1 && args->isStr(0, "info"))
  {
    cliPrintf("PAGE0_BASE_ADDRESS[0x%08X] \r\n", PAGE0_BASE_ADDRESS);
    cliPrintf("PAGE1_BASE_ADDRESS[0x%08X] \r\n", PAGE1_BASE_ADDRESS);
    cliPrintf("PAGE_SIZE[0x%08X] \r\n", PAGE_SIZE);
  }
  else if (args->argc == 1 && args->isStr(0, "page"))
  {
    cliPrintf("page -> [%d] \r\n", EE_FindValidPage(READ_FROM_VALID_PAGE));
  }
  else if (args->argc == 1 && args->isStr(0, "format"))
  {
    EE_Format();
    cliPrintf("EEPROM FORMAT \r\n");
  }
  else if (args->argc == 3 && args->isStr(0, "write"))
  {
    cliPrintf("EEPROM Write A[%d] D[%d] \r\n", args->getData(1), args->getData(2) );
    drvEepromWriteByte(args->getData(1), args->getData(2));
  }
  else if (args->argc == 3 && args->isStr(0, "read"))
  {
    cliPrintf("EEPROM Read A[%d] C[%d] \r\n", args->getData(1), args->getData(2) );

    for( int i=args->getData(1); i<(args->getData(1)+args->getData(2)); i++)
    {
      cliPrintf("A[%d]D[%d] \r\n", i, drvEepromReadByte(i) );
    }
  }

}
#endif
