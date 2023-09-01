#ifndef __BOARD_H__
#define __BOARD_H__
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "soc/gpio_num.h"

// Pin definitions

#define EXTERNAL_NUM_INTERRUPTS 22
#define NUM_DIGITAL_PINS        22
#define NUM_ANALOG_INPUTS       6

#define analogInputToDigitalPin(p)  (((p)<NUM_ANALOG_INPUTS)?(analogChannelToDigitalPin(p)):-1)
#define digitalPinToInterrupt(p)    (((p)<NUM_DIGITAL_PINS)?(p):-1)
#define digitalPinHasPWM(p)         (p < EXTERNAL_NUM_INTERRUPTS)

class MainBoard {
 public:
  esp_err_t init();
  bool readSW(gpio_num_t sw);
  static constexpr gpio_num_t TX = GPIO_NUM_21;
  static constexpr gpio_num_t RX = GPIO_NUM_20;
  static constexpr gpio_num_t SDA = GPIO_NUM_6;
  static constexpr gpio_num_t SCL = GPIO_NUM_7;
  static constexpr gpio_num_t SS    = GPIO_NUM_20;
  static constexpr gpio_num_t MOSI  = GPIO_NUM_10;
  static constexpr gpio_num_t MISO  = GPIO_NUM_9;
  static constexpr gpio_num_t SCK   = GPIO_NUM_8;
  static constexpr gpio_num_t A0 = GPIO_NUM_2;
  static constexpr gpio_num_t A1 = GPIO_NUM_3;
  static constexpr gpio_num_t A2 = GPIO_NUM_4;
  static constexpr gpio_num_t A3 = GPIO_NUM_5;
  static constexpr gpio_num_t D0 = GPIO_NUM_2;
  static constexpr gpio_num_t D1 = GPIO_NUM_3;
  static constexpr gpio_num_t D2 = GPIO_NUM_4;
  static constexpr gpio_num_t D3 = GPIO_NUM_5;
  static constexpr gpio_num_t D4 = GPIO_NUM_6;
  static constexpr gpio_num_t D5 = GPIO_NUM_7;
  static constexpr gpio_num_t D6 = GPIO_NUM_21;
  static constexpr gpio_num_t D7 = GPIO_NUM_20;
  static constexpr gpio_num_t D8 = GPIO_NUM_8;
  static constexpr gpio_num_t D9 = GPIO_NUM_9;
  static constexpr gpio_num_t D10 = GPIO_NUM_10;
  static constexpr gpio_num_t buzzer = GPIO_NUM_10;
  static constexpr gpio_num_t sw1 = GPIO_NUM_7;
  static constexpr gpio_num_t sw2 = GPIO_NUM_0;
  static constexpr gpio_num_t vibe = GPIO_NUM_2;
 private:
  ledc_timer_config_t ledc_timer;
  ledc_channel_config_t ledc_channel;
  esp_err_t beepInit();
  esp_err_t gpioInit();
};
extern MainBoard xiao_c3;
#endif
