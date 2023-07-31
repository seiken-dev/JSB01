#include <Arduino.h>
#include "Wire.h"
#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X
#ifdef ARDUINO_SEEED_XIAO_RP2040
#include <SingleFileDrive.h>
#include "LittleFS.h"
#endif

constexpr uint8_t pin_sonar = D5;
constexpr uint8_t pin_button1 = D0;
constexpr uint8_t pin_button2 = D1;
constexpr uint8_t pin_vibe = D2;

constexpr uint8_t flash_ms = 10;

uint16_t detectRange=2000;
bool changedDetectRange = false;

SFEVL53L1X distanceSensor;

void flash(uint16_t ms = flash_ms) {
  digitalWrite(pin_vibe, HIGH);
  delay(ms);
  digitalWrite(pin_vibe, LOW);
}

enum MB_Type {
  None,
  MB10X0,
  MB10X3,
};
MB_Type sonarType = None;

MB_Type detectMBType() {
  unsigned int f, s;
  if (pulseInLong(pin_sonar, HIGH)) {
    f = millis();
  }
  if(f && pulseInLong(pin_sonar, HIGH)) {
    s = millis();
  } else {
    return None;
  }
  auto type = (s-f < 70) ? MB10X0 : MB10X3;
  switch (type) {
    case MB10X0:
      Serial.println("MB10X0 detected");
      break;
      case MB10X3:
      Serial.println("MB10X3 detected");
      break;
      case None:
        Serial.println("Sonar sensor not detected");
        break;
  }
  return type;
}


bool detectVL() {
  Serial.println("Detecting VL53L1X");

  if (distanceSensor.begin() != 0) //Begin returns 0 on a good init
  {
    Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    return false;
  }
  return true;
}

uint16_t mbRanging(MB_Type type)
{
  if (type == None) {
    return 0;
  }
  uint32_t time = pulseIn(pin_sonar, HIGH, 200*1000);
  if (type == MB10X0) {
    time = time / 145 * 25;
  }
  return (time > 300) ? time : 0;
}

unsigned int vlRanging() {
  distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
  while (!distanceSensor.checkForDataReady())
  {
    delay(1);
  }
  int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();
  return distance;
}

unsigned int ranging() {
  unsigned int distance;
  if (sonarType == None) {
    distance = vlRanging();
  } else {
    distance = mbRanging(sonarType);
  }
  return distance;
}

bool isPressed(uint8_t button) {
  return (digitalRead(button)) ? false : true;
}
void loop1()
{
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
      delay(200);
      flash();
    }
    changedDetectRange = false;
  }
}

#ifdef ARDUINO_XIAO_ESP32C3
void UserCommandTask(void *pvParameters)
{
  while (1) {
    loop1();
    delay(1);
  }
}

#endif

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("start");
  pinMode(pin_button1, INPUT_PULLUP);
  pinMode(pin_button2, INPUT_PULLUP);
  pinMode(pin_vibe, OUTPUT);
  Wire.begin();
  if (detectVL() == false) {
    Wire.end();
    pinMode(pin_sonar, INPUT);
    sonarType = detectMBType();
  }
#ifdef ARDUINO_XIAO_ESP32C3
  xTaskCreateUniversal(UserCommandTask, "User command loop task", 2048, nullptr, 0, nullptr, 0);
#endif
}

void loop() {
  uint16_t distance = ranging();
  if (distance && distance < detectRange) {
    if (!changedDetectRange) flash();
  }
}

#ifdef ARDUINO_SEEED_XIAO_RP2040
void setup1() { return; }
#endif

