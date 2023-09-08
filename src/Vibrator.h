#pragma once

#include "Arduino.h"
#include <cstdint>

class Vibrator {
 public:
  bool begin(uint8_t pin, bool init, bool ledc);
  static void on();
  static void on(uint32_t f, uint16_t ms);
  static void on(uint16_t ms);
  static void off();
  static void setFrequency(uint32_t f);
  static bool isOn() { return _on; };
 private:
  static uint8_t _pin;
  static bool _ledc;
  static uint8_t _pwmSlice;
  static bool _on;
#ifdef ARDUINO_SEEED_XIAO_RP2040
  static int64_t vibOffCB(alarm_id_t id, void *user_data);
#endif
};
extern Vibrator vib;
