#include <Arduino.h>
#include <math.h>

#include "BH1750FVI.h"
#include "Board.h"
#include "Compass.h"
#include "Feedback.h"
#include "FeedbackPattern.h"
#include "MB10xx.h"
#include "Userinput.h"
#include "Vibrator.h"

MB10xx mb;
TactSw btns[2];
BH1750FVI light;
Compass compass;

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
      delay(250);
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
  constexpr float LogMax = 12.0;  // log2(4096) 最大の明るさ
  constexpr uint16_t BaseV = 20;  // 最小振動間隔

  uint16_t lux = light.getLUX();
  float period = 0;  // 振動間隔、光量が少ないほど長くなる
  int v = 0; // 振動間隔を整数にした値
  if (lux) {
    period = LogMax - log2(lux);
    v = static_cast<uint16_t>(period * 40) + BaseV;
  } else {
    v = 0;
  }
  Serial.printf("LUX=%04d period=%d    \r", lux, v);
  return v;
}

BootMode mode = BootMode::none;

#ifdef ARDUINO_XIAO_ESP32C3
void rangingTask(void *param) {
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
  delay(1);  // いらないかも？
}
#endif

void setup() {
  Serial.begin(115200);
  delay(100);  // Waiting for DC converter
  Jsb01.begin();
  btns[0].init(pin_button1);
  btns[1].init(pin_button2);
  vib.begin(pin_vibe, true, true);
  mode = selectBootMode();
  setPattern(0, 0, 0, 0);  // クリア
  feedbackBegin();
  if (mode == BootMode::sonar) {
    Serial.printf("%d : Booting...\n", mode);
#ifdef ARDUINO_XIAO_ESP32C3
    xTaskCreateUniversal(rangingTask, "RangingTask", 2048, nullptr, 5, nullptr,
                         0);
#endif
  } else if (mode == BootMode::light) {
    light.begin();
  } else if (mode == BootMode::compass) {
    compass.begin();
  }
}

/*void compassMode() {
  constexpr int16_t MAX_ANGLE = 150;
  constexpr int16_t MIN_ANGLE = 10;
  constexpr int16_t MAX_PERIOD = 500;

  static bool calibrating = false;
  static unsigned long calTick;
  constexpr uint32_t MIN_CALIBRATE_TIME = 1000;
  static unsigned long btn1Tick = 0;
  static unsigned long btn2Tick = 0;

  if (buttonLongPressed(PIN_BUTTON1, btn1Tick) && !calibrating) {
    calibrating = true;
    compass.startCalibration();
    Serial_println("calibrate QMC5883L...");
    flash(30);
    calTick = millis();
  }
  if (!buttonPressing(PIN_BUTTON1) && calibrating) {
    calibrating = false;
    compass.endCalibration();
    if (millis() - calTick >= MIN_CALIBRATE_TIME) {
      Serial_printf("offsetX=%d offsetY=%d\n", compass.getOffsetX(),
compass.getOffsetY()); flash(); delay(200); flash(); } else { // reset offset
      Serial_println("reset offset");
      compass.setOffset();
      flash();
      delay(200);
    }
    prefs.putInt(PREFS_OFFSET_X, compass.getOffsetX());
    prefs.putInt(PREFS_OFFSET_Y, compass.getOffsetY());
    prefs.putInt(PREFS_OFFSET_Z, compass.getOffsetZ());
    prefs.flush();
  }
  if (calibrating) {
    compass.calibrate();
    period = 0;
    return 0;
  }

  compass.read();

  if (buttonLongPressed(PIN_BUTTON2, btn2Tick) && !calibrating) {
    int32_t degree = compass.getDegree(true);
    if (degree >= 180) {
      degree = degree - 360;
    }
    compass.setDeclinationAngle(-degree);
    flash(30);
    delay(200);
    flash(30);
    delay(500);
  }

  int32_t value = compass.getDegree();
  Serial_printf("X=%d Y=%d Z=%d\n", compass.getX(), compass.getY(),
compass.getZ()); int32_t degree = value; if (degree > 180) { degree = 360 -
degree;
  }
  if (degree >= MAX_ANGLE) {
    period = 0;
  } else if (degree <= MIN_ANGLE) {
    period = MIN_PERIOD;
  } else {
    period = (MAX_PERIOD - MIN_PERIOD) * (degree - MIN_ANGLE) / (MAX_ANGLE -
MIN_ANGLE) + MIN_PERIOD;
  }
  return value;
}
*/

void loop() {
  if (mode == BootMode::sonar) {
    sonarMode();
    delay(10);
  } else if (mode == BootMode::light) {
    int delayTime = measureBrightness();
    vib.on(10);
    delay(delayTime);
  }
}
