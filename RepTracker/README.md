# Joystick-controlled OLED rep tracker

Open `RepTracker.ino` in Arduino IDE, select UNO R4 Minima and upload. Install Adafruit GFX Library and Adafruit SSD1306 with their dependencies. For an UNO R3, set `USE_R4_12_BIT` to `0` and select Arduino Uno.

## Wiring

| Part | Connection |
| --- | --- |
| ADXL335 | X → A0, Y → A2, Z → A3, VCC → 3.3 V, GND → GND |
| Joystick | Vertical axis → A1, active-low switch → D2. Keep the existing power and ground connections. |
| SSD1306 OLED (128×64) | Existing SDA/SCL wiring, I²C address `0x3C` |

A2 is dedicated to the accelerometer Y output. If a shield knob also drives A2, disconnect that signal.

## Use

1. Tilt the joystick up or down to choose an exercise. Return it to center between steps and press it to select.
2. Match the sensor orientation from the recording. Start with your leg or arm lowered, press **Start**, and briefly hold the lowered pose so the detector can arm. Each new set starts at zero.
3. Press the joystick on **Stop** to freeze the total. Start another set or tilt to **Back** and press to return to the exercise list.

The device runs without a serial connection. Sending `r` at 115200 baud optionally clears the counter.

## Exercise profiles

The menu selects the sensor signal and thresholds automatically. The starting baselines are medians from the first two seconds of the leg-raise set and the first three seconds of the curl and lat-raise sets. Values use UNO R4 12-bit ADC counts.

| Exercise | Sensor signal | Recorded starting baseline | Arm and count on return | Movement threshold |
| --- | --- | ---: | --- | --- |
| Leg Raises | Y / A2 | ~1647 | ≥1550 for 100 ms | ≤1400 for 100 ms |
| Bicep Curls | Z / A3 | ~1138 | ≤1300 for 100 ms | ≥1500 for 450 ms |
| Lat Raises | Z − Y / A3 − A2 | ~−161 | ≤−100 for 100 ms | ≥130 for 250 ms |

All profiles smooth incoming samples with a five-sample median and an exponential filter. After confirming movement, the detector requires a return within 0.5–10 seconds to count a rep. The R4 sketch divides input readings by four for the internal 10-bit scale. Sampling is nominally 20 Hz. OLED transfers can introduce timing jitter.

The firmware detector in `RepCounter.h` was replayed with `tests/rep_tracker/profile_counter.cpp`. It counted eight leg raises, zero in the two leg rest recordings, seven completed curls, and twelve lat raises. The curl recording ends while the eighth lift is held up. The profiles were fitted to these recordings. Accuracy on new sets, physical operation, and compatibility of the thresholds with R3 readings still need testing. Different mounting or orientation can change the starting readings enough to prevent arming.

See [the capture notes](DATA_FINDINGS.md) for the calibration details. Use the separate RepBaselineLogger sketch to collect more raw data.
