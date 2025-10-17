#include "Board.h"

JSB01_BOARD Jsb01;

bool JSB01_BOARD::begin() {
  Wire.begin();
  Wire.setClock(400000);  // use 400 kHz I2C
  pref.begin();
  return true;
}

bool JSB01_BOARD::i2cExist(uint8_t addr) {
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() != 0) {
    return false;
  }
  return true;
}

void JSB01_BOARD::i2cWriteByte(uint8_t addr, byte data) {
  Wire.beginTransmission(addr);
  Wire.write(data);
  Wire.endTransmission();
  Serial.printf("Writing 0x%04x\n", data);
}

void JSB01_BOARD::i2cWriteBytes(uint8_t addr, const uint8_t *buffer,
                                size_t size) {
  Wire.beginTransmission(addr);
  Wire.write(buffer, size);
  Wire.endTransmission();
}

uint8_t JSB01_BOARD::i2cReadByte(const uint8_t addr) {
  Wire.requestFrom(addr, 1);
  while (!Wire.available()) {
  };
  uint8_t ret = Wire.read();
  return ret;
}

uint8_t JSB01_BOARD::i2cReadBytes(const uint8_t addr, uint8_t *buffer,
                                  size_t size) {
  Wire.requestFrom(addr, size);
  while (!Wire.available()) {
  };
  for (uint16_t i = 0; i < size; i++) {
    buffer[i] = Wire.read();
  }
  return size;
}

uint8_t JSB01_BOARD::i2cReadRegByte(uint8_t addr, uint8_t reg) {
  i2cWriteByte(addr, reg);
  return i2cReadByte(addr);
}

uint16_t JSB01_BOARD::i2cReadRegWord(uint8_t addr, uint8_t reg) {
  i2cWriteByte(addr, reg);
  Wire.requestFrom(addr, 2);
  while (!Wire.available()) {
  };
  uint8_t lsb = (uint8_t)Wire.read();
  return Wire.read() << 8 | lsb;
}

void JSB01_BOARD::i2cWriteRegByte(uint8_t addr, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}
