#include "Arduino.h"
#include "Vibrator.h"
#include "Feedback.h"

volatile FeedbackPattern p;
volatile bool wantClear = false;
void setPattern(uint16_t first, uint16_t second, uint16_t third, uint16_t fourth)
{
  p.period_1 = first;
  p.period_2 = second;
  p.period_3 = third;
  p.period_4 = fourth;
  wantClear = true;
  // Serial.printf("%d, %d, %d, %d        \r", p.period_1, p.period_2, p.period_3, p.period_4);
}

#ifdef ARDUINO_XIAO_ESP32C3
void IRAM_ATTR feedback()
#else
bool feedback(repeating_timer_t *t)
#endif
{
  static uint8_t i = 0; // インターバルのインデックス
  static uint8_t periodCOunt = 0; // インターバルの数
  static unsigned long expire = 0; // インターバル終了時刻
  if (wantClear) {
    expire = 0;
    i = 0;
    wantClear = false;
  }
  if (expire == 0) {
    if (i >= periodCOunt) i = 0; // 最後のインターバルまで処理したので初期化
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
    if (p.period_1) vib.on(5);
  }
  else if (millis() > expire) {
    expire = 0;
    if (i < periodCOunt) i++;
  }
#ifndef ARDUINO_XIAO_ESP32C3
  return true;
#endif
}

#ifdef ARDUINO_XIAO_ESP32C3
hw_timer_t * feedbackTick = nullptr;

void feedbackBegin() {
  feedbackTick = timerBegin(1, 80, true);
  timerAttachInterrupt(feedbackTick, feedback, true);
  timerAlarmWrite(feedbackTick, 5000, true);
  timerAlarmEnable(feedbackTick);
}
#else
repeating_timer_t feedbackTick;
void feedbackBegin() {
  add_repeating_timer_ms(1, feedback, nullptr, &feedbackTick);
}
#endif
