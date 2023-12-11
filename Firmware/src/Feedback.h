#pragma once
#include "Arduino.h"
#include "Vibrator.h"

void setPattern(uint16_t first, uint16_t second, uint16_t third, uint16_t fourth);
void feedbackBegin();

struct FeedbackPattern {
  uint16_t period_1;
  uint16_t period_2;
  uint16_t period_3;
  uint16_t period_4;
};
