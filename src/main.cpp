#include "SerialUSB.h"
#include <Arduino.h>
#include <cstdint>
constexpr uint8_t pin_sonar = D5;
constexpr uint8_t pin_button1 = D0;
constexpr uint8_t pin_button2 = D1;
constexpr uint8_t pin_vibe = D2;

constexpr uint8_t flash_ms = 10;

static uint16_t detectRange=2000;
bool changedDetectRange = false;

void flash(uint16_t ms = flash_ms) {
  digitalWrite(pin_vibe, HIGH);
  delay(ms);
  digitalWrite(pin_vibe, LOW);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("start");
  pinMode(pin_sonar, INPUT);
  pinMode(pin_button1, INPUT_PULLUP);
  pinMode(pin_button2, INPUT_PULLUP);
  pinMode(pin_vibe, OUTPUT);
}

uint16_t ranging()
{
  uint32_t time = pulseIn(pin_sonar, HIGH, 200*1000);
  return (time > 300) ? time : 0;
}

void loop() {
  uint16_t distance = ranging();
  if (distance < detectRange) {
    if (!changedDetectRange) flash();
  }
}

void setup1() { return; }

bool isPressed(uint8_t button) {
  return (digitalReadFast(button)) ? false : true;
}

void loop1() {
  if (isPressed(pin_button1)) {
    if (detectRange < 4500) detectRange += 500;
    changedDetectRange = true;
  }
  else if (isPressed(pin_button2)) {
    if (detectRange > 0) detectRange -= 500;
    changedDetectRange = true;
  }
  if(changedDetectRange) {
    for (uint c=0; c < detectRange/500; c++) {
      flash();
      delay(200);
    }
    changedDetectRange = false;
  }
}
