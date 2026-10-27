/*
  Ultrasonic Radar Visualization System (Arduino part)
  Electronic Physics Projects 2026-2027 (Tally Grades)

  A servo sweeps an ultrasonic sensor from 15 to 165 degrees and back.
  At each angle the Arduino sends "angle,distance." over USB.
  The Processing sketch Radar.pde draws it as a green radar screen.

  Parts: Arduino Uno, HC-SR04 ultrasonic sensor, servo SG90, buzzer (optional).
  Library: Servo (comes with the Arduino IDE)

  Pins:
    HC-SR04 TRIG -> D10, ECHO -> D11, VCC -> 5V, GND -> GND
    Servo signal -> D12
    Buzzer -> D8 (beeps when something is closer than 20 cm)
*/
#include <Servo.h>

const byte TRIG = 10, ECHO = 11, SERVO_PIN = 12, BUZZER = 8;
const int MAX_CM = 40;        // the radar screen shows up to 40 cm
Servo radarServo;

int readDistanceCm() {
  digitalWrite(TRIG, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  unsigned long us = pulseIn(ECHO, HIGH, 30000UL);   // wait max 30 ms (about 5 m)
  if (us == 0) return 0;                             // no echo: nothing in range
  return us * 0.0343 / 2;                            // speed of sound: 343 m/s
}

void scanAt(int angle) {
  radarServo.write(angle);
  delay(30);                                         // let the servo arrive
  int d = readDistanceCm();
  if (d > 0 && d < 20) tone(BUZZER, 1500, 20);
  Serial.print(angle); Serial.print(","); Serial.print(d); Serial.print(".");
}

void setup() {
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(BUZZER, OUTPUT);
  radarServo.attach(SERVO_PIN);
  Serial.begin(9600);
}

void loop() {
  for (int a = 15; a <= 165; a++) scanAt(a);
  for (int a = 165; a >= 15; a--) scanAt(a);
}
