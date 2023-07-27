#include "Arduino.h"
#include "pins_arduino.h"

// Pin definitions

constexpr uint8_t sonar = PIN_PA1;
constexpr uint8_t sonarEn = PIN_PA2;
constexpr uint8_t vibe = PIN_PA3;
constexpr uint8_t downSw = PIN_PA7;
constexpr uint8_t upSw = PIN_PA6;

static uint16_t distance = 0;

void measurement() {
  distance = pulseIn(sonar, HIGH, 100*1000) / 147;
  if (distance < 300) distance = 0;
}

void feedback() {
  digitalWriteFast(vibe, HIGH);
  delay(10);
  digitalWriteFast(vibe, LOW);
}

void setup() {
  pinModeFast(sonar, INPUT);
  pinModeFast(vibe, OUTPUT);
  pinModeFast(upSw, INPUT_PULLUP);
  pinModeFast(downSw, INPUT_PULLUP);
  pinModeFast(sonarEn, OUTPUT);
  digitalWriteFast(sonarEn, HIGH);
  feedback();
}

void loop() {
  if (digitalReadFast(upSw) == LOW) feedback();
  if (digitalReadFast(downSw) == LOW) feedback();
  measurement();
  if (distance) {
    delay(distance/3);
    feedback();
  }
}
