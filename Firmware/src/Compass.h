#pragma once
#include <Arduino.h>
#define QMC5883P

#define COMPASS "QMC5883P"

enum QMC5883_DataRate {
  QMC5883_DATARATE_10HZ = 0,
  QMC5883_DATARATE_50HZ = 1,
  QMC5883_DATARATE_100HZ = 2,
  QMC5883_DATARATE_200HZ = 3,
};

#ifdef QMC5883P

enum QMC5883_OverSampleRate {
  QMC5883_OSR_8 = 0,
  QMC5883_OSR_4 = 1,
  QMC5883_OSR_2 = 2,
  QMC5883_OSR_1 = 3
};

enum QMC5883_DownSampleRate {
  QMC5883_DSR_1 = 0,
  QMC5883_DSR_2 = 1,
  QMC5883_DSR_4 = 2,
  QMC5883_DSR_8 = 3
};

enum QMC5883_Range {
  QMC5883_RANGE_30GA = 0,
  QMC5883_RANGE_12GA = 1,
  QMC5883_RANGE_8GA = 2,
  QMC5883_RANGE_2GA = 3
};

enum QMC5883_Mode {
  QMC5883_SUSPEND = 0,
  QMC5883_NORMAL = 1,
  QMC5883_SINGLE = 2,
  QMC5883_CONTINUOUS = 3
};

#else  // QMC5883L

#define COMPASS "QMC5883L"

enum QMC5883_OverSampleRate {
  QMC5883_OSR_512 = 0,
  QMC5883_OSR_256 = 1,
  QMC5883_OSR_128 = 2,
  QMC5883_OSR_64 = 3
};

enum QMC5883_Range { QMC5883_RANGE_2GA = 0, QMC5883_RANGE_8GA = 1 };

enum QMC5883_Mode { QMC5883_STANBY = 0, QMC5883_CONTINUOUS = 1 };

#endif

class Compass {
 public:
  Compass();

  bool begin();
  void read();
  int16_t getX() { return rawX - offsetX; }
  int16_t getY() { return rawY - offsetY; }
  int16_t getZ() { return rawZ - offsetZ; }
  int16_t getDegree(bool isRaw = false);
  void startCalibration();
  void calibrate();
  void endCalibration();
  int16_t getOffsetX() { return offsetX; }
  int16_t getOffsetY() { return offsetY; }
  int16_t getOffsetZ() { return offsetZ; }
  void setOffset(int16_t _offsetX = 0, int16_t _offsetY = 0,
                 int16_t _offsetZ = 0);
  void setDeclinationAngle(float degree) { declinationAngle = degree; }
  float getDeclinationAngle() { return declinationAngle; }
  void setDeviceAngle(int16_t degree) { deviceAngle = degree; }
  int16_t getDeviceAngle() { return deviceAngle; }

  void setMeasurementMode(QMC5883_Mode mode);
  void setDataRate(QMC5883_DataRate dataRate);
  void setRange(QMC5883_Range range);
  void setOverSampleRate(QMC5883_OverSampleRate osr);
#ifdef QMC5883P
  void setDownSampleRate(QMC5883_DownSampleRate dsr);
#endif

 private:
  uint8_t readRegByte(uint8_t reg);
  int16_t readRegWord(uint8_t reg);
  void writeRegByte(uint8_t reg, uint8_t value);

  uint8_t addr;
  int16_t rawX, rawY, rawZ;
  int16_t minX, minY, minZ;
  int16_t maxX, maxY, maxZ;
  int16_t offsetX, offsetY, offsetZ;
  float declinationAngle;
  int16_t deviceAngle;
};