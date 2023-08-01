#include <Arduino.h>
#include "Wire.h"
#include "VL53L1X.h"
#ifdef ARDUINO_SEEED_XIAO_RP2040
#include <SingleFileDrive.h>
#include "LittleFS.h"
#endif

VL53L1X Tof;
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
  if (!Tof.init())
  {
    Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    return false;
  }
  Tof.setDistanceMode(VL53L1X::Long);
  Tof.setMeasurementTimingBudget(50000);
  Tof.startContinuous(50);
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
  Tof.read();
  uint16_t distance = Tof.ranging_data.range_mm;
  auto status = Tof.ranging_data.range_status;
  float ambient = Tof.ranging_data.ambient_count_rate_MCPS;
  float peak = Tof.ranging_data.peak_signal_count_rate_MCPS;
  if (isLogging) {
    logFile.printf("%lu,%u,%d,%f,%f\n", millis(), distance, status, ambient, peak);
  }
  if (status == 0) {
    distance = Tof.ranging_data.range_mm;
  }
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
    if (detectRange > 0) {
      detectRange -= 500;
      if (detectRange == 0)  {
        isLogging=false;
        logFile.close();
        Serial.println("Closing logfile.");
      }
    }
    
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
  uint16_t distance = ranging();
  if (distance && distance < detectRange) {
    if (!changedDetectRange) flash();
  }
}

#ifdef ARDUINO_SEEED_XIAO_RP2040
void setup1() { return; }
#endif

