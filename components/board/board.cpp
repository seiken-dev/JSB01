#include "esp_err.h"
#include "hal/gpio_types.h"
#include "nvs.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "board.h"

esp_err_t MainBoard::init() {
  ESP_ERROR_CHECK(gpio_set_direction(D1, GPIO_MODE_OUTPUT));
  ESP_ERROR_CHECK(gpio_set_direction(D2, GPIO_MODE_INPUT));
  return ESP_OK;
}

// Instance
MainBoard xiao_c3;
