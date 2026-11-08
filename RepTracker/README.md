# Seated knee-extension OLED prototype

Open RepTracker.ino in Arduino IDE (keep RepDetector.h in the same folder). This is a separate sketch; RepBaselineLogger is preserved for collecting data. Install Adafruit GFX Library and Adafruit SSD1306 with dependencies. UNO R3 is selected in code (`USE_R4_12_BIT 0`); choose Arduino Uno in IDE. Set the switch to 1 only for R4 Minima and select that board in IDE. R4 readings are divided by four internally to use equivalent 10-bit count units.

## Wiring

Current wiring: ADXL335 X -> A3, Y -> A2, Z -> A1, VCC -> 3.3V, GND -> GND. Shield OLED is already on SDA/SCL (A4/A5), SSD1306 128x64 address 0x3C. This tracker intentionally does NOT read Y/A2 because the lab shield potentiometer occupies it. If that knob signal has not been physically isolated, disconnect the sensor's Y wire from A2 with power off; do not leave two analog outputs connected together. X and Z are sufficient for this particular exploratory detector, provided leg motion changes them. Keep mounting and grip fixed.

## Start the demo

1. Close the CSV logger before uploading. Upload RepTracker.ino and open Serial Monitor at 115200 baud.
2. Sensor against shin just above ankle; leg in starting position. Send `c` and remain still for 3 seconds. The same calibration happens automatically on startup. If it says Moved, retry c. A blank display does not establish sensor failure: check the OLED libraries/wiring separately.
3. When OLED says Send t, send `t`. Perform THREE full, comfortable seated knee extensions. After each return, hold the starting pose for about half a second. Teaching reps must depart from and return to the same calibrated start. They do not add to the displayed counts.
4. If teaching is weak/uneven, check sensor movement/wiring/grip, recalibrate with c, then retry t. Do not force a count by shaking the board.
5. After three accepted examples, counting begins automatically. OLED shows Full and Part counts. A count occurs on return to the start, not at the top of the raise. Try 10 full reps, then 10 partial reps, and compare the displayed counts to your observed counts.
6. `s` pauses, `g` resumes after successful teaching, `r` resets counts while retaining the taught threshold, `c` recalibrates and requires reteaching. Power/reset loses calibration. These commands differ from the old raw-data logger; do not use SaveReadings.ps1 to control this demo.

## Interpretation and limitations

The rest capture and first uncounted movement capture have clearly different X excursions. However, the later counted full and partial recordings have dramatically different resting levels from those earlier files and much smaller movement signals. These cannot establish a validated fixed full/partial boundary. This sketch therefore uses per-session teaching instead of claiming a threshold trained from those labels.

At nominal 20 Hz, X/Z are exponentially smoothed (alpha 0.5), and displacement in ADC signal space from the rest mean is computed. This is not physical displacement or a knee angle. The detector arms after 150 ms at home, begins at a 5-count excursion, and completes after returning within 2.5 counts for 150 ms. Motions shorter than 0.5 seconds or longer than 10 seconds are rejected. The maximum excursion is compared to 70% of the median of three teaching examples. Teaching examples must each exceed 10 counts and their peak ratio must be at most 1.6. These are initial engineering settings, not clinically validated form criteria. Tiny partial motions below 5 counts are ignored; equal-amplitude non-exercise motions may also count. The prototype cannot identify correct exercise form from amplitude alone.

20 Hz sampling and 5 Hz screen refresh leave room for the OLED's blocking transfer. A long delay skips samples rather than inventing catch-up samples. Event lines on Serial report time, detector result (1 full, 2 partial, 3 rejected), peak, full count, partial count. Teaching event result codes use a provisional threshold and should not be interpreted as form judgments.

## Verification

Host replay uses the exact RepDetector.h included by the Arduino sketch. Compile replay.cpp with a C++11 compiler and run it with a saved R3 CSV. Replay estimates rest from the first 2 seconds and uses a provisional 15-count full threshold; it does not perform the live three-rep teaching step. Its purpose is to expose data limitations, not claim fitted accuracy. Built-in checks cover stillness, no count before return, full/partial cycles, jitter, timeout, and opposite motion direction. Arduino library compilation, SRAM use on R3, and physical OLED operation still require an IDE build and hardware verification.
