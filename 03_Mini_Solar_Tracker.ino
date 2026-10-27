/*
  Mini Solar Power Tracking System
  Electronic Physics Projects 2026-2027 (Tally Grades)

  Four light sensors (LDRs) sit around the solar panel, split by a small
  cross-shaped wall. Two servos turn the panel left/right and up/down until
  all four sensors see the same light, so the panel faces the sun.

  Parts: Arduino Uno, 4 LDRs + 4 resistors 10 kOhm, 2 servos SG90 (or MG90S),
         small solar panel (6 V), pan-tilt bracket, 2 resistors 10 kOhm for
         measuring the panel voltage, external 5 V supply for the servos.
  Library: Servo (comes with the Arduino IDE)

  Pins:
    LDR top-left -> A0, top-right -> A1, bottom-left -> A2, bottom-right -> A3
      (each LDR: one leg to 5V, other leg to the pin AND to GND through 10 kOhm)
    Panel voltage -> A4 through a 10k/10k divider (panel + -> 10k -> A4 -> 10k -> GND)
    Horizontal servo -> D9, Vertical servo -> D10
*/
#include <Servo.h>

Servo servoH, servoV;
const byte LDR_TL = A0, LDR_TR = A1, LDR_BL = A2, LDR_BR = A3, PANEL = A4;
const int TOLERANCE = 30;     // ignore small differences (stops shaking)
const int STEP = 1;           // degrees per move
int angleH = 90, angleV = 60;

void setup() {
  Serial.begin(9600);
  servoH.attach(9);
  servoV.attach(10);
  servoH.write(angleH);
  servoV.write(angleV);
  delay(500);
}

void loop() {
  int tl = analogRead(LDR_TL), tr = analogRead(LDR_TR);
  int bl = analogRead(LDR_BL), br = analogRead(LDR_BR);

  int top = (tl + tr) / 2, bottom = (bl + br) / 2;
  int left = (tl + bl) / 2, right = (tr + br) / 2;

  if (abs(left - right) > TOLERANCE) {          // more light on one side: turn to it
    angleH += (left > right) ? STEP : -STEP;
    angleH = constrain(angleH, 0, 180);
    servoH.write(angleH);
  }
  if (abs(top - bottom) > TOLERANCE) {
    angleV += (top > bottom) ? STEP : -STEP;
    angleV = constrain(angleV, 10, 160);
    servoV.write(angleV);
  }

  // Panel voltage: the divider halves it, so multiply by 2
  float panelVolts = analogRead(PANEL) * 5.0 / 1023.0 * 2.0;

  Serial.print("TL:"); Serial.print(tl); Serial.print(" TR:"); Serial.print(tr);
  Serial.print(" BL:"); Serial.print(bl); Serial.print(" BR:"); Serial.print(br);
  Serial.print("  H:"); Serial.print(angleH); Serial.print(" V:"); Serial.print(angleV);
  Serial.print("  Panel:"); Serial.print(panelVolts, 2); Serial.println(" V");
  delay(30);
}
