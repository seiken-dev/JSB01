#include "hal/gpio_types.h"
#include "hal/ledc_types.h"
#include "sdkconfig.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "board.h"
#include "soc/gpio_num.h"
constexpr char tag[] = "Mainboard";

esp_err_t MainBoard::gpioInit() {
  ESP_ERROR_CHECK(gpio_set_direction(buzzer, GPIO_MODE_OUTPUT));
  ESP_ERROR_CHECK(gpio_set_direction(vibe, GPIO_MODE_OUTPUT));
  ESP_ERROR_CHECK(gpio_set_direction(sw1, GPIO_MODE_INPUT));
  ESP_ERROR_CHECK(gpio_set_direction(sw2, GPIO_MODE_INPUT));
  ESP_ERROR_CHECK(gpio_set_pull_mode(sw1, GPIO_PULLUP_ONLY));
  ESP_ERROR_CHECK(gpio_set_pull_mode(sw2, GPIO_PULLUP_ONLY));
  ESP_LOGI(tag, "%s", "GPIO initialization OK");
  return ESP_OK;
}

esp_err_t MainBoard::beepInit()
{
  ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;
  ledc_timer.duty_resolution = LEDC_TIMER_10_BIT;
  ledc_timer.timer_num = ledc_timer_t(LEDC_TIMER_MAX-1);
  ledc_timer.freq_hz = 2000;
  ledc_timer.clk_cfg = LEDC_AUTO_CLK;
  ledc_channel.gpio_num = buzzer;
  ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
  ledc_channel.channel = ledc_channel_t(LEDC_CHANNEL_MAX-1);
  ledc_channel.timer_sel = ledc_timer_t(LEDC_TIMER_MAX-1);
  ledc_channel.duty = 0; // (1 << ledc_timer.duty_resolution-1);
  ledc_timer_config(&ledc_timer);
  ledc_channel_config(&ledc_channel);
  ESP_LOGI(tag, "%s", "Beep initialization OK");
  ledc_set_freq(ledc_channel.speed_mode, ledc_timer.timer_num, 2000);
  ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 1 << (ledc_timer.duty_resolution-1));
  ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
  vTaskDelay(80 / portTICK_PERIOD_MS);
  // Beep off
  ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 0);
  ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
  ledc_set_freq(ledc_channel.speed_mode, ledc_timer.timer_num, 1000);
  // Beep on
  ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 1 << (ledc_timer.duty_resolution-1));
  ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
  vTaskDelay(80 / portTICK_PERIOD_MS);
  // Beep off
  ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 0);
  ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
  return ESP_OK;
}

esp_err_t MainBoard::init() {
  esp_err_t ret = gpioInit();
  if (!ret) beepInit();
  return ret;
}

bool MainBoard::readSW(gpio_num_t sw)
{
  return gpio_get_level(sw) ? false : true;
}

// Instance
MainBoard xiao_c3;
