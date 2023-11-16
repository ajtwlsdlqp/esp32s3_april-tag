/*
 * util.c
 *
 *  Created on: 2020. 1. 27.
 *      Author: Baram
 */




#include "def_err.h"
#include <stdlib.h>

#include "def.h"
#include "util.h"




//-- Internal Variables
//


//-- External Variables
//


//-- Internal Functions
//


//-- External Functions
//





bool utilGetRange(data_range_t *p_range)
{
  bool ret = false;


  p_range->node_byte_index = 0;

  if (p_range->node_addr_start >= p_range->range_start && p_range->node_addr_end <= p_range->range_end)
  {
    ret = true;
  }
  if (p_range->range_start >= p_range->node_addr_start && p_range->range_start < p_range->node_addr_end)
  {
    ret = true;
    p_range->node_byte_index = p_range->range_start - p_range->node_addr_start;
    p_range->node_addr_start = p_range->range_start;
  }
  if (p_range->range_end > p_range->node_addr_start && p_range->range_end <= p_range->node_addr_end)
  {
    ret = true;
    p_range->node_addr_end = p_range->range_end;
  }
  p_range->node_length = p_range->node_addr_end - p_range->node_addr_start;

  return ret;
}


uint32_t utilConvert8ToU32 (uint8_t *p_data)
{
  uint32_t t_data = 0;
  uint8_t i;

  for(i = 0; i < 4; i++)
  {
    t_data |= p_data[i] << (i*8);
  }

  return t_data;
}

uint16_t utilConvert8ToU16 (uint8_t *p_data)
{
  uint16_t t_data = 0;
  uint8_t i;

  for(i = 0; i < 2; i++)
  {
    t_data |= p_data[i] << (i*8);
  }

  return t_data;
}

volatile const unsigned short util_crc_table[256] = {0x0000,
                                0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
                                0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027,
                                0x0022, 0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D,
                                0x8077, 0x0072, 0x0050, 0x8055, 0x805F, 0x005A, 0x804B,
                                0x004E, 0x0044, 0x8041, 0x80C3, 0x00C6, 0x00CC, 0x80C9,
                                0x00D8, 0x80DD, 0x80D7, 0x00D2, 0x00F0, 0x80F5, 0x80FF,
                                0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1, 0x00A0, 0x80A5,
                                0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1, 0x8093,
                                0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
                                0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197,
                                0x0192, 0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE,
                                0x01A4, 0x81A1, 0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB,
                                0x01FE, 0x01F4, 0x81F1, 0x81D3, 0x01D6, 0x01DC, 0x81D9,
                                0x01C8, 0x81CD, 0x81C7, 0x01C2, 0x0140, 0x8145, 0x814F,
                                0x014A, 0x815B, 0x015E, 0x0154, 0x8151, 0x8173, 0x0176,
                                0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162, 0x8123,
                                0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
                                0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104,
                                0x8101, 0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D,
                                0x8317, 0x0312, 0x0330, 0x8335, 0x833F, 0x033A, 0x832B,
                                0x032E, 0x0324, 0x8321, 0x0360, 0x8365, 0x836F, 0x036A,
                                0x837B, 0x037E, 0x0374, 0x8371, 0x8353, 0x0356, 0x035C,
                                0x8359, 0x0348, 0x834D, 0x8347, 0x0342, 0x03C0, 0x83C5,
                                0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1, 0x83F3,
                                0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
                                0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7,
                                0x03B2, 0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E,
                                0x0384, 0x8381, 0x0280, 0x8285, 0x828F, 0x028A, 0x829B,
                                0x029E, 0x0294, 0x8291, 0x82B3, 0x02B6, 0x02BC, 0x82B9,
                                0x02A8, 0x82AD, 0x82A7, 0x02A2, 0x82E3, 0x02E6, 0x02EC,
                                0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2, 0x02D0, 0x82D5,
                                0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1, 0x8243,
                                0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
                                0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264,
                                0x8261, 0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E,
                                0x0234, 0x8231, 0x8213, 0x0216, 0x021C, 0x8219, 0x0208,
                                0x820D, 0x8207, 0x0202 };


static const uint16_t atan_table[256] = {
    0, 229, 458, 687, 916, 1145, 1374, 1603,
    1832, 2061, 2290, 2519, 2748, 2976, 3205, 3433,
    3662, 3890, 4118, 4346, 4574, 4802, 5029, 5257,
    5484, 5711, 5938, 6165, 6391, 6618, 6844, 7070,
    7296, 7521, 7746, 7971, 8196, 8421, 8645, 8869,
    9093, 9317, 9540, 9763, 9986, 10208, 10431, 10652,
    10874, 11095, 11316, 11537, 11757, 11977, 12197, 12416,
    12635, 12853, 13071, 13289, 13507, 13724, 13940, 14157,
    14373, 14588, 14803, 15018, 15232, 15446, 15660, 15873,
    16085, 16297, 16509, 16720, 16931, 17142, 17352, 17561,
    17770, 17979, 18187, 18394, 18601, 18808, 19014, 19220,
    19425, 19630, 19834, 20038, 20241, 20444, 20646, 20848,
    21049, 21250, 21450, 21649, 21848, 22047, 22245, 22443,
    22640, 22836, 23032, 23227, 23422, 23616, 23810, 24003,
    24196, 24388, 24580, 24771, 24961, 25151, 25340, 25529,
    25717, 25905, 26092, 26278, 26464, 26649, 26834, 27018,
    27202, 27385, 27568, 27750, 27931, 28112, 28292, 28471,
    28650, 28829, 29007, 29184, 29361, 29537, 29712, 29887,
    30062, 30236, 30409, 30582, 30754, 30925, 31096, 31266,
    31436, 31605, 31774, 31942, 32109, 32276, 32442, 32608,
    32773, 32938, 33101, 33265, 33428, 33590, 33751, 33913,
    34073, 34233, 34392, 34551, 34709, 34867, 35024, 35180,
    35336, 35492, 35646, 35801, 35954, 36107, 36260, 36412,
    36563, 36714, 36864, 37014, 37163, 37312, 37460, 37607,
    37754, 37901, 38047, 38192, 38337, 38481, 38624, 38768,
    38910, 39052, 39194, 39335, 39475, 39615, 39754, 39893,
    40032, 40169, 40307, 40443, 40580, 40715, 40850, 40985,
    41119, 41253, 41386, 41519, 41651, 41782, 41913, 42044,
    42174, 42303, 42432, 42561, 42689, 42817, 42944, 43070,
    43196, 43322, 43447, 43572, 43696, 43819, 43943, 44065,
    44188, 44309, 44431, 44551, 44672, 44792, 44911, 45030,
    45148, 45266, 45384, 45501, 45618, 45734, 45849, 45965,
};

void utilUpdateCrc(uint16_t *p_crc_cur, uint8_t data_in)
{
  uint16_t crc;
  uint16_t i;

  crc = *p_crc_cur;

  i = ((unsigned short)(crc >> 8) ^ data_in) & 0xFF;
  *p_crc_cur = (crc << 8) ^ util_crc_table[i];
}

// http://www.lomont.org/papers/2003/InvSqrt.pdf
__attribute__((optimize("O2"))) float utilInvSqrt(float x)
{
  float xhalf = 0.5f*x;
  int i = *(int*)&x; // get bits for floating value
  i = 0x5f375a86- (i>>1); // gives initial guess y0
  x = *(float*)&i; // convert bits back to float
  x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
  x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
  return x;
}

// http://www.360doc.com/content/18/0713/09/35575875_770017309.shtml
__attribute__((optimize("O2"))) int16_t utilAngle(int16_t imag, int16_t real)
{
  int16_t abs_real;
  int16_t abs_imag;
  int16_t phase;
  uint16_t zone;
  uint32_t tmp_tan;
  uint32_t delta_tan;
  if (imag == 0 && real >= 0)
  {
    return 0;
  }
  else if (imag == 0 && real < 0)
  {
    return 23040;
  }
  else if (real == 0)
  {
    if (imag > 0)
    {
      return 11520;
    }
    else
    {
      return -11520;
    }
  }
  if (real < 0)
  {
    abs_real = -real;
    zone = 1U;
  }
  else
  {
    abs_real = real;
    zone = 0U;
  }
  if (imag < 0)
  {
    abs_imag = -imag;
    zone += 2U;
  }
  else
  {
    abs_imag = imag;
  }
  if (abs_imag <= abs_real)
  {
    tmp_tan = (uint32_t)abs_imag<<8U;
    tmp_tan /= abs_real;
    if (tmp_tan > 255)
    {
      tmp_tan = 255;
    }
    phase = atan_table[tmp_tan]>>3U;
  }
  else if ((abs_imag>>7U) <= abs_real)
  {
    delta_tan = ((uint32_t)(abs_imag - abs_real))<<15U;
    tmp_tan = (uint32_t)abs_imag + (uint32_t)abs_real;
    delta_tan /= tmp_tan;
    delta_tan >>= 7U;
    if (delta_tan > 255)
    {
      delta_tan = 255;
    }
    phase = atan_table[delta_tan]>>3U;
    phase += 5760;
  }
  else
  {
    phase = 11520;
  }
  switch (zone)
  {
    case 0U:
    {
      break;
    }
    case 1U:
    {
      phase = 23040 - phase;
      break;
    }
  case 2U:
    {
      phase = -phase;
      break;
    }
  case 3U:
    {
      phase = phase - 23040;
      break;
    }
  default:
    {
      break;
    }
  }
  return(phase);
}

int utilSprintfFloat(char* buffer, const float decimal, uint8_t fractional_digit)
{
  uint32_t decimal_times = 1;
  int integer_part = (int)decimal;
  float fraction = 0.0;
  int fraction_part = 0;
  char digit_style[8] = {'%', 'd', '.', '%', '0', '0', 'd', '\0'};
  int length = 0;

  if (fractional_digit > 6) fractional_digit = 6;
  for (uint8_t i = 0; i < fractional_digit; i++)
  {
    decimal_times *= 10;
  }
  if (decimal > integer_part)
  {
    fraction = decimal - integer_part;
  }
  else
  {
    fraction = integer_part - decimal;
  }
  fraction_part = fraction * decimal_times;
  if ((uint32_t)(fraction * decimal_times * 10) % 10 >= 5)
  {
    fraction_part += 1;
    if (fraction_part == decimal_times)
    {
      integer_part += 1;
      fraction_part = 0;
    }
  }

  digit_style[5] += fractional_digit;

  if (fractional_digit == 0)
  {
    length = sprintf(buffer, "%d", integer_part);
  }
  else
  {
    if (decimal < 0 && integer_part == 0)
    {
      buffer[0] = '-';
      buffer++;
      length = 1;
    }
    length += sprintf(buffer, digit_style, integer_part, fraction_part);
  }
  return length;
}

// https://www.sysnet.pe.kr/2/0/1222
__attribute__((optimize("O2"))) uint32_t utilHashStr(char *chars, int len)
{
  uint32_t hash_ret = 0;


  uint32_t poly = 0xEDB88320;
  for (int i = 0; i < len; i++)
  {
    poly = (poly << 1) | (poly >> (32 - 1)); // 1bit Left Shift
    hash_ret = (uint32_t)(poly * hash_ret + (uint32_t)chars[i]);
  }

  return hash_ret;
}

__attribute__((optimize("O2"))) float utilSin(uint16_t degree)
{
  degree %= 360;
  int8_t sign = 1;
  if (degree % 180 == 0)
  {
    return 0.0;
  }
  else if (degree == 90)
  {
    return 1.0;
  }
  else if (degree == 270)
  {
    return -1.0;
  }

  uint8_t deg = degree;
  if (degree > 90 && degree < 180)
  {
    deg = 180 - degree;
  }
  else if (degree > 180 && degree < 270)
  {
    deg = degree - 180;
    sign = -1;
  }
  else if (degree > 270)
  {
    deg = 360 - degree;
    sign = -1;
  }
  float y = 0.0;
  if (deg > 45)
  {
    y = utilSin(90-deg);
    y = 1.0/utilInvSqrt(1 - y*y);
  }
  else
  {
    float x = deg * 3.14159265 / 180;
    float x2 = x * x;
    y = x;
    x *= x2;
    y -= x/6;
    x *= x2;
    y += x/120;
  }
  return y * sign;
}

__attribute__((optimize("O2"))) float utilCos(uint16_t degree)
{
  degree %= 360;
  int8_t sign = 1;
  if (degree % 180 == 90)
  {
    return 0.0;
  }
  else if (degree == 0)
  {
    return 1.0;
  }
  else if (degree == 180)
  {
    return -1.0;
  }

  uint8_t deg = degree;
  if (degree > 90 && degree < 180)
  {
    deg = 180 - degree;
    sign = -1;
  }
  else if (degree > 180 && degree < 270)
  {
    deg = degree - 180;
    sign = -1;
  }
  else if (degree > 270)
  {
    deg = 360 - degree;
  }
  float y = 0.0;
  if (deg > 45)
  {
    y = utilCos(90-deg);
    y = 1.0/utilInvSqrt(1 - y*y);
  }
  else
  {
    float x = deg * 3.14159265 / 180;
    float x2 = x * x;
    y = 1 - x2/2;
    x = x2*x2;
    y += x/24;
  }
  return y * sign;
}

__attribute__((optimize("O2"))) float utilArcSin(float x)
{
  x = constrain(x, -1, 1);
  float x2 = x * x;
  float ret = x;
  x *= x2;
  ret += x/6;
  x *= x2;
  ret += x*3/40;
//  x *= x2;
//  ret += x*5/112;

  return ret*180/3.14159265;
}

__attribute__((optimize("O2"))) void utilXyRotate(int32_t *p_x, int32_t *p_y, int32_t x0, int32_t y0, float cosr, float sinr)
{
  int32_t xr = cosr*(*p_x-x0)-sinr*(*p_y-y0)+x0;
  int32_t yr = sinr*(*p_x-x0)+cosr*(*p_y-y0)+y0;
  *p_x = xr;
  *p_y = yr;
}

int32_t utilRandRange(int32_t start, int32_t stop)
{
  int32_t ret = 0;

  if (stop <= start) return ret;

  ret = rand() % (stop - start) + start;

  return ret;
}
