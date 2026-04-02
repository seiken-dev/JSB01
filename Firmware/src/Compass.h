#ifndef COMPASS_H
#define COMPASS_H

#include <Arduino.h>
#include <EEPROM.h>
#include <SparkFun_MMC5983MA_Arduino_Library.h>

class Compass {
 public:
  Compass();
  bool begin();
  float getHeading();
  void calibrate();

 private:
  SFE_MMC5983MA mag;
  double x_offset;
  double y_offset;
  double z_offset;

  struct CalibrationData {
    uint32_t signature;
    double x_off;
    double y_off;
    double z_off;
  };

  static const uint32_t EEPROM_SIGNATURE = 0xAB12CD34;
  static const int EEPROM_ADDR = 0;

  bool loadCalibration();
  void saveCalibration();
  bool get_xyz(double* x, double* y, double* z);
};

#endif
