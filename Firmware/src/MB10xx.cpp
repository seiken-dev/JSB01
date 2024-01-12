#include "Arduino.h"
#include "MB10xx.h"
#include "wiring_digital.h"
#include <cstdint>
#include <cstdio>

// static callback function
volatile static uint32_t duration = 0;
static void cbRaging() {
  static uint32_t start = 0;
  if (start != 0) {
    uint32_t end = micros();
    duration = end - start;
    if(duration >= 255*147) {
      duration = 0;
      start = end;
    } else {
      start = 0;
    }
  } else {
    start = micros();
    duration = 0;
  }
}

bool MB10xx::begin(uint8_t pin, bool init) {
  if (init) {
    pinMode(pin, INPUT);
  }
  _pin = pin;
  _type = _detectMb();
  return (_type != MB10xx::mb_none) ? true : false;
}

MB10xx::mbtype_t MB10xx::_detectMb() {
  // ２度続けてMB10Xのパルスを検出し、その差の時間から、センサーのタイプを推定する。
  unsigned long detect = pulseIn(_pin, HIGH, 100*1000);
  if(detect == 0 )
    return mb_none;
  unsigned long firstHigh = micros()-detect;
  detect = pulseIn(_pin, HIGH, 100*1000);
  if(detect == 0 )
    return mb_none;
  unsigned long secondHigh = micros()-detect;
  return ((secondHigh - firstHigh) < 75*1000) ? mb_10x0 : mb_10x3;
}

uint32_t MB10xx::ranging() {
  attachInterrupt(_pin, cbRaging, CHANGE);
}

uint32_t MB10xx::getDistance() {
  if (duration == 0) return _currentDistance;
  if (duration >= 147*10 && duration <= 147*251) {
    // 距離はインチ、ミリに変換
    _currentDistance = duration / 7;
    _currentDistance *= 12;
     _currentDistance += (_currentDistance/126); // 50インチで1cm誤差が出ちゃうので、補正
      _currentDistance /= 10; // 切り捨ててミリにする
  }
  else {
    _currentDistance = 0xffffffff;
  }
  return _currentDistance;
}

// static variables
uint8_t MB10xx::_pin;
MB10xx::mbtype_t MB10xx::_type;
uint32_t MB10xx::_currentDistance;

MB10xx Sonar;
