#pragma once
#include <stdint.h>

struct RepProfile {
  float restThreshold;
  float movementThreshold;
  uint32_t movementDwellMs;
};

// Thresholds are normalized to 10-bit units. The captures use 12-bit counts.
constexpr RepProfile LEG_RAISE_PROFILE{1550.0f / 4, 1400.0f / 4, 100};
constexpr RepProfile BICEP_CURL_PROFILE{1300.0f / 4, 1500.0f / 4, 450};
constexpr RepProfile LAT_RAISE_PROFILE{-100.0f / 4, 130.0f / 4, 250}; // Z minus Y.

struct RepCounter {
  uint32_t reps = 0;

  void reset() { *this = RepCounter(); }

  bool update(float sample, uint32_t now, const RepProfile& profile) {
    if (!initialized) {
      for (float& value : history) value = sample;
      filtered = sample;
      initialized = true;
    }
    history[index] = sample;
    index = (index + 1) % 5;
    float sorted[5];
    for (uint8_t i = 0; i < 5; ++i) sorted[i] = history[i];
    for (uint8_t i = 1; i < 5; ++i) {
      const float value = sorted[i];
      int8_t j = i - 1;
      while (j >= 0 && sorted[j] > value) {
        sorted[j + 1] = sorted[j];
        --j;
      }
      sorted[j + 1] = value;
    }
    filtered += 0.5f * (sorted[2] - filtered);

    if (moving && now - started > 10000) {
      moving = armed = restPending = movementPending = false;
    }
    const bool rises = profile.movementThreshold > profile.restThreshold;
    const bool atRest = rises ? filtered <= profile.restThreshold
                              : filtered >= profile.restThreshold;
    const bool atPeak = rises ? filtered >= profile.movementThreshold
                              : filtered <= profile.movementThreshold;
    if (atRest) {
      movementPending = false;
      if (!restPending) {
        restSince = now;
        restPending = true;
      }
      if (now - restSince >= 100) {
        const bool counted = moving && now - started >= 500;
        moving = false;
        armed = true;
        if (counted) {
          ++reps;
          return true;
        }
      }
    } else {
      restPending = false;
      if (atPeak && armed && !moving) {
        if (!movementPending) {
          movementSince = now;
          movementPending = true;
        }
        if (now - movementSince >= profile.movementDwellMs) {
          moving = true;
          armed = false;
          started = now;
          movementPending = false;
        }
      } else {
        movementPending = false;
      }
    }
    return false;
  }

 private:
  float filtered = 0;
  float history[5] = {};
  uint8_t index = 0;
  bool initialized = false;
  bool armed = false;
  bool moving = false;
  bool restPending = false;
  bool movementPending = false;
  uint32_t restSince = 0;
  uint32_t movementSince = 0;
  uint32_t started = 0;
};
