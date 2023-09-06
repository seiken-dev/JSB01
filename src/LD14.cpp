#include <Arduino.h>
#include <cstdint>
#include "LD14.h"

#ifdef ARDUINO_SEEED_XIAO_RP2040
#include "hardware/pwm.h"
#endif

constexpr uint8_t pwmCH = 0;
constexpr uint32_t pwmFreq = 150;
constexpr uint8_t pwmResolution = 12;
#ifdef ARDUINO_XIAO_ESP32C3
hw_timer_t *vibeClock = nullptr;
#endif
bool LD14::begin(uint8_t pin, bool init, bool ledc)
{
  if (init && !ledc) {
    pinMode(pin, OUTPUT);
  }
  _pin = pin;
  if (init && ledc) {
#ifdef ARDUINO_SEEED_XIAO_RP2040
    gpio_set_function(_pin, GPIO_FUNC_PWM);
    _pwmSlice = pwm_gpio_to_slice_num(_pin);
    pwm_set_clkdiv(_pwmSlice,
		   (125000000l / (pwmFreq * (1 << pwmResolution)))
		   );
    pwm_set_wrap(_pwmSlice, (1 << pwmResolution)-1);
    pwm_set_chan_level(_pwmSlice, PWM_CHAN_A, 1 << (pwmResolution-1));
#else
    vibeClock = timerBegin(0, 80, true);
    timerAttachInterrupt(vibeClock, vibeOff, true);
    ledcSetup(pwmCH, pwmFreq, pwmResolution);
    ledcAttachPin(_pin, pwmCH);
#endif
  }
  _ledc = ledc;
  return true;
}

#ifdef ARDUINO_SEEED_XIAO_RP2040
void LD14::on() {
  if (_ledc) {
    pwm_set_enabled(_pwmSlice, true);
  } else {
    digitalWrite(_pin, HIGH);
  }
}

void LD14::on(uint16_t ms)
{
  on();
  add_alarm_in_ms(ms, vibeOffCB, nullptr, false);
}

void LD14::off() {
  if (_ledc) {
    pwm_set_enabled(_pwmSlice, false);
  } else {
    digitalWrite(_pin, LOW);
  }
}
void LD14::setFrequency(uint32_t f)
{
  if (_ledc) {
    pwm_set_enabled(_pwmSlice, false);
    pwm_set_clkdiv(_pwmSlice,
		   (125000000l / (f * (1 << pwmResolution)))
		   );
  }
}
#else
void IRAM_ATTR LD14::on() {
  if (_ledc) {
    ledcWrite(pwmCH, 1 << (pwmResolution - 1));
  } else {
    digitalWrite(_pin, HIGH);
  }
}

void IRAM_ATTR LD14::off() {
  if (_ledc) {
    ledcWrite(pwmCH, 0);
  } else {
    digitalWrite(_pin, LOW);
  }
}

void LD14::setFrequency(uint32_t f)
{
  if (_ledc) ledcChangeFrequency(pwmCH, f, pwmResolution);
}
#endif

int64_t LD14::vibeOffCB(alarm_id_t id, void *user_data)
{
  off();
  return 0;
}

uint8_t LD14::_pin;
bool LD14::_ledc;
uint8_t LD14::_pwmSlice;
