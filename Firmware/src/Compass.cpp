#include "Compass.h"

Compass::Compass() : x_offset(0), y_offset(0), z_offset(0) {}

bool Compass::begin() {
  EEPROM.begin(512);

  if (!mag.begin()) {
    return false;
  }
  Wire.setClock(100000);  // Set I2C frequency to 100kHz

  mag.softReset();
  if (!loadCalibration()) {
    calibrate();
  }
  return true;
}

void Compass::calibrate() {
  constexpr int CalibrationTime = 30000;  // 30 seconds
  double minX = 1e9, maxX = -1e9;
  double minY = 1e9, maxY = -1e9;
  double x, y, z;

  Serial.println("Hard iron calibration: rotate the sensor slowly in a full circle.");
  Serial.print("Collecting data for ");
  Serial.print(CalibrationTime / 1000);
  Serial.println(" seconds...");

  unsigned long startMs = millis();
  while (millis() - startMs < (unsigned long)CalibrationTime) {
    get_xyz(&x, &y, &z);
    if (x < minX) minX = x;
    if (x > maxX) maxX = x;
    if (y < minY) minY = y;
    if (y > maxY) maxY = y;
    delay(50);
  }

  x_offset = (maxX + minX) / 2.0;
  y_offset = (maxY + minY) / 2.0;

  Serial.println("Calibration done.");
  Serial.print("  offset X: ");
  Serial.println(x_offset, 6);
  Serial.print("  offset Y: ");
  Serial.println(y_offset, 6);
  saveCalibration();
}

/**
 * @brief MMC5983MAから磁気センサの値を取得する関数
 * @param x 磁気センサのX軸の値を格納する変数へのポインタ
 * @param y 磁気センサのY軸の値を格納する変数へのポインタ
 * @param z 磁気センサのZ軸の値を格納する変数へのポインタ
 * @return 取得に成功した場合はtrue、失敗した場合はfalse
 * @note
 * MMC5983MAはSET/RESET動作を行うことでオフセットをキャンセルすることができる。
 * ただし、SET/RESET動作は磁気センサの値を大きく変化させるため、SET/RESET動作を行う前後の値を取得して平均を取ることで、より安定した値を得ることができる。さらに、オフセットキャンセル後の値を131072で割ることで、磁場強度をガウス単位で得ることができる。
 */

bool Compass::get_xyz(double* x, double* y, double* z) {
  uint32_t setX = 0, setY = 0, setZ = 0;
  uint32_t resetX = 0, resetY = 0, resetZ = 0;

  mag.performSetOperation();
  mag.getMeasurementXYZ(&setX, &setY, &setZ);

  mag.performResetOperation();
  mag.getMeasurementXYZ(&resetX, &resetY, &resetZ);

  // オフセットキャンセル: (SET - RESET) / 2 + 131072
  *x = ((double)setX - (double)resetX) / 2.0 / 131072.0;
  *y = ((double)setY - (double)resetY) / 2.0 / 131072.0;
  *z = ((double)setZ - (double)resetZ) / 2.0 / 131072.0;
  return true;
}

float Compass::getHeading() {
  double x, y, z;
  get_xyz(&x, &y, &z);
  double heading = 0;
  heading = atan2(x, 0 - y);

  heading /= PI;
  heading *= 180;
  heading += 180;
  return (float)heading;
}

bool Compass::loadCalibration() {
  CalibrationData data;
  EEPROM.get(EEPROM_ADDR, data);

  if (data.signature == EEPROM_SIGNATURE) {
    x_offset = data.x_off;
    y_offset = data.y_off;
    z_offset = data.z_off;
    Serial.println("Calibration loaded from EEPROM.");
    Serial.print("Offsets -> X: ");
    Serial.print(x_offset);
    Serial.print(" Y: ");
    Serial.print(y_offset);
    Serial.print(" Z: ");
    Serial.println(z_offset);
    return true;
  }
  return false;
}

void Compass::saveCalibration() {
  CalibrationData data;
  data.signature = EEPROM_SIGNATURE;
  data.x_off = x_offset;
  data.y_off = y_offset;
  data.z_off = z_offset;

  EEPROM.put(EEPROM_ADDR, data);
  if (EEPROM.commit()) {
    Serial.println("Calibration saved to EEPROM.");
  } else {
    Serial.println("EEPROM commit failed!");
  }
}
