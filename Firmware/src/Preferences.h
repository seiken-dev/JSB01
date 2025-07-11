#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <Arduino.h>
#include <LittleFS.h>

class Preferences {
 public:
  Preferences();
  ~Preferences();

  void begin();
  void end();

  void put(const char* key, const char* value);
  bool get(const char* key, char* value);

 private:
  String content;
};
extern Preferences pref;
#endif
