#ifdef TOF_SENSOR

#include <Wire.h>

#ifdef TOF_ADAFRUIT
#include "Adafruit_VL53L1X.h"
Adafruit_VL53L1X tof = Adafruit_VL53L1X();
#else // Pololu
#include "VL53L1X.h"
VL53L1X tof;
#endif

#ifdef TOF_8x8
#include <SparkFun_VL53L5CX_Library.h>

#define VL53L5CX_ADDR 0x52
#define VL53L1X_MODEL_ID_REG 0x010F
#define VL53L1X_MODEL_ID 0xEACC

SparkFun_VL53L5CX tof8x8;
VL53L5CX_ResultsData resultsData;
static uint8_t imageSize = 8;
static bool isTOF8x8 = false;
static int16_t prevDistance = 0;

#define BUFFER_SIZE 64
#define PERIOD8x8_MS 15
#define PERIOD4x4_MS 60
#define MAX_DELTA 500 // ignore data that is significantly different from its surroundings
int16_t distanceBuffer[BUFFER_SIZE]; // ignore locked data
#endif // TOF_8x8

#define VL53L1X_ADDR 41
#define PERIOD_MS 50
#define BLOCKING false
#define BLOCKING_TIMEOUT 500

#ifdef TOF_8x8
void clearDistanceBuffer() {
	memset(distanceBuffer, -1, sizeof(distanceBuffer));
}

bool checkDelta(int8_t x, int8_t y, int16_t d, int16_t d0) {
	if (x >= 0 && x < imageSize && y >= 0 && y < imageSize) {
		if (d > d0 && d - d0 > MAX_DELTA) {
			return false;
		} else if (d0 - d > MAX_DELTA) {
			return false;
		}
	}
	return true;
}

uint16_t readRegWord(uint8_t addr, uint16_t reg) {
	Wire.beginTransmission(addr);
	Wire.write((uint8_t)(reg >> 8));
	Wire.write((uint8_t)(reg));
	uint8_t status = Wire.endTransmission();
	Wire.requestFrom(addr, 2);
	uint16_t value = (uint16_t)Wire.read() << 8;
	value |= Wire.read();
	return value;
}

#endif // TOF_8x8

// return 0:not found, 1:VL53L1X, -1:VL53L5CX
int8_t TOF_init() {
	Wire.begin();
	Wire.setClock(400000); // use 400 kHz I2C

	Wire.beginTransmission(VL53L1X_ADDR);
	if (Wire.endTransmission() != 0) {
		return 0;
	}
#ifdef TOF_8x8
	uint16_t modeId = readRegWord(VL53L1X_ADDR, VL53L1X_MODEL_ID_REG);
	if (modeId != VL53L1X_MODEL_ID) {
		if (!tof8x8.begin()) {
			return 0;
		}
		isTOF8x8 = true;
		clearDistanceBuffer();
		tof8x8.setResolution(imageSize * imageSize); //Enable all 64 pads
		tof8x8.setRangingFrequency(PERIOD8x8_MS);
		tof8x8.startRanging();
		return -1;
	}
#endif // TOF_8x8

#ifdef TOF_ADAFRUIT
	if (tof.begin(VL53L1X_ADDR, &Wire)) {
		tof.startRanging();
		tof.setTimingBudget(PERIOD_MS);
		return 1;
	}
#else // Pololu
	tof.setTimeout(500);
	if (tof.init()) {
		tof.setDistanceMode(VL53L1X::Long);
		tof.setMeasurementTimingBudget(PERIOD_MS * 1000);
		tof.startContinuous(PERIOD_MS);
		return 1;
	}
#endif
	return 0;
}

int16_t TOF_distance() {
#ifdef TOF_8x8
	if (isTOF8x8) {
		if (tof8x8.isDataReady()) {
			if (tof8x8.getRangingData(&resultsData)) { //Read distance data into array

				int16_t minDistance = INT16_MAX;
				int8_t imageMin = (imageSize == 8) ? 0 : 1;
				int8_t imageMax = (imageSize == 8) ? imageSize - 1 : imageSize - 2;
				for (int8_t y = imageMin; y < imageMax; y++) {
					for (int8_t x = imageMin;  x < imageMax; x++) {
						uint16_t index = y * imageSize + x;
						int16_t d = resultsData.distance_mm[index];
						int16_t d0 = distanceBuffer[index];
						if (d < minDistance && d > 0 && d != d0) {
							if (checkDelta(x - 1, y, d, d0) && checkDelta(x + 1, y, d, d0) && checkDelta(x, y - 1, d, d0) && checkDelta(x, y + 1, d, d0)) {
								minDistance = d;
							}
						}
					}
				}
				memcpy(distanceBuffer, resultsData.distance_mm, sizeof(distanceBuffer));
				if (minDistance == INT16_MAX) {
					minDistance = 0;
				}
				return minDistance;
			}
		}
		return -1;
	}
#endif // TOF_8x8

#ifdef TOF_ADAFRUIT
#if BLOCKING == true
	unsigned long t = millis();
	for (int i = 0; i < 1000; i++) {
		if (tof.dataReady()) break;
		if (millis() - t >= BLOCKING_TIMEOUT) {
			return 0;
		}
		delay(1);
	}
#endif
	uint16_t distance;
	VL53L1X_ERROR status = tof.VL53L1X_GetDistance(&distance);
#if BLOCKING == true
	tof.clearInterrupt();
#endif
	if (status != VL53L1X_ERROR_NONE) {
		return status;
	}
	return distance;
#else // Pololu
	return tof.read(BLOCKING);
#endif
}

void TOF_setFOV(bool isWide) {
#ifdef TOF_8x8
	if (isTOF8x8) {
		imageSize = isWide ? 8 : 4;
		clearDistanceBuffer();
		tof8x8.stopRanging();
		tof8x8.setResolution(imageSize * imageSize);
		tof8x8.setRangingFrequency((imageSize == 8) ? PERIOD8x8_MS : PERIOD4x4_MS);
		tof8x8.startRanging();
		return;
	}
#endif
	uint16_t size = isWide ? 16 : 4;
#ifdef TOF_ADAFRUIT
	tof.VL53L1X_SetROI(size, size);
#else // Pololu
	tof.setROISize(size, size);
#endif
}

#endif // TOF_SENSOR
