# Rep baseline data collection

Target: Arduino UNO R4 Minima and Grove ADXL335 analog accelerometer. This logger intentionally does not decide whether a motion is a rep yet. It records labelled time series for choosing thresholds later.

## Wiring (disconnect USB/power first)

| Grove ADXL335 pin | UNO R4 Minima |
|---|---|
| VCC | 3.3V |
| GND | GND |
| X output | A0 |
| Y output | A1 |
| Z output | A3 |

Use the sensor's printed labels, not wire colours or connector order. These are five connections. Access all three labelled axis outputs using pads/headers if the Grove cable does not expose them all. Leave AREF unconnected. No I2C connections or level converter are needed for these analog outputs. This sketch is for the ADXL335, not the MPU6050 or digital ADXL345.

## Lab OLED shield

The supplied Workshop 4 lab specifies a pre-wired SSD1306 OLED, 128x64 pixels, I2C address 0x3C, reset -1, using SDA/SCL (A4/A5). The shield potentiometer is wired to A2. The sketch now uses X=A0, Y=A1, Z=A3, matching the lab wiring. Leave A2 for the knob; no accelerometer output should connect there. Move wires only with power disconnected.

Disconnect USB before fitting the shield. Align all header pins with the corresponding Arduino sockets without offsetting a row. Follow the lab's rail setup: shield 3.3V to the positive breadboard rail and shield GND to the negative rail. Connect accelerometer VCC/GND to those rails. A3 is identified in the lab as an unlabelled connection just above A2; verify against the actual shield/header before connecting. The shield OLED is already wired and needs no additional display jumpers.

While stopped, the OLED previews the raw axes at about 5 Hz and shows the test label/trial. At trial start it shows RECORDING. Display refresh pauses during recording so full-screen I2C transfers do not interrupt 50 Hz CSV acquisition. Send s to stop and resume preview. Labels are manually selected test categories, not rep classification; rep counting is not implemented yet. If the OLED is absent at 0x3C, the logger continues over serial.

Source: supplied 04 - Connecting Displays & Sensors R1.pdf, pages 6-10. The lab's later ultrasonic activity switches a rail to 5V; that activity is not part of this accelerometer setup.

## Upload and capture

1. Open RepBaselineLogger.ino in Arduino IDE. Install/select the Arduino UNO R4 Boards package and UNO R4 Minima, select its port, and upload. Install **Adafruit GFX Library** and **Adafruit SSD1306** in Library Manager and accept dependencies.
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

Columns: trial ID, user-selected label, sample index (starts at 0), milliseconds since trial began, three raw 12-bit ADC readings (0..4095), optional marker count. Sampling is nominally 50 Hz; use actual t_ms intervals to detect delays. Readings include gravity and are not calibrated acceleration, distance or joint angle. No baseline subtraction or software filtering is applied. A single accelerometer cannot in general establish joint range or all aspects of exercise form.

## Quick checks

- While still, readings should be fairly stable; slowly tilting the board should change one or more axes.
- If an axis is stuck near 0 or 4095, check wiring. Floating inputs can produce plausible-looking values, so also check that readings respond consistently to tilting.
- Keep supply, ADC resolution, mounting and exercise consistent. Remounting may require a new baseline.
- Resetting the Arduino resets trial IDs. Save separate capture files per session and identify them in your notes.

We can use the data to propose an axis/combined signal, filtering, separate departure/return thresholds, minimum motion duration and a return-to-start rule, then evaluate those rules on a separate labelled test capture. Thresholds are exercise/mounting specific. Detecting a repeated motion is not equivalent to validating clinical form.

References:
- https://wiki.seeedstudio.com/Grove-3-Axis_Analog_Accelerometer/
- https://docs.arduino.cc/tutorials/uno-r4-minima/adc-resolution

Validation: source reviewed for shield pin mapping, CSV consistency, and OLED refresh placement outside timed recording. Not compiled or hardware tested here; Arduino toolchain/hardware unavailable.

## Direct CSV capture on Windows

Close Arduino Serial Monitor and Plotter, leave the Arduino connected, then double-click Start-COM5-Logger.cmd. Wait for Ready. In that terminal press r/g/p/n to begin a labelled trial, s to stop, and q to finish. No Enter is needed. Files save automatically under captures/ and can be tracked in Git. The port defaults to COM5; to use another port run SaveReadings.ps1 with -PortName COM6 (or your port). The logger enables DTR for UNO R4 USB serial communication. If port access is denied, close other serial programs before retrying.

## R3 / R4 board switch

The sketch currently sets USE_R4_12_BIT to 0 for UNO R3 (10-bit, 0..1023). Change it to 1 for UNO R4 Minima (12-bit, 0..4095), and select the corresponding board in Arduino IDE. The R3 does not compile the R4-only analogReadResolution call when the switch is 0. Keep captures labelled by board and ADC resolution; existing R4 captures used 12-bit readings. Check the serial port after changing boards. Wiring remains X=A0, Y=A1, Z=A3, VCC=3.3V, GND=GND. R3 compilation and hardware operation have not been verified here.

### Choose the logger port

Double-click Start-COM5-Logger.cmd for COM5 or Start-COM6-Logger.cmd for COM6. Both use the same SaveReadings.ps1 and save to captures/. Use the launcher matching Tools > Port in Arduino IDE; the port is not tied to R3 or R4. Close Serial Monitor/Plotter before launching, and run only one logger per port. Commands are identical in both windows.

## Direct CSV capture on macOS

With Python 3 available, close Arduino Serial Monitor and Plotter, connect the board, and double-click **Start-Mac-Logger.command**. Wait for **Ready**. Press `r`, `g`, `p`, or `n` to start a trial, `s` to stop, and `q` to save and quit. No Enter is needed. Timestamped CSVs save automatically in `captures/`.

The launcher selects a single detected USB serial port. If multiple ports are listed, open Terminal in this folder and pass the device shown under **Tools → Port**. For example, run `./Start-Mac-Logger.command /dev/cu.usbmodem1101`.
