#include <Arduino.h>

constexpr uint8_t pin_sonar = D5;
constexpr uint8_t pin_button1 = D0;
constexpr uint8_t pin_button2 = D1;
constexpr uint8_t pin_vibe = D2;

constexpr uint8_t flash_ms = 10;

uint16_t detectRange=2000;
bool changedDetectRange = false;

void flash(uint16_t ms = flash_ms) {
  digitalWrite(pin_vibe, HIGH);
  delay(ms);
  digitalWrite(pin_vibe, LOW);
}
enum MB_Type {
  MB10X0,
  MB10X3,
  None,
};
MB_Type sonarType = MB10X0;

MB_Type detectMBType() {
  uint f, s;
  if (pulseInLong(pin_sonar, HIGH)) {
    f = millis();
  }
  if(f && pulseInLong(pin_sonar, HIGH)) {
    s = millis();
  }
  return (s-f < 70) ? MB10X0 : MB10X3;
}
  
uint16_t ranging(MB_Type type)
{
  uint32_t time = pulseIn(pin_sonar, HIGH, 200*1000);
  if (type == MB10X0) {
    time = time / 145 * 25;
  }
  Serial.printf("%lu\n", time);
  return (time > 300) ? time : 0;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("start");
  pinMode(pin_sonar, INPUT);
  pinMode(pin_button1, INPUT_PULLUP);
  pinMode(pin_button2, INPUT_PULLUP);
  pinMode(pin_vibe, OUTPUT);
  sonarType = detectMBType();
  Serial.printf("MB_TYPE: %d\n", sonarType);
}

void loop() {
  uint16_t distance = ranging(sonarType);
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
