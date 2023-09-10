#include <Arduino.h>
#include <cstdint>
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
  mb.begin(pin_sonar, false);
  vib.begin(pin_vibe, false, false);
  vib.setFrequency(150);
  vib.on();
  delay(100);
  vib.off();
  setPattern(0, 0, 00, 0); // クリア
#ifdef ARDUINO_XIAO_ESP32C3
  xTaskCreateUniversal(rangingTask, "RangingTask", 2048, nullptr, 5, nullptr, 0);
#endif
}
uint16_t patterns[][4] = {
  {0, 0, 0, 0},
  {30, 30, 30, 200},
  {30, 30, 2000, 0},
  {30, 30, 30, 500},
  {50, 50, 50, 500},
  {50, 50, 500, 0},
  {100, 100, 100, 500},
  {100, 100, 500, 0},
  {200, 200, 500, 0},
};

void loop() {
  feedback();
  static uint16_t previousUnit = 0;
  uint16_t unit = mb.getDistance() / 500;
  if (unit != previousUnit) {
    if (unit < 8) {
      previousUnit = unit;
      setPattern(patterns[unit][0], patterns[unit][1],
		 patterns[unit][2], patterns[unit][3]);
      // Serial.printf("%d \r", unit);
    }
  }
  // Serial.printf("%3d:%3d\r", previousUnit, unit);
  delay(5);
}
