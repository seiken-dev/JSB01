#include <LittleFS.h>
#include "Prefs.h"

bool Prefs::begin(const char* path) {
	if (!initialized) {
		initialized = true;
#ifdef ARDUINO_XIAO_ESP32C3
		LittleFS.begin(true);
#else
		LittleFS.begin();
#endif
	}
	_path = path;
	if (exists()) {
		return readPrefsFile();
	}
	return false;
}

void Prefs::end() {
	if (modified) {
		flush();
	}
	LittleFS.end();
	initialized = false;
}

bool Prefs::exists() {
	return LittleFS.exists(_path);
}

bool Prefs::remove(const char* path) {
	return LittleFS.remove(path);
}

bool Prefs::readPrefsFile() {
	lines.clear();
	File file = LittleFS.open(_path, "r");
	if (!file) return false;
	int size = file.size();
	int total = 0;
	while (total < size) {
		String line = file.readStringUntil('\n');
		int len = line.length();
		total += len + 1;
		line.trim();
//		Serial.print(line);
//		Serial.printf(" (%d)\n", len);
		if (line.length() == 0) break;
		lines.push_back(line);
	}
//	Serial.printf("read %d lines\n", lines.size());
	file.close();
	return true;
}

bool Prefs::flush() {
	File file = LittleFS.open(_path, "w");
	if (!file) return false;
	for (String& line : lines) {
		file.println(line);
	}
	file.close();
	modified = false;
	return true;
}

int Prefs::findKey(const char* key) {
	for (int i = 0; i < lines.size(); i++) {
		String& line = lines[i];
		if (line.startsWith(key)) {
			return i;
		}
	}
	return -1;
}

void Prefs::putInt(const char* key, int32_t value) {
	putString(key, String(value));
}

void Prefs::putFloat(const char* key, float value, unsigned int decimalPlaces) {
	putString(key, String(value, decimalPlaces));
}

void Prefs::putBool(const char* key, bool value) {
	putString(key, String(value));
}

void Prefs::putString(const char* key, const String& value) {
	int index = findKey(key);
	if (index != -1) {
		lines.erase(lines.begin() + index);
	}
	String line(key);
	line += "=" + value;
	lines.push_back(line);
}


int32_t Prefs::getInt(const char* key, int32_t defaultValue) {
	String value = getString(key);
	if (value.length() == 0) return defaultValue;
	return value.toInt();
}

float Prefs::getFloat(const char* key, float defaultValue) {
	String value = getString(key);
	if (value.length() == 0) return defaultValue;
	return value.toFloat();
}

bool Prefs::getBool(const char* key, bool defaultValue) {
	String value = getString(key);
	if (value.length() == 0) return defaultValue;
	return value.toInt() != 0;
}

String Prefs::getString(const char* key) {
	int index = findKey(key);
	if (index == -1) return "";
	String& line = lines[index];
	int n = line.indexOf("=");
	if (n == -1) return "";
	return line.substring(n + 1);
}
