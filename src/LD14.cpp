#include <Arduino.h>
#include "LD14.h"
#include "esp32-hal-gpio.h"

constexpr uint8_t pwmCH = 0;
constexpr uint32_t pwmFreq = 150;
constexpr uint8_t pwmResolution = 10;

bool LD14::begin(uint8_t pin, bool init, bool ledc)
{
  if (init) {
    pinMode(pin, OUTPUT);
  }
  _pin = pin;
  if (init && ledc) {
    ledcSetup(pwmCH, pwmFreq, pwmResolution);
    ledcAttachPin(_pin, pwmCH);
  }
  _ledc = ledc;
  return true;
}

void LD14::on() {
  if (_ledc) {
    ledcWrite(pwmCH, 1 << (pwmResolution - 1));
  } else {
    digitalWrite(_pin, HIGH);
  }
}

void LD14::off() {
  if (_ledc) {
    ledcWrite(pwmCH, 0);
  } else {
    digitalWrite(_pin, LOW);
  }
}
