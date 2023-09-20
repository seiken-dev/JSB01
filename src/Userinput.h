#pragma once

constexpr uint16_t threshold = 50;
constexpr uint16_t longThreshold = 800;

class TactSw {
public:
  enum status_t {
    none,
    press,
    pressed,
    longpressed,
    pressing,
    release,
  };
  bool init(uint8_t pin);
  uint32_t pressTime();
  status_t check();
private:
  uint8_t _pin;
  bool _prev;
  uint32_t _start;
  bool _processed;
};
