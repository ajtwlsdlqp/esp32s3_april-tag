/*
 * dat.h
 *
 *  Created on: 2020. 7. 31.
 *      Author: HanCheol Cho
 */

#ifndef SRC_AP_DAT_H_
#define SRC_AP_DAT_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "hw_def.h"

#ifdef _USE_HW_DAT


#define DAT_HASH_SECTION      HW_DAT_HASH_SECTION
#define DAT_MEM_SECTION       HW_DAT_MEM_SECTION


#define DAT_MAGIC_FILE        0x4446494C   // "DFIL"
#define DAT_MAGIC_FOLDER      0x44464f4C   // "DFOL"
#define DAT_FOLDER_MAX        256






typedef struct dat_file_t_
{
  uint32_t magic_number;        // "DFIL"
  char     name[32];
} dat_file_t;


typedef struct
{
  uint32_t magic_number;        // "DFOL"
  char     name[32];
  uint32_t file_count;
  dat_file_t  *p_files[DAT_FOLDER_MAX];
} dat_folder_t;



void *datGetFilePtr(const char *fmt, ...);



#endif

#ifdef __cplusplus
}
#endif


#endif /* SRC_AP_DAT_H_ */
