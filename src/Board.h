#pragma once
constexpr uint8_t pin_sonar = D6;
constexpr uint8_t pin_button1 = D0;
constexpr uint8_t pin_button2 = D1;
constexpr uint8_t pin_vibe = D2;

class JSB01_CLASS {
public:
  bool begin();
  bool i2cExist(uint8_t addr);
  void i2cWriteByte(uint8_t addr, byte data);
  void i2cWriteBytes(uint8_t addr, const uint8_t *buffer, size_t size);
  uint8_t i2cReadByte(const uint8_t addr);
  uint8_t i2cReadBytes(const uint8_t addr, uint8_t *buffer, size_t size);
  void i2cWriteRegByte(uint8_t addr, uint8_t reg, uint8_t value);
  uint8_t i2cReadRegByte(uint8_t addr, uint8_t reg);
  uint16_t i2cReadRegWord(uint8_t addr, uint8_t reg);
};

extern JSB01_CLASS Jsb01;
