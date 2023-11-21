#pragma once

#include "Arduino.h"
#include <cstdint>

class Vibrator {
 public:
  static bool begin(uint8_t pin, bool init=true, bool ledc=true);
  static void on();
  static void on(uint32_t f, uint16_t ms);
  static void on(uint16_t ms);
  static void off();
  static void setFrequency(unsigned int f);
  static bool isOn() { return _on; };
 private:
  static uint8_t _pin;
  static bool _ledc;
  static bool _on;
  static unsigned int _freq;
#ifdef ARDUINO_RASPBERRY_PI_PICO
  static int64_t vibOffCB(alarm_id_t id, void *user_data);
#endif
};
extern Vibrator vib;
