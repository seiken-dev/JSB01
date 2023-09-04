#pragma once

#include "Arduino.h"

class LD14 {
 public:
  bool begin(uint8_t pin, bool init, bool ledc);
  void on();
  void off();
 private:
  uint8_t _pin;
  bool _ledc;
};
