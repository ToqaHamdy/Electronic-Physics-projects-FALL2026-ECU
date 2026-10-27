/*
  Ultrasonic Radar Visualization System (laptop part, Processing 4)
  Download Processing from processing.org, open this file, set PORT below,
  close the Arduino Serial Monitor, then press Run.
*/
import processing.serial.*;

String PORT = "COM3";          // Windows: COM3, COM4... (Arduino IDE > Tools > Port)
int MAX_CM = 40;               // same as in the Arduino code

Serial port;
int angle = 0, distance = 0;
float[] dots = new float[181]; // last distance seen at each angle

void setup() {
  size(1200, 700);
  smooth();
  background(0);
  port = new Serial(this, PORT, 9600);
  port.bufferUntil('.');
}

void serialEvent(Serial p) {
  String data = p.readStringUntil('.');
  if (data == null) return;
  data = trim(data.substring(0, data.length() - 1));
  int comma = data.indexOf(',');
  if (comma < 0) return;
  angle = int(data.substring(0, comma));
  distance = int(data.substring(comma + 1));
  if (angle >= 0 && angle <= 180) dots[angle] = distance;
}

void draw() {
  fill(0, 8); noStroke(); rect(0, 0, width, height);   // fading trail
  translate(width / 2, height - 80);
  float r = width * 0.45;

  stroke(98, 245, 31); strokeWeight(2); noFill();     // radar grid
  for (int i = 1; i <= 4; i++) arc(0, 0, 2 * r * i / 4, 2 * r * i / 4, PI, TWO_PI);
  for (int a = 0; a <= 180; a += 30) line(0, 0, r * cos(radians(a)), -r * sin(radians(a)));

  strokeWeight(9); stroke(255, 10, 10);               // objects in red
  for (int a = 0; a <= 180; a++) {
    if (dots[a] > 0 && dots[a] < MAX_CM) {
      float d = dots[a] * r / MAX_CM;
      point(d * cos(radians(a)), -d * sin(radians(a)));
    }
  }

  strokeWeight(4); stroke(30, 250, 60);               // the sweeping line
  line(0, 0, r * cos(radians(angle)), -r * sin(radians(angle)));

  resetMatrix(); fill(98, 245, 31); textSize(24);
  text("Angle: " + angle + "°", 40, height - 30);
  text(distance > 0 && distance < MAX_CM ? "Object: " + distance + " cm" : "Out of range", 300, height - 30);
  for (int i = 1; i <= 4; i++) text((MAX_CM * i / 4) + " cm", width / 2 + r * i / 4 - 30, height - 50);
}
