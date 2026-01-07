#include <Arduino.h>

#include "BH1750FVI.h"
#include "Board.h"
#include "Compass.h"
#include "Feedback.h"
#include "FeedbackPattern.h"
#include "MB10xx.h"
#include "Userinput.h"
#include "Vibrator.h"

MB10xx mb;        // 超音波センサ
TactSw btns[2];   // タクトスイッチ
BH1750FVI light;  // 照度センサ
Compass compass;  // コンパスセンサ

// 超音波センサーの最大検出距離 (単位: 50cm)
constexpr int MaxDetectUnit = 10;

int commandDispatch() {
  TactSw::status_t btn1 = btns[0].check();
  TactSw::status_t btn2 = btns[1].check();
  if (btn1 == TactSw::longpressed && btn2 == TactSw::none) {
    return 3;
  } else if (btn1 == TactSw::none && btn2 == TactSw::longpressed) {
    return 4;
  } else if (btn1 == TactSw::longpressed && btn2 == TactSw::pressing) {
    return 5;
  } else if (btn1 == TactSw::pressing && btn2 == TactSw::longpressed) {
    return 5;
  } else if (btn1 == TactSw::longpressed && btn2 == TactSw::longpressed) {
    return 5;
  } else if (btn1 == TactSw::pressed && btn2 == TactSw::none) {
    return 1;
  } else if (btn1 == TactSw::none && btn2 == TactSw::pressed) {
    return 2;
  }
  return 0;
}

enum class BootMode { none, sonar, light, compass };

BootMode selectBootMode() {
  uint32_t expire = millis() + 1000;  // wait 1 Sec
  BootMode let = BootMode::none;
  int cmd = 0;
  while (millis() < expire) {
    cmd = commandDispatch();
    if (cmd) break;
  }
  if (cmd == 3) {
    let = BootMode::light;
    vib.on(200);
    delay(300);
    vib.on(200);
  } else if (cmd == 4) {
    let = BootMode::compass;
    vib.on(200);
    delay(300);
    vib.on(200);
    delay(300);
    vib.on(200);
  } else {
    let = BootMode::sonar;
    vib.on(200);
  }
  return let;
}

void sonarMode() {
  static uint16_t previousUnit = 0;
  static uint8_t maxRange = 4;
  uint8_t cmd = commandDispatch();
  if (cmd) {
    previousUnit = 0;
    setPattern(0, 0, 0, 0);
    if (cmd == 1 && maxRange < MaxDetectUnit) {
      maxRange++;
      vib.on(100);
      delay(150);
    } else if (cmd == 2 && maxRange > 1) {
      maxRange--;
      vib.on(100);
      delay(150);
    } else if (cmd == 5) {
      maxRange = 4;
      vib.on(200);
      delay(500);
    }
  }

  uint16_t unit = mb.getDistance();
  unit /= 500;  // 50cm単位でフィードバックするので、５０で割ってある
  if (unit != previousUnit) {
    if (unit < maxRange) {
      setPattern(patterns[unit][0], patterns[unit][1], patterns[unit][2],
                 patterns[unit][3]);
    } else {
      setPattern(0, 0, 0, 0);
    }
    previousUnit = unit;
  }
}

uint16_t measureBrightness() {
  constexpr float LogMax = 11.0;  // log2(4096) 最大の明るさ
  constexpr uint16_t BaseV = 20;  // 最小振動間隔

  uint16_t lux = light.getLUX();
  float period = 0;  // 振動間隔、光量が少ないほど長くなる
  int v = 0;         // 振動間隔を整数にした値
  if (lux) {
    period = LogMax - log2(lux);
    if (period >= 0) {
      v = static_cast<uint16_t>(period * 40) + BaseV;
    }
  }
  Serial.printf("LUX=%04d period=%d    \r", lux, v);
  return v;
}

BootMode mode = BootMode::none;

#ifdef ARDUINO_XIAO_ESP32C3
void rangingTask(void* param) {
  while (1) {
    mb.ranging();
    delay(1);
  }
}
#else
void setup1() {
  while (mode == BootMode::none) {
    delay(100);  // 動作モードの決定を待つ
  }
  if (mode == BootMode::sonar) {
    mb.begin(pin_sonar, true);
  }
  return;
}

void loop1() {
  if (mode == BootMode::sonar) {
    mb.ranging();
  }
}
#endif

void setup() {
  Serial.begin(115200);
  // シリアルデバイスが準備できるまで待つ
  delay(100);
  Jsb01.begin();
  btns[0].init(pin_button1);
  btns[1].init(pin_button2);
  vib.begin(pin_vibe, true, true);
  mode = selectBootMode();
  setPattern(0, 0, 0, 0);  // 振動パターンをクリア
  feedbackBegin();
  if (mode == BootMode::sonar) {
#ifdef ARDUINO_XIAO_ESP32C3
    xTaskCreateUniversal(rangingTask, "RangingTask", 2048, nullptr, 5, nullptr,
                         0);
#endif
    // 初期化はsetup1で行う
  } else if (mode == BootMode::light) {
    light.begin();
  } else if (mode == BootMode::compass) {
    compass.begin();
  }
}

/**
 * @brief 角度を振動パターンに変換
 * @param heading 角度(0〜360)
 * @return 振動継続時間
 * @note ８方位を振動継続で知らせる。北（0D ±22.5d） が最も長く、南（180d ±22.5d）が最も短い。西と東は中間の長さ。５段階の数値を返す。
 */
int headingToVibration(float heading) {
  if (heading < 0) heading += 360;
  if (heading >= 360) heading -= 360;

  // 0:N, 1:NE, 2:E, 3:SE, 4:S, 5:SW, 6:W, 7:NW
  int sector = static_cast<int>((heading + 22.5) / 45.0) % 8;

  // 北(0)からの距離(0~4)
  int diff = (sector > 4) ? (8 - sector) : sector;

  // 北(0)が最も長く(500ms)、南(4)が最も短い(100ms)
  Serial.printf("%03d\r", (500 - diff * 100));
  return 500 - diff * 100;
}

void compassMode() {
  float heading = compass.getHeading();
  if (heading >= 0) {
    int duration = headingToVibration(heading);
    vib.on(duration);
    delay(500);  // 1000ms周期で振動
  } else {
    Serial.println("Failed to read heading.");
  }
  uint8_t cmd = commandDispatch();
  // 両方のスイッチが長押しされたらキャリブレーションを実行
  if (cmd == 5) {
    vib.on(200);
    delay(400);
    vib.on(200);
    delay(400);
    vib.on(200);
    delay(400);
    compass.calibrate();
    vib.on(200);
    delay(400);
    vib.on(200);
    delay(400);
    vib.on(200);
    delay(400);
  }
}

void loop() {
  if (mode == BootMode::sonar) {
    sonarMode();
    delay(10);  // スイッチの検出間隔、チャタリング防止
  } else if (mode == BootMode::light) {
    int delayTime = measureBrightness();
    if (delayTime) {
      vib.on(10);
      delay(delayTime);
    }
  } else if (mode == BootMode::compass) {
    compassMode();
    delay(20);
  }
}
