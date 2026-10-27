/*
  Smart Pill Reminder Box
  Electronic Physics Projects 2026-2027 (Tally Grades)

  A box with 4 compartments (for example Morning, Noon, Evening, Night).
  At each dose time the LED of the right compartment lights up, the buzzer
  beeps and the LCD says which compartment to open. The patient presses the
  compartment's button after taking the pill. No press within 30 minutes =
  the dose is marked MISSED. The LCD keeps count of taken and missed doses.

  Parts: Arduino Uno, RTC module DS3231 (with its coin battery), LCD 16x2 I2C,
         4 LEDs + 4 resistors 220 Ohm, 4 push buttons (or 4 IR obstacle
         sensors under the lids), buzzer, a pill box with 4 compartments.
  Libraries: RTClib (by Adafruit) and LiquidCrystal I2C

  Pins:
    RTC and LCD share I2C: SDA -> A4, SCL -> A5, VCC -> 5V, GND -> GND
    Compartment LEDs 1..4 -> D4, D5, D6, D7
    Compartment buttons 1..4 -> D9, D10, D11, D12 (other legs to GND)
    Buzzer -> D8
*/
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);   // if the screen stays empty, try 0x3F

const byte LED_PINS[4] = {4, 5, 6, 7};
const byte BTN_PINS[4] = {9, 10, 11, 12};
const byte BUZZER = 8;
const char* NAMES[4] = {"Morning", "Noon", "Evening", "Night"};

// The doctor's schedule: hour, minute, compartment (1..4). Edit to fit the patient.
struct Dose { byte hour, minute, box; };
const Dose SCHEDULE[] = { {8, 0, 1}, {14, 0, 2}, {20, 0, 3}, {23, 0, 4} };
const byte DOSES = sizeof(SCHEDULE) / sizeof(SCHEDULE[0]);
const unsigned long WAIT_MS = 30UL * 60 * 1000;    // 30 minutes to take the pill

int activeBox = 0;                    // 0 = nothing due, else 1..4
unsigned long dueSince = 0;
int lastMinuteChecked = -1;
int taken = 0, missed = 0;

void allLeds(bool on) { for (int i = 0; i < 4; i++) digitalWrite(LED_PINS[i], on); }

void showClock(const DateTime& now) {
  char line[17];
  lcd.setCursor(0, 0);
  snprintf(line, sizeof(line), "%02d:%02d:%02d        ", now.hour(), now.minute(), now.second());
  lcd.print(line);
  lcd.setCursor(0, 1);
  snprintf(line, sizeof(line), "Taken %d Miss %d   ", taken, missed);
  lcd.print(line);
}

void startDose(int box) {
  activeBox = box;
  dueSince = millis();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Take your pill!");
  lcd.setCursor(0, 1); lcd.print("Box "); lcd.print(box); lcd.print(" "); lcd.print(NAMES[box - 1]);
}

void endDose(bool wasTaken) {
  if (wasTaken) { taken++; tone(BUZZER, 1500, 150); } else { missed++; tone(BUZZER, 300, 800); }
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(wasTaken ? "Well done :)" : "Dose MISSED!");
  Serial.print("Box "); Serial.print(activeBox); Serial.println(wasTaken ? " taken" : " missed");
  activeBox = 0;
  allLeds(false);
  delay(2000);
  lcd.clear();
}

void setup() {
  for (int i = 0; i < 4; i++) { pinMode(LED_PINS[i], OUTPUT); pinMode(BTN_PINS[i], INPUT_PULLUP); }
  pinMode(BUZZER, OUTPUT);
  Serial.begin(9600);
  lcd.init(); lcd.backlight();
  if (!rtc.begin()) { lcd.print("RTC not found!"); while (true); }
  if (rtc.lostPower()) rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));   // set clock from the computer
  lcd.clear();
}

void loop() {
  DateTime now = rtc.now();

  // Is a dose due this minute? (checked once per minute)
  if (activeBox == 0 && now.minute() != lastMinuteChecked) {
    lastMinuteChecked = now.minute();
    for (int i = 0; i < DOSES; i++)
      if (SCHEDULE[i].hour == now.hour() && SCHEDULE[i].minute == now.minute()) startDose(SCHEDULE[i].box);
  }

  if (activeBox == 0) { showClock(now); delay(200); return; }

  // A dose is due: blink its LED and beep every 5 s
  bool blinkOn = (millis() / 400) % 2;
  digitalWrite(LED_PINS[activeBox - 1], blinkOn);
  if ((millis() - dueSince) % 5000 < 300) tone(BUZZER, 2000, 200);

  for (int i = 0; i < 4; i++) {
    if (digitalRead(BTN_PINS[i]) == LOW) {
      if (i + 1 == activeBox) { endDose(true); return; }
      tone(BUZZER, 400, 300);                      // wrong compartment
      lcd.setCursor(0, 1); lcd.print("Wrong box! Use "); lcd.print(activeBox);
      delay(800);
      lcd.setCursor(0, 1); lcd.print("Box "); lcd.print(activeBox); lcd.print(" "); lcd.print(NAMES[activeBox - 1]); lcd.print("   ");
    }
  }
  if (millis() - dueSince > WAIT_MS) endDose(false);
}
