#ifdef TOF_SENSOR

#include <Wire.h>

#ifdef TOF_ADAFRUIT
  #include "Adafruit_VL53L1X.h"
  Adafruit_VL53L1X tof = Adafruit_VL53L1X();
#else // Pololu
  #include "VL53L1X.h"
  VL53L1X tof;
#endif

#define VL53L1X_ADDR 41
#define PERIOD_MS 50
#define BLOCKING false
#define BLOCKING_TIMEOUT 500

bool TOF_init() {
	Wire.begin();
	Wire.setClock(400000); // use 400 kHz I2C
	Wire.beginTransmission(VL53L1X_ADDR);
	if (Wire.endTransmission() != 0) return false;

#ifdef TOF_ADAFRUIT
	if (tof.begin(VL53L1X_ADDR, &Wire)) {
		tof.startRanging();
		tof.setTimingBudget(PERIOD_MS);
		return true;
	}
#else // Pololu
	tof.setTimeout(500);
	if (tof.init()) {
		tof.setDistanceMode(VL53L1X::Long);
		tof.setMeasurementTimingBudget(PERIOD_MS * 1000);
		tof.startContinuous(PERIOD_MS);
		return true;
	}
#endif
	return false;
}

int16_t TOF_distance() {
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
	uint16_t size = isWide ? 16 : 4;
#ifdef TOF_ADAFRUIT
	tof.VL53L1X_SetROI(size, size);
#else // Pololu
	tof.setROISize(size, size);
#endif
}

#endif // TOF_SENSOR
