#include <Arduino.h>
#include "hardware.h"
#include "MB10xx.h"
#include "LD14.h"

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
#elif defined ARDUINO_SEEED_XIAO_RP2040
void loop1()
{
  mb.ranging();
  // delay(1); // いらないかも？
}
#endif

void setup() {
  Serial.begin();
  delay(2000);
  Serial.println("Booting...");
  ioInit();
  mb.begin(pin_sonar, false);
  vibe.begin(pin_vibe, false, false);
  vibe.setFrequency(1500);
  vibe.on();
  delay(50);
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
  vibe.on();
  delay(5);
  vibe.off();
  delay(85);
  Serial.printf("%u  \r", mb.getPrev());
}
