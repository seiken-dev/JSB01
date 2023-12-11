#include <Arduino.h>
#include "Vibrator.h"

bool Vibrator::begin(uint8_t pin, bool init)
{
  if (init) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  _pin = pin;
  return true;
}

void Vibrator::on() {
  tone(_pin, _freq);
  _on = true;
}

void Vibrator::on(uint32_t f, uint16_t ms)
{
  _freq = f;
  tone(_pin, _freq, ms);
}

void Vibrator::on(uint16_t ms)
{
  tone(_pin, _freq, ms);
}

void Vibrator::off() {
  noTone(_pin);
  _on = false;
}

void Vibrator::setFrequency(unsigned int f)
{
  _freq = f;
}

uint8_t Vibrator::_pin;
bool Vibrator::_on;
unsigned int Vibrator::_freq;
Vibrator vib;
