/*
  Automatic Water Tank Level Controller
  Electronic Physics Projects 2026-2027 (Tally Grades)

  An ultrasonic sensor at the top of the tank measures the distance to the
  water. The Arduino turns the pump ON when the tank is below 25 % and OFF at
  90 %. LEDs and the LCD show the level. If the pump runs too long without the
  level rising (dry pump or broken sensor), it stops and the buzzer sounds.
  A button turns the pump on/off by hand (manual mode).

  Parts: Arduino Uno, HC-SR04, relay module 5 V (1 channel), small water pump
         5 V (or 12 V with its own adapter), LCD 16x2 I2C, 3 LEDs (red, yellow,
         green) + 3 resistors 220 Ohm, buzzer, push button, a plastic box as tank.
  Library: LiquidCrystal I2C (by Frank de Brabander)

  Pins:
    HC-SR04 TRIG -> D10, ECHO -> D11
    Relay IN -> D7 (pump through the relay COM/NO contacts)
    Manual button -> D2 (to GND)
    LEDs: low (red) -> D3, half (yellow) -> D4, full (green) -> D5
    Buzzer -> D8, LCD I2C -> SDA = A4, SCL = A5
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);   // if the screen stays empty, try 0x3F

const byte TRIG = 10, ECHO = 11, RELAY = 7, BUTTON = 2;
const byte LED_LOW = 3, LED_HALF = 4, LED_FULL = 5, BUZZER = 8;

// Measure your tank with a ruler and write the numbers here (cm from the sensor):
const float EMPTY_CM = 20.0;          // distance to the bottom (empty tank)
const float FULL_CM  = 4.0;           // distance to the water when full (keep >= 3 cm)
const int ON_BELOW = 25, OFF_ABOVE = 90;           // percent
const unsigned long MAX_PUMP_MS = 60000UL;          // stop if 60 s pass with no rise
const bool RELAY_ACTIVE_LOW = true;   // most relay modules turn ON with LOW

bool pumpOn = false, manual = false, fault = false;
unsigned long pumpStart = 0;
int levelAtStart = 0;

float distanceCm() {
  float sum = 0; int good = 0;
  for (int i = 0; i < 5; i++) {                     // 5 readings, ignore failed ones
    digitalWrite(TRIG, LOW);  delayMicroseconds(2);
    digitalWrite(TRIG, HIGH); delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    unsigned long us = pulseIn(ECHO, HIGH, 25000UL);
    if (us > 0) { sum += us * 0.0343 / 2; good++; }
    delay(20);
  }
  return good ? sum / good : -1;                    // -1 = sensor error
}

void setPump(bool on) {
  if (on && !pumpOn) { pumpStart = millis(); }
  pumpOn = on;
  digitalWrite(RELAY, (on ^ RELAY_ACTIVE_LOW) ? HIGH : LOW);
}

void setup() {
  pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);
  pinMode(RELAY, OUTPUT); pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LED_LOW, OUTPUT); pinMode(LED_HALF, OUTPUT); pinMode(LED_FULL, OUTPUT); pinMode(BUZZER, OUTPUT);
  setPump(false);
  Serial.begin(9600);
  lcd.init(); lcd.backlight();
}

void loop() {
  float d = distanceCm();
  int level = (d < 0) ? -1 : constrain((int)((EMPTY_CM - d) / (EMPTY_CM - FULL_CM) * 100), 0, 100);

  if (digitalRead(BUTTON) == LOW) {                  // manual on/off, also clears a fault
    manual = !pumpOn; fault = false;
    setPump(manual);
    if (pumpOn) levelAtStart = level;
    while (digitalRead(BUTTON) == LOW) {}
  }

  if (!fault) {
    if (level < 0) { fault = true; }                                  // sensor error
    else if (!manual && !pumpOn && level < ON_BELOW) { setPump(true); levelAtStart = level; }
    else if (pumpOn && level >= OFF_ABOVE) { setPump(false); manual = false; }   // full: always stop
    if (pumpOn && millis() - pumpStart > MAX_PUMP_MS && level <= levelAtStart + 2) fault = true;
  }
  if (fault) { setPump(false); manual = false; }

  digitalWrite(LED_LOW,  level >= 0 && level < 30);
  digitalWrite(LED_HALF, level >= 30 && level < 80);
  digitalWrite(LED_FULL, level >= 80);
  if (fault && (millis() / 400) % 2) tone(BUZZER, 1000); else noTone(BUZZER);

  lcd.clear();
  lcd.setCursor(0, 0);
  if (level < 0) lcd.print("Level: --");
  else { lcd.print("Level: "); lcd.print(level); lcd.print("%"); }
  lcd.setCursor(0, 1);
  if (fault)        lcd.print("FAULT! press btn");
  else if (pumpOn)  lcd.print(manual ? "Pump ON (manual)" : "Pump ON (auto)");
  else              lcd.print("Pump OFF");

  Serial.print("Distance: "); Serial.print(d, 1); Serial.print(" cm  Level: ");
  Serial.print(level); Serial.print(" %  Pump: "); Serial.println(pumpOn ? "ON" : "OFF");
  delay(300);
}
