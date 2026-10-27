/*
  Anti-Theft Flooring System
  Electronic Physics Projects 2026-2027 (Tally Grades)

  4 floor tiles, each with a piezo disc under it. When the system is ARMED and
  someone steps on a tile, the alarm goes off and the LCD shows which tile.

  Parts: Arduino Uno, 4 piezo discs + 4 resistors 1 MOhm, push button,
         buzzer, red LED + green LED + 2 resistors 220 Ohm, LCD 16x2 with I2C.
  Library: LiquidCrystal I2C (by Frank de Brabander)

  Pins:
    Piezo tiles 1..4 -> A0, A1, A2, A3 (each piezo has a 1 MOhm resistor to GND)
    Arm/Disarm button -> D2 (other leg to GND)
    Buzzer -> D8   Red LED -> D9   Green LED -> D10
    LCD I2C -> SDA = A4, SCL = A5, VCC = 5V, GND = GND
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);   // if the screen stays empty, try 0x3F

const byte TILE_PINS[4] = {A0, A1, A2, A3};
const byte BUTTON = 2, BUZZER = 8, RED_LED = 9, GREEN_LED = 10;
const int  STEP_THRESHOLD = 100;      // raise it if the alarm goes off by itself
const unsigned long EXIT_DELAY = 10000; // 10 s to leave the room after arming

enum State { DISARMED, ARMING, ARMED, ALARM };
State state = DISARMED;
unsigned long stateStart = 0;
int alarmTile = 0;

bool buttonPressed() {
  static bool last = HIGH;
  bool now = digitalRead(BUTTON);
  bool pressed = (last == HIGH && now == LOW);
  last = now;
  if (pressed) delay(30);               // simple debounce
  return pressed;
}

void show(const char* line1, const char* line2) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(line1);
  lcd.setCursor(0, 1); lcd.print(line2);
}

void setState(State s) {
  state = s;
  stateStart = millis();
  noTone(BUZZER);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, s == ARMED || s == ARMING);
  if (s == DISARMED) show("System OFF", "Press to arm");
  if (s == ARMING)   show("Arming...", "Leave the room");
  if (s == ARMED)    show("System ARMED", "Floor protected");
}

int steppedTile() {                     // returns 1..4, or 0 if nobody stepped
  for (int i = 0; i < 4; i++) {
    if (analogRead(TILE_PINS[i]) > STEP_THRESHOLD) return i + 1;
  }
  return 0;
}

void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  setState(DISARMED);
}

void loop() {
  bool pressed = buttonPressed();

  switch (state) {
    case DISARMED:
      if (pressed) setState(ARMING);
      break;

    case ARMING: {
      unsigned long left = (EXIT_DELAY - (millis() - stateStart)) / 1000;
      lcd.setCursor(15, 0); lcd.print(left % 10);
      digitalWrite(GREEN_LED, (millis() / 250) % 2);   // blink while arming
      if (pressed) setState(DISARMED);
      else if (millis() - stateStart >= EXIT_DELAY) setState(ARMED);
      break;
    }

    case ARMED: {
      if (pressed) { setState(DISARMED); break; }
      int tile = steppedTile();
      if (tile > 0) {
        alarmTile = tile;
        state = ALARM; stateStart = millis();
        char line2[17];
        snprintf(line2, sizeof(line2), "Tile %d stepped!", tile);
        show("!! INTRUDER !!", line2);
        Serial.print("Alarm on tile "); Serial.println(tile);
      }
      break;
    }

    case ALARM: {
      bool high = (millis() / 300) % 2;               // siren: two tones
      tone(BUZZER, high ? 2000 : 1200);
      digitalWrite(RED_LED, high);
      if (pressed) setState(DISARMED);                 // the owner turns it off
      break;
    }
  }
}
