#pragma once
// Maxbotic ultrasonic sensor MB10xx
#include <cstdint>

class MB10xx {
  public:
  enum mbtype_t {
    mb_none,
    mb_10x0,
    mb_10x3,
  };
  bool begin(uint8_t pin, bool init);
  uint32_t ranging();
  uint32_t getPrev() { return _currentDistance; }
  mbtype_t detectMb();
private:
  uint8_t _pin;
  mbtype_t _type;
  uint32_t _currentDistance;
};
