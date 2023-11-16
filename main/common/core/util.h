/*
 * util.h
 *
 *  Created on: 2020. 1. 27.
 *      Author: Baram
 */

#ifndef SRC_COMMON_CORE_UTIL_H_
#define SRC_COMMON_CORE_UTIL_H_



#ifdef __cplusplus
 extern "C" {
#endif



typedef struct
{
  uint32_t range_start;
  uint32_t range_end;

  uint32_t node_addr_start;
  uint32_t node_addr_end;
  uint32_t node_length;
  uint32_t node_byte_index;
} data_range_t;




bool utilGetRange(data_range_t *p_range);
uint32_t utilConvert8ToU32 (uint8_t *p_data);
uint16_t utilConvert8ToU16 (uint8_t *p_data);
float utilInvSqrt(float x);
int16_t utilAngle(int16_t imag, int16_t real);
uint32_t utilHashStr(char *chars, int len);
float utilSin(uint16_t degree);
float utilCos(uint16_t degree);
float utilArcSin(float x);
void utilXyRotate(int32_t *p_x, int32_t *p_y, int32_t x0, int32_t y0, float cosr, float sinr);
int32_t utilRandRange(int32_t start, int32_t stop);

void utilUpdateCrc(uint16_t *p_crc_cur, uint8_t data_in);

int utilSprintfFloat(char* buffer, const float decimal, uint8_t fractional_digit);

#ifdef __cplusplus
}
#endif


#endif /* SRC_COMMON_CORE_UTIL_H_ */
