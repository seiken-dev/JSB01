#include <Arduino.h>
#include "Wire.h"
#include "SparkFun_VL53L5CX_Library.h"
#include "VibeFeedback.h"
#ifdef ARDUINO_SEEED_XIAO_RP2040
#include <SingleFileDrive.h>
#include "LittleFS.h"
#endif

SparkFun_VL53L5CX ToF;
constexpr uint8_t pin_sonar = D5;
constexpr uint8_t pin_button1 = D0;
constexpr uint8_t pin_button2 = D1;
constexpr uint8_t pin_vibe = D2;

constexpr uint8_t flash_ms = 10;

uint16_t detectRange=2000;
bool changedDetectRange = false;

File logFile;
bool isLogging = false;


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
  if (!ToF.begin())
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
  return 0;
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


#ifdef ARDUINO_XIAO_ESP32C3
void UserCommandTask(void *pvParameters)
{
  while (1) {
    loop1();
    delay(1);
  }
}

#endif

#ifdef ARDUINO_SEEED_XIAO_RP2040
// Called when the USB stick connected to a PC and the drive opened
// Note this is from a USB IRQ so no printing to SerialUSB/etc.
void plug(uint32_t i) {
  (void) i;
  isLogging = false;
  logFile.close();
}

// Called when the USB is ejected or removed from a PC
// Note this is from a USB IRQ so no printing to SerialUSB/etc.
void unplug(uint32_t i) {
  (void) i;
  isLogging = true;
}

// Called when the PC tries to delete the single file
// Note this is from a USB IRQ so no printing to SerialUSB/etc.
void deleteCSV(uint32_t i) {
  (void) i;
  LittleFS.remove("vl53l1x.csv");
}
#endif

void setup() {
#ifdef ARDUINO_SEEED_XIAO_RP2040
  if (!LittleFS.begin()) {
    Serial.println("Filesystem initialization error.");
    return;
  }
  String logFileName = "vl53l1x.csv";
  logFile = LittleFS.open(logFileName.c_str(), "a");
  logFile.printf("time,distance,status,ambient,peak\n");
  isLogging=true;
  delay(100);
  singleFileDrive.onDelete(deleteCSV);
  singleFileDrive.onPlug(plug);
  singleFileDrive.onUnplug(unplug);
  singleFileDrive.begin(logFileName.c_str(), logFileName.c_str());
#endif
  Serial.begin(115200);
  delay(3000);
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
  vibe.feedback(VibeFeedback::s_400);
  delay(3000);
  vibe.feedback(VibeFeedback::s__nodata);
  delay(3000);
  vibe.feedback(VibeFeedback::s_300);
  delay(3000);
  vibe.feedback(VibeFeedback::s_200);
  delay(3000);
  vibe.feedback(VibeFeedback::s_150);
  delay(3000);
  vibe.feedback(VibeFeedback::s_120);
  delay(3000);
  vibe.feedback(VibeFeedback::s_100);
  delay(3000);
  vibe.feedback(VibeFeedback::s_70);
  delay(3000);
  vibe.feedback(VibeFeedback::s_40);
  delay(3000);
}
