#pragma once
#include "Arduino.h"
#include <vector>

class VibeFeedback {
 public:
  enum DistanceStatus : uint8_t {
    s_err = 0,
    s_400,
    s_300,
    s_200,
    s_150,
    s_120,
    s_100,
    s_70,
    s_40,
    s__nodata
  };
  bool begin();
  bool begin(uint8_t vibePin);
  bool feedback(DistanceStatus s);
  bool stop();
  DistanceStatus getCurrent() { return _current; };
  bool viberation();
 private:
  volatile DistanceStatus _current = s__nodata;
  volatile DistanceStatus _next;
  uint8_t _pin = D2;
};
extern VibeFeedback vibe;
