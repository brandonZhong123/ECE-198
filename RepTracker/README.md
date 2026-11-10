# Simple OLED rep counter

Open RepTracker.ino in Arduino IDE, select UNO R4 Minima and upload. Install Adafruit GFX Library and Adafruit SSD1306 with dependencies. The board switch USE_R4_12_BIT is 1 for the current R4; change it to 0 and select Arduino Uno when using an R3.

ADXL335 wiring: X=A0, Y=A1, Z=A3, VCC=3.3V, GND=GND. Only Y/A1 is used for this counter. A2 remains reserved for the shield knob. OLED is the prewired lab SSD1306 128x64 at 0x3C on SDA/SCL. Disconnect power before changing wiring.

## Use

Close the CSV logger, upload, and start with the sensor in the same position and orientation used for the November 9 seated knee-extension recording. Lower the leg to its starting pose briefly before exercising. The OLED shows REPS and the count. There are no trials, teaching steps or Serial commands required. The counter increments only after the leg completes an excursion and returns. Reset the Arduino to clear the count; optional Serial r at 115200 also clears it. No serial connection is required after upload. Keep wires from pulling the sensor or rotating it.

## Rule and evidence

The confirmed 8-rep capture readings-20261109-204841-037.csv contains a clear Y cycle between roughly 1600 and 1250 in R4 12-bit counts. The detector requires Y above 1550 to arm, below 1400 to begin, then above 1550 to count. Both crossings require 100 ms dwell. Cycles under 500 ms or not returning within 10 seconds are rejected. A five-sample median followed by an exponential filter rejects brief spikes. Internal values are divided by four on the R4, giving equivalent 10-bit thresholds 350 and 387.5. Sample rate is nominally 20 Hz; screen updates at 5 Hz. The blocking OLED transfer can introduce timing jitter.

The exact YRepCounter.h used by the sketch was compiled in tests/rep_tracker/y_counter.cpp and replayed on the confirmed R4 capture at intervals of at least 50 ms. It counted 0 in rest trial 2 and 8 in good-rep trial 3. Synthetic checks cover arming, count-on-return, steady rest, small excursions, isolated spikes, and timeout. These are same-recording checks used to select the rule, not independent validation. Test a NEW counted set before relying on it. Physical Arduino compilation and OLED operation have not been verified here.

This prototype counts sufficient-amplitude movement cycles. It does not score full/partial form, measure joint angles, or exclude every non-exercise motion. Different mounting, exercise, supply or board may need new thresholds. The saved R3 data does not validate the current thresholds. If it stays at zero, check that Y/A1 reaches above 1550 at start and below 1400 during extension in 12-bit R4 mode using the separate baseline logger. Returning the sensor to a different orientation can prevent arming. Do not adjust thresholds just to force a desired count.

The older RepDetector.h and tests/rep_tracker/replay.cpp describe the earlier teaching-based experiment; the current sketch uses YRepCounter.h only. RepBaselineLogger remains available for collecting raw data, with its own board switch. Do not run its CSV logger against this count-only sketch.
