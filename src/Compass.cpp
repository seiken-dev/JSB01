#include "Compass.h"

#include "math.h"

#define QMC5883_ADDRESS (0x0D)
#define QMC5883_REG_CONFIG_1 (0x09)
#define QMC5883_REG_CONFIG_2 (0x0A)
#define QMC5883_REG_IDENT_B (0x0B)
#define QMC5883_REG_IDENT_C (0x20)
#define QMC5883_REG_IDENT_D (0x21)
#define QMC5883_REG_OUT_X (0x00)
#define QMC5883_REG_OUT_Y (0x02)
#define QMC5883_REG_OUT_Z (0x04)

bool Compass::begin() {
  if (!Jsb01.i2cExist(addr)) return false;
  Jsb01.i2cWriteRegByte(addr, QMC5883_REG_IDENT_B, 0X01);
  Jsb01.i2cWriteRegByte(addr, QMC5883_REG_IDENT_C, 0X40);
  Jsb01.i2cWriteRegByte(addr, QMC5883_REG_IDENT_D, 0X01);

  setSamples(QMC5883_SAMPLES_64);
  setDataRate(QMC5883_DATARATE_50HZ);
  setRange(QMC5883_RANGE_8GA);
  setMeasurementMode(QMC5883_CONTINOUS);
  return true;
}

void Compass::read() {
  rawX = Jsb01.i2cReadRegWord(addr, QMC5883_REG_OUT_X);
  rawY = Jsb01.i2cReadRegWord(addr, QMC5883_REG_OUT_Y);
  rawZ = Jsb01.i2cReadRegWord(addr, QMC5883_REG_OUT_Z);
  //	Serial.printf("raw X=%d Y=%d Z=%d\n", rawX, rawY,rawZ);
}

int16_t Compass::getDegree(bool isRaw) {
  float heading = atan2(getY(), getX());
  int32_t degree = (int32_t)round(heading * 180.f / 3.1416f);
  if (!isRaw) {
    degree += declinationAngle;
  }
  degree += deviceAngle;
  if (degree >= 360) {
    degree -= 360;
  } else if (degree < 0) {
    degree += 360;
  }
  return degree;
}

void Compass::startCalibration() {
  minX = minY = minZ = INT16_MAX;
  maxX = maxY = maxZ = INT16_MIN;
}

void Compass::calibrate() {
  read();

  if (rawX < minX) {
    minX = rawX;
  } else if (rawX > maxX) {
    maxX = rawX;
  }
  if (rawY < minY) {
    minY = rawY;
  } else if (rawY > maxY) {
    maxY = rawY;
  }
  if (rawZ < minZ) {
    minZ = rawZ;
  } else if (rawZ > maxZ) {
    maxZ = rawZ;
  }
}

void Compass::endCalibration() {
  offsetX = (minX + maxX) / 2;
  offsetY = (minY + maxY) / 2;
  offsetZ = (minZ + maxZ) / 2;
}

void Compass::setOffset(int16_t _offsetX, int16_t _offsetY, int16_t _offsetZ) {
  offsetX = _offsetX;
  offsetY = _offsetY;
  offsetZ = _offsetZ;
}

void Compass::setMeasurementMode(QMC5883_Mode mode) {
  uint8_t value = Jsb01.i2cReadRegByte(addr, QMC5883_REG_CONFIG_1);
  value &= 0xFC;
  value |= mode;
  Jsb01.i2cWriteRegByte(addr, QMC5883_REG_CONFIG_1, value);
}

void Compass::setDataRate(QMC5883_DataRate dataRate) {
  uint8_t value = Jsb01.i2cReadRegByte(addr, QMC5883_REG_CONFIG_1);
  value &= 0xF3;
  value |= (dataRate << 2);
  Jsb01.i2cWriteRegByte(addr, QMC5883_REG_CONFIG_1, value);
}

void Compass::setRange(QMC5883_Range range) {
  uint8_t value = Jsb01.i2cReadRegByte(addr, QMC5883_REG_CONFIG_1);
  value &= 0xCF;
  value |= (range << 4);
  Jsb01.i2cWriteRegByte(addr, QMC5883_REG_CONFIG_1, value);
}

void Compass::setSamples(QMC5883_Samples samples) {
  uint8_t value = Jsb01.i2cReadRegByte(addr, QMC5883_REG_CONFIG_1);
  value &= 0x3F;
  value |= (samples << 6);
  Jsb01.i2cWriteRegByte(addr, QMC5883_REG_CONFIG_1, value);
}
