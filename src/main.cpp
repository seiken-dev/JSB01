#include <Arduino.h>
#include "hardware.h"
#include "MB10xx.h"
#include "Vibrator.h"

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
void loop1()
{
  mb.ranging();
  // delay(1); // いらないかも？
}
#endif

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Booting...");
  ioInit();
  mb.begin(pin_sonar, false);
  vib.begin(pin_vibe, false, false);
  vib.setFrequency(1500);
  vib.on();
  delay(10);
  vib.off();
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
    vib.on(15);
  }
  if (millis() > expire) {
    vib.on(15);
    expire = millis()+period;
  }
  previousUnit = unit;
}
