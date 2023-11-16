/*
 * led.c
 *
 *  Created on: 2021. 1. 8.
 *      Author: HanCheol Cho
 */




#include "led.h"
#include "driver/ledc.h"


#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_MAX_DUTY           (8191) // Max duty. ((2 ** 13) - 1) = 8191


typedef struct
{
  uint32_t pin_number;
  uint8_t  on_state;
  uint8_t  pin_level;
} led_port_t;


static led_port_t led_port_tbl[LED_MAX_CH] =
{
    {48, 0, 0},
    {40, 0, 0},
    {39, 0, 0},
};


static bool is_init = false;


bool ledInit(void)
{
  ledc_timer_config_t ledc_timer;
  ledc_channel_config_t ledc_channel;

  ledc_timer.speed_mode =       LEDC_MODE;
  ledc_timer.duty_resolution =  LEDC_DUTY_RES;
  ledc_timer.freq_hz =          5000;
  ledc_timer.clk_cfg =          LEDC_AUTO_CLK;

  ledc_channel.speed_mode =     LEDC_MODE;
  ledc_channel.intr_type =      LEDC_INTR_DISABLE;
  ledc_channel.duty =           0;
  ledc_channel.hpoint =         0;

  for (int i=0; i<LED_MAX_CH; i++)
  {
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer.timer_num = i;
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel.channel = i;
    ledc_channel.timer_sel = i;
    ledc_channel.gpio_num = led_port_tbl[i].pin_number;
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ledOff(i);
  }

  is_init = true;

  return is_init;
}

bool ledIsInit(void)
{
  return is_init;
}

void ledOn(uint8_t ch)
{
  if (ch >= LED_MAX_CH) return;
  uint16_t duty = led_port_tbl[ch].on_state * LEDC_MAX_DUTY;

  led_port_tbl[ch].pin_level = led_port_tbl[ch].on_state;

  // Set duty
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, ch, duty));
  // Update duty to apply the new value
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, ch));
}

void ledOff(uint8_t ch)
{
  if (ch >= LED_MAX_CH) return;
  uint16_t duty = (1 - led_port_tbl[ch].on_state) * LEDC_MAX_DUTY;

  led_port_tbl[ch].pin_level = !led_port_tbl[ch].on_state;

  // Set duty
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, ch, duty));
  // Update duty to apply the new value
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, ch));
}

void ledToggle(uint8_t ch)
{
  if (ch >= LED_MAX_CH) return;

  led_port_tbl[ch].pin_level = !led_port_tbl[ch].pin_level;

  // Set duty
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, ch, led_port_tbl[ch].pin_level * LEDC_MAX_DUTY));
  // Update duty to apply the new value
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, ch));
}

void ledSetDuty(uint8_t ch, uint8_t duty)
{
  if (ch >= LED_MAX_CH) return;
  if (duty > 100) duty = 100;

  if (!led_port_tbl[ch].on_state)
  {
    duty = 100 - duty;
  }

  // Set duty
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, ch, LEDC_MAX_DUTY * duty / 100));
  // Update duty to apply the new value
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, ch));
}
