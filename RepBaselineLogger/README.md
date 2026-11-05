# Rep baseline data collection

Target: Arduino UNO R4 Minima and the Grove ADXL335 listed in the supplied parts list. This logger intentionally does not decide whether a motion is a rep yet. It records labelled time series for choosing thresholds later.

## Wiring (power disconnected)

Use the sensor board's printed labels, not wire colours or connector order:

| Sensor | UNO R4 Minima |
|---|---|
| VCC | 3.3V |
| GND | GND |
| X output | A0 |
| Y output | A1 |
| Z output | A2 |

This assumes access to the three analog axis outputs. Some Grove connections do not expose all axes through a single cable: use the labelled axis pads/headers as needed. Do not connect an axis output to a digital pin or assume the device uses I2C. If your board has SDA/SCL instead of X/Y/Z, it needs a different sketch. A bare ADXL335 chip must not receive 5V; this wiring uses 3.3V. Leave AREF unconnected.

## Upload and capture

1. Open RepBaselineLogger.ino in Arduino IDE. Install/select the Arduino UNO R4 Boards package and UNO R4 Minima, select its port, and upload. No sensor library is needed.
2. Open Serial Monitor at 115200 baud. Send `h` for the CSV column header. Newline or CR line endings are ignored.
3. Secure the sensor to one position on a limb. Choose ONE exercise (for example, an arm curl). Note the side, location and orientation; keep those fixed across trials.
4. Send a label command below. It begins a fresh trial immediately, with elapsed time reset to zero. Wait about 2 seconds before moving and remain still about 2 seconds after the final motion, then send `s`.
5. Copy the header and complete numeric rows to a plain-text file named readings.csv. Disable autoscroll to select text if needed. The CSV is streamed to the computer, not saved on the Arduino; preserve the output before closing the monitor. For longer captures use a serial terminal with file logging at 115200 baud, with timestamps/echo disabled. Only one program can own the serial port at a time.

| Send | Trial label | Suggested first dataset |
|---|---|---|
| r | Rest | 10 seconds still in starting pose |
| g | Full/good reps, judged by observer | 10 complete reps at normal pace |
| p | Partial reps, judged by observer | 10 intentionally incomplete reps |
| n | Non-rep movement | 15 seconds of repositioning/small movements |
| s | Stop recording | Use between trials |
| h | Print header | Use before recording |
| m | Increment optional manual marker | Observer can send near each completed motion |

Repeat full reps in separate trials at a slower and a faster comfortable pace. For this initial bench prototype use team members performing familiar movements. An observer's definition of a full rep is the ground truth; the sensor does not establish clinical correctness.

`m` is optional and approximate because of human reaction/serial delay. The marker number persists in subsequent rows until changed. It is not an automatic rep count. If testing alone, simply report the observed count for each trial instead.

## Data to send back

Send the complete CSV, not just maxima, averages or a few rows. Include:

- Exercise name and what you considered a full versus partial rep.
- Sensor model, board model, wiring/power and any code changes.
- Attachment location, side and orientation (photo useful), plus whether it slipped.
- For each trial ID: actual observed motion count, pace, and what happened in non-rep trials.

Example trial notes (fill with YOUR observed counts):

| trial | exercise | label | observed count | notes |
|---|---|---|---|---|
| 1 | arm curl | r | 0 | still, arm down |
| 2 | arm curl | g | 10 | full out-and-back cycles |
| 3 | arm curl | p | 10 | only halfway up, then back |
| 4 | arm curl | n | 0 | repositioning and small movements |

Columns: trial ID, user-selected label, sample index (starts at 0), milliseconds since trial began, three raw 12-bit ADC readings (0..4095), optional marker count. Sampling is nominally 50 Hz; use actual t_ms intervals to detect delays. Readings include gravity and are not calibrated acceleration, distance or joint angle. No baseline subtraction or filtering is applied, so the original signal is retained. A single accelerometer cannot in general establish joint range or all aspects of exercise form.

## Quick checks

- While still, readings should be fairly stable; slowly tilting the board should change one or more axes.
- If an axis is stuck near 0 or 4095, check wiring. Stable midscale readings alone do not prove the sensor is connected: floating inputs can produce plausible-looking values.
- Keep supply, ADC resolution, mounting and exercise consistent. Remounting may require a new baseline.
- Resetting the Arduino resets trial IDs. Save separate capture files per session and identify them in your notes.

We can use the data to propose an axis/combined signal, filtering, separate departure/return thresholds, minimum motion duration and a return-to-start rule, then evaluate those rules on a separate labelled test capture. Thresholds are exercise/mounting specific. Detecting a repeated motion is not equivalent to validating clinical form.

References: https://wiki.seeedstudio.com/Grove-3-Axis_Analog_Accelerometer/ and https://docs.arduino.cc/tutorials/uno-r4-minima/adc-resolution

Validation: reviewed against the Arduino R4 ADC documentation; not compiled or hardware tested in this workspace (Arduino toolchain/hardware unavailable).
