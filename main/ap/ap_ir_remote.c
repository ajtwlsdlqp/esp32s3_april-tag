/*
 * ap_ir_remote.c
 *
 *  Created on: Feb 6, 2023
 *      Author: LDH
 */

#include "ap.h"
#include "ap_ir_remote.h"


#include "ir_remote.h"
#include "cli.h"
#include "../common/hw/include/ir_remote.h"

#include "esp_spiffs.h"


#include "hw.h"
extern airb_info_t info;

static void setOnIrOrder (void);
static void setOffIrOrder(void);


uint8_t SendIrTxData (void)
{
  uint8_t err = 0;

  if( info.global_ir_buff[0].val == 0xFFFFFFFF && info.global_ir_buff[1].val == 0xFFFFFFFF) return 0xFF;

  err = rmt_write_items(GetIrTxChannel(), info.global_ir_buff, 120, true);
  cliPrintf("err[0x%08X]\n", err);

  rmt_rx_memory_reset(GetIrTxChannel());

  return err;
}

uint8_t GetIrRxData (void)
{
  uint8_t err = 0;
  rmt_item32_t *items = NULL;
  uint32_t length = 0;
  bool ret = false;

  uint32_t pre_rx_holdtime = millis();


  while(cliKeepLoop())
  {
    items = (rmt_item32_t *) xRingbufferReceive(pGetIrRingBuff(), &length, 1000);
    if (items)
    {

      cliPrintf("len : %d\n", length);

      //length /= 4; // one RMT = 4 Bytes

      for (int i=0; i<length; i++)
      {
        cliPrintf("%d : %d,%d  %d %d\n", i,
                  items[i].level0, items[i].duration0,
                  items[i].level1, items[i].duration1);
      }
      cliPrintf("\n");

      if (length > 100)
      {
        for (int i=0; i<length; i++)
        {
          info.global_ir_buff[i] = items[i];
        }
        ret = true;
      }

      //after parsing the data, return spaces to ringbuffer.
      vRingbufferReturnItem(pGetIrRingBuff(), (void *)items);
    }
    delay(1);
    if( ret == true) break;
    if( millis() - pre_rx_holdtime > 3000)
    {
      err = 1;
      break;
    }

    cliPrintf("END \n");
  }

  return err;
}

void clearIrBuffer (void)
{
  rmt_rx_memory_reset(GetIrRxChannel());
}






// Test Function -> no need to edit
void irTestDefaultAc(uint8_t mode)
{
  if( mode >= 2) return;

  if( mode == 0)  // turn off
  {
    setOffIrOrder();
  }
  else if( mode == 1)
  {
    setOnIrOrder();
  }

  SendIrTxData();

  clearIrBuffer();
  //
}

void setOffIrOrder(void)
{
  info.global_ir_buff[0].val = 0x90252291;
  info.global_ir_buff[1].val = 0x864201E4;
  info.global_ir_buff[2].val = 0x824701E2;
  info.global_ir_buff[3].val = 0x822701C8;
  info.global_ir_buff[4].val = 0x824201E1;
  info.global_ir_buff[5].val = 0x865A01C6;
  info.global_ir_buff[6].val = 0x820F01E0;
  info.global_ir_buff[7].val = 0x824101C7;
  info.global_ir_buff[8].val = 0x822901E2;
  info.global_ir_buff[9].val = 0x865001C8;
  info.global_ir_buff[10].val = 0x867101C7;
  info.global_ir_buff[11].val = 0x822901C6;
  info.global_ir_buff[12].val = 0x824101E3;
  info.global_ir_buff[13].val = 0x822801E0;
  info.global_ir_buff[14].val = 0x824901C8;
  info.global_ir_buff[15].val = 0x824101C9;
  info.global_ir_buff[16].val = 0x822F01C6;
  info.global_ir_buff[17].val = 0x824001E3;
  info.global_ir_buff[18].val = 0x824A01C5;
  info.global_ir_buff[19].val = 0x822A01DF;
  info.global_ir_buff[20].val = 0x824201AE;
  info.global_ir_buff[21].val = 0x822701F9;
  info.global_ir_buff[22].val = 0x863201C7;
  info.global_ir_buff[23].val = 0x826401C7;
  info.global_ir_buff[24].val = 0x863A01C6;
  info.global_ir_buff[25].val = 0x824F01E0;
  info.global_ir_buff[26].val = 0x825C01C8;
  info.global_ir_buff[27].val = 0x822801C7;
  info.global_ir_buff[28].val = 0x864001C7;
  info.global_ir_buff[29].val = 0x800001E1;
  info.global_ir_buff[30].val = 0x0000E;
  info.global_ir_buff[31].val = 0x400559AC;
  info.global_ir_buff[32].val = 0x0000D;
  info.global_ir_buff[33].val = 0x400559AC;
  info.global_ir_buff[34].val = 0x0000C;
  info.global_ir_buff[35].val = 0x400559AC;
  info.global_ir_buff[36].val = 0x0000B;
  info.global_ir_buff[37].val = 0x400559AC;
  info.global_ir_buff[38].val = 0x0000A;
  info.global_ir_buff[39].val = 0x400559AC;
  info.global_ir_buff[40].val = 0x00009;
  info.global_ir_buff[41].val = 0x400559AC;
  info.global_ir_buff[42].val = 0x00008;
  info.global_ir_buff[43].val = 0x400559AC;
  info.global_ir_buff[44].val = 0x00007;
  info.global_ir_buff[45].val = 0x400559AC;
  info.global_ir_buff[46].val = 0x00006;
  info.global_ir_buff[47].val = 0x400559AC;
  info.global_ir_buff[48].val = 0x00005;
  info.global_ir_buff[49].val = 0x400559AC;
  info.global_ir_buff[50].val = 0x00004;
  info.global_ir_buff[51].val = 0x400559AC;
  info.global_ir_buff[52].val = 0x00003;
  info.global_ir_buff[53].val = 0x400559AC;
  info.global_ir_buff[54].val = 0x00002;
  info.global_ir_buff[55].val = 0x400559AC;
  info.global_ir_buff[56].val = 0x00001;
  info.global_ir_buff[57].val = 0x400559AC;
  info.global_ir_buff[58].val = 0x00000;
  info.global_ir_buff[59].val = 0xFFF9C800;
  info.global_ir_buff[60].val = 0x80000000;
  info.global_ir_buff[61].val = 0xFFF9C800;
  info.global_ir_buff[62].val = 0x40000000;
  info.global_ir_buff[63].val = 0xFFF9C800;
  info.global_ir_buff[64].val = 0x20000000;
  info.global_ir_buff[65].val = 0xFFF9C800;
  info.global_ir_buff[66].val = 0x10000000;
  info.global_ir_buff[67].val = 0xFFF9C800;
  info.global_ir_buff[68].val = 0x8000000;
  info.global_ir_buff[69].val = 0xFFF9C800;
  info.global_ir_buff[70].val = 0x4000000;
  info.global_ir_buff[71].val = 0xFFF9C800;
  info.global_ir_buff[72].val = 0x2000000;
  info.global_ir_buff[73].val = 0xFFF9C800;
  info.global_ir_buff[74].val = 0x1000000;
  info.global_ir_buff[75].val = 0xFFF9C800;
  info.global_ir_buff[76].val = 0x800000;
  info.global_ir_buff[77].val = 0xFFF9C800;
  info.global_ir_buff[78].val = 0x400000;
  info.global_ir_buff[79].val = 0xFFF9C800;
  info.global_ir_buff[80].val = 0x200000;
  info.global_ir_buff[81].val = 0xFFF9C800;
  info.global_ir_buff[82].val = 0x100000;
  info.global_ir_buff[83].val = 0xFFF9C800;
  info.global_ir_buff[84].val = 0x80000;
  info.global_ir_buff[85].val = 0xFFF9C800;
  info.global_ir_buff[86].val = 0x40000;
  info.global_ir_buff[87].val = 0xFFFDC800;
  info.global_ir_buff[88].val = 0x20000;
  info.global_ir_buff[89].val = 0xFFFFC800;
  info.global_ir_buff[90].val = 0x10000;
  info.global_ir_buff[91].val = 0xFFFFC800;
  info.global_ir_buff[92].val = 0x08000;
  info.global_ir_buff[93].val = 0xFFFFC800;
  info.global_ir_buff[94].val = 0x04000;
  info.global_ir_buff[95].val = 0xFFFFC800;
  info.global_ir_buff[96].val = 0x02000;
  info.global_ir_buff[97].val = 0xFFFFE800;
  info.global_ir_buff[98].val = 0x01000;
  info.global_ir_buff[99].val = 0xFFFFF800;
  info.global_ir_buff[100].val = 0x00800;
  info.global_ir_buff[101].val = 0xFFFFF800;
  info.global_ir_buff[102].val = 0x00400;
  info.global_ir_buff[103].val = 0xFFFFFC00;
  info.global_ir_buff[104].val = 0x00200;
  info.global_ir_buff[105].val = 0xFFFFFE00;
  info.global_ir_buff[106].val = 0x00100;
  info.global_ir_buff[107].val = 0xFFFFFF00;
  info.global_ir_buff[108].val = 0x00080;
  info.global_ir_buff[109].val = 0xFFFFFF80;
  info.global_ir_buff[110].val = 0x00040;
  info.global_ir_buff[111].val = 0xFFFFFFC0;
  info.global_ir_buff[112].val = 0x00020;
  info.global_ir_buff[113].val = 0xFFFFFFE0;
  info.global_ir_buff[114].val = 0x00010;
  info.global_ir_buff[115].val = 0xFFFFFFF0;
  info.global_ir_buff[116].val = 0x00008;
  info.global_ir_buff[117].val = 0xFFFFFFF8;
  info.global_ir_buff[118].val = 0x00004;
  info.global_ir_buff[119].val = 0xFFFFFFFC;
}

void setOnIrOrder (void)
{
  info.global_ir_buff[0].val = 0x905E2272;
  info.global_ir_buff[1].val = 0x863001FE;
  info.global_ir_buff[2].val = 0x822E01E4;
  info.global_ir_buff[3].val = 0x823001F9;
  info.global_ir_buff[4].val = 0x822401CB;
  info.global_ir_buff[5].val = 0x863701E2;
  info.global_ir_buff[6].val = 0x825D01E1;
  info.global_ir_buff[7].val = 0x823001AB;
  info.global_ir_buff[8].val = 0x827701C5;
  info.global_ir_buff[9].val = 0x824001AE;
  info.global_ir_buff[10].val = 0x823701C9;
  info.global_ir_buff[11].val = 0x823E01E3;
  info.global_ir_buff[12].val = 0x824A01C8;
  info.global_ir_buff[13].val = 0x824001C8;
  info.global_ir_buff[14].val = 0x825D01AD;
  info.global_ir_buff[15].val = 0x824701C8;
  info.global_ir_buff[16].val = 0x863801C8;
  info.global_ir_buff[17].val = 0x865A01E0;
  info.global_ir_buff[18].val = 0x822E01C8;
  info.global_ir_buff[19].val = 0x820E01FB;
  info.global_ir_buff[20].val = 0x865D01E0;
  info.global_ir_buff[21].val = 0x824801C6;
  info.global_ir_buff[22].val = 0x824201C7;
  info.global_ir_buff[23].val = 0x825301C6;
  info.global_ir_buff[24].val = 0x826301AD;
  info.global_ir_buff[25].val = 0x863001E1;
  info.global_ir_buff[26].val = 0x826401C7;
  info.global_ir_buff[27].val = 0x864301C5;
  info.global_ir_buff[28].val = 0x824D01C4;
  info.global_ir_buff[29].val = 0x800001AC;
  info.global_ir_buff[30].val = 0x40000000;
  info.global_ir_buff[31].val = 0xFFF9C800;
  info.global_ir_buff[32].val = 0x20000000;
  info.global_ir_buff[33].val = 0xFFF9C800;
  info.global_ir_buff[34].val = 0x10000000;
  info.global_ir_buff[35].val = 0xFFF9C800;
  info.global_ir_buff[36].val = 0x8000000;
  info.global_ir_buff[37].val = 0xFFF9C800;
  info.global_ir_buff[38].val = 0x4000000;
  info.global_ir_buff[39].val = 0xFFF9C800;
  info.global_ir_buff[40].val = 0x2000000;
  info.global_ir_buff[41].val = 0xFFF9C800;
  info.global_ir_buff[42].val = 0x1000000;
  info.global_ir_buff[43].val = 0xFFF9C800;
  info.global_ir_buff[44].val = 0x800000;
  info.global_ir_buff[45].val = 0xFFF9C800;
  info.global_ir_buff[46].val = 0x400000;
  info.global_ir_buff[47].val = 0xFFF9C800;
  info.global_ir_buff[48].val = 0x200000;
  info.global_ir_buff[49].val = 0xFFF9C800;
  info.global_ir_buff[50].val = 0x100000;
  info.global_ir_buff[51].val = 0xFFF9C800;
  info.global_ir_buff[52].val = 0x80000;
  info.global_ir_buff[53].val = 0xFFF9C800;
  info.global_ir_buff[54].val = 0x40000;
  info.global_ir_buff[55].val = 0xFFFDC800;
  info.global_ir_buff[56].val = 0x20000;
  info.global_ir_buff[57].val = 0xFFFFC800;
  info.global_ir_buff[58].val = 0x10000;
  info.global_ir_buff[59].val = 0xFFFFC800;
  info.global_ir_buff[60].val = 0x08000;
  info.global_ir_buff[61].val = 0xFFFFC800;
  info.global_ir_buff[62].val = 0x04000;
  info.global_ir_buff[63].val = 0xFFFFC800;
  info.global_ir_buff[64].val = 0x02000;
  info.global_ir_buff[65].val = 0xFFFFE800;
  info.global_ir_buff[66].val = 0x01000;
  info.global_ir_buff[67].val = 0xFFFFF800;
  info.global_ir_buff[68].val = 0x00800;
  info.global_ir_buff[69].val = 0xFFFFF800;
  info.global_ir_buff[70].val = 0x00400;
  info.global_ir_buff[71].val = 0xFFFFFC00;
  info.global_ir_buff[72].val = 0x00200;
  info.global_ir_buff[73].val = 0xFFFFFE00;
  info.global_ir_buff[74].val = 0x00100;
  info.global_ir_buff[75].val = 0xFFFFFF00;
  info.global_ir_buff[76].val = 0x00080;
  info.global_ir_buff[77].val = 0xFFFFFF80;
  info.global_ir_buff[78].val = 0x00040;
  info.global_ir_buff[79].val = 0xFFFFFFC0;
  info.global_ir_buff[80].val = 0x00020;
  info.global_ir_buff[81].val = 0xFFFFFFE0;
  info.global_ir_buff[82].val = 0x00010;
  info.global_ir_buff[83].val = 0xFFFFFFF0;
  info.global_ir_buff[84].val = 0x00008;
  info.global_ir_buff[85].val = 0xFFFFFFF8;
  info.global_ir_buff[86].val = 0x00004;
  info.global_ir_buff[87].val = 0xFFFFFFFC;
  info.global_ir_buff[88].val = 0x00002;
  info.global_ir_buff[89].val = 0xFFFFFFFE;
  info.global_ir_buff[90].val = 0x00001;
  info.global_ir_buff[91].val = 0x40035190;
  info.global_ir_buff[92].val = 0x40035300;
  info.global_ir_buff[93].val = 0x5F12FF3B;
  info.global_ir_buff[94].val = 0xEC4F5EC9;
  info.global_ir_buff[95].val = 0x00000;
  info.global_ir_buff[96].val = 0x00000;
  info.global_ir_buff[97].val = 0x00000;
  info.global_ir_buff[98].val = 0x00000;
  info.global_ir_buff[99].val = 0x00000;
  info.global_ir_buff[100].val = 0x00000;
  info.global_ir_buff[101].val = 0x00000;
  info.global_ir_buff[102].val = 0x00000;
  info.global_ir_buff[103].val = 0x00000;
  info.global_ir_buff[104].val = 0x00000;
  info.global_ir_buff[105].val = 0x00000;
  info.global_ir_buff[106].val = 0x00000;
  info.global_ir_buff[107].val = 0x00000;
  info.global_ir_buff[108].val = 0x00000;
  info.global_ir_buff[109].val = 0x00000;
  info.global_ir_buff[110].val = 0x00000;
  info.global_ir_buff[111].val = 0x00000;
  info.global_ir_buff[112].val = 0x00000;
  info.global_ir_buff[113].val = 0x00000;
  info.global_ir_buff[114].val = 0x00000;
  info.global_ir_buff[115].val = 0x00000;
  info.global_ir_buff[116].val = 0x00000;
  info.global_ir_buff[117].val = 0x00000;
  info.global_ir_buff[118].val = 0x00000;
  info.global_ir_buff[119].val = 0x00000;
}
