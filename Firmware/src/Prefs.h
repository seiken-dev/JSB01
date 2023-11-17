#pragma once

#include <Arduino.h>
#include <vector>

class Prefs {
public:
	Prefs() : initialized(false), modified(false) {}

	bool begin(const char* path);
	void end();
	bool exists();
	bool remove(const char* path);
	bool flush();

	void putInt(const char* key, int32_t value);
	void putFloat(const char* key, float value, unsigned int decimalPlaces = 2);
	void putBool(const char* key, bool value);
	void putString(const char* key, const String& value);

	int32_t getInt(const char* key, int32_t defaultValue = 0);
	float getFloat(const char* key, float defaultValue = 0.0f);
	bool getBool(const char* key, bool defaultValue = false);
	String getString(const char* key);

private:
	bool readPrefsFile();
	int findKey(const char* key);

	const char* _path;
	bool initialized;
	bool modified;
	std::vector<String> lines;
};