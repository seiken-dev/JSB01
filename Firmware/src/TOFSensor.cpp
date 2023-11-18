#ifdef TOF_SENSOR

#include <Wire.h>
#include "TOFSensor.h"

#define VL53L3X

#ifdef VL53L3X

#include <vl53lx_class.h>
#define VL53L3CX_ADDR 0x29

static VL53LX tof(&Wire, D10);

TOFType TOFSensor::begin() {
	Wire.begin();
	Wire.setClock(400000); // use 400 kHz I2C

	Wire.beginTransmission(VL53L3CX_ADDR);
	if (Wire.endTransmission() != 0) {
		return TOFType::None;
	}
	tof.begin();
	tof.VL53LX_Off();
	tof.InitSensor(0x12);
	tof.VL53LX_SetDistanceMode(VL53LX_DISTANCEMODE_LONG);
	tof.VL53LX_StartMeasurement();
	return TOFType::L1X;
}

int16_t TOFSensor::getDistance() {
	VL53LX_MultiRangingData_t MultiRangingData;
	VL53LX_MultiRangingData_t *pMultiRangingData = &MultiRangingData;
	uint8_t NewDataReady = 0;
	int status;
	int16_t distance = -1;

	do {
		status = tof.VL53LX_GetMeasurementDataReady(&NewDataReady);
	} while (!NewDataReady);

	if (status == 0 && NewDataReady != 0) {
		status = tof.VL53LX_GetMultiRangingData(pMultiRangingData);
		int no_of_object_found = pMultiRangingData->NumberOfObjectsFound;

		for (int i = 0; i < no_of_object_found; i++) {
			int16_t d = pMultiRangingData->RangeData[i].RangeMilliMeter;
			if (distance == -1 || d < distance) {
				distance = d;
			}
		}
		if (status==0) {
			status = tof.VL53LX_ClearInterruptAndStartMeasurement();
		}
	}
	return distance;
}

void TOFSensor::setFOV(bool isWide) {
}

#else // VL53L3X

#ifdef TOF_ADAFRUIT
#include "Adafruit_VL53L1X.h"
static Adafruit_VL53L1X tof = Adafruit_VL53L1X();
#else // Pololu
#include "VL53L1X.h"
static VL53L1X tof;
#endif

#ifdef TOF_8x8
#include <SparkFun_VL53L5CX_Library.h>

#define VL53L1X_MODEL_ID_REG 0x010F
#define VL53L1X_MODEL_ID 0xEACC

static SparkFun_VL53L5CX tof8x8;
static VL53L5CX_ResultsData resultsData;
static bool isTOF8x8 = false;
static uint8_t imageSize = 8;

#define BUFFER_SIZE 64
#define PERIOD8x8_MS 15
#define PERIOD4x4_MS 60
#define MAX_DELTA 500 // ignore data that is significantly different from its surroundings
static int16_t distanceBuffer[BUFFER_SIZE]; // ignore locked data
#endif // TOF_8x8

#define VL53L1X_ADDR 41
#define PERIOD_MS 50
#define BLOCKING false
#define BLOCKING_TIMEOUT 500

#ifdef TOF_8x8
static void clearDistanceBuffer() {
	memset(distanceBuffer, -1, sizeof(distanceBuffer));
}

static bool checkDelta(int8_t x, int8_t y, int16_t d, int16_t d0) {
	if (x >= 0 && x < imageSize && y >= 0 && y < imageSize) {
		if (d > d0 && d - d0 > MAX_DELTA) {
			return false;
		} else if (d0 - d > MAX_DELTA) {
			return false;
		}
	}
	return true;
}

static uint16_t readRegWord(uint8_t addr, uint16_t reg) {
	Wire.beginTransmission(addr);
	Wire.write((uint8_t)(reg >> 8));
	Wire.write((uint8_t)(reg));
	uint8_t status = Wire.endTransmission();
	Wire.requestFrom(addr, (uint8_t)2);
	uint16_t value = (uint16_t)Wire.read() << 8;
	value |= Wire.read();
	return value;
}

#endif // TOF_8x8

TOFType TOFSensor::begin() {
	Wire.begin();
	Wire.setClock(400000); // use 400 kHz I2C

	Wire.beginTransmission(VL53L1X_ADDR);
	if (Wire.endTransmission() != 0) {
		return TOFType::None;
	}
#ifdef TOF_8x8
	uint16_t modeId = readRegWord(VL53L1X_ADDR, VL53L1X_MODEL_ID_REG);
	if (modeId != VL53L1X_MODEL_ID) {
		if (!tof8x8.begin()) {
			return TOFType::None;
		}
		isTOF8x8 = true;
		clearDistanceBuffer();
		tof8x8.setResolution(imageSize * imageSize); //Enable all 64 pads
		tof8x8.setRangingFrequency(PERIOD8x8_MS);
		tof8x8.startRanging();
		return TOFType::L5CX;
	}
#endif // TOF_8x8

#ifdef TOF_ADAFRUIT
	if (tof.begin(VL53L1X_ADDR, &Wire)) {
		tof.startRanging();
		tof.setTimingBudget(PERIOD_MS);
		return TOFType::L1X;
	}
#else // Pololu
	tof.setTimeout(500);
	if (tof.init()) {
		tof.setDistanceMode(VL53L1X::Long);
		tof.setMeasurementTimingBudget(PERIOD_MS * 1000);
		tof.startContinuous(PERIOD_MS);
		return TOFType::L1X;
	}
#endif
	return TOFType::None;
}

int16_t TOFSensor::getDistance() {
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

void TOFSensor::setFOV(bool isWide) {
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

#endif // VL53L3X

#endif // TOF_SENSOR
