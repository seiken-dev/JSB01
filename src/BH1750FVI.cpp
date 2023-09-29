#include "BH1750FVI.h"
#include "Board.h"

BH1750FVI::BH1750FVI(uint8_t addr) { _addr = addr; }

uint16_t BH1750FVI::getLUX() {
  uint16_t lux;
  uint8_t buffer[2];
  Jsb01.i2cReadBytes(_addr, buffer, 2);
  lux = buffer[0] << 8 | buffer[1];
  return lux;
}

void BH1750FVI::powerOn() {
  if (Jsb01.i2cExist(_addr) == false) {
    Serial.println("Device not found.");
  } else {
    Serial.println("Device found.");
    Jsb01.i2cWriteByte(_addr, POWER_ON);
  }
}

void BH1750FVI::powerOff() {
  Jsb01.i2cWriteByte(_addr, POWER_DOWN);
}

void BH1750FVI::setMode(byte mode) {
  Jsb01.i2cWriteByte(_addr, mode);
}

bool BH1750FVI::begin() {
  powerOn();
  setMode(CONTINUOUSLY_H_RESOLUTION_MODE);
  return true;
}
