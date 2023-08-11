#include "VibeFeedback.h"
#include "RP2040.h"
#include "vl53l5cx_api.h"
#include <iterator>
#include <new>

VibeFeedback vibe;
// 振動パターンのリスト。On時間とOff時間を交互の並べて書く。おそらく0でないほうが望ましい。エラーチェックは適当なので、書く側で考えてね。
static const std::vector<uint16_t> _v_err = {20, 20};
static const std::vector<uint16_t> _v_400 = {15, 270, 15, 700};
static const std::vector<uint16_t> _v_300 = {15, 270, 15, 500};
static const std::vector<uint16_t> _v_200 = {15, 170, 15, 500};
static const std::vector<uint16_t> _v_150 = {15, 70, 15, 500};
static const std::vector<uint16_t> _v_120 = {15, 70, 15, 70, 15, 300};
static const std::vector<uint16_t> _v_100 = {15, 50, 15, 50, 15, 300};
static const std::vector<uint16_t> _v_70 = {15, 85};
static const std::vector<uint16_t> _v_40 = {15, 55};
static const std::vector<uint16_t> _v_none = {0, 10};

bool VibeFeedback::feedback(DistanceStatus s, bool isBlocking)
{
  if (_isBlocking == true) return false;
  _next = s;
  _isBlocking = isBlocking;
  return true;
}

VibeFeedback::DistanceStatus VibeFeedback::operator++()
{
  return VibeFeedback::s__nodata;
}

bool VibeFeedback::viberation()
{
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
  if (_isBlocking == true) {
    // ユーザコマンド、フィードバックを一瞬停止
    delay(100);
  }
  for(auto i : _pattern) {
    // 振動時間が0のときにHIGHが書き込まれるのを防ぐ
    if (i > 0 && vibeOn) digitalWrite(_pin, HIGH);
    if (!_isBlocking &&_current != _next) {
      // 次のフィードバックがセットされたので振動してたら止めて終了
      digitalWrite(_pin, LOW);
      return false;
    }
    delay(i);
    if (i > 0 && vibeOn) digitalWrite(_pin, LOW);
    vibeOn = !vibeOn;
  }
  if (_isBlocking && _current == _next) {
    // ユーザコマンドの終了
    _isBlocking=false;
    delay(100);
    _next = s__nodata;
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
