/*
  Fire Fighting Robot
  Electronic Physics Projects 2026-2027 (Tally Grades)

  Three flame sensors (left, center, right) look for a flame. The robot turns
  toward the strongest flame and drives to it. When the flame is close, it
  stops, turns the water pump on and sweeps the nozzle with a servo until the
  flame is out. With no flame in sight it turns slowly to search.

  Parts: Arduino Uno, robot chassis 2WD with 2 DC motors + wheels, L298N motor
         driver, 3 flame sensor modules (IR, with analog pin AO), mini water
         pump 5 V + tube + small bottle, relay module 5 V (or transistor TIP120
         + diode), servo SG90, buzzer, 2 x 18650 batteries + holder + switch.
  Library: Servo (comes with the Arduino IDE)

  Pins:
    L298N: ENA -> D5, IN1 -> D7, IN2 -> D8, ENB -> D6, IN3 -> D12, IN4 -> D13
           (remove the ENA/ENB jumpers), 12V -> battery +, GND -> battery - and Arduino GND
    Flame sensors AO: left -> A0, center -> A1, right -> A2 (VCC 5V, GND)
    Pump relay IN -> D4, Servo -> D3, Buzzer -> A3
*/
#include <Servo.h>

const byte ENA = 5, IN1 = 7, IN2 = 8, ENB = 6, IN3 = 12, IN4 = 13;
const byte FLAME_L = A0, FLAME_C = A1, FLAME_R = A2, BUZZER = A3;
const byte PUMP_RELAY = 4, SERVO_PIN = 3;
const bool RELAY_ACTIVE_LOW = true;   // most relay modules turn ON with LOW

// Flame sensors give LOWER numbers for a stronger flame. Check yours in the Serial Monitor.
const int SEE_FIRE = 800;             // below this: a flame is seen
const int NEAR_FIRE = 250;            // below this (center): close enough to spray
const int DRIVE_SPEED = 150, TURN_SPEED = 140, SEARCH_SPEED = 110;   // 0..255

Servo nozzle;

void motors(int left, int right) {     // -255..255 for each side
  digitalWrite(IN1, left >= 0);  digitalWrite(IN2, left < 0);
  digitalWrite(IN3, right >= 0); digitalWrite(IN4, right < 0);
  analogWrite(ENA, abs(left));
  analogWrite(ENB, abs(right));
}
void stopMotors() { motors(0, 0); }

void pump(bool on) { digitalWrite(PUMP_RELAY, (on ^ RELAY_ACTIVE_LOW) ? HIGH : LOW); }

void putOutFire() {
  stopMotors();
  tone(BUZZER, 2000, 300);
  pump(true);
  for (int round = 0; round < 6; round++) {               // sweep left and right
    for (int a = 50; a <= 130; a += 5) { nozzle.write(a); delay(20); }
    for (int a = 130; a >= 50; a -= 5) { nozzle.write(a); delay(20); }
    if (analogRead(FLAME_C) > SEE_FIRE && analogRead(FLAME_L) > SEE_FIRE && analogRead(FLAME_R) > SEE_FIRE) break;
  }
  pump(false);
  nozzle.write(90);
  delay(1000);
}

void setup() {
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(PUMP_RELAY, OUTPUT); pinMode(BUZZER, OUTPUT);
  pump(false);
  nozzle.attach(SERVO_PIN);
  nozzle.write(90);
  Serial.begin(9600);
  delay(2000);                                             // 2 s to put the robot down
}

void loop() {
  int l = analogRead(FLAME_L), c = analogRead(FLAME_C), r = analogRead(FLAME_R);
  Serial.print("L:"); Serial.print(l); Serial.print(" C:"); Serial.print(c); Serial.print(" R:"); Serial.println(r);

  if (c < NEAR_FIRE) { putOutFire(); return; }              // close: spray

  int strongest = min(l, min(c, r));
  if (strongest > SEE_FIRE) {                               // nothing seen: search slowly
    motors(-SEARCH_SPEED, SEARCH_SPEED);
    delay(80);
    stopMotors();
    delay(120);
    return;
  }

  if (strongest == c)      motors(DRIVE_SPEED, DRIVE_SPEED);    // flame ahead: go forward
  else if (strongest == l) motors(-TURN_SPEED, TURN_SPEED);     // flame on the left: turn left
  else                     motors(TURN_SPEED, -TURN_SPEED);     // flame on the right: turn right
  delay(60);
}
