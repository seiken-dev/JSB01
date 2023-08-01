#include <Arduino.h>
#include "mydefs.h"

#ifdef TOF_SENSOR
int8_t TOF_init();
int16_t TOF_distance();
void TOF_setFOV(bool isWide = true);

#define LONGPRESS_MS 1000
#endif // TOF_SENSOR

//#define DEBUG_SERIAL
//#define XIAO_RP2040
//#define AT_TINY

#ifdef XIAO_RP2040
#undef PIN_LED
#define PIN_LED PIN_LED_G
#define LED_ON LOW
#define LED_OFF HIGH
#else
#define LED_ON HIGH
#define LED_OFF LOW
#endif

#ifdef XIAO_RP2040
#define PIN_SONAR D5
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

#define TIMEOUT (500 * 1000)

#define RANGE_MIN 1
#define RANGE_MAX 5
#define RANGE_DEFAULT 3

#define FLASH_MS 20
#define FLASH_RATIO 4
#define MIN_PERIOD 75

void flash(uint16_t ms = FLASH_MS) {
#ifdef PIN_LED
	digitalWrite(PIN_LED, LED_ON);
#endif
    digitalWrite(PIN_VIB, HIGH);
	delay(ms);
#ifdef PIN_LED
	digitalWrite(PIN_LED, LED_OFF);
#endif
    digitalWrite(PIN_VIB, LOW);
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

#ifdef TOF_SENSOR
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
#endif // TOF_SENSOR

enum Sonar {
	NONE, MB10x0, MB10x3, TOF, TOF8x8
};
Sonar sonar = Sonar::NONE;

#ifdef DEBUG_SERIAL
static const char* sonarNames[] = { "NONE", "MB10x0", "MB10x3", "VL53L1X", "VL53L5CX" };
#endif

void setup() {
	Serial_begin();
	Serial_println("start");

#ifdef XIAO_RP2040
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
	delay(1000); // wait USB Serial

	Serial_print("detect sonar...");

	uint32_t value = (uint32_t)pulseIn(PIN_SONAR, HIGH, TIMEOUT);
	if (value != 0) {
		delay(20);
		unsigned long t = millis();
		pulseIn(PIN_SONAR, HIGH, TIMEOUT);
		uint32_t interval = (uint32_t)(millis() - t);
		if (interval > 20) {
			sonar = interval < 60 ? Sonar::MB10x0 : Sonar::MB10x3; // MB10x0:20Hz, MB10x3:10Hz
		}
	} else {
#ifdef TOF_SENSOR
		uint8_t r = TOF_init();
		if (r > 0) {
			sonar = Sonar::TOF;
		} else if (r < 0) {
			sonar = Sonar::TOF8x8;
		}
#endif
	}

	Serial_printf("%s\n", sonarNames[sonar]);

	if (sonar != Sonar::NONE) {
		flash();
	    delay(200);
		flash();
	} else {
		flash();
	    delay(500);
		flash();
	    delay(500);
		flash();
#ifdef XIAO_RP2040
	    digitalWrite(PIN_LED_R, LED_ON);
#endif
	}
    delay(500);
}

#define MIN_DELAY 20

void loop() {
	static unsigned long startTick = 0;
	static int32_t prevDistance = 0;
	static uint32_t period = 0;
	static uint16_t maxRange = RANGE_DEFAULT;
	static bool btn1 = false;
	static bool btn2 = false;
#ifdef TOF_SENSOR
	static unsigned long btn1Tick = 0;
	static unsigned long btn2Tick = 0;
#endif
	unsigned long tick = millis();

	if (period != 0 && tick - startTick >= period) {
		flash();
		startTick = tick;
	}

#ifdef TOF_SENSOR
	if (sonar == Sonar::TOF) {
		if (buttonLongPressed(PIN_BUTTON1, btn1Tick)) {
			TOF_setFOV(true);
			Serial_println("ROI 16x16");
			flash(30);
			delay(100);
			flash(80);
			delay(500);
		}
		if (buttonLongPressed(PIN_BUTTON2, btn2Tick)) {
			TOF_setFOV(false);
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
	if (sonar == Sonar::TOF) {
		distance = TOF_distance();
	} else {
#endif
	distance = (int32_t)pulseIn(PIN_SONAR, HIGH, TIMEOUT);
#ifdef TOF_SENSOR
	}
#endif
	if (sonar == Sonar::MB10x0) {
		distance = distance * 24 / 139; // ≒ / 147.f * 25.4f
	}

	uint32_t elapse = (uint32_t)(millis() - tick);

	if (distance <= 0 || distance >= maxRange * 1000) {
		period = 0;
	} else {
		period = distance / FLASH_RATIO;
		if (period < MIN_PERIOD) {
			period = MIN_PERIOD;
		}
	}
	if (distance != prevDistance) {
		if (period != 0) {
			Serial_printf("%d T=%dms (%d)\n", distance, period, elapse);
		} else {
			Serial_printf("%d (%d)\n", distance, elapse);
		}
	}
	prevDistance = distance;

	if (elapse < MIN_DELAY) {
		delay(MIN_DELAY - elapse);
	}
}
