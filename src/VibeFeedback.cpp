#include "Arduino.h"
#include <vector>

// Pin definitions
constexpr uint8_t pin_button1 = D0;
constexpr uint8_t pin_button2 = D1;
constexpr uint8_t pin_vibe = D2;

enum DistanceStatus : uint8_t {
  s_err = 0,
  s_400,
  s_300,
  s_200,
  s_150,
  s_120,
  s_100,
  s_70,
  s_40,
  s__nodata
};

// 振動パターンのリスト。On時間とOff時間を交互の並べて書く。おそらく0でないほうが望ましい。エラーチェックは適当なので、書く側で考えてね。
const std::vector<uint16_t> _v_err = {7, 30};
const std::vector<uint16_t> _v_400 = {15, 300,15, 500};
const std::vector<uint16_t> _v_300 = {15, 200, 15, 500};
const std::vector<uint16_t> _v_200 = {15, 100, 15, 500};
const std::vector<uint16_t> _v_150 = {15, 70, 15, 500};
const std::vector<uint16_t> _v_120 = {15, 50, 15, 50, 15, 300};
const std::vector<uint16_t> _v_100 = {15, 30, 15, 30, 15, 200};
const std::vector<uint16_t> _v_70 = {15, 15};
const std::vector<uint16_t> _v_40 = {15, 10};
const std::vector<uint16_t> _v_none = {10};

DistanceStatus distance=s__nodata;

void vibeFeedback(DistanceStatus st) {
  bool vibeOn = true; // 振動or おやすみ
  std::vector<uint16_t> _pattern;
  switch (st) {
    case s_err:
      _pattern = _v_err;
      break;
    case s_400:
      _pattern = _v_400;
      break;
    case s_300:
      _pattern = _v_300;
      break;
    case s_200:
      _pattern = _v_200;
      break;
    case s_150:
      _pattern = _v_150;
      break;
    case s_120:
      _pattern = _v_120;
      break;
    case s_100:
      _pattern = _v_100;
      break;
    case s_70:
      _pattern = _v_70;
      break;
    case s_40:
      _pattern = _v_40;
      break;
    case s__nodata:
      _pattern = _v_none;
      break;
  }

  for(auto i : _pattern) {
    if (vibeOn) digitalWrite(pin_vibe, HIGH);
    delay(i);
    if (vibeOn) digitalWrite(pin_vibe, LOW);
    vibeOn = !vibeOn;
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  pinMode(pin_vibe, OUTPUT);
  pinMode(pin_button1, INPUT_PULLUP);
  pinMode(pin_button2, INPUT_PULLUP);
}

void loop() {
  vibeFeedback(s_120);
}
