/*
 * uart.c
 *
 *  Created on: 2021. 1. 11.
 *      Author: HanCheol Cho
 */




#include "uart.h"
#include "qbuffer.h"
#include "driver/uart.h"

#include "cdc.h"
#include "driver/usb_serial_jtag.h"

#ifdef _USE_HW_UART


#define UART_RX_Q_BUF_LEN       (1024+10)



typedef struct
{
  bool is_open;

  uart_port_t   port;
  uint32_t      baud;
  uart_config_t config;
  QueueHandle_t queue;

  qbuffer_t     qbuffer;
  uint8_t       rx_buf[UART_RX_Q_BUF_LEN];
  uint8_t       wr_buf[UART_RX_Q_BUF_LEN];
} uart_tbl_t;


static uart_tbl_t uart_tbl[UART_MAX_CH];
static size_t socket_wr_len = 0;
static size_t socket_rx_len = 0;
static uint8_t send_buf[UART_RX_Q_BUF_LEN];


static usb_serial_jtag_driver_config_t usb_serial_jtag_config;

bool uartInit(void)
{
  for (int i=0; i<UART_MAX_CH; i++)
  {
    uart_tbl[i].is_open = false;
  }

  return true;
}

bool uartOpen(uint8_t ch, uint32_t baud)
{
  bool ret = false;


  switch(ch)
  {
    case _DEF_UART1:

      if (uart_tbl[ch].is_open == true)
      {
        uart_tbl[ch].baud = baud;
        uart_set_baudrate(uart_tbl[ch].port, baud);
      }
      else
      {
        uart_tbl[ch].port = UART_NUM_0;
        uart_tbl[ch].baud = baud;
        uart_tbl[ch].config.baud_rate = baud;
        uart_tbl[ch].config.data_bits = UART_DATA_8_BITS;
        uart_tbl[ch].config.parity    = UART_PARITY_DISABLE;
        uart_tbl[ch].config.stop_bits = UART_STOP_BITS_1;
        uart_tbl[ch].config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
        uart_tbl[ch].config.source_clk = UART_SCLK_APB;


        qbufferCreate(&uart_tbl[ch].qbuffer, &uart_tbl[ch].rx_buf[0], UART_RX_Q_BUF_LEN);

        uart_driver_install(UART_NUM_0, UART_RX_Q_BUF_LEN*2, UART_RX_Q_BUF_LEN*2, 0, NULL, 0);
        uart_param_config(UART_NUM_0, &uart_tbl[ch].config);

        //Set UART pins (using UART0 default pins ie no changes.)
        uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

        uart_flush(UART_NUM_0);

        uart_tbl[ch].is_open = true;
      }
      ret = true;
      break;

    case _DEF_UART2:
      if (uart_tbl[ch].is_open == true)
      {
        uart_tbl[ch].baud = baud;
        uart_set_baudrate(uart_tbl[ch].port, baud);
      }
      else
      {
        uart_tbl[ch].port = UART_NUM_1;
        uart_tbl[ch].baud = baud;
        uart_tbl[ch].config.baud_rate = baud;
        uart_tbl[ch].config.data_bits = UART_DATA_8_BITS;
        uart_tbl[ch].config.parity    = UART_PARITY_DISABLE;
        uart_tbl[ch].config.stop_bits = UART_STOP_BITS_1;
        uart_tbl[ch].config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
        uart_tbl[ch].config.source_clk = UART_SCLK_APB;


        qbufferCreate(&uart_tbl[ch].qbuffer, &uart_tbl[ch].rx_buf[0], UART_RX_Q_BUF_LEN);

        uart_driver_install(uart_tbl[ch].port, UART_RX_Q_BUF_LEN*2, UART_RX_Q_BUF_LEN*2, 0, NULL, 0);
        uart_param_config(uart_tbl[ch].port, &uart_tbl[ch].config);

        // uart_set_pin(uart_tbl[ch].port, 12, 14, 13, UART_PIN_NO_CHANGE); // Tx Rx TxEn
        uart_set_pin(uart_tbl[ch].port, 9, 3, 46, UART_PIN_NO_CHANGE);
        uart_set_mode(uart_tbl[ch].port, UART_MODE_RS485_HALF_DUPLEX);

        uart_flush(uart_tbl[ch].port);

        uart_tbl[ch].is_open = true;
      }
      ret = true;
      break;

    case _DEF_UART3 : //web socket ... init in wifi_ap.c
      qbufferCreate(&uart_tbl[ch].qbuffer, &uart_tbl[ch].rx_buf[0], UART_RX_Q_BUF_LEN);

      uart_tbl[ch].is_open = true;
      ret = true;
      break;

    case _DEF_UART4 :
      uart_tbl[ch].is_open = true;
      uart_tbl[ch].baud = 1000000;

      uartUsbCdcInit();
      ret = true;
      break;

  }

  return ret;
}

uint32_t uartAvailable(uint8_t ch)
{
  uint32_t ret = 0;
  size_t len;


  if (uart_tbl[ch].is_open != true) return 0;

  switch(ch)
  {
    case _DEF_UART1:
    case _DEF_UART2:
      uart_get_buffered_data_len(uart_tbl[ch].port, &len);
      if (len > (UART_RX_Q_BUF_LEN - 1))
      {
        len = UART_RX_Q_BUF_LEN - 1;
      }
      if (len > 0)
      {
        uart_read_bytes(uart_tbl[ch].port, uart_tbl[ch].wr_buf, len, 10);
        qbufferWrite(&uart_tbl[ch].qbuffer, uart_tbl[ch].wr_buf, len);
      }
      ret = qbufferAvailable(&uart_tbl[ch].qbuffer);
      break;

    case _DEF_UART3:
      len = socket_wr_len;

      if (len > (UART_RX_Q_BUF_LEN - 1))
      {
        len = UART_RX_Q_BUF_LEN - 1;
      }
      if (len > 0)
      {
        qbufferWrite(&uart_tbl[ch].qbuffer, uart_tbl[ch].wr_buf, len);
        socket_wr_len = 0;
      }
      ret = qbufferAvailable(&uart_tbl[ch].qbuffer);
      break;

    case _DEF_UART4:
      ret = uartUsbCdcAvailable();
      break;
  }

  return ret;
}

uint8_t uartRead(uint8_t ch)
{
  uint8_t ret = 0;

  switch(ch)
  {
    case _DEF_UART1:
    case _DEF_UART2:
    case _DEF_UART3:
      qbufferRead(&uart_tbl[ch].qbuffer, &ret, 1);
      break;

    case _DEF_UART4:
      ESP_LOGE("USB_CDC", "uartRead");
      ret = uartUsbCdcRead();
      break;
  }

  return ret;
}

uint32_t uartWrite(uint8_t ch, uint8_t *p_data, uint32_t length)
{
  uint32_t ret = 0;

  if (uart_tbl[ch].is_open != true) return 0;


  switch(ch)
  {
    case _DEF_UART1:
    case _DEF_UART2:
      ret = uart_write_bytes(uart_tbl[ch].port, (const char*)p_data, (size_t)length);
      break;

    case _DEF_UART3:
      //TODO
      socket_rx_len = length;

      if( socket_rx_len > 0)
      {
        memcpy(send_buf, (const char*)p_data, (size_t)length);
      }

      ret = socket_rx_len; // dummy return;
      break;

    case _DEF_UART4:
      ret = uartUsbCdcWrite(p_data, length);
      break;

  }

  return ret;
}

uint32_t uartPrintf(uint8_t ch, const char *fmt, ...)
{
  char buf[256];
  va_list args;
  int len;
  uint32_t ret;

  va_start(args, fmt);
  len = vsnprintf(buf, 256, fmt, args);

  ret = uartWrite(ch, (uint8_t *)buf, len);

  va_end(args);


  return ret;
}

uint32_t uartGetBaud(uint8_t ch)
{
  return uart_tbl[ch].baud;
}

uint8_t * uartGetRxBuffer(void)
{
  return send_buf;
}

uint8_t * uartGetWrBuffer(void)
{
  return uart_tbl[_DEF_UART3].wr_buf;
}

void setSocketUpateLen(size_t data)
{
  socket_wr_len = data;
}

size_t getSocketUpdateLen(void)
{
  return socket_rx_len;
}

void confirmSocketUpdatedLen(void)
{
  socket_rx_len = 0;
}


bool uartUsbCdcInit(void)
{
  bool ret ;
  esp_err_t err;

  qbufferCreate(&uart_tbl[_DEF_UART4].qbuffer, &uart_tbl[_DEF_UART4].rx_buf[0], UART_RX_Q_BUF_LEN);

  usb_serial_jtag_config.tx_buffer_size = 1024;
  usb_serial_jtag_config.rx_buffer_size = 1024;

  err = usb_serial_jtag_driver_uninstall();
  // ESP_LOGE("USB_CDC", "Err [%d] ", err);

  err = usb_serial_jtag_driver_install(&usb_serial_jtag_config);
  // ESP_LOGE("USB_CDC", "Err [%d] ", err);

  if (err == ESP_OK)
  {
    ret = true;
  }
  else
  {
    ret = false;
  }

  return ret;
}

uint32_t uartUsbCdcAvailable(void)
{
  uint32_t w_len;
  int      r_len;
  uint32_t ret;

  w_len = uart_tbl[_DEF_UART4].qbuffer.length - qbufferAvailable(&uart_tbl[_DEF_UART4].qbuffer) - 1;
  r_len = usb_serial_jtag_read_bytes(uart_tbl[_DEF_UART4].rx_buf, w_len, 0);

  qbufferWrite(&uart_tbl[_DEF_UART4].qbuffer, uart_tbl[_DEF_UART4].rx_buf, r_len);

  ret = qbufferAvailable(&uart_tbl[_DEF_UART4].qbuffer);

  return ret;
}

uint8_t uartUsbCdcRead(void)
{
  uint8_t ret;

  qbufferRead(&uart_tbl[_DEF_UART4].qbuffer, &ret, 1);

  return ret;
}

void uartUsbCdcDataIn(uint8_t rx_data)
{
  uartUsbCdcWrite(&rx_data, 1);
}

uint32_t uartUsbCdcWrite(uint8_t *p_data, uint32_t length)
{
  int ret;

  ret = usb_serial_jtag_write_bytes(p_data, length, 100);

  return (uint32_t)ret;
}


#endif
