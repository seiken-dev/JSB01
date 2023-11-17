#include <Wire.h>
#include "Compass.h"

#define QMC5883_ADDRESS (0x0D)
#define QMC5883_REG_CONFIG_1 (0x09)
#define QMC5883_REG_CONFIG_2 (0x0A)
#define QMC5883_REG_IDENT_B (0x0B)
#define QMC5883_REG_IDENT_C (0x20)
#define QMC5883_REG_IDENT_D (0x21)
#define QMC5883_REG_OUT_X (0x00)
#define QMC5883_REG_OUT_Y (0x02)
#define QMC5883_REG_OUT_Z (0x04)

Compass::Compass() :
	addr(QMC5883_ADDRESS),
	offsetX(0),
	offsetY(0),
	offsetZ(0),
	declinationAngle(0.0f),
	deviceAngle(0)
{}

bool Compass::begin() {
	Wire.begin();
	Wire.setClock(400000);

	Wire.beginTransmission(addr);
	if (Wire.endTransmission() != 0) return false;

	writeRegByte(QMC5883_REG_IDENT_B, 0X01);
	writeRegByte(QMC5883_REG_IDENT_C, 0X40);
	writeRegByte(QMC5883_REG_IDENT_D, 0X01);

	setSamples(QMC5883_SAMPLES_64);
	setDataRate(QMC5883_DATARATE_50HZ);
	setRange(QMC5883_RANGE_8GA);
	setMeasurementMode(QMC5883_CONTINOUS);
	return true;
}

void Compass::read() {
	rawX = readRegWord(QMC5883_REG_OUT_X);
	rawY = readRegWord(QMC5883_REG_OUT_Y);
	rawZ = readRegWord(QMC5883_REG_OUT_Z);
//	Serial.printf("raw X=%d Y=%d Z=%d\n", rawX, rawY,rawZ);
}

int16_t Compass::getDegree(bool isRaw) {
	float heading = atan2(getY(), getX());
	int32_t degree = (int32_t)round(heading * 180.f / 3.1416f);
	if (!isRaw) {
		degree += declinationAngle;
	}
	degree += deviceAngle;
	if (degree >= 360) {
		degree -= 360;
	} else if (degree < 0) {
		degree += 360;
	}
	return degree;
}

void Compass::startCalibration() {
	minX = minY = minZ = INT16_MAX;
	maxX = maxY = maxZ = INT16_MIN;
}

void Compass::calibrate() {
	read();

	if (rawX < minX) {
		minX = rawX;
	} else if (rawX > maxX) {
		maxX = rawX;
	}
	if (rawY < minY) {
		minY = rawY;
	} else if (rawY > maxY) {
		maxY = rawY;
	}
	if (rawZ < minZ) {
		minZ = rawZ;
	} else if (rawZ > maxZ) {
		maxZ = rawZ;
	}
}

void Compass::endCalibration() {
	offsetX = (minX + maxX) / 2;
	offsetY = (minY + maxY) / 2;
	offsetZ = (minZ + maxZ) / 2;
}

void Compass::setOffset(int16_t _offsetX, int16_t _offsetY, int16_t _offsetZ) {
	offsetX = _offsetX;
	offsetY = _offsetY;
	offsetZ = _offsetZ;
}

void Compass::setMeasurementMode(QMC5883_Mode mode) {
	uint8_t value = readRegByte(QMC5883_REG_CONFIG_1);
	value &= 0xFC;
	value |= mode;
	writeRegByte(QMC5883_REG_CONFIG_1, value);
}

void Compass::setDataRate(QMC5883_DataRate dataRate) {
	uint8_t value = readRegByte(QMC5883_REG_CONFIG_1);
	value &= 0xF3;
	value |= (dataRate << 2);
	writeRegByte(QMC5883_REG_CONFIG_1, value);
}

void Compass::setRange(QMC5883_Range range) {
	uint8_t value = readRegByte(QMC5883_REG_CONFIG_1);
	value &= 0xCF;
	value |= (range << 4);
	writeRegByte(QMC5883_REG_CONFIG_1, value);
}

void Compass::setSamples(QMC5883_Samples samples) {
	uint8_t value = readRegByte(QMC5883_REG_CONFIG_1);
	value &= 0x3F;
	value |= (samples << 6);
	writeRegByte(QMC5883_REG_CONFIG_1, value);
}

uint8_t Compass::readRegByte(uint8_t reg) {
	Wire.beginTransmission(addr);
	Wire.write(reg);
	Wire.endTransmission();
	Wire.requestFrom(addr, 1);
	while(!Wire.available()) {};
	return Wire.read();
}

int16_t Compass::readRegWord(uint8_t reg) {
	Wire.beginTransmission(addr);
	Wire.write(reg);
	Wire.endTransmission();
	Wire.requestFrom(addr, 2);
	while(!Wire.available()) {};
	uint8_t lsb = (uint8_t)Wire.read();
	return  Wire.read() << 8 | lsb;
}

void Compass::writeRegByte(uint8_t reg, uint8_t value) {
	Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}


