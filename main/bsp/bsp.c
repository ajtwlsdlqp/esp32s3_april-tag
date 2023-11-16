/*
 * bsp.c
 *
 *  Created on: 2021. 1. 8.
 *      Author: HanCheol Cho
 */




#include "bsp.h"





void bspInit(void)
{
  esp_log_level_set("*", ESP_LOG_INFO);
}

void delay(uint32_t delay_ms)
{
  vTaskDelay(delay_ms / portTICK_PERIOD_MS);
}

uint32_t IRAM_ATTR micros(void)
{
    return (uint32_t ) (esp_timer_get_time());
}

uint32_t  IRAM_ATTR millis(void)
{
    return (uint32_t ) (esp_timer_get_time() / 1000ULL);
}

uint32_t bspGetCpuFreqMhz(void)
{
  rtc_cpu_freq_config_t conf;
  rtc_clk_cpu_freq_get_config(&conf);

  return conf.freq_mhz;
}
