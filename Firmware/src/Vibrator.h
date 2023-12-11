#pragma once

#include "Arduino.h"

class Vibrator {
 public:
  static bool begin(uint8_t pin, bool init=true);
  static void on();
  static void on(uint32_t f, uint16_t ms);
  static void on(uint16_t ms);
  static void off();
  static void setFrequency(unsigned int f);
  static bool isOn() { return _on; };
 private:
  static uint8_t _pin;
  static bool _on;
  static unsigned int _freq;
};

extern Vibrator vib;
