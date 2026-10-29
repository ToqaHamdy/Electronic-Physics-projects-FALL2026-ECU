/*
  Dino Jump Game
  Electronic Physics Projects 2026-2027 (Tally Grades)

  A small dinosaur runs on an OLED screen. Cactuses (and later birds) come
  from the right; press the button to jump over them. The game gets faster,
  and the best score is kept even after the ESP32 is switched off.

  Parts: ESP32 DevKit, OLED 0.96" 128x64 I2C (SSD1306), 1 push button, buzzer.
  Libraries: Adafruit SSD1306 and Adafruit GFX (by Adafruit)
  Board: install "esp32 by Espressif Systems" in Tools > Board > Boards Manager,
         then choose "ESP32 Dev Module".

  Pins:
    Jump button -> GPIO 4 (other leg to GND)
    Buzzer      -> GPIO 25 (other leg to GND)
    OLED SDA -> GPIO 21, SCL -> GPIO 22, VCC -> 3V3, GND -> GND
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>                     // keeps the best score in the ESP32 memory

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
Preferences memory;

const byte JUMP_BTN = 4, BUZZER = 25;
const int GROUND_Y = 54, DINO_X = 12, DINO_W = 12, DINO_H = 14;
const int MAX_OBSTACLES = 3;

struct Obstacle { bool active; float x; int w, h, y; bool bird; };
Obstacle obs[MAX_OBSTACLES];
float dinoY = GROUND_Y - DINO_H;              // top of the dinosaur
float jumpSpeed = 0;                           // pixels per frame, minus = going up
bool onGround = true;
float speed = 2.0;                             // how fast the world moves left
unsigned long score = 0, best = 0;
bool gameOver = false;
int legStep = 0, groundShift = 0;

bool pressed() {                               // true once per press
  static bool last = false;
  bool down = (digitalRead(JUMP_BTN) == LOW);
  bool p = down && !last;
  last = down;
  return p;
}

void newGame() {
  for (int i = 0; i < MAX_OBSTACLES; i++) obs[i].active = false;
  dinoY = GROUND_Y - DINO_H; jumpSpeed = 0; onGround = true;
  speed = 2.0; score = 0; gameOver = false;
}

void spawnObstacle() {
  for (int i = 0; i < MAX_OBSTACLES; i++)      // keep a gap, so every obstacle can be jumped
    if (obs[i].active && obs[i].x > 128 - 50 - speed * 8) return;
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (!obs[i].active) {
      Obstacle &o = obs[i];
      o.active = true; o.x = 128;
      o.bird = score > 400 && random(0, 4) == 0;           // birds after 400 points
      if (o.bird) { o.w = 12; o.h = 6; o.y = GROUND_Y - 22; }
      else        { o.w = random(5, 10); o.h = random(10, 16); o.y = GROUND_Y - o.h; }
      return;
    }
  }
}

void drawDino(int y) {
  oled.fillRect(DINO_X + 4, y, 8, 6, SSD1306_WHITE);        // head
  oled.drawPixel(DINO_X + 9, y + 1, SSD1306_BLACK);          // eye
  oled.fillRect(DINO_X, y + 6, 9, 5, SSD1306_WHITE);         // body
  oled.fillRect(DINO_X - 3, y + 6, 3, 2, SSD1306_WHITE);     // tail
  if (!onGround || legStep < 4) {                            // legs, running
    oled.fillRect(DINO_X + 1, y + 11, 2, 3, SSD1306_WHITE);
    oled.fillRect(DINO_X + 6, y + 11, 2, 2, SSD1306_WHITE);
  } else {
    oled.fillRect(DINO_X + 1, y + 11, 2, 2, SSD1306_WHITE);
    oled.fillRect(DINO_X + 6, y + 11, 2, 3, SSD1306_WHITE);
  }
}

void drawObstacle(const Obstacle &o) {
  int x = (int)o.x;
  if (o.bird) {                                              // a bird with flapping wings
    oled.fillRect(x, o.y + 2, o.w, 3, SSD1306_WHITE);
    if (legStep < 4) oled.drawLine(x + 3, o.y + 2, x + 6, o.y - 2, SSD1306_WHITE);
    else             oled.drawLine(x + 3, o.y + 4, x + 6, o.y + 8, SSD1306_WHITE);
  } else {                                                   // a cactus with two arms
    oled.fillRect(x + o.w / 2 - 1, o.y, 3, o.h, SSD1306_WHITE);
    oled.fillRect(x, o.y + 4, 2, 4, SSD1306_WHITE);
    oled.fillRect(x + o.w - 2, o.y + 2, 2, 4, SSD1306_WHITE);
  }
}

void drawFrame() {
  oled.clearDisplay();
  oled.drawFastHLine(0, GROUND_Y, 128, SSD1306_WHITE);       // the ground
  for (int x = -groundShift; x < 128; x += 9)                // moving dots on the ground
    oled.drawPixel(x, GROUND_Y + 3, SSD1306_WHITE);
  for (int i = 0; i < MAX_OBSTACLES; i++)
    if (obs[i].active) drawObstacle(obs[i]);
  drawDino((int)dinoY);
  oled.setTextSize(1); oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);  oled.print(F("HI ")); oled.print(best);
  oled.setCursor(80, 0); oled.print(score / 10);
  oled.display();
}

bool hits(const Obstacle &o) {                               // boxes touch, with a small margin
  int dx = DINO_X + 1, dy = (int)dinoY + 1, dw = DINO_W - 3, dh = DINO_H - 2;
  return dx < o.x + o.w && dx + dw > o.x && dy < o.y + o.h && dy + dh > o.y;
}

void setup() {
  pinMode(JUMP_BTN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  randomSeed(esp_random());                   // the ESP32 makes real random numbers
  Wire.begin(21, 22);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { while (true); }  // OLED not found
  memory.begin("dino", false);
  best = memory.getULong("best", 0);
  newGame();
  drawFrame();
}

void loop() {
  if (gameOver) {
    if (pressed()) newGame();
    return;
  }

  if (pressed() && onGround) { jumpSpeed = -5.2; onGround = false; tone(BUZZER, 1200, 30); }
  if (!onGround) {                                           // the jump: up, then gravity pulls down
    dinoY += jumpSpeed;
    jumpSpeed += 0.45;
    if (dinoY >= GROUND_Y - DINO_H) { dinoY = GROUND_Y - DINO_H; onGround = true; jumpSpeed = 0; }
  }

  if (random(0, 100) < 4) spawnObstacle();
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (!obs[i].active) continue;
    obs[i].x -= speed;
    if (obs[i].x + obs[i].w < 0) obs[i].active = false;
    else if (hits(obs[i])) gameOver = true;
  }

  legStep = (legStep + 1) % 8;
  groundShift = (groundShift + (int)speed) % 9;
  score++;
  if (score % 1000 == 0) tone(BUZZER, 1800, 60);             // every 100 points on the screen
  speed = 2.0 + score / 500.0;                               // faster over time
  if (speed > 6) speed = 6;
  drawFrame();

  if (gameOver) {
    if (score / 10 > best) { best = score / 10; memory.putULong("best", best); }
    tone(BUZZER, 200, 600);
    oled.fillRect(24, 18, 80, 26, SSD1306_BLACK);
    oled.drawRect(24, 18, 80, 26, SSD1306_WHITE);
    oled.setCursor(36, 23); oled.print(F("GAME OVER"));
    oled.setCursor(30, 33); oled.print(F("press button"));
    oled.display();
    delay(800);
  }
  delay(20);                                                 // about 40 frames a second
}
