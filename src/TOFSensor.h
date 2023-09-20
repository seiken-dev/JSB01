#pragma once

#ifdef TOF_SENSOR

enum class TOFType : uint8_t { None, L1X, L5CX };

class TOFSensor {
public:
	TOFType begin();
	int16_t getDistance();
	void setFOV(bool isWide = true);
};

#endif // TOF_SENSOR