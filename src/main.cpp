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
#else
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
  delay(10);
  vibe.off();
#ifdef ARDUINO_XIAO_ESP32C3
  xTaskCreateUniversal(rangingTask, "RangingTask", 2048, nullptr, 5, nullptr, 0);
#endif
}
  
void loop() {
  static uint16_t previousUnit = 0;
  static unsigned int expire = 0;
  uint16_t unit = mb.getDistance() / 400;
  uint16_t period = unit * 100; // バイブ間隔
  Serial.printf("%-5d    \r", period);
  if (unit == 0) return;
  if (previousUnit != unit) {
    expire = millis()+period;
    vibe.on(15);
  }
  if (millis() > expire) {
    vibe.on(15);
    expire = millis()+period;
  }
  previousUnit = unit;
}
