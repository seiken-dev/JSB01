#include "Arduino.h"
#include "pins_arduino.h"

// Pin definitions

constexpr uint8_t sonar_pin = PIN_PA2;
constexpr uint8_t vibe_pin = PIN_PA3;
constexpr uint8_t downSw_pin = PIN_PA7;
constexpr uint8_t upSw_pin = PIN_PA6;

static uint16_t distance = 0;

void measurement() {
  distance = pulseIn(sonar_pin, HIGH, 100*1000) / 147;
  if (distance < 300) distance = 0;
}

void feedback() {
  digitalWriteFast(vibe_pin, HIGH);
  delay(10);
  digitalWriteFast(vibe_pin, LOW);
}

void setup() {
  pinModeFast(sonar_pin, INPUT);
  pinModeFast(vibe_pin, OUTPUT);
  pinModeFast(upSw_pin, INPUT_PULLUP);
  pinModeFast(downSw_pin, INPUT_PULLUP);
  delay(500);
  feedback();
}

void loop() {
  if (digitalReadFast(upSw_pin) == LOW) feedback();
  if (digitalReadFast(downSw_pin) == LOW) feedback();
  measurement();
  if (distance) {
    delay(distance/3);
    feedback();
  }
}
