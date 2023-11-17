#pragma once

enum QMC5883_Samples {
	QMC5883_SAMPLES_512 = 0b00,
	QMC5883_SAMPLES_256 = 0b01,
	QMC5883_SAMPLES_128 = 0b10,
	QMC5883_SAMPLES_64 = 0b11
};

enum QMC5883_DataRate {
	QMC5883_DATARATE_10HZ = 0b00,
	QMC5883_DATARATE_50HZ = 0b01,
	QMC5883_DATARATE_100HZ = 0b10,
	QMC5883_DATARATE_200HZ = 0b11,
};

enum QMC5883_Range {
	QMC5883_RANGE_2GA = 0b00,
	QMC5883_RANGE_8GA = 0b01,
};

enum QMC5883_Mode {
	QMC5883_SINGLE = 0b00,
	QMC5883_CONTINOUS = 0b01,
};

class Compass {
public:
	Compass();

	bool begin();
	void read();
	int16_t getX() { return rawX - offsetX; }
	int16_t getY() { return rawY - offsetY; }
	int16_t getZ() { return rawZ - offsetZ; }
	int16_t getDegree(bool isRaw = false);
	void startCalibration();
	void calibrate();
	void endCalibration();
	int16_t getOffsetX() { return offsetX; }
	int16_t getOffsetY() { return offsetY; }
	int16_t getOffsetZ() { return offsetZ; }
	void setOffset(int16_t _offsetX = 0, int16_t _offsetY = 0, int16_t _offsetZ = 0);
	void setDeclinationAngle(float degree) { declinationAngle = degree; }
	float getDeclinationAngle() { return declinationAngle; }
	void setDeviceAngle(int16_t degree) { deviceAngle = degree; }
	int16_t getDeviceAngle() { return deviceAngle; }

	void setMeasurementMode(QMC5883_Mode mode);
	void setDataRate(QMC5883_DataRate dataRate);
	void setRange(QMC5883_Range range);
	void setSamples(QMC5883_Samples samples);

private:
	uint8_t readRegByte(uint8_t reg);
	int16_t readRegWord(uint8_t reg);
	void writeRegByte(uint8_t reg, uint8_t value);

	uint8_t addr;
	int16_t rawX, rawY, rawZ;
	int16_t minX, minY, minZ;
	int16_t maxX, maxY, maxZ;
	int16_t offsetX, offsetY, offsetZ;
	float declinationAngle;
	int16_t deviceAngle;

};