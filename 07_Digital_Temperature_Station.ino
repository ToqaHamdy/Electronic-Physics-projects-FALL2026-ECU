/*
  Digital Temperature Monitoring Station
  Electronic Physics Projects 2026-2027 (Tally Grades)

  Reads temperature and humidity every 2 s, shows them on the LCD with the
  lowest and highest temperature since start, and sounds an alarm when the
  temperature passes a limit. Two buttons raise and lower the limit.

  Parts: Arduino Uno, DHT22 sensor (or DHT11) + 10 kOhm resistor, LCD 16x2 I2C,
         2 push buttons, green LED + red LED + 2 resistors 220 Ohm, buzzer.
  Libraries: DHT sensor library (by Adafruit) and LiquidCrystal I2C

  Pins:
    DHT22 data -> D2 (10 kOhm from data to 5V if your sensor has no board)
    Limit UP button -> D3, Limit DOWN button -> D4 (other legs to GND)
    Buzzer -> D8, Green LED -> D9, Red LED -> D10
    LCD I2C -> SDA = A4, SCL = A5
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);   // if the screen stays empty, try 0x3F
DHT dht(2, DHT22);                    // DHT11 users: write DHT11 here

const byte UP_BTN = 3, DOWN_BTN = 4, BUZZER = 8, GREEN_LED = 9, RED_LED = 10;
float limitC = 35.0;                  // alarm above this temperature
float minC = 1000, maxC = -1000;
float tempC = NAN, humidity = NAN;
unsigned long lastRead = 0, lastButton = 0;
bool showMinMax = false;

void showScreen() {
  lcd.clear();
  if (isnan(tempC)) { lcd.print("Sensor error!"); lcd.setCursor(0, 1); lcd.print("Check DHT wires"); return; }
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(tempC, 1); lcd.print((char)223); lcd.print("C ");
  lcd.print("H:"); lcd.print(humidity, 0); lcd.print("%");
  lcd.setCursor(0, 1);
  if (showMinMax) { lcd.print("Lo"); lcd.print(minC, 1); lcd.print(" Hi"); lcd.print(maxC, 1); }
  else            { lcd.print("Limit: "); lcd.print(limitC, 1); lcd.print((char)223); lcd.print("C"); }
}

void setup() {
  pinMode(UP_BTN, INPUT_PULLUP); pinMode(DOWN_BTN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT); pinMode(GREEN_LED, OUTPUT); pinMode(RED_LED, OUTPUT);
  Serial.begin(9600);
  dht.begin();
  lcd.init(); lcd.backlight();
  lcd.print("Temp Station");
  delay(1500);
}

void loop() {
  // buttons change the limit by 0.5 degree
  if (millis() - lastButton > 200) {
    if (digitalRead(UP_BTN) == LOW)   { limitC += 0.5; lastButton = millis(); showMinMax = false; showScreen(); }
    if (digitalRead(DOWN_BTN) == LOW) { limitC -= 0.5; lastButton = millis(); showMinMax = false; showScreen(); }
  }

  if (millis() - lastRead >= 2000) {          // the DHT22 needs 2 s between readings
    lastRead = millis();
    tempC = dht.readTemperature();
    humidity = dht.readHumidity();
    if (!isnan(tempC)) {
      if (tempC < minC) minC = tempC;
      if (tempC > maxC) maxC = tempC;
      Serial.print(tempC, 1); Serial.print(","); Serial.println(humidity, 0);   // works with Serial Plotter
    }
    showMinMax = !showMinMax;                 // second line switches every 2 s
    showScreen();
  }

  bool alarm = !isnan(tempC) && tempC > limitC;
  digitalWrite(GREEN_LED, !alarm);
  digitalWrite(RED_LED, alarm && (millis() / 300) % 2);
  if (alarm && (millis() / 300) % 2) tone(BUZZER, 2200); else noTone(BUZZER);
}
