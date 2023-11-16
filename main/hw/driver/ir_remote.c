/*
 * ir_remote.c
 *
 *  Created on: 2021. 1. 15.
 *      Author: HanCheol Cho
 */




#include "ir_remote.h"
#include "cli.h"
#include "driver/rmt.h"

#include "esp_spiffs.h"

#define RX_PIN_NUM	15
#define TX_PIN_NUM	16


static bool is_init = false;

rmt_channel_t ir_tx_channel = RMT_CHANNEL_0;
rmt_channel_t ir_rx_channel = RMT_CHANNEL_4;


RingbufHandle_t ir_rx_rb = NULL;

#ifdef _USE_HW_CLI
static void cliIrRemote(cli_args_t *args);
#endif

bool irRemoteInit(void)
{
  bool ret = true;

  if (is_init == true)
  {
    return true;
  }

  rmt_set_gpio(ir_rx_channel, RMT_MODE_RX, RX_PIN_NUM, false);

  rmt_config_t rmt_rx_config = RMT_DEFAULT_CONFIG_RX(RX_PIN_NUM, ir_rx_channel);
  rmt_config(&rmt_rx_config);
  rmt_driver_install(ir_rx_channel, 1000, 0);


  rmt_get_ringbuf_handle(ir_rx_channel, &ir_rx_rb);
  rmt_rx_start(ir_rx_channel, true);


  rmt_set_gpio(ir_tx_channel, RMT_MODE_TX, TX_PIN_NUM, false);

  rmt_config_t rmt_tx_config = RMT_DEFAULT_CONFIG_TX(TX_PIN_NUM, ir_tx_channel);
  rmt_tx_config.tx_config.carrier_en = true;
  rmt_tx_config.tx_config.idle_output_en = true;

#if 0
  rmt_tx_config.tx_config.carrier_level = RMT_CARRIER_LEVEL_HIGH;
  rmt_tx_config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
#else
  rmt_tx_config.tx_config.carrier_level = RMT_CARRIER_LEVEL_LOW;
  rmt_tx_config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
#endif
  rmt_config(&rmt_tx_config);
  rmt_driver_install(ir_tx_channel, 0, 0);

#ifdef _USE_HW_CLI
  cliAdd("ir", cliIrRemote);
#endif

  return ret;
}





#ifdef _USE_HW_CLI
rmt_item32_t tx_items[256];
uint32_t     tx_length;

static void turnOnIrOrder (void);
static void tyrnOffIrOrder(void);

void cliIrRemote(cli_args_t *args)
{
  bool ret = false;
  rmt_item32_t *items = NULL;

  uint8_t div_cnt = 0;

  rmt_get_clk_div(ir_tx_channel, &div_cnt);
  cliPrintf("div cnt %d\n", div_cnt);

  if (args->argc == 1 && args->isStr(0, "rx"))
  {
    uint32_t length = 0;


    while(cliKeepLoop())
    {
      items = (rmt_item32_t *) xRingbufferReceive(ir_rx_rb, &length, 1000);
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
            tx_items[i] = items[i];
          }
          tx_length = length;
        }
        //rmt_write_items(ir_tx_channel, items, length, true);


        //after parsing the data, return spaces to ringbuffer.
        vRingbufferReturnItem(ir_rx_rb, (void *)items);
      }
      delay(1);
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "tx"))
  {
    //while(cliKeepLoop())
    {
      if (tx_length > 0)
      {
        rmt_write_items(ir_tx_channel, tx_items, tx_length, true);
      }
    }

    rmt_rx_memory_reset(ir_rx_channel);
  }
  else if (args->argc == 1 && args->isStr(0, "on"))
  {
	  cliPrintf("IR Turn On Test \n");
	  turnOnIrOrder();
  }
  else if (args->argc == 1 && args->isStr(0, "off"))
  {
	  cliPrintf("IR Turn Off Test \n");
	  tyrnOffIrOrder();
  }

  if (ret != true)
  {
    cliPrintf("ir rx\n");
  }
}

void tyrnOffIrOrder(void)
{
	esp_err_t err;

	tx_items[0].val = 0x90252291;
	tx_items[1].val = 0x864201E4;
	tx_items[2].val = 0x824701E2;
	tx_items[3].val = 0x822701C8;
	tx_items[4].val = 0x824201E1;
	tx_items[5].val = 0x865A01C6;
	tx_items[6].val = 0x820F01E0;
	tx_items[7].val = 0x824101C7;
	tx_items[8].val = 0x822901E2;
	tx_items[9].val = 0x865001C8;
	tx_items[10].val = 0x867101C7;
	tx_items[11].val = 0x822901C6;
	tx_items[12].val = 0x824101E3;
	tx_items[13].val = 0x822801E0;
	tx_items[14].val = 0x824901C8;
	tx_items[15].val = 0x824101C9;
	tx_items[16].val = 0x822F01C6;
	tx_items[17].val = 0x824001E3;
	tx_items[18].val = 0x824A01C5;
	tx_items[19].val = 0x822A01DF;
	tx_items[20].val = 0x824201AE;
	tx_items[21].val = 0x822701F9;
	tx_items[22].val = 0x863201C7;
	tx_items[23].val = 0x826401C7;
	tx_items[24].val = 0x863A01C6;
	tx_items[25].val = 0x824F01E0;
	tx_items[26].val = 0x825C01C8;
	tx_items[27].val = 0x822801C7;
	tx_items[28].val = 0x864001C7;
	tx_items[29].val = 0x800001E1;
	tx_items[30].val = 0x0000E;
	tx_items[31].val = 0x400559AC;
	tx_items[32].val = 0x0000D;
	tx_items[33].val = 0x400559AC;
	tx_items[34].val = 0x0000C;
	tx_items[35].val = 0x400559AC;
	tx_items[36].val = 0x0000B;
	tx_items[37].val = 0x400559AC;
	tx_items[38].val = 0x0000A;
	tx_items[39].val = 0x400559AC;
	tx_items[40].val = 0x00009;
	tx_items[41].val = 0x400559AC;
	tx_items[42].val = 0x00008;
	tx_items[43].val = 0x400559AC;
	tx_items[44].val = 0x00007;
	tx_items[45].val = 0x400559AC;
	tx_items[46].val = 0x00006;
	tx_items[47].val = 0x400559AC;
	tx_items[48].val = 0x00005;
	tx_items[49].val = 0x400559AC;
	tx_items[50].val = 0x00004;
	tx_items[51].val = 0x400559AC;
	tx_items[52].val = 0x00003;
	tx_items[53].val = 0x400559AC;
	tx_items[54].val = 0x00002;
	tx_items[55].val = 0x400559AC;
	tx_items[56].val = 0x00001;
	tx_items[57].val = 0x400559AC;
	tx_items[58].val = 0x00000;
	tx_items[59].val = 0xFFF9C800;
	tx_items[60].val = 0x80000000;
	tx_items[61].val = 0xFFF9C800;
	tx_items[62].val = 0x40000000;
	tx_items[63].val = 0xFFF9C800;
	tx_items[64].val = 0x20000000;
	tx_items[65].val = 0xFFF9C800;
	tx_items[66].val = 0x10000000;
	tx_items[67].val = 0xFFF9C800;
	tx_items[68].val = 0x8000000;
	tx_items[69].val = 0xFFF9C800;
	tx_items[70].val = 0x4000000;
	tx_items[71].val = 0xFFF9C800;
	tx_items[72].val = 0x2000000;
	tx_items[73].val = 0xFFF9C800;
	tx_items[74].val = 0x1000000;
	tx_items[75].val = 0xFFF9C800;
	tx_items[76].val = 0x800000;
	tx_items[77].val = 0xFFF9C800;
	tx_items[78].val = 0x400000;
	tx_items[79].val = 0xFFF9C800;
	tx_items[80].val = 0x200000;
	tx_items[81].val = 0xFFF9C800;
	tx_items[82].val = 0x100000;
	tx_items[83].val = 0xFFF9C800;
	tx_items[84].val = 0x80000;
	tx_items[85].val = 0xFFF9C800;
	tx_items[86].val = 0x40000;
	tx_items[87].val = 0xFFFDC800;
	tx_items[88].val = 0x20000;
	tx_items[89].val = 0xFFFFC800;
	tx_items[90].val = 0x10000;
	tx_items[91].val = 0xFFFFC800;
	tx_items[92].val = 0x08000;
	tx_items[93].val = 0xFFFFC800;
	tx_items[94].val = 0x04000;
	tx_items[95].val = 0xFFFFC800;
	tx_items[96].val = 0x02000;
	tx_items[97].val = 0xFFFFE800;
	tx_items[98].val = 0x01000;
	tx_items[99].val = 0xFFFFF800;
	tx_items[100].val = 0x00800;
	tx_items[101].val = 0xFFFFF800;
	tx_items[102].val = 0x00400;
	tx_items[103].val = 0xFFFFFC00;
	tx_items[104].val = 0x00200;
	tx_items[105].val = 0xFFFFFE00;
	tx_items[106].val = 0x00100;
	tx_items[107].val = 0xFFFFFF00;
	tx_items[108].val = 0x00080;
	tx_items[109].val = 0xFFFFFF80;
	tx_items[110].val = 0x00040;
	tx_items[111].val = 0xFFFFFFC0;
	tx_items[112].val = 0x00020;
	tx_items[113].val = 0xFFFFFFE0;
	tx_items[114].val = 0x00010;
	tx_items[115].val = 0xFFFFFFF0;
	tx_items[116].val = 0x00008;
	tx_items[117].val = 0xFFFFFFF8;
	tx_items[118].val = 0x00004;
	tx_items[119].val = 0xFFFFFFFC;


	tx_length = 120;

	//while(cliKeepLoop())
	{
	  if (tx_length > 0)
	  {
		  err = rmt_write_items(ir_tx_channel, tx_items, tx_length, true);
		  cliPrintf("err[0x%08X]\n", err);
	  }
	}
    rmt_rx_memory_reset(ir_rx_channel);
}

void turnOnIrOrder (void)
{
	esp_err_t err;

	tx_items[0].val = 0x905E2272;
	tx_items[1].val = 0x863001FE;
	tx_items[2].val = 0x822E01E4;
	tx_items[3].val = 0x823001F9;
	tx_items[4].val = 0x822401CB;
	tx_items[5].val = 0x863701E2;
	tx_items[6].val = 0x825D01E1;
	tx_items[7].val = 0x823001AB;
	tx_items[8].val = 0x827701C5;
	tx_items[9].val = 0x824001AE;
	tx_items[10].val = 0x823701C9;
	tx_items[11].val = 0x823E01E3;
	tx_items[12].val = 0x824A01C8;
	tx_items[13].val = 0x824001C8;
	tx_items[14].val = 0x825D01AD;
	tx_items[15].val = 0x824701C8;
	tx_items[16].val = 0x863801C8;
	tx_items[17].val = 0x865A01E0;
	tx_items[18].val = 0x822E01C8;
	tx_items[19].val = 0x820E01FB;
	tx_items[20].val = 0x865D01E0;
	tx_items[21].val = 0x824801C6;
	tx_items[22].val = 0x824201C7;
	tx_items[23].val = 0x825301C6;
	tx_items[24].val = 0x826301AD;
	tx_items[25].val = 0x863001E1;
	tx_items[26].val = 0x826401C7;
	tx_items[27].val = 0x864301C5;
	tx_items[28].val = 0x824D01C4;
	tx_items[29].val = 0x800001AC;
	tx_items[30].val = 0x40000000;
	tx_items[31].val = 0xFFF9C800;
	tx_items[32].val = 0x20000000;
	tx_items[33].val = 0xFFF9C800;
	tx_items[34].val = 0x10000000;
	tx_items[35].val = 0xFFF9C800;
	tx_items[36].val = 0x8000000;
	tx_items[37].val = 0xFFF9C800;
	tx_items[38].val = 0x4000000;
	tx_items[39].val = 0xFFF9C800;
	tx_items[40].val = 0x2000000;
	tx_items[41].val = 0xFFF9C800;
	tx_items[42].val = 0x1000000;
	tx_items[43].val = 0xFFF9C800;
	tx_items[44].val = 0x800000;
	tx_items[45].val = 0xFFF9C800;
	tx_items[46].val = 0x400000;
	tx_items[47].val = 0xFFF9C800;
	tx_items[48].val = 0x200000;
	tx_items[49].val = 0xFFF9C800;
	tx_items[50].val = 0x100000;
	tx_items[51].val = 0xFFF9C800;
	tx_items[52].val = 0x80000;
	tx_items[53].val = 0xFFF9C800;
	tx_items[54].val = 0x40000;
	tx_items[55].val = 0xFFFDC800;
	tx_items[56].val = 0x20000;
	tx_items[57].val = 0xFFFFC800;
	tx_items[58].val = 0x10000;
	tx_items[59].val = 0xFFFFC800;
	tx_items[60].val = 0x08000;
	tx_items[61].val = 0xFFFFC800;
	tx_items[62].val = 0x04000;
	tx_items[63].val = 0xFFFFC800;
	tx_items[64].val = 0x02000;
	tx_items[65].val = 0xFFFFE800;
	tx_items[66].val = 0x01000;
	tx_items[67].val = 0xFFFFF800;
	tx_items[68].val = 0x00800;
	tx_items[69].val = 0xFFFFF800;
	tx_items[70].val = 0x00400;
	tx_items[71].val = 0xFFFFFC00;
	tx_items[72].val = 0x00200;
	tx_items[73].val = 0xFFFFFE00;
	tx_items[74].val = 0x00100;
	tx_items[75].val = 0xFFFFFF00;
	tx_items[76].val = 0x00080;
	tx_items[77].val = 0xFFFFFF80;
	tx_items[78].val = 0x00040;
	tx_items[79].val = 0xFFFFFFC0;
	tx_items[80].val = 0x00020;
	tx_items[81].val = 0xFFFFFFE0;
	tx_items[82].val = 0x00010;
	tx_items[83].val = 0xFFFFFFF0;
	tx_items[84].val = 0x00008;
	tx_items[85].val = 0xFFFFFFF8;
	tx_items[86].val = 0x00004;
	tx_items[87].val = 0xFFFFFFFC;
	tx_items[88].val = 0x00002;
	tx_items[89].val = 0xFFFFFFFE;
	tx_items[90].val = 0x00001;
	tx_items[91].val = 0x40035190;
	tx_items[92].val = 0x40035300;
	tx_items[93].val = 0x5F12FF3B;
	tx_items[94].val = 0xEC4F5EC9;
	tx_items[95].val = 0x00000;
	tx_items[96].val = 0x00000;
	tx_items[97].val = 0x00000;
	tx_items[98].val = 0x00000;
	tx_items[99].val = 0x00000;
	tx_items[100].val = 0x00000;
	tx_items[101].val = 0x00000;
	tx_items[102].val = 0x00000;
	tx_items[103].val = 0x00000;
	tx_items[104].val = 0x00000;
	tx_items[105].val = 0x00000;
	tx_items[106].val = 0x00000;
	tx_items[107].val = 0x00000;
	tx_items[108].val = 0x00000;
	tx_items[109].val = 0x00000;
	tx_items[110].val = 0x00000;
	tx_items[111].val = 0x00000;
	tx_items[112].val = 0x00000;
	tx_items[113].val = 0x00000;
	tx_items[114].val = 0x00000;
	tx_items[115].val = 0x00000;
	tx_items[116].val = 0x00000;
	tx_items[117].val = 0x00000;
	tx_items[118].val = 0x00000;
	tx_items[119].val = 0x00000;


	tx_length = 120;

	//while(cliKeepLoop())
	{
	  if (tx_length > 0)
	  {
		  err = rmt_write_items(ir_tx_channel, tx_items, tx_length, true);
		  cliPrintf("err[0x%08X]\n", err);
	  }
	}
    rmt_rx_memory_reset(ir_rx_channel);
}

rmt_item32_t * pGetRxItemData(void)
{
  return tx_items;
}

void pSetTxItemData(rmt_item32_t *p_data)
{
  tx_length = 120;
  for( int i=0; i< tx_length; i++)
  {
    tx_items[i] = *p_data;
  }
}

rmt_channel_t GetIrTxChannel (void)
{
  return ir_tx_channel;
}

rmt_channel_t GetIrRxChannel (void)
{
  return ir_rx_channel;
}

RingbufHandle_t *pGetIrRingBuff (void)
{
  return ir_rx_rb;
}


#endif

