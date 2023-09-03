#include <Arduino.h>
#include "hardware.h"
#include "MB10xx.h"

MB10xx mb;

void ioInit() {
  pinMode(pin_sonar, INPUT);
  pinMode(pin_vibe, OUTPUT);
  pinMode(pin_button1, INPUT_PULLUP);
  pinMode(pin_button2, INPUT_PULLUP);
}

void setup() {
  Serial.begin();
  delay(3000);
  Serial.println("Booting...");
  ioInit();
  mb.begin(pin_sonar, false);
}
  
void loop1() {}

void setup1() {}

void loop() {
  Serial.printf("%u  \r", mb.ranging());
  
}
