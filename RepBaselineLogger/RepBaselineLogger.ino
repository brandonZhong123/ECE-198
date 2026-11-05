/* UNO R4 Minima + ADXL335 analog outputs: X=A0, Y=A1, Z=A2.
   Serial: 115200 baud. No external libraries.
   Send a single character (newline/CR ignored):
   r=rest, g=good/full reps, p=partial reps, n=non-rep movements.
   Each label command STARTS A NEW TRIAL and starts streaming.
   s=stop. h=CSV header. m=manual marker (optional).
   Labels describe your test, NOT an automatic classification.
   Raw ADC counts retain gravity; they are not calibrated g or angles.
*/
#include <Arduino.h>

const uint8_t X_PIN = A0, Y_PIN = A1, Z_PIN = A2;
const uint32_t SAMPLE_US = 20000UL; // nominal 50 Hz
bool recording = false;
char label = 'r';
uint32_t trial = 0, marker = 0, sequence = 0;
uint32_t trialStartMs = 0, nextSampleUs = 0;

void printHeader() {
  Serial.println("trial,label,sample,t_ms,x_raw,y_raw,z_raw,marker");
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12); // R4: 0..4095
  pinMode(X_PIN, INPUT);
  pinMode(Y_PIN, INPUT);
  pinMode(Z_PIN, INPUT);
  // No wait for Serial: send h after opening Serial Monitor for a header.
}

void loop() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == 'h') printHeader();
    else if (c == 's') recording = false;
    else if (c == 'm' && recording) ++marker;
    else if (c == 'r' || c == 'g' || c == 'p' || c == 'n') {
      label = c;
      ++trial;
      marker = 0;
      sequence = 0;
      trialStartMs = millis();
      nextSampleUs = micros();
      recording = true;
    }
  }
  if (!recording) return;
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
