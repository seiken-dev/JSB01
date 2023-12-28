#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <cstdint>
#include "Board.h"
#include "Vibrator.h"
#include "Feedback.h"
#include "FeedbackPattern.h"
#include "MB10xx.h"
#include "Userinput.h"

constexpr uint16_t measurementInterval = 50; //MB10X0は、50mSec感覚での測定なので、これでいいかな
volatile uint32_t maxDetectRange = 4; // デフォルトの検出距離 (4*50cm)
void cbSonarMode(TimerHandle_t xTimerID) {
  static uint32_t previousUnit = 0;
  uint32_t unit = Sonar.getDistance();
  unit = (unit && unit <= 5500) ? unit / 500-1 : 0xffffffff;
  if (unit > 11) {
    setPattern(0,0,0,0);
  } else {
    if (unit < maxDetectRange && unit != previousUnit) {
      setPattern(patterns[unit][0], patterns[unit][1], patterns[unit][2],
		 patterns[unit][3]);
    }
  }
  previousUnit = unit;
}

SoftwareTimer SonarTick;
int SonarMode() {
  Sonar.begin(pin_sonar, true);
  Sonar.ranging();
  SonarTick.begin(measurementInterval, cbSonarMode);
  SonarTick.start();
}

void setup() {
  Serial.begin(115200);
  Jsb01.begin();
  vib.begin(pin_vibe, true);
  vib.setFrequency(145);
  vib.on(100);
  delay(1000); // for Serial
  feedbackBegin();
  SonarMode();
}

void loop() {
  delay(1000);
}
