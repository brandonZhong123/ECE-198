#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdio.h>
#include <string.h>
#include "RepCounter.h"

// 1 = UNO R4 Minima, 0 = UNO R3.
#define USE_R4_12_BIT 1

// ADXL335 X is wired for future use.
constexpr uint8_t ACCEL_X_PIN = A0;
constexpr uint8_t ACCEL_Y_PIN = A2;
constexpr uint8_t ACCEL_Z_PIN = A3;
constexpr uint8_t JOYSTICK_Y_PIN = A1;
constexpr uint8_t JOYSTICK_BUTTON_PIN = 2;
constexpr uint8_t OLED_ADDRESS = 0x3C;
constexpr uint8_t NO_REFERENCE_PIN = UINT8_MAX;

constexpr uint8_t ADC_SHIFT = USE_R4_12_BIT ? 2 : 0;
constexpr float ADC_SCALE = USE_R4_12_BIT ? 0.25f : 1.0f;
constexpr uint32_t SAMPLE_MS = 50;
constexpr uint32_t DEBOUNCE_MS = 30;
constexpr bool INVERT_JOYSTICK_Y = false;

enum class Screen { Exercises, Ready, Tracking };

struct Exercise {
  const char* name;
  uint8_t pin;
  uint8_t referencePin;
  RepProfile profile;
};

Adafruit_SSD1306 display(128, 64, &Wire, -1);
RepCounter counter;
constexpr Exercise exercises[] = {
    {"Leg Raises", ACCEL_Y_PIN, NO_REFERENCE_PIN, LEG_RAISE_PROFILE},
    {"Bicep Curls", ACCEL_Z_PIN, NO_REFERENCE_PIN, BICEP_CURL_PROFILE},
    {"Lat Raises", ACCEL_Z_PIN, ACCEL_Y_PIN, LAT_RAISE_PROFILE},
};
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

void drawExerciseMenu() {
  display.setCursor(3, 3);
  display.print(F("Choose exercise"));
  display.drawFastHLine(0, 13, 128, SSD1306_WHITE);

  for (int i = 0; i < EXERCISE_COUNT; ++i) {
    const int y = 16 + 12 * i;
    const bool selected = i == exerciseIndex;
    if (selected) {
      display.fillRoundRect(1, y, 126, 11, 3, SSD1306_WHITE);
      display.fillTriangle(6, y + 2, 6, y + 8, 10, y + 5, SSD1306_BLACK);
    }
    display.setTextColor(selected ? SSD1306_BLACK : SSD1306_WHITE);
    display.setCursor(15, y + 2);
    display.print(exercises[i].name);
  }
}

void drawWorkoutAction(int y, const char* label, bool selected) {
  if (selected) {
    display.fillRoundRect(83, y, 44, 13, 3, SSD1306_WHITE);
  } else {
    display.drawRoundRect(83, y, 44, 13, 3, SSD1306_WHITE);
  }
  display.setTextColor(selected ? SSD1306_BLACK : SSD1306_WHITE);
  display.setCursor(105 - 3 * strlen(label), y + 3);
  display.print(label);
}

void drawWorkout(bool tracking) {
  display.setCursor(3, 3);
  display.print(exercises[exerciseIndex].name);
  if (tracking) {
    display.fillRoundRect(92, 0, 36, 12, 3, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(98, 3);
    display.print(F("LIVE"));
    display.setTextColor(SSD1306_WHITE);
  } else {
    display.drawRoundRect(92, 0, 36, 12, 3, SSD1306_WHITE);
    display.setCursor(95, 3);
    display.print(F("READY"));
  }
  display.drawFastHLine(0, 13, 128, SSD1306_WHITE);
  display.drawFastVLine(78, 18, 32, SSD1306_WHITE);

  char count[11]; // Up to ten digits for a uint32_t, plus the terminator.
  snprintf(count, sizeof(count), "%lu", static_cast<unsigned long>(counter.reps));
  const int digitWidth = 6 * strlen(count); // Default font at size 1.
  int textSize = 4;
  while (digitWidth * textSize > 74) {
    --textSize;
  }
  display.setTextSize(textSize);
  display.setCursor((78 - digitWidth * textSize) / 2, 31 - 4 * textSize);
  display.print(count);
  display.setTextSize(1);
  display.setCursor(27, 45);
  display.print(F("REPS"));

  if (tracking) {
    drawWorkoutAction(26, "Stop", true);
  } else {
    drawWorkoutAction(18, "Start", !backSelected);
    drawWorkoutAction(35, "Back", backSelected);
  }
}

void drawScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  const bool tracking = screen == Screen::Tracking;
  if (screen == Screen::Exercises) {
    drawExerciseMenu();
  } else {
    drawWorkout(tracking);
  }

  display.drawFastHLine(0, 53, 128, SSD1306_WHITE);
  display.setTextColor(SSD1306_WHITE);
  if (tracking) {
    display.setCursor(25, 56);
    display.print(F("Press to stop"));
  } else {
    display.setCursor(3, 56);
    display.print(F("Tilt: move"));
    display.setCursor(72, 56);
    display.print(F("Press: OK"));
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
    const Exercise& exercise = exercises[exerciseIndex];
    int sample = analogRead(exercise.pin);
    if (exercise.referencePin != NO_REFERENCE_PIN) {
      sample -= analogRead(exercise.referencePin);
    }
    if (counter.update(sample * ADC_SCALE, now, exercise.profile)) {
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
