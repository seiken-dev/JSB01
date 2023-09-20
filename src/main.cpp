#include <Arduino.h>
#include <cstdint>
#include "hardware.h"
#include "MB10xx.h"
#include "Vibrator.h"
#include "Feedback.h"
#include "FeedbackPattern.h"
#include "Userinput.h"

MB10xx mb;
TactSw btns[2];

void ioInit() {
  pinMode(pin_sonar, INPUT);
  pinMode(pin_vibe, OUTPUT);
}

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
  return;
}

void loop1()
{
  mb.ranging();
  delay(1); // いらないかも？
}
#endif

void setup() {
  Serial.begin(115200);
  delay(700);
  Serial.println("Booting...");
  ioInit();
  btns[0].init(pin_button1);
  btns[1].init(pin_button2);
  mb.begin(pin_sonar, false);
  vib.begin(pin_vibe, true, true);
  vib.on();
  delay(500);
  vib.off();
  setPattern(0, 0, 00, 0); // クリア
#ifdef ARDUINO_XIAO_ESP32C3
  xTaskCreateUniversal(rangingTask, "RangingTask", 2048, nullptr, 5, nullptr, 0);
#endif
  feedbackBegin();
}

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

void sonarMode() {
  static uint16_t previousUnit = 0;
  static uint8_t maxRange = 10;
  uint8_t cmd = commandDispatch();
  if(cmd) {
    previousUnit = 0;
    if(cmd == 1 && maxRange <= 8) {
      maxRange++;
      vib.on(100);
    }
    else if (cmd == 2 && maxRange > 0) {
      maxRange--;
      vib.on(100);
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

void loop() {
  sonarMode();
  delay(1);
}
