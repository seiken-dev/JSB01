#pragma once

#include "Arduino.h"

class LD14 {
 public:
  bool begin(uint8_t pin, bool init, bool ledc);
  static void on();
  static void on(uint16_t ms);
  static void off();
  static void setFrequency(uint32_t f);
 private:
  static uint8_t _pin;
  static bool _ledc;
  static uint8_t _pwmSlice;
#ifdef ARDUINO_SEEED_XIAO_RP2040
  static int64_t vibeOffCB(alarm_id_t id, void *user_data);
#endif
};
