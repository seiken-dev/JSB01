#include <Arduino.h>
#include "Vibrator.h"

#ifdef ARDUINO_SEEED_XIAO_RP2040
#include "hardware/pwm.h"
#endif

constexpr uint8_t pwmCH = 0;
constexpr uint32_t pwmFreq = 130;
constexpr uint8_t pwmResolution = 13;
#ifdef ARDUINO_XIAO_ESP32C3
hw_timer_t *vibClock = nullptr;
#endif
bool Vibrator::begin(uint8_t pin, bool init, bool ledc)
{
  if (init) {
    pinMode(pin, OUTPUT);
  }
  _pin = pin;
  if (init && ledc) {
#ifdef ARDUINO_XIAO_ESP32C3
    vibClock = timerBegin(0, 80, true);
    
    timerAttachInterrupt(vibClock, off, true);
    ledcSetup(pwmCH, pwmFreq, pwmResolution);
    ledcAttachPin(_pin, pwmCH);
#endif
    _freq = pwmFreq;
  }
  _ledc = ledc;
  _on = false;
  return true;
}

#ifdef ARDUINO_XIAO_ESP32C3
void IRAM_ATTR Vibrator::on() {
  if (_ledc) {
    ledcWrite(pwmCH, 1 << (pwmResolution - 1));
  } else {
    digitalWrite(_pin, HIGH);
  }
}

void IRAM_ATTR Vibrator::on(uint16_t ms) {
  timerWrite(vibClock, 0);
  on();
  timerAlarmWrite(vibClock, ms*1000, false);
  timerAlarmEnable(vibClock);
}

void IRAM_ATTR Vibrator::off() {
  if (_ledc) {
    ledcWrite(pwmCH, 0);
  } else {
    digitalWrite(_pin, LOW);
  }
}

void Vibrator::setFrequency(unsigned int f)
{
  if (_ledc) ledcChangeFrequency(pwmCH, f, pwmResolution);
  _freq = f;
}
#else
void Vibrator::on() {
  if (_ledc) {
    tone(_pin, _freq);
  } else {
    digitalWrite(_pin, HIGH);
  }
  _on = true;
}
void Vibrator::on(uint16_t ms)
{
  if (_ledc) {
    tone(_pin, _freq, ms);
  } else {
    on();
    add_alarm_in_ms(ms, vibOffCB, nullptr, false);
  }
}

void Vibrator::off() {
  if (_ledc) {
    noTone(_pin);
  } else {
    digitalWrite(_pin, LOW);
  }
  _on = false;
}
void Vibrator::setFrequency(unsigned int f)
{
  if (_ledc) {
    _freq = f;
  }
}

int64_t Vibrator::vibOffCB(alarm_id_t id, void *user_data)
{
  off();
  return 0;
}
#endif

uint8_t Vibrator::_pin;
bool Vibrator::_ledc;
bool Vibrator::_on;
unsigned int Vibrator::_freq;
Vibrator vib;
