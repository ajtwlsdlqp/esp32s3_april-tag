/*
 * gpio.c
 *
 *  Created on: 2021. 1. 11.
 *      Author: HanCheol Cho
 */




#include "gpio.h"
#include "cli.h"
#include "driver/gpio.h"

typedef struct
{
  uint32_t pin_number;
  uint8_t  on_state;
  uint8_t  pin_level;
} led_port_t;

typedef struct
{
  uint32_t      pin;
  uint8_t       mode;
  uint8_t       on_state;
  uint8_t       off_state;
  uint8_t       pin_value;
  uint8_t       init_value;
} gpio_tbl_t;


static gpio_tbl_t gpio_tbl[GPIO_MAX_CH] =
    {
        {38,  _DEF_OUTPUT, 1, 0, 0, 1},       // 0. TP1
    };



#ifdef _USE_HW_CLI
static void cliGpio(cli_args_t *args);
#endif



bool gpioInit(void)
{
  bool ret = true;


  for (int i=0; i<GPIO_MAX_CH; i++)
  {
    gpioPinMode(i, gpio_tbl[i].mode);
    gpioPinWrite(i, gpio_tbl[i].init_value);
  }

#ifdef _USE_HW_CLI
  cliAdd("gpio", cliGpio);
#endif

  return ret;
}

bool gpioPinMode(uint8_t ch, uint8_t mode)
{
  bool ret = true;
  gpio_config_t io_conf;

  io_conf.intr_type     = GPIO_INTR_DISABLE;


  if (ch >= GPIO_MAX_CH)
  {
    return false;
  }

  switch(mode)
  {
    case _DEF_INPUT:
      io_conf.mode          = GPIO_MODE_INPUT;
      io_conf.pull_down_en  = GPIO_PULLDOWN_DISABLE;
      io_conf.pull_up_en    = GPIO_PULLUP_DISABLE;
      break;

    case _DEF_INPUT_PULLUP:
      io_conf.mode          = GPIO_MODE_INPUT;
      io_conf.pull_down_en  = GPIO_PULLDOWN_DISABLE;
      io_conf.pull_up_en    = GPIO_PULLUP_ENABLE;
      break;

    case _DEF_INPUT_PULLDOWN:
      io_conf.mode          = GPIO_MODE_INPUT;
      io_conf.pull_down_en  = GPIO_PULLDOWN_ENABLE;
      io_conf.pull_up_en    = GPIO_PULLUP_DISABLE;
      break;

    case _DEF_OUTPUT:
      io_conf.mode          = GPIO_MODE_OUTPUT;
      io_conf.pull_down_en  = GPIO_PULLDOWN_DISABLE;
      io_conf.pull_up_en    = GPIO_PULLUP_DISABLE;
      break;

    case _DEF_OUTPUT_PULLUP:
      io_conf.mode          = GPIO_MODE_OUTPUT;
      io_conf.pull_down_en  = GPIO_PULLDOWN_DISABLE;
      io_conf.pull_up_en    = GPIO_PULLUP_ENABLE;
      break;

    case _DEF_OUTPUT_PULLDOWN:
      io_conf.mode          = GPIO_MODE_OUTPUT;
      io_conf.pull_down_en  = GPIO_PULLDOWN_ENABLE;
      io_conf.pull_up_en    = GPIO_PULLUP_DISABLE;
      break;
  }

  io_conf.pin_bit_mask = (1<<gpio_tbl[ch].pin);
  gpio_config(&io_conf);

  return ret;
}

void gpioPinWrite(uint8_t ch, uint8_t value)
{
  if (ch >= GPIO_MAX_CH)
  {
    return;
  }

  if (value)
  {
    gpio_tbl[ch].pin_value = gpio_tbl[ch].on_state;
    gpio_set_level(gpio_tbl[ch].pin, gpio_tbl[ch].pin_value);
  }
  else
  {
    gpio_tbl[ch].pin_value = gpio_tbl[ch].off_state;
    gpio_set_level(gpio_tbl[ch].pin, gpio_tbl[ch].pin_value);
  }
}

uint8_t gpioPinRead(uint8_t ch)
{
  uint8_t ret = 0;

  if (ch >= GPIO_MAX_CH)
  {
    return false;
  }

  switch(gpio_tbl[ch].mode)
  {
    case _DEF_INPUT:
    case _DEF_INPUT_PULLUP:
    case _DEF_INPUT_PULLDOWN:
      if (gpio_get_level(gpio_tbl[ch].pin) == gpio_tbl[ch].on_state)
      {
        ret = 1;
      }
      break;

    case _DEF_OUTPUT:
    case _DEF_OUTPUT_PULLUP:
    case _DEF_OUTPUT_PULLDOWN:
      ret = gpio_tbl[ch].pin_value;
      break;
  }

  return ret;
}

void gpioPinToggle(uint8_t ch)
{
  if (ch >= GPIO_MAX_CH)
  {
    return;
  }

  gpio_tbl[ch].pin_value = !gpio_tbl[ch].pin_value;
  gpio_set_level(gpio_tbl[ch].pin, gpio_tbl[ch].pin_value);
}





#ifdef _USE_HW_CLI
void cliGpio(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "show") == true)
  {
    while(cliKeepLoop())
    {
      for (int i=0; i<GPIO_MAX_CH; i++)
      {
        cliPrintf("%d", gpioPinRead(i));
      }
      cliPrintf("\n");
      delay(100);
    }
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "read") == true)
  {
    uint8_t ch;

    ch = (uint8_t)args->getData(1);

    while(cliKeepLoop())
    {
      cliPrintf("gpio read %d : %d\n", ch, gpioPinRead(ch));
      delay(100);
    }

    ret = true;
  }

  if (args->argc == 3 && args->isStr(0, "write") == true)
  {
    uint8_t ch;
    uint8_t data;

    ch   = (uint8_t)args->getData(1);
    data = (uint8_t)args->getData(2);

    gpioPinWrite(ch, data);

    cliPrintf("gpio write %d : %d\n", ch, data);
    ret = true;
  }

  if (ret != true)
  {
    cliPrintf("gpio show\n");
    cliPrintf("gpio read ch[0~%d]\n", GPIO_MAX_CH-1);
    cliPrintf("gpio write ch[0~%d] 0:1\n", GPIO_MAX_CH-1);
  }
}
#endif
