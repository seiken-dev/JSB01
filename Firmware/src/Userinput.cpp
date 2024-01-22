#include "Arduino.h"
#include "Userinput.h"

bool TactSw::init(uint8_t pin) {
  _pin = pin;
  pinMode(_pin, INPUT_PULLUP);
  _prev = false;
  _start = 0;
  _processed = false;
  return true;
}

TactSw::status_t TactSw::check() {
  TactSw::status_t ret = none;
  PinStatus status = digitalRead(_pin);
  if (status == LOW) {
    if (_prev == false) {
      _prev = true;
      _start = millis();
      ret = press;
    }
    else {
      if (millis()-_start > longThreshold && !_processed) {
	_processed = true;
	ret =  longpressed;
      }
      else if (millis()-_start > threshold)
	ret = pressing;
    }
  }
  else if (status == HIGH) {
    if (_prev) {
      if (millis()-_start > threshold && millis()-_start < longThreshold) {
	ret = pressed;
      }
      _start = 0;
      _prev = false;
      _processed = false;
    }
  }
  return ret;
}
