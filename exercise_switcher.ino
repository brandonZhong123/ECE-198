#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64      // change to 32 if you have a 128x32 display
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- Joystick pins ----------
const int PIN_Y  = A1;
const int PIN_SW = 2;

// ---------- Your list ----------
const char* exercises[] = {"Leg Raises","Bicep Curls","Lat Raises",};
const int EXERCISE_COUNT = sizeof(exercises) / sizeof(exercises[0]);

// ---------- Joystick tuning ----------
const int LOW_THRESH  = 300;   // center reads about 512 (0-1023 range)
const int HIGH_THRESH = 700;
const bool INVERT_Y   = false; // set true if up/down feel reversed

// ---------- State ----------
int  currentIndex  = 0;
bool locked        = false;
bool stickCentered = true;

// Button debounce
const unsigned long DEBOUNCE_MS = 30;
int lastReading = HIGH;
int buttonState = HIGH;
unsigned long lastChange = 0;

void drawScreen() {
  display.clearDisplay();

  // Header: position in the list
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Exericse ");
  display.print(currentIndex + 1);
  display.print("/");
  display.print(EXERCISE_COUNT);

  // Selected string
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print(exercises[currentIndex]);

  // Footer: lock status
  if (locked) {
    display.fillRect(0, 52, SCREEN_WIDTH, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setTextSize(1);
    display.setCursor(2, 54);
    display.print("LOCKED - press to unlock");
  } else {
    display.setTextSize(1);
    display.setCursor(0, 54);
    display.print("Up/Down to switch exercises");
  }

  display.display();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_SW, INPUT_PULLUP);   // KY-023 button pulls to GND when pressed

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found at 0x3C. Check wiring.");
    while (true) {}
  }
  drawScreen();
}

void loop() {
  // ----- Button: toggle lock on each press -----
  int reading = digitalRead(PIN_SW);
  if (reading != lastReading) {
    lastChange = millis();
    lastReading = reading;
  }
  if (millis() - lastChange > DEBOUNCE_MS && reading != buttonState) {
    buttonState = reading;
    if (buttonState == LOW) {      // just pressed
      locked = !locked;
      Serial.println(locked ? "Locked" : "Unlocked");
      drawScreen();
    }
  }

  // ----- Joystick: one step per push, wraps around -----
  int y = analogRead(PIN_Y);
  if (INVERT_Y) y = 1023 - y;

  if (!locked) {
    if (stickCentered) {
      if (y > HIGH_THRESH) {                      // down
        currentIndex = (currentIndex + 1) % EXERCISE_COUNT;
        stickCentered = false;
        drawScreen();
      } else if (y < LOW_THRESH) {                // up
        currentIndex = (currentIndex - 1 + EXERCISE_COUNT) % EXERCISE_COUNT;
        stickCentered = false;
        drawScreen();
      }
    }
  }

  // Stick must return near center before the next step
  if (y > LOW_THRESH + 50 && y < HIGH_THRESH - 50) {
    stickCentered = true;
  }

  delay(5);
}
