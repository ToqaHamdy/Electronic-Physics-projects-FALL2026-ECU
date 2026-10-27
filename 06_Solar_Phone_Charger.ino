/*
  Solar Powered Phone Charger (monitor)
  Electronic Physics Projects 2026-2027 (Tally Grades)

  The charging itself is done by modules, no code needed:
    Solar panel 6 V -> TP4056 charger (with protection) -> 18650 battery
    18650 battery -> MT3608 boost (set to 5.1 V) -> USB socket -> phone
  The Arduino measures the panel and battery voltages and shows on the LCD:
  panel volts, battery volts, battery %, and whether the sun is charging it.

  Parts: Arduino Uno, LCD 16x2 I2C, 4 resistors 10 kOhm (two dividers),
         red LED + 220 Ohm (low battery), plus the charger parts above.
  Library: LiquidCrystal I2C (by Frank de Brabander)

  Pins:
    Panel +  -> 10k -> A0 -> 10k -> GND   (divider halves the voltage)
    Battery + -> 10k -> A1 -> 10k -> GND
    Low battery LED -> D9
    LCD I2C -> SDA = A4, SCL = A5
  All grounds (panel -, TP4056 -, battery -, Arduino GND) are joined.
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);   // if the screen stays empty, try 0x3F

const byte PANEL_PIN = A0, BATTERY_PIN = A1, LOW_LED = 9;
const float DIVIDER = 2.0;            // 10k + 10k halves the voltage
const float VREF = 5.0;               // measure your 5V pin with a multimeter and write it here

float readVolts(byte pin) {
  long sum = 0;
  for (int i = 0; i < 20; i++) { sum += analogRead(pin); delay(2); }   // average 20 readings
  return (sum / 20.0) * VREF / 1023.0 * DIVIDER;
}

int batteryPercent(float v) {          // Li-ion: about 3.0 V empty, 4.2 V full
  int p = (int)((v - 3.0) / (4.2 - 3.0) * 100);
  return constrain(p, 0, 100);
}

void setup() {
  pinMode(LOW_LED, OUTPUT);
  Serial.begin(9600);
  lcd.init(); lcd.backlight();
}

void loop() {
  float panel = readVolts(PANEL_PIN);
  float battery = readVolts(BATTERY_PIN);
  int percent = batteryPercent(battery);
  bool charging = panel > battery + 0.5;            // the panel must be higher to charge

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sun "); lcd.print(panel, 1); lcd.print("V ");
  lcd.print(charging ? "CHARGE" : "NO SUN");   // 16 characters max per line
  lcd.setCursor(0, 1);
  lcd.print("Bat "); lcd.print(battery, 2); lcd.print("V "); lcd.print(percent); lcd.print("%");

  digitalWrite(LOW_LED, battery < 3.3 ? HIGH : LOW); // warn before the battery is empty

  Serial.print("Panel: "); Serial.print(panel, 2);
  Serial.print(" V  Battery: "); Serial.print(battery, 2);
  Serial.print(" V  "); Serial.print(percent); Serial.println(" %");
  delay(1000);
}
