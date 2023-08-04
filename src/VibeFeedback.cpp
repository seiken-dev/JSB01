#include "VibeFeedback.h"

VibeFeedback vibe;
// 振動パターンのリスト。On時間とOff時間を交互の並べて書く。おそらく0でないほうが望ましい。エラーチェックは適当なので、書く側で考えてね。
static const std::vector<uint16_t> _v_err = {20, 20};
static const std::vector<uint16_t> _v_400 = {15, 300,15, 770};
static const std::vector<uint16_t> _v_300 = {15, 200, 15, 500};
static const std::vector<uint16_t> _v_200 = {15, 100, 15, 500};
static const std::vector<uint16_t> _v_150 = {15, 70, 15, 500};
static const std::vector<uint16_t> _v_120 = {15, 50, 15, 50, 15, 300};
static const std::vector<uint16_t> _v_100 = {15, 30, 15, 30, 15, 200};
static const std::vector<uint16_t> _v_70 = {15, 40};
static const std::vector<uint16_t> _v_40 = {15, 20};
static const std::vector<uint16_t> _v_none = {0, 10};

bool VibeFeedback::feedback(DistanceStatus s)
{
  _next = s;
  return true;
}

bool VibeFeedback::viberation() {
  bool vibeOn = true; // 振動or おやすみ
  std::vector<uint16_t> _pattern;
  switch (_next) {
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
  _current = _next;
  for(auto i : _pattern) {
    if (vibeOn) digitalWrite(_pin, HIGH);
    for(uint _delay=0; _delay < i; _delay+=5) {
      if (_current != _next) {
        // 次のフィードバックがセットされたの終了
        return false;
      }
      delay(5);
    }
    if (vibeOn) digitalWrite(_pin, LOW);
    vibeOn = !vibeOn;
  }
  return true;
}

bool VibeFeedback::stop() {
  _next = s__nodata;
  return true;
}

// RP2040 multitasking

void setup1() {
  // いつ動くのかなあ
  return;
}

void loop1() {
  vibe.viberation();
}
