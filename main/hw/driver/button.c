/*
 * button.c
 *
 *  Created on: 2020. 2. 1.
 *      Author: Baram
 */




#include "driver/gpio.h"
#include "button.h"
#include "cli.h"


typedef struct
{
  bool          virtual;
  uint32_t      pin;
  uint8_t       on_state;
  uint8_t       off_state;
} button_tbl_t;




const button_tbl_t button_port_tbl[BUTTON_MAX_CH] =
{
  {false, 15, _DEF_LOW, _DEF_HIGH},  // MODE
};



typedef struct
{
  bool        pressed;
  bool        pressed_event;
  uint16_t    pressed_cnt;
  uint32_t    pressed_start_time;
  uint32_t    pressed_end_time;

  bool        released;
  bool        released_event;
  uint32_t    released_start_time;
  uint32_t    released_end_time;

  bool        repeat_update;
  uint32_t    repeat_cnt;
  uint32_t    repeat_time_detect;
  uint32_t    repeat_time_delay;
  uint32_t    repeat_time;

} button_t;


static button_t button_tbl[BUTTON_MAX_CH];


#ifdef _USE_HW_CLI
void cliButton(cli_args_t *args);
#endif

static bool buttonGetPin(uint8_t ch);

static volatile uint32_t any_btn_cumulative_cnt;
static bool is_init = false;
static bool is_enable = true;

static bool is_control;
static uint32_t control_time;
static uint32_t control_pressed_time;
static uint32_t control_released_time;

void button_isr(void)
{
  uint8_t i;
//  (void)arg;
  uint32_t repeat_time;

  for (i=0; i<BUTTON_MAX_CH; i++)
  {

    if (buttonGetPin(i))
    {
      if (button_tbl[i].pressed == false)
      {
        button_tbl[i].pressed_event = true;
        button_tbl[i].pressed_start_time = millis();

        if(millis() - control_pressed_time > 2000 &&
            millis() - control_released_time > 2000)
        {
          control_time = millis();
        }
        if(millis() - control_time > 10 * 1000)
        {
          is_control = true;
        }
        control_pressed_time = millis();
      }

      button_tbl[i].pressed = true;
      button_tbl[i].pressed_cnt++;
      any_btn_cumulative_cnt++;

      if (button_tbl[i].repeat_cnt == 0)
      {
        repeat_time = button_tbl[i].repeat_time_detect;
      }
      else if (button_tbl[i].repeat_cnt == 1)
      {
        repeat_time = button_tbl[i].repeat_time_delay;
      }
      else
      {
        repeat_time = button_tbl[i].repeat_time;
      }
      if (button_tbl[i].pressed_cnt >= repeat_time)
      {
        button_tbl[i].pressed_cnt = 0;
        button_tbl[i].repeat_cnt++;
        button_tbl[i].repeat_update = true;
      }

      button_tbl[i].pressed_end_time = millis();

      button_tbl[i].released = false;
    }
    else
    {
      if (button_tbl[i].pressed == true)
      {
        button_tbl[i].released_event = true;
        button_tbl[i].released_start_time = millis();

        if(millis() - control_pressed_time > 2000 &&
            millis() - control_released_time > 2000)
        {
          control_time = millis();
        }
        if(millis() - control_time > 10 * 1000)
        {
          is_control = true;
        }
        control_released_time = millis();
      }

      button_tbl[i].pressed  = false;
      button_tbl[i].released = true;
      button_tbl[i].repeat_cnt = 0;
      button_tbl[i].repeat_update = false;

      button_tbl[i].released_end_time = millis();
    }
  }
}



bool buttonInit(void)
{
  uint32_t i;
  gpio_config_t io_conf = {};

  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en = 1;


  for (i=0; i<BUTTON_MAX_CH; i++)
  {
    button_tbl[i].pressed_cnt    = 0;
    button_tbl[i].pressed        = 0;
    button_tbl[i].released       = 0;
    button_tbl[i].released_event = 0;

    button_tbl[i].repeat_cnt     = 0;
    button_tbl[i].repeat_time_detect = 50;
    button_tbl[i].repeat_time_delay  = 500;
    button_tbl[i].repeat_time        = 200;

    button_tbl[i].repeat_update = false;

    if (button_port_tbl[i].virtual != true)
    {
      io_conf.pin_bit_mask = 1ULL<<button_port_tbl[i].pin;
      gpio_config(&io_conf);
    }
  }

#ifdef _USE_HW_CLI
  cliAdd("button", cliButton);
#endif


  is_init = true;

  return true;
}

bool buttonIsInit(void)
{
  return is_init;
}

void buttonResetTime(uint8_t ch)
{
  button_tbl[ch].pressed_start_time    = 0;
  button_tbl[ch].pressed_end_time      = 0;
  button_tbl[ch].released_start_time   = 0;
  button_tbl[ch].released_end_time     = 0;

  button_tbl[ch].pressed_event = false;
  button_tbl[ch].released_event = false;
}

bool buttonGetPin(uint8_t ch)
{
  if (ch >= BUTTON_MAX_CH)
  {
    return false;
  }

  if (button_port_tbl[ch].virtual != true)
  {
    if (gpio_get_level(button_port_tbl[ch].pin) == button_port_tbl[ch].on_state)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool buttonGetControl(void)
{
  bool ret;
  ret = is_control;
  is_control = false;
  return ret;
}

void buttonEnable(bool enable)
{
  is_enable = enable;
}

bool buttonGetPressed(uint8_t ch)
{
  if (ch >= BUTTON_MAX_CH || is_enable == false)
  {
    return false;
  }

  return button_tbl[ch].pressed;
}

bool buttonOsdGetPressed(uint8_t ch)
{
  if (ch >= BUTTON_MAX_CH)
  {
    return false;
  }

  return button_tbl[ch].pressed;
}

uint8_t  buttonGetPressedCount(void)
{
  uint32_t i;
  uint8_t ret = 0;

  for (i=0; i<BUTTON_MAX_CH; i++)
  {
    if (buttonGetPressed(i) == true)
    {
      ret++;
    }
  }

  return ret;
}

bool buttonGetPressedEvent(uint8_t ch)
{
  bool ret;


  if (ch >= BUTTON_MAX_CH || is_enable == false) return false;

  ret = button_tbl[ch].pressed_event;

  button_tbl[ch].pressed_event = 0;

  return ret;
}

uint32_t buttonGetPressedTime(uint8_t ch)
{
  volatile uint32_t ret;


  if (ch >= BUTTON_MAX_CH || is_enable == false) return 0;


  ret = button_tbl[ch].pressed_end_time - button_tbl[ch].pressed_start_time;

  return ret;
}

void buttonSetRepeatTime(uint8_t ch, uint32_t detect_ms, uint32_t repeat_delay_ms, uint32_t repeat_ms)
{
  if (ch >= BUTTON_MAX_CH || is_enable == false) return;

  button_tbl[ch].repeat_update = false;
  button_tbl[ch].repeat_cnt = 0;
  button_tbl[ch].pressed_cnt = 0;

  button_tbl[ch].repeat_time_detect = detect_ms;
  button_tbl[ch].repeat_time_delay  = repeat_delay_ms;
  button_tbl[ch].repeat_time        = repeat_ms;
}

uint32_t buttonGetRepeatEvent(uint8_t ch)
{
  volatile uint32_t ret = 0;

  if (ch >= BUTTON_MAX_CH || is_enable == false) return 0;

  if (button_tbl[ch].repeat_update)
  {
    button_tbl[ch].repeat_update = false;
    ret = button_tbl[ch].repeat_cnt;
  }

  return ret;
}

bool buttonGetReleased(uint8_t ch)
{
  bool ret;


  if (ch >= BUTTON_MAX_CH || is_enable == false) return false;

  ret = button_tbl[ch].released;

  return ret;
}

bool buttonGetReleasedEvent(uint8_t ch)
{
  bool ret;


  if (ch >= BUTTON_MAX_CH || is_enable == false) return false;

  ret = button_tbl[ch].released_event;

  button_tbl[ch].released_event = 0;

  return ret;
}

uint32_t buttonGetReleasedTime(uint8_t ch)
{
  volatile uint32_t ret;


  if (ch >= BUTTON_MAX_CH || is_enable == false) return 0;


  ret = button_tbl[ch].released_end_time - button_tbl[ch].released_start_time;

  return ret;
}


bool buttonWasAnyInputForTimeMs(uint32_t time_ms)
{
  bool ret = true;
  static uint32_t pre_time_ms_to_check_any_input;
  static uint32_t pre_any_input_cnt;

  if(pre_any_input_cnt != any_btn_cumulative_cnt)
  {
    pre_time_ms_to_check_any_input = millis();
    pre_any_input_cnt = any_btn_cumulative_cnt;
  }
  else if(millis() - pre_time_ms_to_check_any_input >= time_ms)
  {
    ret = false;
  }

  if (time_ms == 0)
  {
    pre_time_ms_to_check_any_input = millis();
  }

  return ret;
}

bool buttonDetectInput(void)
{
  bool ret = true;
  static uint32_t pre_input_cnt;

  if(pre_input_cnt != any_btn_cumulative_cnt)    pre_input_cnt = any_btn_cumulative_cnt;
  else    ret = false;
  return ret;
}

#ifdef _USE_HW_CLI
void cliButton(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "show") == true)
  {
    while(cliKeepLoop())
    {
      for (int i=0; i<BUTTON_MAX_CH; i++)
      {
        cliPrintf("%d", buttonGetPressed(i));
      }
      cliPrintf("\n");
      delay(100);
    }
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "time") == true)
  {
    uint8_t ch;

    ch = (uint8_t)args->getData(1);

    while(cliKeepLoop())
    {
      cliPrintf("BUTTON%d, Time :  %d ms \n", ch+1, buttonGetPressedTime(ch));
      delay(100);
    }

    ret = true;
  }

  if (args->argc == 5 && args->isStr(0, "repeat") == true)
  {
    uint8_t ch;
    uint32_t repeat_time_detect;
    uint32_t repeat_time_delay;
    uint32_t repeat_time;

    ch = (uint8_t)args->getData(1);
    repeat_time_detect = (uint32_t)args->getData(2);
    repeat_time_delay  = (uint32_t)args->getData(3);
    repeat_time        = (uint32_t)args->getData(4);

    buttonSetRepeatTime(ch, repeat_time_detect, repeat_time_delay, repeat_time);

    while(cliKeepLoop())
    {
      uint32_t repeat_cnt;

      repeat_cnt = buttonGetRepeatEvent(ch);

      if(repeat_cnt > 0)
      {
        cliPrintf("BUTTON%d, Time :  %d ms, %d ms, %d ms, repeat %d\n", ch+1, repeat_time_detect, repeat_time_delay, repeat_time, repeat_cnt);
      }
      delay(100);
    }

    ret = true;
  }

  if (ret != true)
  {
    cliPrintf("button [show/time] channel(1~%d) ...\n", BUTTON_MAX_CH);
    cliPrintf("button repeat channel(1~%d) time_detect time_delay time_repeat \n", BUTTON_MAX_CH);
  }
}
#endif
