#include <Arduino.h>
#include <iomanip>
#include "hardware.h"
#include "MB10xx.h"
#include "Vibrator.h"
#include "Feedback.h"

MB10xx mb;

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
void setup1() {
  mb.begin(pin_sonar, false);
  return;
}

void loop1()
{
  mb.ranging();
  delay(1); // いらないかも？
}
#endif

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Booting...");
  ioInit();
  vib.begin(pin_vibe, false, false);
  vib.setFrequency(1500);
  vib.on();
  delay(10);
  vib.off();
  setPattern(100, 100, 500, 0);
#ifdef ARDUINO_XIAO_ESP32C3
  xTaskCreateUniversal(rangingTask, "RangingTask", 2048, nullptr, 5, nullptr, 0);
#endif
}
  
void loop() {
  feedback();
  static uint16_t previousUnit = 0;
  uint16_t unit = mb.getDistance() / 500;
  if (unit != previousUnit) {
    previousUnit = unit;
    Serial.printf("%3d:%3d\r", previousUnit, unit);
  }
}
