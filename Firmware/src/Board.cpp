#include "Arduino.h"
#include "Board.h"

JSB01_CLASS Jsb01;

bool JSB01_CLASS::begin() {
  Wire.begin();
  Wire.setClock(400000);  // use 400 kHz I2C
  return true;
}

bool JSB01_CLASS::i2cExist(uint8_t addr) {
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() != 0) {
    return false;
  }
  return true;
}

void JSB01_CLASS::i2cWriteByte(uint8_t addr, byte data) {
  Wire.beginTransmission(addr);
  Wire.write(data);
  Wire.endTransmission();
}

void JSB01_CLASS::i2cWriteBytes(uint8_t addr, const uint8_t *buffer,
                                size_t size) {
  Wire.beginTransmission(addr);
  Wire.write(buffer, size);
  Wire.endTransmission();
}

uint8_t JSB01_CLASS::i2cReadByte(const uint8_t addr) {
  Wire.requestFrom(addr, 1);
  while (!Wire.available()) {
  };
  uint8_t ret = Wire.read();
  return ret;
}

uint8_t JSB01_CLASS::i2cReadBytes(const uint8_t addr, uint8_t *buffer,
                                  size_t size) {
  Wire.requestFrom(addr, size);
  while (!Wire.available()) {
  };
  for (uint16_t i = 0; i < size; i++) {
    buffer[i] = Wire.read();
  }
  return size;
}

uint8_t JSB01_CLASS::i2cReadRegByte(uint8_t addr, uint8_t reg) {
  i2cWriteByte(addr, reg);
  return i2cReadByte(addr);
}

uint16_t JSB01_CLASS::i2cReadRegWord(uint8_t addr, uint8_t reg) {
  i2cWriteByte(addr, reg);
  Wire.requestFrom(addr, 2);
  while (!Wire.available()) {
  };
  uint8_t lsb = (uint8_t)Wire.read();
  return Wire.read() << 8 | lsb;
}

void JSB01_CLASS::i2cWriteRegByte(uint8_t addr, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}
