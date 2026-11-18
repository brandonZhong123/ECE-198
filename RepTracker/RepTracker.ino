#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "YRepCounter.h"

// 1 = UNO R4 Minima, 0 = UNO R3.
#define USE_R4_12_BIT 1

// ADXL335 X and Z are wired for future use. The detector samples Y.
constexpr uint8_t ACCEL_X_PIN = A3;
constexpr uint8_t ACCEL_Y_PIN = A2;
constexpr uint8_t ACCEL_Z_PIN = A0;
constexpr uint8_t JOYSTICK_Y_PIN = A1;
constexpr uint8_t JOYSTICK_BUTTON_PIN = 2;
constexpr uint8_t OLED_ADDRESS = 0x3C;

constexpr uint8_t ADC_SHIFT = USE_R4_12_BIT ? 2 : 0;
constexpr float ADC_SCALE = USE_R4_12_BIT ? 0.25f : 1.0f;
constexpr uint32_t SAMPLE_MS = 50;
constexpr uint32_t DEBOUNCE_MS = 30;
constexpr bool INVERT_JOYSTICK_Y = false;

enum class Screen { Exercises, Ready, Tracking };

Adafruit_SSD1306 display(128, 64, &Wire, -1);
YRepCounter counter;
const char* const exercises[] = {"Leg Raises", "Bicep Curls", "Lat Raises"};
constexpr int EXERCISE_COUNT = sizeof(exercises) / sizeof(exercises[0]);

Screen screen = Screen::Exercises;
int exerciseIndex = 0;
bool backSelected = false;
bool oledReady = false;
bool screenDirty = true;
bool stickCentered = false;
int lastButtonReading = HIGH;
int buttonState = HIGH;
uint32_t buttonChangedAt = 0;
uint32_t lastSample = 0;

void drawScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  if (screen == Screen::Exercises) {
    display.print(F("Choose exercise"));
    for (int i = 0; i < EXERCISE_COUNT; ++i) {
      display.setCursor(0, 14 + 12 * i);
      display.print(i == exerciseIndex ? "> " : "  ");
      display.print(exercises[i]);
    }
    display.setCursor(0, 55);
    display.print(F("Press to select"));
  } else {
    display.print(exercises[exerciseIndex]);
    display.setCursor(0, 13);
    display.print(F("Reps"));
    display.setTextSize(2);
    display.setCursor(0, 24);
    display.print(counter.reps);
    display.setTextSize(1);
    display.setCursor(0, 45);
    if (screen == Screen::Tracking) {
      display.print(F("> Stop tracking"));
    } else {
      display.print(backSelected ? "  Start tracking" : "> Start tracking");
      display.setCursor(0, 55);
      display.print(backSelected ? "> Back" : "  Back");
    }
  }

  display.display();
}

int readJoystickStep() {
  int y = analogRead(JOYSTICK_Y_PIN) >> ADC_SHIFT;
  if (INVERT_JOYSTICK_Y) y = 1023 - y;

  if (y > 350 && y < 650) {
    stickCentered = true;
  } else if (stickCentered && (y < 300 || y > 700)) {
    stickCentered = false;
    return y > 700 ? 1 : -1;
  }
  return 0;
}

bool buttonPressed(uint32_t now) {
  const int reading = digitalRead(JOYSTICK_BUTTON_PIN);
  if (reading != lastButtonReading) {
    lastButtonReading = reading;
    buttonChangedAt = now;
  }
  if (reading == buttonState || now - buttonChangedAt < DEBOUNCE_MS) {
    return false;
  }
  buttonState = reading;
  return buttonState == LOW;
}

void selectMenuItem(uint32_t now) {
  switch (screen) {
    case Screen::Exercises:
      counter.reset();
      backSelected = false;
      screen = Screen::Ready;
      break;
    case Screen::Ready:
      if (backSelected) {
        screen = Screen::Exercises;
      } else {
        counter.reset();
        lastSample = now;
        screen = Screen::Tracking;
      }
      break;
    case Screen::Tracking:
      backSelected = false;
      screen = Screen::Ready;
      break;
  }
  screenDirty = true;
}

void setup() {
  Serial.begin(115200);
#if USE_R4_12_BIT
  analogReadResolution(12);
#endif
  pinMode(ACCEL_X_PIN, INPUT);
  pinMode(ACCEL_Y_PIN, INPUT);
  pinMode(ACCEL_Z_PIN, INPUT);
  pinMode(JOYSTICK_Y_PIN, INPUT);
  pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);

  Wire.begin();
  Wire.beginTransmission(OLED_ADDRESS);
  if (Wire.endTransmission() == 0) {
    oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  }
  if (!oledReady) {
    Serial.println(F("OLED unavailable at 0x3C. Check wiring."));
  }
}

void loop() {
  const uint32_t now = millis();
  const int step = readJoystickStep();
  if (buttonPressed(now)) {
    selectMenuItem(now);
  } else if (step != 0) {
    if (screen == Screen::Exercises) {
      exerciseIndex = (exerciseIndex + step + EXERCISE_COUNT) % EXERCISE_COUNT;
      screenDirty = true;
    } else if (screen == Screen::Ready) {
      backSelected = !backSelected;
      screenDirty = true;
    }
  }

  while (Serial.available()) {
    if (Serial.read() == 'r') {
      counter.reset();
      screenDirty = true;
    }
  }

  if (screen == Screen::Tracking && now - lastSample >= SAMPLE_MS) {
    lastSample = now;
    if (counter.update(analogRead(ACCEL_Y_PIN) * ADC_SCALE, now)) {
      Serial.print(F("Rep #: "));
      Serial.println(counter.reps);
      screenDirty = true;
    }
  }

  if (oledReady && screenDirty) {
    drawScreen();
    screenDirty = false;
  }
}
