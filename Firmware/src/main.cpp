#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <cstdint>
#include "Board.h"
#include "Vibrator.h"
#include "Feedback.h"
#include "FeedbackPattern.h"
#include "MB10xx.h"

void setup() {
  Serial.begin(115200);
  Jsb01.begin();
  vib.begin(pin_vibe, true);
  vib.setFrequency(145);
  vib.on(100);
  delay(1000); // for Serial
  Sonar.begin(pin_sonar, true);
  Sonar.ranging();
  feedbackBegin();
}

void loop() {
  static uint32_t previousUnit = 0;
  uint32_t unit = Sonar.getDistance();
  unit = (unit && unit <= 5500) ? unit / 500-1 : 0xffffffff;
  if (unit > 11) {
    setPattern(0,0,0,0);
  } else {
    if (unit != previousUnit) {
      setPattern(patterns[unit][0], patterns[unit][1], patterns[unit][2],
		 patterns[unit][3]);
    }
  }
  Serial.printf("%02u\r", previousUnit);
  previousUnit = unit;
  delay(50);
}
