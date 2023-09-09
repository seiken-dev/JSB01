#include <Arduino.h>
#include <cstdint>
#include "Vibrator.h"

#ifdef ARDUINO_SEEED_XIAO_RP2040
#include "hardware/pwm.h"
#endif

constexpr uint8_t pwmCH = 0;
constexpr uint32_t pwmFreq = 126;
constexpr uint8_t pwmResolution = 13;
#ifdef ARDUINO_XIAO_ESP32C3
hw_timer_t *vibeClock = nullptr;
#endif
bool Vibrator::begin(uint8_t pin, bool init, bool ledc)
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
		   (float(F_CPU) / (pwmFreq * (1 << pwmResolution)))
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
  _on = false;
  return true;
}

#ifdef ARDUINO_SEEED_XIAO_RP2040
void Vibrator::on() {
  if (_ledc) {
    pwm_set_enabled(_pwmSlice, true);
  } else {
    digitalWrite(_pin, HIGH);
  }
  _on = true;
}

void Vibrator::on(uint16_t ms)
{
  on();
  add_alarm_in_ms(ms, vibOffCB, nullptr, false);
}

void Vibrator::off() {
  if (_ledc) {
    pwm_set_enabled(_pwmSlice, false);
  } else {
    digitalWrite(_pin, LOW);
  }
  _on = false;
}
void Vibrator::setFrequency(uint32_t f)
{
  if (_ledc) {
    pwm_set_enabled(_pwmSlice, false);
    pwm_set_clkdiv(_pwmSlice,
		   (float(F_CPU) / (f * (1 << pwmResolution)))
		   );
  }
}
#else
void IRAM_ATTR Vibrator::on() {
  if (_ledc) {
    ledcWrite(pwmCH, 1 << (pwmResolution - 1));
  } else {
    digitalWrite(_pin, HIGH);
  }
}

void IRAM_ATTR Vibrator::off() {
  if (_ledc) {
    ledcWrite(pwmCH, 0);
  } else {
    digitalWrite(_pin, LOW);
  }
}

void Vibrator::setFrequency(uint32_t f)
{
  if (_ledc) ledcChangeFrequency(pwmCH, f, pwmResolution);
}
#endif

int64_t Vibrator::vibOffCB(alarm_id_t id, void *user_data)
{
  off();
  return 0;
}

uint8_t Vibrator::_pin;
bool Vibrator::_ledc;
uint8_t Vibrator::_pwmSlice;
bool Vibrator::_on;
Vibrator vib;
