#ifndef COMPASS_H
#define COMPASS_H

#include <Arduino.h>
#include <Adafruit_QMC5883P.h>
#include <EEPROM.h>

class Compass {
public:
    Compass();
    bool begin();
    float getHeading();
    void calibrate();

private:
    Adafruit_QMC5883P qmc;
    int16_t x_offset;
    int16_t y_offset;
    int16_t z_offset;

    struct CalibrationData {
        uint32_t signature;
        int16_t x_off;
        int16_t y_off;
        int16_t z_off;
    };

    static const uint32_t EEPROM_SIGNATURE = 0xAB12CD34;
    static const int EEPROM_ADDR = 0;

    bool loadCalibration();
    void saveCalibration();
};

#endif
