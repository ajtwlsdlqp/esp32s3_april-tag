/*
 * spi.c
 *
 *  Created on: 2021. 1. 12.
 *      Author: HanCheol Cho
 */




#include "spi.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"



#define SPI_DMA_MAX_SIZE     (4095)



typedef struct
{
  bool is_open;
  bool is_tx_done;
  bool is_error;

  uint32_t clk_fre;
  uint8_t pin_clk;
  uint8_t pin_mosi;
  uint32_t max_buffer_size; // DMA used

  uint32_t buffer_size;
  uint32_t half_buffer_size;
  uint32_t node_cnt;
  uint32_t half_node_cnt;
  uint32_t dma_size;
  uint8_t horizontal;
  uint8_t dc_state;
  lldesc_t *dma;
  uint8_t *buffer;
  QueueHandle_t event_queue;

  void (*func_tx)(void);
} spi_t;



spi_t spi_tbl[SPI_MAX_CH];


static void spiSetupPin(uint8_t ch);
static void spiSetupCfg(uint8_t ch);
static void spiSetupDMA(uint8_t ch);



static void IRAM_ATTR spiISR(void *arg)
{
  BaseType_t HPTaskAwoken = pdFALSE;
  typeof(GPSPI3.dma_int_st) int_st = GPSPI3.dma_int_st;
  GPSPI3.dma_int_clr.val = int_st.val;

  if (int_st.out_eof)
  {
    xQueueSendFromISR(spi_tbl[_DEF_SPI1].event_queue, (void *)&int_st.val, &HPTaskAwoken);
  }

  if (HPTaskAwoken == pdTRUE)
  {
    portYIELD_FROM_ISR();
  }
}

bool spiInit(void)
{
  bool ret = true;


  for (int i=0; i<SPI_MAX_CH; i++)
  {
    spi_tbl[i].is_open = false;
    spi_tbl[i].is_tx_done = true;
    spi_tbl[i].is_error = false;
    spi_tbl[i].func_tx = NULL;
  }

  return ret;
}

bool spiBegin(uint8_t ch)
{
  bool ret = false;
  spi_t *p_spi = &spi_tbl[ch];


  if (p_spi->is_open == true)
  {
    return true;
  }

  logPrintf("[ ] spi\n");

  p_spi->pin_clk  = 19;
  p_spi->pin_mosi = 20;
  p_spi->max_buffer_size = 4 * 1024;

  spiSetupPin(ch);
  spiSetupCfg(ch);
  spiSetupDMA(ch);

  p_spi->event_queue = xQueueCreate(1, sizeof(int));
  p_spi->buffer_size = p_spi->max_buffer_size;

  p_spi->is_open = true;

  return ret;
}

bool spiIsBegin(uint8_t ch)
{
  return spi_tbl[ch].is_open;
}

void spiSetupPin(uint8_t ch)
{
  spi_t *p_spi = &spi_tbl[ch];


  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[p_spi->pin_clk], PIN_FUNC_GPIO);
  gpio_set_direction(p_spi->pin_clk, GPIO_MODE_OUTPUT);
  gpio_set_pull_mode(p_spi->pin_clk, GPIO_FLOATING);
  gpio_matrix_out(p_spi->pin_clk, SPI3_CLK_OUT_MUX_IDX, 0, 0);

  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[p_spi->pin_mosi], PIN_FUNC_GPIO);
  gpio_set_direction(p_spi->pin_mosi, GPIO_MODE_OUTPUT);
  gpio_set_pull_mode(p_spi->pin_mosi, GPIO_FLOATING);
  gpio_matrix_out(p_spi->pin_mosi, SPI3_D_OUT_IDX, 0, 0);

}

void spiSetupCfg(uint8_t ch)
{

  REG_CLR_BIT(DPORT_PERIP_CLK_EN0_REG, DPORT_SPI3_CLK_EN);
  REG_SET_BIT(DPORT_PERIP_CLK_EN0_REG, DPORT_SPI3_CLK_EN);
  REG_SET_BIT(DPORT_PERIP_RST_EN0_REG, DPORT_SPI3_RST);
  REG_CLR_BIT(DPORT_PERIP_RST_EN0_REG, DPORT_SPI3_RST);
  REG_CLR_BIT(DPORT_PERIP_CLK_EN0_REG, DPORT_SPI3_DMA_CLK_EN);
  REG_SET_BIT(DPORT_PERIP_CLK_EN0_REG, DPORT_SPI3_DMA_CLK_EN);
  REG_SET_BIT(DPORT_PERIP_RST_EN0_REG, DPORT_SPI3_DMA_RST);
  REG_CLR_BIT(DPORT_PERIP_RST_EN0_REG, DPORT_SPI3_DMA_RST);

  int div = 2;
  int clk_freq = 40000000;

  if (clk_freq == 80000000)
  {
    GPSPI3.clock.clk_equ_sysclk = 1;
  }
  else
  {
    GPSPI3.clock.clk_equ_sysclk = 0;
    div = 80000000 / clk_freq;
  }

  GPSPI3.ctrl1.clk_mode = 0;
  GPSPI3.clock.clkdiv_pre = 1 - 1;
  GPSPI3.clock.clkcnt_n = div - 1;
  GPSPI3.clock.clkcnt_l = div - 1;
  GPSPI3.clock.clkcnt_h = ((div >> 1) - 1);

  GPSPI3.misc.ck_dis = 0;

  GPSPI3.user1.val = 0;
  GPSPI3.slave.val = 0;
  GPSPI3.misc.ck_idle_edge = 0;
  GPSPI3.user.ck_out_edge = 0;
  GPSPI3.ctrl.wr_bit_order = 0;
  GPSPI3.ctrl.rd_bit_order = 0;
  GPSPI3.user.val = 0;
  GPSPI3.user.cs_setup = 1;
  GPSPI3.user.cs_hold = 1;
  GPSPI3.user.usr_mosi = 1;
  GPSPI3.user.usr_mosi_highpart = 0;

  GPSPI3.dma_conf.val = 0;
  GPSPI3.dma_conf.out_rst = 1;
  GPSPI3.dma_conf.out_rst = 0;
  GPSPI3.dma_conf.ahbm_fifo_rst = 1;
  GPSPI3.dma_conf.ahbm_fifo_rst = 0;
  GPSPI3.dma_conf.ahbm_rst = 1;
  GPSPI3.dma_conf.ahbm_rst = 0;
  GPSPI3.dma_out_link.dma_tx_ena = 1;
  GPSPI3.dma_conf.out_eof_mode = 1;
  GPSPI3.cmd.usr = 0;

  GPSPI3.dma_int_clr.val = ~0;
  GPSPI3.dma_int_ena.val = 0;
  GPSPI3.dma_int_ena.out_eof = 1;

  intr_handle_t intr_handle = NULL;
  esp_intr_alloc(ETS_SPI3_DMA_INTR_SOURCE, 0, spiISR, NULL, &intr_handle);
}

void spiSetupDMA(uint8_t ch)
{
  spi_t *p_spi = &spi_tbl[ch];

  int cnt = 0;

  if (p_spi->max_buffer_size >= SPI_DMA_MAX_SIZE * 2)
  {
    p_spi->dma_size = SPI_DMA_MAX_SIZE;

    for (cnt = 0;; cnt++)
    { /*!< Find the buffer size that is divisible by dma_size */
      if ((p_spi->max_buffer_size - cnt) % p_spi->dma_size == 0)
      {
          break;
      }
    }

    p_spi->buffer_size = p_spi->max_buffer_size - cnt;
  }
  else
  {
    p_spi->dma_size = p_spi->max_buffer_size / 2;
    p_spi->buffer_size = p_spi->dma_size * 2;
  }

  p_spi->half_buffer_size = p_spi->buffer_size / 2;

  p_spi->node_cnt = (p_spi->buffer_size) / p_spi->dma_size; /*!< Number of DMA nodes */
  p_spi->half_node_cnt = p_spi->node_cnt / 2;

  logPrintf("[ ] \tspi_buffer_size: %d, lcd_dma_size: %d, lcd_dma_node_cnt: %d\n", p_spi->buffer_size, p_spi->dma_size, p_spi->node_cnt);

  p_spi->dma    = (lldesc_t *)heap_caps_malloc(p_spi->node_cnt * sizeof(lldesc_t), MALLOC_CAP_DMA);
  p_spi->buffer = (uint8_t *)heap_caps_malloc(p_spi->buffer_size * sizeof(uint8_t), MALLOC_CAP_DMA);

  if (p_spi->dma != NULL) logPrintf("[O] \tspi dma malloc OK\n");
  else                    logPrintf("[X] \tspi dma malloc fail\n");

  if (p_spi->buffer != NULL) logPrintf("[O] \tspi buffer malloc OK\n");
  else                       logPrintf("[X] \tspi buffer malloc fail\n");
}

void spiSetDataMode(uint8_t ch, uint8_t dataMode)
{
  spi_t  *p_spi = &spi_tbl[ch];


  if (p_spi->is_open == false) return;


  switch( dataMode )
  {
    // CPOL=0, CPHA=0
    case SPI_MODE0:
      break;

    // CPOL=0, CPHA=1
    case SPI_MODE1:
      break;

    // CPOL=1, CPHA=0
    case SPI_MODE2:
      break;

    // CPOL=1, CPHA=1
    case SPI_MODE3:
      break;
  }
}

void spiSetBitWidth(uint8_t ch, uint8_t bit_width)
{
  spi_t  *p_spi = &spi_tbl[ch];

  if (p_spi->is_open == false) return;

  if (bit_width == 16)
  {
  }
}

uint8_t spiTransfer8(uint8_t ch, uint8_t data)
{
  uint8_t ret = 0;
  spi_t  *p_spi = &spi_tbl[ch];


  if (p_spi->is_open == false) return 0;

  spiDmaTxTransfer(ch, &data, 1, 10);

  return ret;
}

uint16_t spiTransfer16(uint8_t ch, uint16_t data)
{
  uint16_t ret = 0;


  uint8_t tBuf[2];
  //uint8_t rBuf[2];
  spi_t  *p_spi = &spi_tbl[ch];


  if (p_spi->is_open == false) return 0;

  tBuf[1] = (uint8_t)data;
  tBuf[0] = (uint8_t)(data>>8);

  spiDmaTxTransfer(ch, &tBuf[0], 2, 10);

  //ret = rBuf[0];
  //ret <<= 8;
  //ret += rBuf[1];


  return ret;
}

bool spiTransfer(uint8_t ch, uint8_t *tx_buf, uint8_t *rx_buf, uint32_t length, uint32_t timeout)
{
  bool ret = true;

  spi_t  *p_spi = &spi_tbl[ch];

  if (p_spi->is_open == false) return false;

  return ret;
}

void spiDmaTxStart(uint8_t spi_ch, uint8_t *p_buf, uint32_t length)
{
  spi_t  *p_spi = &spi_tbl[spi_ch];

  if (p_spi->is_open == false) return;

  p_spi->is_tx_done = false;
  //HAL_SPI_Transmit_DMA(p_spi->h_spi, p_buf, length);
}

bool spiDmaTxTransfer(uint8_t ch, void *buf, uint32_t length, uint32_t timeout)
{
  bool ret = true;
  spi_t *p_spi = &spi_tbl[ch];


  int event  = 0;
  int x = 0, cnt = 0, size = 0;
  int end_pos = 0;


  /*!< Generate a data DMA linked list */
  for (x = 0; x < p_spi->node_cnt; x++)
  {
    p_spi->dma[x].size   = p_spi->dma_size;
    p_spi->dma[x].length = p_spi->dma_size;
    p_spi->dma[x].buf    = (p_spi->buffer + p_spi->dma_size * x);
    p_spi->dma[x].eof    = !((x + 1) % p_spi->half_node_cnt);
    p_spi->dma[x].empty  = (uint32_t)&p_spi->dma[(x + 1) % p_spi->node_cnt];
  }

  p_spi->dma[p_spi->half_node_cnt - 1].empty = 0;
  p_spi->dma[p_spi->node_cnt - 1].empty = 0;
  cnt = length / p_spi->half_buffer_size;
  /*!< Start the signal */
  xQueueSend(p_spi->event_queue, &event, 0);

      /*!< Processing a complete piece of data, ping-pong operation */
  for (x = 0; x < cnt; x++)
  {
    memcpy((uint8_t *)p_spi->dma[(x % 2) * p_spi->half_node_cnt].buf, buf, p_spi->half_buffer_size);
    buf += p_spi->half_buffer_size;
    xQueueReceive(p_spi->event_queue, (void *)&event, portMAX_DELAY);
    GPSPI3.mosi_dlen.usr_mosi_bit_len = p_spi->half_buffer_size * 8 - 1;
    GPSPI3.dma_out_link.addr = ((uint32_t)&p_spi->dma[(x % 2) * p_spi->half_node_cnt]) & 0xfffff;
    GPSPI3.dma_out_link.start = 1;
    ets_delay_us(1);
    GPSPI3.cmd.usr = 1;
  }

  cnt = length % p_spi->half_buffer_size;

  /*!< Processing remaining incomplete segment data */
  if (cnt)
  {
    memcpy((uint8_t *)p_spi->dma[(x % 2) * p_spi->half_node_cnt].buf, buf, cnt);

    /*!< Handle the case where the data length is an integer multiple of lcd_obj->dma_size */
    if (cnt % p_spi->dma_size)
    {
      end_pos = (x % 2) * p_spi->half_node_cnt + cnt / p_spi->dma_size;
      size = cnt % p_spi->dma_size;
    }
    else
    {
      end_pos = (x % 2) * p_spi->half_node_cnt + cnt / p_spi->dma_size - 1;
      size = p_spi->dma_size;
    }

    /*!< Handle the tail node to make it a DMA tail */
    p_spi->dma[end_pos].size = size;
    p_spi->dma[end_pos].length = size;
    p_spi->dma[end_pos].eof = 1;
    p_spi->dma[end_pos].empty = 0;
    xQueueReceive(p_spi->event_queue, (void *)&event, portMAX_DELAY);
    GPSPI3.mosi_dlen.usr_mosi_bit_len = cnt * 8 - 1;
    GPSPI3.dma_out_link.addr = ((uint32_t)&p_spi->dma[(x % 2) * p_spi->half_node_cnt]) & 0xfffff;
    GPSPI3.dma_out_link.start = 1;
    ets_delay_us(1);
    GPSPI3.cmd.usr = 1;
  }

  xQueueReceive(p_spi->event_queue, (void *)&event, portMAX_DELAY);

  return ret;
}

bool spiDmaTxIsDone(uint8_t ch)
{
  spi_t  *p_spi = &spi_tbl[ch];

  if (p_spi->is_open == false)     return true;

  return p_spi->is_tx_done;
}

void spiAttachTxInterrupt(uint8_t ch, void (*func)())
{
  spi_t  *p_spi = &spi_tbl[ch];


  if (p_spi->is_open == false)     return;

  p_spi->func_tx = func;
}



#if 0
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  spi_t  *p_spi;

  if (hspi->Instance == spi_tbl[_DEF_SPI1].h_spi->Instance)
  {
    p_spi = &spi_tbl[_DEF_SPI1];

    p_spi->is_tx_done = true;

    if (p_spi->func_tx != NULL)
    {
      (*p_spi->func_tx)();
    }
  }
}
#endif
