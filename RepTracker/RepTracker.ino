/* PBVR seated knee-extension demo, UNO R3/R4 + ADXL335 + lab OLED.
   X=A3, Y=A2, Z=A1; VCC=3.3V; GND=GND.
   Y is NOT read: A2 is occupied by the shield knob unless isolated.
   OLED SSD1306 128x64, address 0x3C (prewired shield).
   Libraries: Adafruit GFX + Adafruit SSD1306, with dependencies.
   Serial Monitor: 115200. Do NOT run the old CSV logger with this sketch.
   c = calibrate rest (3 seconds); t = teach THREE full reps;
   s = stop; g = resume counting after teaching; r = reset counts.
   Startup automatically calibrates rest. Stay in the starting pose.
   Teaching/thresholds are session-only; power/reset requires reteaching.
*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RepDetector.h"

// BOARD SWITCH: 0 = UNO R3 (default 10-bit); 1 = UNO R4 (12-bit).
#define USE_R4_12_BIT 0
const uint8_t X_PIN = A3, Z_PIN = A1;
const float ADC_SCALE = USE_R4_12_BIT ? 0.25f : 1.0f;
// 20 Hz leaves time for a ~25ms full OLED transfer at 400kHz.
const uint32_t SAMPLE_MS = 50;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
RepDetector detector;
enum Mode { REST, READY, TEACH, COUNT, PAUSED, BAD_REST };
Mode mode = REST;
bool oledReady = false, learned = false;
uint32_t lastSample = 0, lastDraw = 0, restStart = 0;
float sx = 0, sz = 0, minX, maxX, minZ, maxZ;
unsigned samples = 0, taught = 0;
float peaks[3];
const __FlashStringHelper *feedback = nullptr; // Set by startRest() in setup().

void startRest() {
  mode = REST; learned = false; taught = 0;
  restStart = millis(); samples = 0; sx = sz = 0;
  minX = minZ = 2000; maxX = maxZ = -1;
  feedback = F("Hold still 3 seconds");
}
void draw() {
  if (!oledReady) return;
  display.clearDisplay(); display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1); display.setCursor(0,0);
  display.println(F("PBVR LEG REP DEMO"));
  display.print(F("Full: ")); display.print(detector.full);
  display.print(F(" Part: ")); display.println(detector.partial);
  display.println(feedback);
  display.print(F("Motion: ")); display.println(detector.distance,1);
  display.print(F("Full limit: ")); display.println(detector.fullThreshold,1);
  if (mode == TEACH) { display.print(F("Teach reps: ")); display.print(taught); display.println(F("/3")); }
  else display.println(F("c rest | t teach"));
  display.println(F("s stop | g go"));
  display.println(F("r reset counts"));
  display.display();
}
void setup() {
  Serial.begin(115200);
#if USE_R4_12_BIT
  analogReadResolution(12);
#endif
  Wire.begin(); Wire.beginTransmission(0x3C);
  if (Wire.endTransmission() == 0)
    oledReady = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  startRest();
}
void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'c') startRest();
    else if (c == 's' && mode != REST && mode != BAD_REST) {
      mode = PAUSED; detector.active = detector.armed = detector.homePending = false;
      feedback = F("Stopped");
    } else if (c == 't' && mode != REST && mode != BAD_REST) {
      detector.reset(detector.bx, detector.bz); taught = 0; learned = false;
      mode = TEACH; feedback = F("Do 3 full reps");
    } else if (c == 'g' && learned && mode != REST && mode != BAD_REST) {
      detector.active = detector.armed = detector.homePending = false;
      mode = COUNT; feedback = F("Return to start");
    } else if (c == 'r' && (mode == COUNT || mode == PAUSED)) {
      detector.reset(detector.bx, detector.bz); feedback = F("Counts reset");
    } else if (c == 'h') {
      Serial.println(F("c=rest calibration, t=3 teaching reps, g=go, s=stop, r=reset"));
    }
  }
  uint32_t now = millis();
  if (now-lastSample >= SAMPLE_MS) {
    lastSample = now;
    float x = analogRead(X_PIN)*ADC_SCALE;
    float z = analogRead(Z_PIN)*ADC_SCALE;
    if (mode == REST) {
      sx += x; sz += z; ++samples;
      if (x < minX) minX=x; if (x > maxX) maxX=x;
      if (z < minZ) minZ=z; if (z > maxZ) maxZ=z;
      if (now-restStart >= 3000) {
        detector.reset(sx/samples, sz/samples);
        if (maxX-minX > 8 || maxZ-minZ > 8) {
          mode = BAD_REST; feedback = F("Moved! c to retry");
        } else { mode = READY; feedback = F("Send t: teach 3 reps"); }
      }
    } else if (mode == TEACH || mode == COUNT) {
      int event = detector.update(x,z,now);
      if (event) {
        if (mode == TEACH && event != 3) {
          peaks[taught++] = detector.peak;
          detector.full = detector.partial = 0; // teaching reps not counted
          if (taught == 3) {
            float lo=peaks[0], hi=peaks[0], sum=0;
            for (uint8_t i=0;i<3;++i) { sum+=peaks[i]; if(peaks[i]<lo)lo=peaks[i]; if(peaks[i]>hi)hi=peaks[i]; }
            // Reject weak or inconsistent teaching instead of claiming accuracy.
            if (lo < 10 || hi > lo*1.6f) {
              mode = READY; taught=0; feedback = F("Weak/uneven: retry t");
            } else {
              detector.fullThreshold = 0.70f*(sum-lo-hi); // 70% median excursion
              learned = true; mode = COUNT; feedback = F("Ready: count reps");
            }
          }
        } else if (event == 1) feedback = F("Full rep +1");
        else if (event == 2) feedback = F("Partial rep +1");
        else feedback = F("Rejected: return home");
        Serial.print(F("event,")); Serial.print(now); Serial.print(',');
        Serial.print(event); Serial.print(','); Serial.print(detector.peak,2);
        Serial.print(','); Serial.print(detector.full); Serial.print(','); Serial.println(detector.partial);
      }
    }
  }
  if (now-lastDraw >= 200) { lastDraw=now; draw(); }
}
