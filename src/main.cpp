#include <Arduino.h>
#include "esp32-hal-timer.h"
#include "esp_attr.h"
#include "hardware.h"
#include "MB10xx.h"
#include "LD14.h"

MB10xx mb;
LD14 vibe;

void ioInit() {
  pinMode(pin_sonar, INPUT);
  // pinMode(pin_vibe, OUTPUT);
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
void IRAM_ATTR vibeFunc(void) {
  return;
}
#endif


void setup() {
  Serial.begin();
  delay(2000);
  Serial.println("Booting...");
  ioInit();
  mb.begin(pin_sonar, false);
  vibe.begin(pin_vibe, true, false);
  vibe.on();
  delay(15);
  vibe.off();
#ifdef ARDUINO_XIAO_ESP32C3
  vibeClock = timerBegin(3, 80, true);
  timerAttachInterrupt(vibeClock, vibeFunc, true);
  timerAlarmWrite(vibeClock, 10*1000, true);
  xTaskCreateUniversal(rangingTask, "RangingTask", 2048, nullptr, 5, nullptr, 0);
#endif
}
  
void loop() {
}
