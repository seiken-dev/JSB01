#include <Arduino.h>
#include <cstdint>
#include "esp32-hal-gpio.h"
#include "esp32-hal-timer.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "hardware.h"
#include "MB10xx.h"
#include "LD14.h"
#include "pins_arduino.h"

MB10xx mb;
LD14 vibe;

void ioInit() {
  pinMode(pin_sonar, INPUT);
  pinMode(pin_vibe, OUTPUT);
  pinMode(pin_button1, INPUT_PULLUP);
  pinMode(pin_button2, INPUT_PULLUP);
}

#ifdef ARDUINO_XIAO_ESP32C3
void rangingTask(void *param)
{
  while(1) {
    mb.ranging();
    delay(1);
  }
}

hw_timer_t *vibeClock;
volatile bool vibeOn = false;
volatile uint16_t duration = 0;
volatile uint16_t passed = 0;
void IRAM_ATTR vibeFunc(void) {
  if (vibeOn && passed == 0) {
    vibe.on();
    vibeOn = false;
  }
  else if (passed >= duration) {
    vibe.off();
    passed = 0;
  } else {
    passed++;
  }
  return;
}
#endif

void setup() {
  Serial.begin();
  delay(2000);
  Serial.println("Booting...");
  ioInit();
  mb.begin(pin_sonar, false);
  vibe.begin(D10, true, true);
  vibe.setFrequency(1500);
  vibe.on();
  delay(15);
  vibe.off();
#ifdef ARDUINO_XIAO_ESP32C3
  vibeClock = timerBegin(0, 80, true);
  timerAttachInterrupt(vibeClock, vibeFunc, true);
  timerAlarmWrite(vibeClock, 5 * 1000, true);
  timerAlarmEnable(vibeClock);
  // xTaskCreateUniversal(rangingTask, "RangingTask", 2048, nullptr, 5, nullptr, 0);
#endif
}
  
void loop() {
  uint32_t s = millis();
  vibeOn = true;
  duration = 2;
  delay(500);
  Serial.printf("%u ms delayed  \r", millis()-s);
}
