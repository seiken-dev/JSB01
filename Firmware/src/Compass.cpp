#include "Compass.h"

Compass::Compass() : x_offset(0), y_offset(0), z_offset(0) {}

bool Compass::begin() {
  EEPROM.begin(512);

  if (!qmc.begin()) {
    return false;
  }
  Wire.setClock(100000); // Set I2C frequency to 100kHz

  qmc.setRange(QMC5883P_RANGE_8G);
  qmc.setMode(QMC5883P_MODE_CONTINUOUS);
  qmc.setODR(QMC5883P_ODR_50HZ);
  qmc.setOSR(QMC5883P_OSR_4);

  if (!loadCalibration()) {
    calibrate();
  }
  return true;
}

void Compass::calibrate() {
  constexpr int CalibrationTime = 30000; // 30 seconds
  Serial.println("Calibrating Magnetometer...");
  Serial.println("Rotate the sensor in all directions for 30 seconds.");
  int16_t xMin = 32767, yMin = 32767, zMin = 32767;
  int16_t xMax = -32768, yMax = -32768, zMax = -32768;

  unsigned long startTime = millis();
  while (millis() - startTime < CalibrationTime) {
    int16_t x, y, z;
    if (qmc.getRawMagnetic(&x, &y, &z)) {
      if (x < xMin) xMin = x;
      if (y < yMin) yMin = y;
      if (z < zMin) zMin = z;
      if (x > xMax) xMax = x;
      if (y > yMax) yMax = y;
      if (z > zMax) zMax = z;
    }
    delay(10);
  }

  x_offset = (xMax + xMin) / 2;
  y_offset = (yMax + yMin) / 2;
  z_offset = (zMax + zMin) / 2;

  Serial.print("Calibration Complete! Offsets -> X: ");
  Serial.print(x_offset);
  Serial.print(" Y: ");
  Serial.print(y_offset);
  Serial.print(" Z: ");
  Serial.println(z_offset);

  saveCalibration();
}

float Compass::getHeading() {
  int16_t x, y, z;

  if (qmc.getRawMagnetic(&x, &y, &z)) {
    x -= x_offset;
    y -= y_offset;
    z -= z_offset;

    float heading = atan2(y, x);
    float headingDegrees = heading * 180.0 / PI;
    headingDegrees += 180;

    if (headingDegrees >= 360) headingDegrees -= 360;
    if (headingDegrees < 0) headingDegrees += 360;

    return headingDegrees;
  }
  return -1.0;
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
