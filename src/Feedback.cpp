#include "Arduino.h"
#include "Vibrator.h"
#include "Feedback.h"

struct FeedbackPattern {
  uint16_t period_1;
  uint16_t period_2;
  uint16_t period_3;
  uint16_t period_4;
};

FeedbackPattern p;
void setPattern(uint16_t first, uint16_t second, uint16_t third, uint16_t fourth)
{
  p.period_1 = first;
  p.period_2 = second;
  p.period_3 = third;
  p.period_4 = fourth;
}

void feedback() {
  static uint8_t i = 0;
  static uint8_t periodCOunt = 0;
  static unsigned long expire = 0;
  if (expire == 0) {
    if (p.period_1) vib.on(20);
    if (i >= periodCOunt) i = 0;
    // インターバルの数をセット
    if (p.period_4) periodCOunt=4;
    else if (p.period_3) periodCOunt=3;
    else if (p.period_2) periodCOunt=2;
    else if (p.period_1) periodCOunt=1;
    // インターバルの長さをセット
    if (i == 0 && p.period_1) expire = millis()+p.period_1;
    else if (i == 1 && p.period_2) expire = millis()+p.period_2;
    else if (i == 2 && p.period_3) expire = millis()+p.period_3;
    else if (i == 3 && p.period_4) expire = millis()+p.period_4;
  }
  else if (millis() > expire) {
    vib.off();
    expire = 0;
    if (i < periodCOunt) i++;
  }
}
