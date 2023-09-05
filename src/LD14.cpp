#include <Arduino.h>
#include "LD14.h"
#include "hardware/regs/pwm.h"
#include "hardware/sync.h"
#ifdef ARDUINO_SEEED_XIAO_RP2040
#include "hardware/pwm.h"
#endif

constexpr uint8_t pwmCH = 0;
constexpr uint8_t pwmSlice = 5;
constexpr uint32_t pwmFreq = 150;
constexpr uint8_t pwmResolution = 12;

bool LD14::begin(uint8_t pin, bool init, bool ledc)
{
  if (init && !ledc) {
    pinMode(pin, OUTPUT);
  }
  _pin = pin;
  if (init && ledc) {
#ifdef ARDUINO_SEEED_XIAO_RP2040
    Serial.println(pwm_gpio_to_slice_num(_pin));
    gpio_set_function(_pin, GPIO_FUNC_PWM);
    pwm_set_clkdiv(pwmSlice,
		   (125000000l / (pwmFreq * (1 << pwmResolution)))
		   );
    pwm_set_wrap(pwmSlice, (1 << pwmResolution)-1);
    pwm_set_chan_level(pwmSlice, PWM_CHAN_A, (1 << pwmResolution-1));
#else
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
    pwm_set_enabled(pwmSlice, true);
  } else {
    digitalWrite(_pin, HIGH);
  }
}  
void LD14::off() {
  if (_ledc) {
    pwm_set_enabled(pwmSlice, false);
  } else {
    digitalWrite(_pin, LOW);
  }
}
void LD14::setFrequency(uint32_t f)
{
  pwm_set_enabled(pwmSlice, false);
  pwm_set_clkdiv(pwmSlice,
		   (125000000l / (f * (1 << pwmResolution)))
		   );
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
