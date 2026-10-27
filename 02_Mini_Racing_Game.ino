/*
  Mini Racing Game
  Electronic Physics Projects 2026-2027 (Tally Grades)

  A car drives on a 3-lane road on an OLED screen. Cars come down the road;
  move left or right with two buttons to avoid them. The game gets faster.

  Parts: Arduino Uno, OLED 0.96" 128x64 I2C (SSD1306), 2 push buttons, buzzer.
  Libraries: Adafruit SSD1306 and Adafruit GFX (by Adafruit)

  Pins:
    Left button -> D2, Right button -> D3 (other legs to GND)
    Buzzer -> D8
    OLED SDA -> A4, SCL -> A5, VCC -> 5V, GND -> GND
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 oled(128, 64, &Wire, -1);

const byte LEFT_BTN = 2, RIGHT_BTN = 3, BUZZER = 8;
const int ROAD_X = 34, LANE_W = 20, CAR_W = 12, CAR_H = 14, PLAYER_Y = 48;
const int MAX_ENEMIES = 3;

struct Enemy { bool active; int lane; float y; };
Enemy enemies[MAX_ENEMIES];
int playerLane = 1;              // 0 = left, 1 = middle, 2 = right
float speed = 1.0;               // pixels per frame
unsigned long score = 0, best = 0;
bool gameOver = false;
int stripe = 0;

int laneX(int lane) { return ROAD_X + lane * LANE_W + (LANE_W - CAR_W) / 2; }

bool pressed(byte pin) {         // true once per press
  static bool last[14] = {0};
  bool down = (digitalRead(pin) == LOW);
  bool p = down && !last[pin];
  last[pin] = down;
  return p;
}

void newGame() {
  for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
  playerLane = 1; speed = 1.0; score = 0; gameOver = false;
}

void spawnEnemy() {
  for (int i = 0; i < MAX_ENEMIES; i++)              // keep a gap, so there is always a way out
    if (enemies[i].active && enemies[i].y < 22) return;
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].active) {
      enemies[i].active = true;
      enemies[i].lane = random(0, 3);
      enemies[i].y = -CAR_H;
      return;
    }
  }
}

void drawCar(int x, int y, bool filled) {
  if (filled) oled.fillRoundRect(x, y, CAR_W, CAR_H, 3, SSD1306_WHITE);
  else        oled.drawRect(x, y, CAR_W, CAR_H, SSD1306_WHITE);
  oled.fillRect(x - 2, y + 2, 2, 3, SSD1306_WHITE);             // wheels
  oled.fillRect(x + CAR_W, y + 2, 2, 3, SSD1306_WHITE);
  oled.fillRect(x - 2, y + CAR_H - 5, 2, 3, SSD1306_WHITE);
  oled.fillRect(x + CAR_W, y + CAR_H - 5, 2, 3, SSD1306_WHITE);
}

void drawFrame() {
  oled.clearDisplay();
  oled.drawFastVLine(ROAD_X - 1, 0, 64, SSD1306_WHITE);           // road edges
  oled.drawFastVLine(ROAD_X + 3 * LANE_W, 0, 64, SSD1306_WHITE);
  for (int lane = 1; lane < 3; lane++)                              // moving stripes
    for (int y = -8 + stripe; y < 64; y += 16)
      oled.drawFastVLine(ROAD_X + lane * LANE_W, y, 8, SSD1306_WHITE);
  for (int i = 0; i < MAX_ENEMIES; i++)
    if (enemies[i].active) drawCar(laneX(enemies[i].lane), (int)enemies[i].y, false);
  drawCar(laneX(playerLane), PLAYER_Y, true);
  oled.setTextSize(1); oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);  oled.print(F("SCORE"));
  oled.setCursor(0, 10); oled.print(score);
  oled.setCursor(98, 0); oled.print(F("BEST"));
  oled.setCursor(98, 10); oled.print(best);
  oled.display();
}

void setup() {
  pinMode(LEFT_BTN, INPUT_PULLUP);
  pinMode(RIGHT_BTN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  randomSeed(analogRead(A0));                 // A0 left empty: random noise
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { while (true); }  // OLED not found
  newGame();
}

void loop() {
  if (gameOver) {
    if (pressed(LEFT_BTN) || pressed(RIGHT_BTN)) newGame();
    return;
  }

  if (pressed(LEFT_BTN)  && playerLane > 0) { playerLane--; tone(BUZZER, 900, 20); }
  if (pressed(RIGHT_BTN) && playerLane < 2) { playerLane++; tone(BUZZER, 900, 20); }

  if (random(0, 100) < (long)(3 + score / 200)) spawnEnemy();     // more cars over time

  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].active) continue;
    enemies[i].y += speed;
    if (enemies[i].y > 64) { enemies[i].active = false; score += 10; }
    bool sameLane = enemies[i].lane == playerLane;
    bool touching = enemies[i].y + CAR_H > PLAYER_Y && enemies[i].y < PLAYER_Y + CAR_H;
    if (sameLane && touching) gameOver = true;
  }

  stripe = (stripe + (int)speed) % 16;
  speed = 1.0 + score / 300.0;                             // faster every 300 points
  if (speed > 4) speed = 4;
  score++;
  drawFrame();

  if (gameOver) {
    if (score > best) best = score;
    tone(BUZZER, 200, 600);
    oled.fillRect(24, 20, 80, 26, SSD1306_BLACK);
    oled.drawRect(24, 20, 80, 26, SSD1306_WHITE);
    oled.setCursor(36, 25); oled.print(F("GAME OVER"));
    oled.setCursor(30, 35); oled.print(F("press button"));
    oled.display();
    delay(800);
  }
  delay(20);                                                // about 40 frames a second
}
