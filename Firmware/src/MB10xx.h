#pragma once
// Maxbotic ultrasonic sensor MB10xx
#include "Board.h"
#include <cstdint>

class MB10xx {
  public:
  enum mbtype_t {
    mb_none,
    mb_10x0,
    mb_10x3,
  };
  static bool begin(uint8_t pin, bool init);
  static uint32_t ranging();
  static uint32_t getDistance();
  mbtype_t GetMBType() { return _type; };
private:
  static mbtype_t _detectMb();
  static uint8_t _pin;
  static mbtype_t _type;
  static uint32_t _currentDistance;
};

extern MB10xx Sonar;
