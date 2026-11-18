# Joystick-controlled OLED rep tracker

Open `RepTracker.ino` in Arduino IDE, select UNO R4 Minima and upload. Install Adafruit GFX Library and Adafruit SSD1306 with their dependencies. For an UNO R3, set `USE_R4_12_BIT` to `0` and select Arduino Uno.

## Wiring

| Part | Connection |
| --- | --- |
| ADXL335 | X → A3, Y → A2, Z → A0, VCC → 3.3 V, GND → GND |
| Joystick | Vertical axis → A1, active-low switch → D2. Keep the existing power and ground connections. |
| SSD1306 OLED (128×64) | Existing SDA/SCL wiring, I²C address `0x3C` |

A2 is dedicated to the accelerometer Y output. If a shield knob also drives A2, disconnect that signal.

## Use

1. Tilt the joystick up or down to choose an exercise. Return it to center between steps and press it to select.
2. Get into the starting pose and press **Start tracking**. Each new set starts at zero.
3. Press the joystick on **Stop tracking** to stop counting and keep the total on screen. Start another set or tilt to **Back** and press to return to the exercise list.

All three menu entries use the same Y-axis detector. Its thresholds were tuned on the seated knee-extension recording. Bicep curls and lat raises need physical testing with their own sensor mounting. Match the recording's sensor orientation for leg exercises and let the sensor settle in the lowered position to arm the counter.

The device runs without a serial connection. Sending `r` at 115200 baud optionally clears the counter.

## Rule and evidence

The confirmed 8-rep capture readings-20261109-204841-037.csv contains a clear Y cycle between roughly 1600 and 1250 in R4 12-bit counts. The detector requires Y above 1550 to arm, below 1400 to begin, then above 1550 to count. Both crossings require 100 ms dwell. Cycles under 500 ms or not returning within 10 seconds are rejected. A five-sample median followed by an exponential filter rejects brief spikes. Internal values are divided by four on the R4, giving equivalent 10-bit thresholds 350 and 387.5. Sample rate is nominally 20 Hz; the screen updates when the menu or count changes. The blocking OLED transfer can introduce timing jitter.

The exact YRepCounter.h used by the sketch was compiled in tests/rep_tracker/y_counter.cpp and replayed on the confirmed R4 capture at intervals of at least 50 ms. It counted 0 in rest trial 2 and 8 in good-rep trial 3. Synthetic checks cover arming, count-on-return, steady rest, small excursions, isolated spikes, and timeout. These are same-recording checks used to select the rule, not independent validation. Test a NEW counted set before relying on it. Joystick, OLED and accelerometer operation still need to be checked on the physical device.

This prototype counts sufficient-amplitude movement cycles. It does not score full/partial form, measure joint angles, or exclude every non-exercise motion. Different mounting, exercise, supply or board may need new thresholds. The saved R3 data does not validate the current thresholds. If it stays at zero, check that Y/A2 reaches above 1550 at start and below 1400 during extension in 12-bit R4 mode using the separate baseline logger after matching its pin definitions to this wiring. Returning the sensor to a different orientation can prevent arming. Do not adjust thresholds just to force a desired count.

The older RepDetector.h and tests/rep_tracker/replay.cpp describe the earlier teaching-based experiment; the current sketch uses YRepCounter.h only. RepBaselineLogger remains available for collecting raw data, with its own board switch. Do not run its CSV logger against this count-only sketch.
