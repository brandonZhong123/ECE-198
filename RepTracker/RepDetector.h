#pragma once
#include <stdint.h>
#include <math.h>

// All values are normalized to 10-bit ADC counts, including on an R4.
// Uses X and Z only: shield potentiometer may occupy Y/A2.
struct RepDetector {
  float bx = 0, bz = 0, fx = 0, fz = 0, distance = 0;
  float peak = 0, fullThreshold = 15;
  bool active = false, armed = false;
  uint32_t started = 0, homeSince = 0;
  bool homePending = false;
  unsigned full = 0, partial = 0, rejected = 0;
  static constexpr float START = 5.0f;
  static constexpr float HOME = 2.5f;
  void reset(float x, float z) {
    bx = fx = x; bz = fz = z; distance = peak = 0;
    active = armed = homePending = false;
    full = partial = rejected = 0;
  }
  // Returns 1=full, 2=partial, 3=timeout/too-fast; 0=no completed event.
  int update(float x, float z, uint32_t now) {
    fx += 0.5f * (x - fx); fz += 0.5f * (z - fz);
    distance = sqrtf((fx-bx)*(fx-bx) + (fz-bz)*(fz-bz));
    if (!active) {
      if (distance <= HOME) {
        if (!homePending) { homeSince = now; homePending = true; }
        if (now-homeSince >= 150) armed = true;
      } else homePending = false;
      if (armed && distance >= START) {
        active = true; armed = false; started = now; peak = distance;
        homePending = false;
      }
      return 0;
    }
    if (distance > peak) peak = distance;
    if (now-started > 10000) {
      active = armed = homePending = false; ++rejected; return 3;
    }
    if (distance <= HOME) {
      if (!homePending) { homeSince = now; homePending = true; }
      if (now-homeSince >= 150) {
        active = false; armed = true;
        if (now-started < 500) { ++rejected; return 3; }
        if (peak >= fullThreshold) { ++full; return 1; }
        ++partial; return 2;
      }
    } else homePending = false;
    return 0;
  }
};
