#pragma once

#ifdef TOF_SENSOR

enum class TOFType : uint8_t { None, L1X, L5CX };

class TOFSensor {
public:
	static TOFType begin();
	static int16_t getDistance();
	static void setFOV(bool isWide = true);
};

#endif // TOF_SENSOR