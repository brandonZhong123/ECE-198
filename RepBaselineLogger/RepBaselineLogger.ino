/* UNO R3 or R4 Minima + Grove ADXL335: X=A0, Y=A1, Z=A3.
   Power: sensor VCC=3.3V, GND=GND. See README.
   A2 is reserved for the shield knob; do not connect an axis there.
   Libraries: Adafruit GFX and Adafruit SSD1306 (install dependencies).
   Serial: 115200 baud.
   Send a single character (newline/CR ignored):
   r=rest, g=good/full reps, p=partial reps, n=non-rep movements.
   Each label command STARTS A NEW TRIAL and starts streaming.
   s=stop. h=CSV header. m=manual marker (optional).
   Labels describe your test, NOT an automatic classification.
   Raw ADC counts include gravity; not calibrated g or angles.
   R3 mode: 10-bit (0..1023). R4 mode: 12-bit (0..4095).
*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// BOARD SWITCH: leave 0 for UNO R3. Change to 1 for UNO R4 Minima.
// Also select the matching board in Arduino IDE before uploading.
#define USE_R4_12_BIT 1

Adafruit_SSD1306 display(128, 64, &Wire, -1);
bool oledReady = false;
uint32_t lastPreviewMs = 0;
const uint8_t X_PIN = A3, Y_PIN = A1, Z_PIN = A0;
const uint32_t SAMPLE_US = 20000UL; // nominal 50 Hz
bool recording = false;
char label = 'r';
uint32_t trial = 0, marker = 0, sequence = 0;
uint32_t trialStartMs = 0, nextSampleUs = 0;

// Refresh only while stopped, plus once before each trial starts.
// A full OLED transfer can exceed a 20 ms sample interval.
void drawScreen(bool active) {
  if (!oledReady) return;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("PBVR BASELINE LOGGER");
  display.print(active ? "RECORDING " : "STOPPED   ");
  display.print("trial "); display.println(trial);
  display.print("Label: "); display.println(label);
  if (active) {
    display.println("CSV -> Serial Monitor");
    display.println("Preview paused");
    display.println("Send s to stop");
  } else {
    display.print("X A0: "); display.println(analogRead(X_PIN));
    display.print("Y A1: "); display.println(analogRead(Y_PIN));
    display.print("Z A3: "); display.println(analogRead(Z_PIN));
    display.println("r rest g full p part");
    display.println("n other | s stop");
  }
  display.display();
}

void printHeader() {
  Serial.println("trial,label,sample,t_ms,x_raw,y_raw,z_raw,marker");
}

void setup() {
  Serial.begin(115200);
#if USE_R4_12_BIT
  analogReadResolution(12); // R4 only. Excluded from compilation in R3 mode.
#endif
  // With the switch OFF (0), both boards use default 10-bit readings.
  pinMode(X_PIN, INPUT);
  pinMode(Y_PIN, INPUT);
  pinMode(Z_PIN, INPUT);
  Wire.begin();
  Wire.beginTransmission(0x3C);
  if (Wire.endTransmission() == 0) {
    oledReady = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  }
  drawScreen(false); // Serial logging still works if no display is found.
  // No wait for Serial: send h after opening Serial Monitor for a header.
}

void loop() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\r' || c == '\n' || c == ' ') continue;
    if (c == 'h') printHeader();
    else if (c == 's') {
      recording = false;
      drawScreen(false);
    }
    else if (c == 'm' && recording) ++marker;
    else if (c == 'r' || c == 'g' || c == 'p' || c == 'n') {
      label = c;
      ++trial;
      marker = 0;
      sequence = 0;
      drawScreen(true); // Complete transfer BEFORE starting the sample clock.
      trialStartMs = millis();
      nextSampleUs = micros();
      recording = true;
    }
  }
  if (!recording) {
    if (millis() - lastPreviewMs >= 200UL) {
      lastPreviewMs = millis();
      drawScreen(false);
    }
    return;
  }
  uint32_t nowUs = micros();
  if ((int32_t)(nowUs - nextSampleUs) < 0) return;
  nextSampleUs += SAMPLE_US;
  // If delayed, skip overdue slots rather than burst fake catch-up samples.
  if ((int32_t)(nowUs - nextSampleUs) >= 0) nextSampleUs = nowUs + SAMPLE_US;
  uint32_t tMs = millis() - trialStartMs;
  int x = analogRead(X_PIN);
  int y = analogRead(Y_PIN);
  int z = analogRead(Z_PIN);
  Serial.print(trial); Serial.print(',');
  Serial.print(label); Serial.print(',');
  Serial.print(sequence++); Serial.print(',');
  Serial.print(tMs); Serial.print(',');
  Serial.print(x); Serial.print(',');
  Serial.print(y); Serial.print(',');
  Serial.print(z); Serial.print(',');
  Serial.println(marker);
}
