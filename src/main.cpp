#include <Arduino.h>
#include "Board.h"
#include "MB10xx.h"
#include "BH1750FVI.h"
#include "Vibrator.h"
#include "Feedback.h"
#include "FeedbackPattern.h"
#include "Userinput.h"
#include <math.h>
MB10xx mb;
TactSw btns[2];
BH1750FVI light;

int commandDispatch() {
  TactSw::status_t btn1 = btns[0].check();
  TactSw::status_t btn2 = btns[1].check();
  if(btn1 == TactSw::longpressed && btn2 == TactSw::none) {
    return 3;
  }
  else if(btn1 == TactSw::none && btn2 == TactSw::longpressed) {
    return 4;
  }
  else if(btn1 == TactSw::longpressed && btn2 == TactSw::pressing) {
    return 5;
  }
  else if(btn1 == TactSw::pressing && btn2 == TactSw::longpressed) {
    return 5;
  }
  else if(btn1 == TactSw::pressed && btn2 == TactSw::none) {
    return 1;
  }
  else if(btn1 == TactSw::none && btn2 == TactSw::pressed) {
    return 2;
  }
  return 0;
}

enum class BootMode { none, sonar, light, compass };

BootMode selectBootMode() {
  uint32_t expire = millis()+1000; // wait 1 Sec 
  BootMode let = BootMode::none;
  int cmd = 0;
  while(millis() < expire) {
    cmd = commandDispatch();
    if (cmd) break;
  }
  if (cmd == 3) {
    let = BootMode::light;
    vib.on(200);
    delay(300);
    vib.on(200);
  }
  else if (cmd == 4) {
    let = BootMode::compass;
    vib.on(200);
    delay(300);
    vib.on(200);
    delay(300);
    vib.on(200);
  }
  else {
    let = BootMode::sonar;
    vib.on(200);
  }
  return let;
}

void sonarMode() {
  static uint16_t previousUnit = 0;
  static uint8_t maxRange = 4;
  uint8_t cmd = commandDispatch();
  if(cmd) {
    previousUnit = 0;
    if(cmd == 1 && maxRange <= 10) {
      maxRange++;
      vib.on(150);
    }
    else if (cmd == 2 && maxRange > 1) {
      maxRange--;
      vib.on(150);
    }
    else if (cmd == 5) {
      maxRange = 4;
      vib.on(200);
    }
  }

  uint16_t unit = mb.getDistance() / 500;
  Serial.printf("%u     \r", unit);
  if (unit != previousUnit) {
    if (unit < maxRange) {
      setPattern(patterns[unit][0], patterns[unit][1],
		 patterns[unit][2], patterns[unit][3]);
    } else {
      setPattern(0,0,0,0);
    }
    previousUnit = unit;
  }
}

uint16_t lightMode() {
  constexpr int16_t MAX_LUX = 5000;
  constexpr int16_t MIN_LUX = 20;
  constexpr int16_t MAX_PERIOD = 1000;
  constexpr int16_t MIN_PERIOD = 10;

  uint16_t lux = light.getLUX();
  uint16_t period = 0;
  if (lux < MIN_LUX) {
    period = 0;
  } else if (lux >= MAX_LUX) {
    period = MIN_PERIOD;
  } else {
    float logLux = log10f(lux);
    float logLuxMin = log10f(MIN_LUX);
    float logLuxMax = log10f(MAX_LUX);

    period = MAX_PERIOD - (uint16_t)((float)(MAX_PERIOD - MIN_PERIOD) * (logLux - logLuxMin) / (logLuxMax - logLuxMin));
  }
  Serial.printf("%03d\r", period);
  return period;
}

BootMode mode = BootMode::none;

#ifdef ARDUINO_XIAO_ESP32C3
void rangingTask(void *param)
{
  while(1) {
    mb.ranging();
    delay(1);
  }
}
#else
void setup1() {
  while (mode == BootMode::none) {
    delay(100); // 動作モードの決定を待つ
  }
  if (mode == BootMode::sonar) {
    mb.begin(pin_sonar, true);
  }
  return;
}

void loop1()
{
  if (mode == BootMode::sonar) {
    mb.ranging();
  }
  delay(1); // いらないかも？
}
#endif

void setup() {
  Serial.begin(115200);
  delay(100); // Waiting for DC converter
  Jsb01.begin();
  btns[0].init(pin_button1);
  btns[1].init(pin_button2);
  vib.begin(pin_vibe, true, true);
  mode = selectBootMode();
  if(mode == BootMode::sonar) {
    Serial.printf("%d : Booting...\n", mode);
    setPattern(0, 0, 00, 0); // クリア
#ifdef ARDUINO_XIAO_ESP32C3
    xTaskCreateUniversal(rangingTask, "RangingTask", 2048, nullptr, 5, nullptr, 0);
#endif
    feedbackBegin();
  }
  else if (mode == BootMode::light) {
    light.begin();
  }
}

void loop() {
  if(mode == BootMode::sonar) {
    sonarMode();
    delay(1);
  }
  else if (mode == BootMode::light) {
    uint16_t delayTime = lightMode();
    if(delayTime) {
      vib.on(20);
      delay(delayTime);
    }
  }
}
