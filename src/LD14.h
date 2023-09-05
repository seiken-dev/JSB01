#pragma once

#include "Arduino.h"
#include <cstdint>

class LD14 {
 public:
  bool begin(uint8_t pin, bool init, bool ledc);
  void on();
  void off();
  void setFrequency(uint32_t f);
 private:
  uint8_t _pin;
  bool _ledc;
};
