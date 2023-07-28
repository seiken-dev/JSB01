#include <Arduino.h>
#include <cstdint>
constexpr uint8_t pin_sonar = D5;
constexpr uint8_t pin_button1 = D0;
constexpr uint8_t pin_button2 = D1;
constexpr uint8_t pin_vibe = D2;

constexpr uint8_t flash_ms = 10;

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
  delay(400); // wait USB Serial
  flash();;
  delay(100);
  flash();
}

uint16_t ranging()
{
  uint32_t time = pulseIn(pin_sonar, HIGH, 200*1000);
  return (time > 10*147) ? uint16_t(time/147) : 0;
}
void loop() {
  uint16_t distance = ranging();
  if (distance < 80) flash();
}
