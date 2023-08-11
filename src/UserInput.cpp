#include <Arduino.h>
#include "hardware.h"
#include "UserInput.h"

constexpr uint buttonThreshold = 5;
constexpr uint longThreshold = 500;
enum UserCMd : uint8_t {
  noCmd = 0,
  bt1Click,
  bt2Click,
  bt1Long,
  bt2Long,
};

void nop() {
  // 何もしません
  Serial.print("nop ");
  return;
}

void (*cbButton1Click)(void) {nop};
void (*cbButton1Long)(void) {nop};
void (*cbButton2Click)(void) {nop};
void (*cbButton2Long)(void) {nop};

void checkPin()
{
  static unsigned long btn1Expire = 0, btn2Expire=0;  
  static PinStatus cBtn1, cBtn2, pBtn1=HIGH, pBtn2=HIGH; // 呼ばれたときと直前のピン状態
  cBtn1 = digitalRead(pin_button1);
  cBtn2 = digitalRead(pin_button2);
  static UserCMd cmd = noCmd;
  if (pBtn1 == HIGH && cBtn1 == LOW) {
    // Btn1 Pushed
    btn1Expire = millis()+buttonThreshold;
    pBtn1 = LOW;
  }
  if (pBtn2 == HIGH && cBtn2 == LOW) {
    // Btn2 Pushed
    btn2Expire = millis()+buttonThreshold;
    pBtn2 = LOW;
  }
  // ここにコマンドを書く
  if (cmd == noCmd && cBtn1 == HIGH && btn1Expire && millis() > btn1Expire) {
    cmd = bt1Click;
    cbButton1Click();
    btn1Expire = 0;
  }
  else if (cmd == noCmd && cBtn1 == LOW && btn1Expire && millis() > btn1Expire+longThreshold-buttonThreshold) {
    cmd = bt1Long;
    cbButton1Long();
    btn1Expire = 0;
  }
  else if (cmd == noCmd && cBtn2 == HIGH && btn2Expire && millis() > btn2Expire) {
    cmd = bt2Click;
    cbButton2Click();
    btn2Expire = 0;
  }
  else if (cmd == noCmd && cBtn2 == LOW && btn2Expire && millis() > btn2Expire+longThreshold-buttonThreshold) {
    cmd = bt2Long;
    cbButton2Long();
    btn2Expire = 0;
  }
  if (cmd != noCmd && pBtn1 == LOW && cBtn1 == HIGH) {
    // Released
    btn1Expire = 0;
    pBtn1 = HIGH;
    cmd=noCmd;
  }
  if (cmd != noCmd && pBtn2 == LOW && cBtn2 == HIGH) {
    // Released
    btn2Expire = 0;
    pBtn2 = HIGH;
    cmd=noCmd;
  }
}
  
repeating_timer timer;
bool cbCheckPin(repeating_timer *t) {
  checkPin();
  return true;
}

// public functions
void inputBegin() {
  add_repeating_timer_ms(10, cbCheckPin, nullptr, &timer);
  return;
}
