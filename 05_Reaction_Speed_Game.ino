/*
  Reaction Speed Game (2 players)
  Electronic Physics Projects 2026-2027 (Tally Grades)

  Press START. After a random wait (2 to 5 s) the green GO light turns on and
  the buzzer beeps. The first player to press their button wins, and the LCD
  shows the reaction time in milliseconds. Pressing before GO is a false start:
  the other player wins. The best time is kept.

  Parts: Arduino Uno, 3 push buttons, 3 LEDs (green, red, blue) + 3 resistors
         220 Ohm, buzzer, LCD 16x2 with I2C.
  Library: LiquidCrystal I2C (by Frank de Brabander)

  Pins:
    Player 1 button -> D2, Player 2 button -> D3, START button -> D4 (all to GND)
    Buzzer -> D8, GO LED (green) -> D9, P1 LED (red) -> D10, P2 LED (blue) -> D11
    LCD I2C -> SDA = A4, SCL = A5
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);   // if the screen stays empty, try 0x3F

const byte P1_BTN = 2, P2_BTN = 3, START_BTN = 4;
const byte BUZZER = 8, GO_LED = 9, P1_LED = 10, P2_LED = 11;

unsigned long bestTime = 0;           // 0 = no record yet
int wins1 = 0, wins2 = 0;

bool down(byte pin) { return digitalRead(pin) == LOW; }

void show(const char* a, const char* b) {
  lcd.clear(); lcd.setCursor(0, 0); lcd.print(a); lcd.setCursor(0, 1); lcd.print(b);
}

void lightsOff() { digitalWrite(GO_LED, LOW); digitalWrite(P1_LED, LOW); digitalWrite(P2_LED, LOW); }

void announce(int winner, unsigned long ms, bool falseStart) {
  if (winner == 1) wins1++; else wins2++;
  digitalWrite(winner == 1 ? P1_LED : P2_LED, HIGH);
  char a[17], b[17];
  if (falseStart) {
    snprintf(a, sizeof(a), "False start P%d!", winner == 1 ? 2 : 1);
    snprintf(b, sizeof(b), "P%d wins %d:%d", winner, wins1, wins2);
    tone(BUZZER, 200, 700);
  } else {
    if (bestTime == 0 || ms < bestTime) bestTime = ms;
    snprintf(a, sizeof(a), "P%d wins! %lu ms", winner, ms);
    snprintf(b, sizeof(b), "Best %lu  %d:%d", bestTime, wins1, wins2);
    tone(BUZZER, 1800, 150);
  }
  show(a, b);
  Serial.println(a);
}

void setup() {
  pinMode(P1_BTN, INPUT_PULLUP); pinMode(P2_BTN, INPUT_PULLUP); pinMode(START_BTN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT); pinMode(GO_LED, OUTPUT); pinMode(P1_LED, OUTPUT); pinMode(P2_LED, OUTPUT);
  Serial.begin(9600);
  randomSeed(analogRead(A0));          // A0 left empty: random noise
  lcd.init(); lcd.backlight();
  show("Reaction Game", "Press START");
}

void loop() {
  if (!down(START_BTN)) return;        // wait for START
  lightsOff();
  show("Get ready...", "Wait for GREEN");
  while (down(START_BTN)) {}           // wait until START is released

  unsigned long waitUntil = millis() + random(2000, 5000);
  while (millis() < waitUntil) {       // a press now is a false start
    if (down(P1_BTN)) { announce(2, 0, true); return; }
    if (down(P2_BTN)) { announce(1, 0, true); return; }
  }

  digitalWrite(GO_LED, HIGH);
  tone(BUZZER, 1000, 80);
  unsigned long goTime = millis();
  show("GO!!!", "");

  while (true) {
    bool p1 = down(P1_BTN), p2 = down(P2_BTN);
    if (p1 || p2) {
      unsigned long ms = millis() - goTime;
      announce(p1 ? 1 : 2, ms, false);   // if both in the same millisecond, P1 wins
      return;
    }
    if (millis() - goTime > 5000) { show("Too slow!", "Press START"); return; }
  }
}
