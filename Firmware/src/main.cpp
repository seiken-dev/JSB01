#include <Arduino.h>
#include <M5_BH1750FVI.h>
#include "Compass.h"
#include "Prefs.h"
#include "mydefs.h"

#define LD14
// #define VIBRATE_TEST // Vibrates twice per second for 20 ms

#if defined(ARDUINO_SEEED_XIAO_RP2040) || (ARDUINO_XIAO_ESP32C3)
#define XIAO
#endif

#ifdef XIAO
#undef PIN_LED
#ifdef ARDUINO_SEEED_XIAO_RP2040
#define PIN_LED PIN_LED_G
#define LED_ON LOW
#define LED_OFF HIGH
#endif
#else
#define LED_ON HIGH
#define LED_OFF LOW
#endif

#ifdef XIAO
#define PIN_SONAR D6
#define PIN_BUTTON1 D0
#define PIN_BUTTON2 D1
#define PIN_VIB D2
#endif
#ifdef AT_TINY
#define PIN_SONAR PIN_PA2
#define PIN_BUTTON1 PIN_PA6
#define PIN_BUTTON2 PIN_PA7
#define PIN_VIB PIN_PA3
#endif

#ifdef TOF_SENSOR
#include "TOFSensor.h"
TOFSensor tof;
#endif // TOF_SENSOR

Compass compass;
M5_BH1750FVI lightSensor;

Prefs prefs;

constexpr unsigned long PULSEIN_TIMEOUT = 500 * 1000;
constexpr uint32_t MIN_PERIOD = 75;
constexpr uint32_t LONGPRESS_MS = 1000;

constexpr float DECLINATION_ANGLE = -8.0f; // at Tokyo
constexpr int32_t COMPASS_ANGLE = 180;

#ifdef ARDUINO_XIAO_ESP32C3
const char* PREFS_PATH = "/sonar.ini";
#else
const char* PREFS_PATH = "sonar.ini";
#endif
const char* PREFS_OFFSET_X = "OffsetX";
const char* PREFS_OFFSET_Y = "OffsetY";
const char* PREFS_OFFSET_Z = "OffsetZ";

#ifdef LD14
constexpr unsigned int LD14_FREQ = 150;
#endif

void flash(uint16_t ms = 20) {
#ifdef PIN_LED
	digitalWrite(PIN_LED, LED_ON);
#endif
#ifdef LD14
	tone(PIN_VIB, LD14_FREQ, ms);
#else
    digitalWrite(PIN_VIB, HIGH);
#endif
	delay(ms);
#ifdef PIN_LED
	digitalWrite(PIN_LED, LED_OFF);
#endif
#ifndef LD14
    digitalWrite(PIN_VIB, LOW);
#endif
}

bool buttonPressing(uint8_t pin) {
	return digitalRead(pin) == LOW;
}

bool buttonPressed(uint8_t pin, bool& btn) {
	bool b = digitalRead(pin) == LOW;
	if (!btn && b) {
		btn = true;
		return true;
	}
	btn = b;
	return false;
}

bool buttonLongPressed(uint8_t pin, unsigned long& btnTick) {
	bool b = digitalRead(pin) == LOW;
	if (b) {
		unsigned long t = millis();
		if (btnTick == 0) {
			btnTick = t;
		} else if (btnTick > 1 && t - btnTick >= LONGPRESS_MS) {
			btnTick = 1;
			return true;
		}
	} else {
		btnTick = 0;
	}
	return false;
}

enum class Sonar : uint8_t {
	None, MB10x0, MB10x3, TOF, TOF8x8
};
Sonar sonar = Sonar::None;

enum class Sensor : uint8_t {
	None, Distance, GeoMag, Light
};
Sensor sensor = Sensor::None;

#ifdef DEBUG_SERIAL
static const char* sonarNames[] = { "NONE", "MB10x0", "MB10x3", "VL53L1X", "VL53L5CX" };
#endif

bool initDistanceSensor();
bool initGeoMagSensor();
bool initLightSensor();

void setup() {
	Serial_begin();
	Serial_println("start");

#ifdef ARDUINO_SEEED_XIAO_RP2040
	pinMode(NEOPIXEL_POWER, OUTPUT);
	digitalWrite(NEOPIXEL_POWER, HIGH);
	pinMode(PIN_LED_R, OUTPUT);
	pinMode(PIN_LED_G, OUTPUT);
	pinMode(PIN_LED_B, OUTPUT);
    digitalWrite(PIN_LED_R, LED_OFF);
    digitalWrite(PIN_LED_G, LED_OFF);
    digitalWrite(PIN_LED_B, LED_OFF);
#else
  #ifdef PIN_LED
    pinMode(PIN_LED, OUTPUT);
  #endif
#endif

	pinMode(PIN_SONAR, INPUT);
	pinMode(PIN_BUTTON1, INPUT_PULLUP);
	pinMode(PIN_BUTTON2, INPUT_PULLUP);
	pinMode(PIN_VIB, OUTPUT);

	delay(500); // wait USB Serial

	bool result;
	if (buttonPressing(PIN_BUTTON1)) {
		sensor = Sensor::GeoMag;
		if (result = initGeoMagSensor()) {
			flash();
			delay(200);
			flash(50);
		}
	} else if (buttonPressing(PIN_BUTTON2)) {
		sensor = Sensor::Light;
		if (result = initLightSensor()) {
			flash();
			delay(200);
			flash();
			delay(200);
			flash(50);
		}
	} else {
		sensor = Sensor::Distance;
		if (result = initDistanceSensor()) {
			flash();
			delay(200);
			flash();
		}
	}
	if (!result) {
		sensor = Sensor::None;
		flash();
	    delay(500);
		flash();
	    delay(500);
		flash();
#ifdef ARDUINO_SEEED_XIAO_RP2040
	    digitalWrite(PIN_LED_R, LED_ON);
#endif
		while(true) {
			delay(100);
		}
	}
    delay(500);
}

bool initDistanceSensor() {
	Serial_print("detect sonar...");

	uint32_t value = (uint32_t)pulseIn(PIN_SONAR, HIGH, PULSEIN_TIMEOUT);
	if (value != 0) {
		delay(20);
		unsigned long t = millis();
		pulseIn(PIN_SONAR, HIGH, PULSEIN_TIMEOUT);
		uint32_t interval = (uint32_t)(millis() - t);
		if (interval > 20) {
			sonar = interval < 60 ? Sonar::MB10x0 : Sonar::MB10x3; // MB10x0:20Hz, MB10x3:10Hz
		}
	} else {
#ifdef TOF_SENSOR
		TOFType type = tof.begin();
		if (type == TOFType::L1X) {
			sonar = Sonar::TOF;
		} else if (type == TOFType::L5CX) {
			sonar = Sonar::TOF8x8;
		}
#endif
	}
	Serial_printf("%s\n", sonarNames[(int)sonar]);

	if (sonar == Sonar::None) {
		return false;
	}
	return true;
}

bool initGeoMagSensor() {
	Serial_print("GeoMag Sensor(QMC5883L)...");
	if (!compass.begin()) {
		Serial_println(" not found");
		return false;
	}
	Serial_println(" start");
	compass.setDeclinationAngle(DECLINATION_ANGLE);
	compass.setDeviceAngle(COMPASS_ANGLE);

	if (prefs.begin(PREFS_PATH)) {
		int16_t offsetX = (int16_t)prefs.getInt(PREFS_OFFSET_X);
		int16_t offsetY = (int16_t)prefs.getInt(PREFS_OFFSET_Y);
		int16_t offsetZ = (int16_t)prefs.getInt(PREFS_OFFSET_Z);
		compass.setOffset(offsetX, offsetY, offsetZ);
		Serial_printf("offsetX=%d offsetY=%d\n", offsetX, offsetY);
	}
	return true;
}

bool initLightSensor() {
	Serial_print("Light Sensor(BH1750FVI)...");
	if (!lightSensor.begin(&Wire)) {
		Serial_println(" not found");
		return false;
	}
	Serial_println(" start");
	lightSensor.setMode(CONTINUOUSLY_H_RESOLUTION_MODE);
	return true;
}

int32_t loopDistanceSensor(unsigned long tick, uint32_t& period);
int32_t loopGeoMagSensor(unsigned long tick, uint32_t& period);
int32_t loopLightSensor(unsigned long tick, uint32_t& period);

void loop() {
	static unsigned long startTick = 0;
	static int32_t prevValue = -1;
	static uint32_t period = 0;

	unsigned long tick = millis();
	if (period != 0 && tick - startTick >= period) {
		flash();
		startTick = tick;
	}

	int minDelay;
	int value;
	if (sensor == Sensor::Distance) {
		value = loopDistanceSensor(tick, period);
		minDelay = 20;
	} else if (sensor == Sensor::GeoMag) {
		value = loopGeoMagSensor(tick, period);
		minDelay = 100;
	} else if (sensor == Sensor::Light) {
		value = loopLightSensor(tick, period);
		minDelay = 100;
	}
	if (period > 0 && period < MIN_PERIOD) {
		period = MIN_PERIOD;
	}

	uint32_t elapse = (uint32_t)(millis() - tick);
	if (value != prevValue) {
		if (period != 0) {
			Serial_printf("%d T=%dms (%d)\n", value, period, elapse);
		} else {
			Serial_printf("%d\n", value);
		}
	}
	prevValue = value;

	if (elapse < minDelay) {
		delay(minDelay - elapse);
	}
}

int32_t loopDistanceSensor(unsigned long tick,  uint32_t& period) {
	constexpr uint8_t RANGE_MIN = 1;
	constexpr uint8_t RANGE_MAX = 5;
	constexpr uint8_t RANGE_DEFAULT = 3;
	constexpr int32_t FLASH_RATIO = 4;

	static uint8_t maxRange = RANGE_DEFAULT;
	static bool btn1 = false;
	static bool btn2 = false;
#ifdef TOF_SENSOR
	static unsigned long btn1Tick = 0;
	static unsigned long btn2Tick = 0;
#endif

#ifdef TOF_SENSOR
	if (sonar == Sonar::TOF || sonar == Sonar::TOF8x8) {
		if (buttonLongPressed(PIN_BUTTON1, btn1Tick)) {
			tof.setFOV(true);
			Serial_println("ROI 16x16");
			flash(30);
			delay(100);
			flash(80);
			delay(500);
		}
		if (buttonLongPressed(PIN_BUTTON2, btn2Tick)) {
			tof.setFOV(false);
			Serial_println("ROI 4x4");
			flash(80);
			delay(100);
			flash(30);
			delay(500);
		}
	}
#endif

	int8_t updown = 0;
	if (buttonPressed(PIN_BUTTON1, btn1)) {
		updown = 1;
	} else if (buttonPressed(PIN_BUTTON2, btn2)) {
		updown = -1;
	}
	if (updown != 0) {
		maxRange += updown;
		if (maxRange < RANGE_MIN) {
			maxRange = RANGE_MIN;
		} else if (maxRange > RANGE_MAX) {
			maxRange = RANGE_MAX;
		}
		Serial_printf("max range=%dM\n", maxRange);
		for (int i = 0; i < maxRange; i++) {
			flash(30);
			delay(200);
		}
		delay(500);
	}

	int32_t distance;
#ifdef TOF_SENSOR
	if (sonar == Sonar::TOF || sonar == Sonar::TOF8x8) {
		distance = tof.getDistance();
	} else {
#endif
	distance = (int32_t)pulseIn(PIN_SONAR, HIGH, PULSEIN_TIMEOUT);
#ifdef TOF_SENSOR
	}
#endif
	if (sonar == Sonar::MB10x0) {
		distance = distance * 24 / 139; // ≒ / 147.f * 25.4f
	}

#ifdef VIBRATE_TEST
	period = 500 - 20;
#else
	if (distance <= 0 || distance >= maxRange * 1000) {
		period = 0;
	} else {
		period = distance / FLASH_RATIO;
	}
#endif // VIBRATE_TEST
	return distance;
}

int32_t loopGeoMagSensor(unsigned long tick, uint32_t& period) {
	constexpr int16_t MAX_ANGLE = 150;
	constexpr int16_t MIN_ANGLE = 10;
	constexpr int16_t MAX_PERIOD = 1000;

	static bool calibrating = false;
	static unsigned long calTick;
	constexpr uint32_t MIN_CALIBRATE_TIME = 1000;
	static unsigned long btn1Tick = 0;
	static unsigned long btn2Tick = 0;

	if (buttonLongPressed(PIN_BUTTON1, btn1Tick) && !calibrating) {
		calibrating = true;
		compass.startCalibration();
		Serial_println("calibrate QMC5883L...");
		flash(30);
		calTick = millis();
	}
	if (!buttonPressing(PIN_BUTTON1) && calibrating) {
		calibrating = false;
		compass.endCalibration();
		if (millis() - calTick >= MIN_CALIBRATE_TIME) {
			Serial_printf("offsetX=%d offsetY=%d\n", compass.getOffsetX(), compass.getOffsetY());
			flash();
			delay(200);
			flash();
		} else { // reset offset
			Serial_println("reset offset");
			compass.setOffset();
			flash();
			delay(200);
		}
		prefs.putInt(PREFS_OFFSET_X, compass.getOffsetX());
		prefs.putInt(PREFS_OFFSET_Y, compass.getOffsetY());
		prefs.putInt(PREFS_OFFSET_Z, compass.getOffsetZ());
		prefs.flush();
	}
	if (calibrating) {
		compass.calibrate();
		period = 0;
		return 0;
	}

	compass.read();

	if (buttonLongPressed(PIN_BUTTON2, btn2Tick) && !calibrating) {
		int32_t degree = compass.getDegree(true);
		if (degree >= 180) {
			degree = degree - 360;
		}
		compass.setDeclinationAngle(-degree);
		flash(30);
		delay(200);
		flash(30);
		delay(500);
	}

	int32_t value = compass.getDegree();
	Serial_printf("X=%d Y=%d Z=%d\n", compass.getX(), compass.getY(), compass.getZ());
	int32_t degree = value;
	if (degree > 180) {
		degree = 360 - degree;
	}
	if (degree >= MAX_ANGLE) {
		period = 0;
	} else if (degree <= MIN_ANGLE) {
		period = MIN_PERIOD;
	} else {
		period = (MAX_PERIOD - MIN_PERIOD) * (degree - MIN_ANGLE) / (MAX_ANGLE - MIN_ANGLE) + MIN_PERIOD;
	}
	return value;
}

int32_t loopLightSensor(unsigned long tick, uint32_t& period) {
	constexpr int16_t MAX_LUX = 5000;
	constexpr int16_t MIN_LUX = 20;
	constexpr int16_t MAX_PERIOD = 1000;

	uint16_t lux = lightSensor.getLUX();
	if (lux < MIN_LUX) {
		period = 0;
	} else if (lux >= MAX_LUX) {
		period = MIN_PERIOD;
	} else {
		float logLux = log10f(lux);
		float logLuxMin = log10f(MIN_LUX);
		float logLuxMax = log10f(MAX_LUX);

		period = MAX_PERIOD - (uint16_t)((float)(MAX_PERIOD - MIN_PERIOD) * (logLux - logLuxMin) / (logLuxMax - logLuxMin));
	}
	return lux;
}
