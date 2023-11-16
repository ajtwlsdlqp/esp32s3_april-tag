/*
 * def.h
 *
 *  Created on: 2021. 1. 8.
 *      Author: HanCheol Cho
 */

#ifndef MAIN_COMMON_DEF_H_
#define MAIN_COMMON_DEF_H_


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "def_err.h"



#define _DEF_LED1                   0
#define _DEF_LED2                   1
#define _DEF_LED3                   2
#define _DEF_LED4                   3


#define _DEF_UART1                  0
#define _DEF_UART2                  1
#define _DEF_UART3                  2   // Websocket
#define _DEF_UART4                  3



#define _DEF_SPI1                   0
#define _DEF_SPI2                   1
#define _DEF_SPI3                   2
#define _DEF_SPI4                   3


#define _DEF_I2C1                   0
#define _DEF_I2C2                   1
#define _DEF_I2C3                   2
#define _DEF_I2C4                   3


#define _DEF_TYPE_S08               0
#define _DEF_TYPE_U08               1
#define _DEF_TYPE_S16               2
#define _DEF_TYPE_U16               3
#define _DEF_TYPE_S32               4
#define _DEF_TYPE_U32               5
#define _DEF_TYPE_F32               6
#define _DEF_TYPE_STR               7

#define _DEF_TYPE_UKN               0xFFFF


#define _DEF_DXL_BAUD_9600          0
#define _DEF_DXL_BAUD_57600         1
#define _DEF_DXL_BAUD_115200        2
#define _DEF_DXL_BAUD_1000000       3
#define _DEF_DXL_BAUD_2000000       4
#define _DEF_DXL_BAUD_3000000       5
#define _DEF_DXL_BAUD_4000000       6
#define _DEF_DXL_BAUD_4500000       7


#define _DEF_DXL_XL330              0


#define _DEF_DXL_MODEL_XL330_M080   1190
#define _DEF_DXL_MODEL_XL330_M290   1200


#define _DEF_LOG_ERR                0
#define _DEF_LOG_WARN               1
#define _DEF_LOG_INFO               2
#define _DEF_LOG_DIAG               3
#define _DEF_LOG_NONE               4


#define _DEF_HIGH                   1
#define _DEF_LOW                    0


#define _DEF_INPUT                  0
#define _DEF_INPUT_PULLUP           1
#define _DEF_INPUT_PULLDOWN         2
#define _DEF_OUTPUT                 3
#define _DEF_OUTPUT_PULLUP          4
#define _DEF_OUTPUT_PULLDOWN        5



#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#ifndef max
//#define max(a,b) (((a) > (b)) ? (a) : (b))
//#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef map
#define map(value, in_min, in_max, out_min, out_max) (((value) - (in_min)) * ((out_max) - (out_min)) / ((in_max) - (in_min)) + (out_min))
#endif

#ifndef GET_RANDOM_NUM_WITH_RANGE
#define GET_RANDOM_NUM_WITH_RANGE(range_min, range_max) \
  ((rand() % (range_max + 1 - range_min)) + range_min)
#endif



typedef uint32_t  err_code_t;



typedef union
{
  uint8_t  u8Data[4];
  uint16_t u16Data[2];
  uint32_t u32Data;

  int8_t   s8Data[4];
  int16_t  s16Data[2];
  int32_t  s32Data;

  uint8_t  u8D;
  uint16_t u16D;
  uint32_t u32D;

  int8_t   s8D;
  int16_t  s16D;
  int32_t  s32D;

  float    f32D;
} data_t;


typedef struct
{
  data_t data;
  bool   ret;
} data_ret_t;


typedef struct
{
  uint8_t boot_name[32];
  uint8_t boot_ver[32];
  uint32_t magic_number;
  uint32_t addr_fw;
  uint32_t image_start;
  uint32_t image_end;
  uint32_t image_size;
} boot_tag_t;


typedef struct
{
  uint32_t magic_number;

  //-- fw info
  //
  uint8_t  version_str[32];
  uint8_t  board_str  [32];
  uint8_t  name_str   [32];
  uint8_t  date_str   [32];
  uint8_t  time_str   [32];
  uint32_t addr_tag;
  uint32_t addr_fw;

  uint32_t load_start;
  uint32_t load_size;
  uint32_t version_num;
  uint32_t reserved   [29];

  //-- tag info
  //
  uint32_t tag_flash_type;
  uint32_t tag_flash_start;
  uint32_t tag_flash_end;
  uint32_t tag_flash_length;
  uint32_t tag_flash_crc;
  uint32_t tag_length;
  uint8_t  tag_date_str[32];
  uint8_t  tag_time_str[32];
} flash_tag_t;

typedef struct
{
  uint32_t magic_number;

  //-- contents_info
  //
  uint8_t  version_str[32];
  uint8_t  board_str  [32];
  uint8_t  name_str   [32];
  uint8_t  date_str   [32];
  uint8_t  time_str   [32];
  uint32_t addr_tag;
  uint32_t addr_contents;

  uint32_t load_start;
  uint32_t load_size;
  uint32_t reserved   [29];
} contents_tag_t;

#define FLASH_MAGIC_NUMBER      0x5555AAAA




#define IMG_TAG_MAGIC           0x5555AAAA
#define IMG_TAG_BUF_SIZE        20*1024

typedef struct
{
  uint32_t magic_number;        // 4
  uint16_t image_group;         // 2
  uint16_t image_id;            // 2
  uint32_t file_type;           // 4
  uint16_t width;               // 2
  uint16_t height;              // 2
  int32_t  compressed_size;     // 4
  int32_t  decompressed_size;   // 4
  uint32_t file_size;           // 4
  uint32_t reserved;            // 4, 32B

  uint8_t  data[IMG_TAG_BUF_SIZE];
} img_tag_t;


typedef struct
{
  uint32_t magic_number;
  uint32_t addr_start;
  uint32_t addr_end;
} hash_tag_header_t;

typedef struct
{
  uint32_t hash_id;
  uint32_t data;
} hash_tag_t;

typedef struct
{
  uint8_t year;
  uint8_t month;
  uint8_t date;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t reserved[2];
} date_time_t;

typedef struct
{
  uint8_t mac_addr[8];
  date_time_t date_time;
  uint16_t pba_ver;
  uint8_t reserved[6];
  uint8_t mac_hash[4];
} manufacture_tag_t;

#ifdef HW_VER_INFO_DETAILED_STRUCT
typedef struct
{
  uint8_t cnt;
  uint8_t type;
}ir_sensor_hw_t;

typedef struct
{
  uint8_t pin_cnt;
  uint8_t dxl_detection;
  uint8_t chaging_detection;
  uint8_t type;
}pogopin_hw_t;

typedef struct
{
  uint8_t cnt;
  uint8_t max_sample_rate_in_kbps;
  uint8_t bit_per_sample;
  uint8_t type;
}mic_hw_t;

typedef struct
{
  uint8_t cnt;
  uint8_t chip_type;
  uint16_t width;
  uint16_t height;
}lcd_hw_t;

typedef struct
{
  uint8_t cnt;
  uint8_t capacity_in_mb;
  uint8_t chip_type;
  uint8_t qspi_available;
}ext_flash_hw_t;

typedef struct
{
  uint8_t cnt;
  uint8_t max_data_rate_in_kbps;
  uint8_t bit_per_data;
  uint8_t gain;
}speaker_hw_t;

typedef struct
{
  uint8_t chip_type;
  uint8_t max_voltage_in_0_1v;
  uint8_t min_voltage_in_0_1v;
  uint8_t gain;
}charger_hw_t;

typedef struct
{
  uint8_t max_input_voltage;
  uint8_t type;
  uint8_t reserved[2];
}voltage_detection_hw_t;

typedef struct
{
  uint8_t cell_cnt;
  uint8_t capacity_in_100mah;
  uint8_t reserved[2];
}battery_hw_t;

typedef struct
{
  uint8_t available;
  uint8_t reserved;
}debug_uart_hw_t;
#endif

typedef struct
{
  uint32_t magic_code;
  uint16_t tag_ver;
  uint16_t pba_ver;
  uint16_t validation;
}pba_ver_t;

typedef struct
{
  uint16_t tag_ver;
  uint16_t pba_ver;
  date_time_t version_timestamp;
  uint8_t ir_sensor_type;
  uint8_t pogopin_sensor_type;
  uint8_t mic_type;
  uint8_t lcd_type;
  uint8_t imu_type;
  uint8_t ext_flash_type;
  uint8_t eeprom_type;
  uint8_t speaker_type;
  uint8_t charger_type;
  uint8_t voltage_detection_type;
  uint8_t battery_type;
  uint8_t debug_uart_type;
  uint8_t joystick_type;
  uint8_t button_type;
  uint8_t led_type;
  uint8_t reserved[5];
} pba_info_tag_t;  // 32byte



#define PLUGIN_MAGIC_NUMBER         0x504C5547  // "PLUG"

typedef struct
{
  uint32_t magic_number;
  char     name[32];
  uint32_t file_type;           // 4
  uint16_t version;             // 2
  uint16_t header_length;       // 2
  char     date_str[32];        // 32
  char     time_str[32];        // 32
  uint8_t  reserved[12];        // 12
  uint32_t file_address;        // 4
  uint32_t file_length;         // 4

} plugin_file_t;




#endif /* MAIN_COMMON_DEF_H_ */
