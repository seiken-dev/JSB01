#include "Arduino.h"
#include "MB10xx.h"

bool MB10xx::begin(uint8_t pin, bool init) {
  if (init) {
    pinMode(pin, INPUT);
  }
  _pin = pin;
  _type = detectMb();
  return (_type != MB10xx::mb_none) ? true : false;
}

MB10xx::mbtype_t MB10xx::detectMb() {
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
  _currentDistance = pulseIn(_pin, HIGH, 500*1000);
  if (_currentDistance >= 147*20 && _currentDistance <= 147*200) {
    // インチをミリに変換
    if (_type == MB10xx::mb_10x0) {
      _currentDistance /= 7;
      _currentDistance *= 12;
      _currentDistance += (_currentDistance/126); // 50インチで1cm誤差が出ちゃうので、補正
      _currentDistance /= 10; // 切り捨ててミリにする
    }
  }
  else {
    _currentDistance = 0;
  }
  return _currentDistance;
}
