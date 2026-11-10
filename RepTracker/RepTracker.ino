/* Simple OLED rep counter based on confirmed 8-rep recording.
 ADXL335: X=A0,Y=A1,Z=A3,VCC=3.3V,GND=GND. Only Y/A1 is read.
 A2 is reserved for shield knob. SSD1306 128x64 OLED: I2C 0x3C.
 Libraries: Adafruit GFX + Adafruit SSD1306 (accept dependencies).
 No trials/teaching. Start in the leg-down pose, then exercise.
 Reset Arduino to clear count. Optional Serial 'r' also clears it.
 Counts movement cycles, not validated exercise quality.
*/
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "YRepCounter.h"

// BOARD SWITCH: 1=UNO R4 Minima (current), 0=UNO R3.
// Select corresponding board in Arduino IDE as well.
#define USE_R4_12_BIT 1
const uint8_t Y_PIN=A1;
const float ADC_SCALE=USE_R4_12_BIT ? 0.25f : 1.0f;
const uint32_t SAMPLE_MS=50; // 20Hz leaves time for OLED's blocking transfer.
Adafruit_SSD1306 display(128,64,&Wire,-1);
YRepCounter counter;
bool oledReady=false;
uint32_t lastSample=0,lastDraw=0;
void drawCount(){
 if(!oledReady)return;
 display.clearDisplay();display.setTextColor(SSD1306_WHITE);
 display.setTextSize(2);display.setCursor(35,0);display.print(F("REPS"));
 display.setTextSize(3);display.setCursor(0,28);display.print(counter.reps);
 display.display();
}
void setup(){
 Serial.begin(115200);
#if USE_R4_12_BIT
 analogReadResolution(12);
#endif
 pinMode(Y_PIN,INPUT);
 Wire.begin();Wire.beginTransmission(0x3C);
 if(Wire.endTransmission()==0)
  oledReady=display.begin(SSD1306_SWITCHCAPVCC,0x3C);
 drawCount();
}
void loop(){
 while(Serial.available())if(Serial.read()=='r')counter.reset();
 uint32_t now=millis();
 if(now-lastSample>=SAMPLE_MS){
  lastSample=now;
  if(counter.update(analogRead(Y_PIN)*ADC_SCALE,now)){
   Serial.print(F("Reps: "));Serial.println(counter.reps);
  }
 }
 if(now-lastDraw>=200){lastDraw=now;drawCount();}
}
