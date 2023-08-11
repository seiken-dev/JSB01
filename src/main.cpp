#include <Arduino.h>
#include "Wire.h"
#include "SparkFun_VL53L5CX_Library.h"
#include "VibeFeedback.h"
#include "UserInput.h"
#include "hardware.h"

#ifdef ARDUINO_SEEED_XIAO_RP2040
#include <SingleFileDrive.h>
#include "LittleFS.h"
#endif

SparkFun_VL53L5CX ToF;

uint detectRangeList[] = {400, 700, 1000, 1200, 1500, 2000, 3000, 4000};
uint detectRangeNum = 4;
// ユーザ操作定義
enum class UserCmd : uint8_t {
  none=0,
  range_up,
  range_down,
  wide,
  narrow,
  reset=0x10
};

// 測距センタータイプ
enum class SensorType: uint8_t {
  None,
  MB10X0,
  MB10X3,
  VL53L1X,
  VL53L5CX
};
SensorType sensor = SensorType::None;

SensorType detectMBType() {
  unsigned int f, s;
  if (pulseInLong(pin_sonar, HIGH)) {
    f = millis();
  }
  if(f && pulseInLong(pin_sonar, HIGH)) {
    s = millis();
  } else {
    return SensorType::None;
  }
  SensorType type = (s-f < 70) ? SensorType::MB10X0 : SensorType::MB10X3;
  return type;
}

SensorType detectVL() {
  Serial.println("Detecting VL53L1X");
  if (!ToF.begin())
  {
    Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    return SensorType::None;
  }
  return SensorType::VL53L1X;
}

uint16_t mbRanging(SensorType type)
{
  if (type == SensorType::None) {
    return 0;
  }
  uint32_t time = pulseIn(pin_sonar, HIGH, 200*1000);
  if (type == SensorType::MB10X0) {
    time = time / 145 * 25;
  }
  return (time > 300) ? time : 0;
}

unsigned int vlRanging() {
  return 0;
}

unsigned int ranging() {
  unsigned int distance=0;
  if (sensor == SensorType::VL53L1X) {
    distance = vlRanging();
  } else {
    distance = mbRanging(sensor);
  }
  return distance;
}

#ifdef ARDUINO_XIAO_ESP32C3
void UserCommandTask(void *pvParameters)
{
  while (1) {
    loop1();
  }
}
#endif

#ifdef ARDUINO_SEEED_XIAO_RP2040
File logFile;
bool isLogging = false;

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

void onClick1(void) {
  if (detectRangeNum < 7) detectRangeNum++;
  Serial.printf("%u ", detectRangeList[detectRangeNum]);
}

void onPress1(void)
{
  Serial.print("L1 ");
}

void onClick2(void)
{
  if (detectRangeNum > 0) detectRangeNum--;
  Serial.printf("%u ", detectRangeList[detectRangeNum]);
}

void onPress2(void)
{
  Serial.print("L2 ");
}

void setup() {
  Serial.begin(115200);
  delay(500);
#ifdef ARDUINO_SEEED_XIAO_RP2040
  if (!LittleFS.begin()) {
    Serial.println("Filesystem initialization error.");
    return;
  }
  String logFileName = "vl53l1x.csv";
  logFile = LittleFS.open(logFileName.c_str(), "a");
  logFile.printf("time,distance,status,ambient,peak\n");
  isLogging=true;
  singleFileDrive.onDelete(deleteCSV);
  singleFileDrive.onPlug(plug);
  singleFileDrive.onUnplug(unplug);
  singleFileDrive.begin(logFileName.c_str(), logFileName.c_str());
#endif
  Serial.println("start");
  pinMode(pin_button1, INPUT_PULLUP);
  pinMode(pin_button2, INPUT_PULLUP);
  pinMode(pin_vibe, OUTPUT);
  Wire.begin();
  if (detectVL() != SensorType::None) {
    sensor = SensorType::VL53L1X;
  }
  else {
    Wire.end();
    pinMode(pin_sonar, INPUT);
    sensor = detectMBType();
  }

    cbButton1Click = onClick1;
  cbButton1Long = onPress1;
  cbButton2Click = onClick2;
  cbButton2Long = onPress2;
  inputBegin();

#ifdef ARDUINO_XIAO_ESP32C3
  xTaskCreateUniversal(UserCommandTask, "User command loop task", 2048, nullptr, 0, nullptr, 0);
#endif
}

void loop()
{
  uint16_t distance = ranging();
  // Serial.println(distance);

  if (distance < detectRangeList[detectRangeNum]) {
    // Feedback
    if (distance < 400) vibe.feedback(VibeFeedback::s_40);
    else if (distance < 700) vibe.feedback(VibeFeedback::s_70);
    else if (distance < 1000) vibe.feedback(VibeFeedback::s_100);
    else if (distance < 1200) vibe.feedback(VibeFeedback::s_120);
    else if (distance < 1500) vibe.feedback(VibeFeedback::s_150);
    else if (distance < 2000) vibe.feedback(VibeFeedback::s_200);
    else if (distance < 3000) vibe.feedback(VibeFeedback::s_300);
    else if (distance < 4000) vibe.feedback(VibeFeedback::s_400);
    else if (distance == 0) vibe.feedback(VibeFeedback::s__nodata);
  }
  else vibe.feedback(VibeFeedback::s__nodata);
}
