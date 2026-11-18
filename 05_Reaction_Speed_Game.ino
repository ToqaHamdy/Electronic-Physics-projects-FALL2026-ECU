/*
  Reaction Speed Lab (Reaction Speed Game, advanced)
  Electronic Physics Projects 2026-2027 (Tally Grades)

  A reaction-time lab with 4 modes, timed to the microsecond with interrupts:
    1 DUEL   2 players, first to press after GO wins. Best of 5 rounds.
    2 LIGHT  1 player, 5 rounds, reacts to the light (no beep).
    3 SOUND  1 player, 5 rounds, reacts to the beep (no light).
             Compare LIGHT and SOUND: people react faster to sound.
    4 CHOICE 1 player, 5 rounds: RED light = left button, BLUE light = right button.
             Choosing takes longer than just reacting (Hick's law).
  Pressing before GO is a false start. After 5 rounds the screen shows the
  average, the best, the spread (standard deviation) and the physics of your
  time: how far a falling ruler drops (d = 1/2 g t^2) and how far a car at
  60 km/h moves before you react (d = v t). The best average of each mode is
  kept in the ESP32 memory, and every result is shown live on a phone:
  join the Wi-Fi "Reaction-Lab" (password physics2026) and open 192.168.4.1

  Parts: ESP32 DevKit, OLED 0.96" 128x64 I2C (SSD1306), 4 push buttons
         (2 big arcade buttons for the players), RGB LED (common cathode)
         + 3 resistors 220 Ohm, passive buzzer.
  Libraries: Adafruit SSD1306 and Adafruit GFX (by Adafruit)
  Board: install "esp32 by Espressif Systems" in Tools > Board > Boards Manager,
         then choose "ESP32 Dev Module".

  Pins:
    Left / Player 1 button  -> GPIO 32     Right / Player 2 button -> GPIO 33
    START button            -> GPIO 27     MODE button             -> GPIO 26
    (the other leg of every button to GND)
    RGB LED red / green / blue -> GPIO 16 / 17 / 19 (through 220 Ohm), long leg to GND
    Buzzer -> GPIO 18 (other leg to GND)
    OLED SDA -> GPIO 21, SCL -> GPIO 22, VCC -> 3V3, GND -> GND
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
Preferences memory;
WebServer web(80);

const byte BTN_L = 32, BTN_R = 33, BTN_START = 27, BTN_MODE = 26;
const byte LED_R = 16, LED_G = 17, LED_B = 19, BUZZER = 18;
const int ROUNDS = 5;
const char* MODE_NAME[] = { "DUEL", "LIGHT", "SOUND", "CHOICE" };

/* the moment each player button is pressed, written by the interrupts */
volatile unsigned long pressL = 0, pressR = 0;
void IRAM_ATTR onLeft()  { if (pressL == 0) pressL = micros(); }
void IRAM_ATTR onRight() { if (pressR == 0) pressR = micros(); }

enum State { MENU, WAITING, GO, RESULT, SUMMARY };
State state = MENU;
int mode = 0, round_ = 0, wins1 = 0, wins2 = 0, wrong = 0;
unsigned long goAt = 0, waitUntil = 0, shownAt = 0;
float times[ROUNDS];                        // reaction times of this game, in ms
int target = 0;                             // CHOICE: 0 = red (left), 1 = blue (right)
float bestAvg[4];                           // best 5-round average of each mode (ms)
String lastLine = "", logHtml = "";         // what the phone page shows

void startRound();

void rgb(bool r, bool g, bool b) { digitalWrite(LED_R, r); digitalWrite(LED_G, g); digitalWrite(LED_B, b); }

bool clicked(byte pin) {                    // START and MODE: true once per press
  static bool last[40] = {0};
  bool down = digitalRead(pin) == LOW;
  bool c = down && !last[pin];
  last[pin] = down;
  return c;
}

void screen(const String& a, const String& b, const String& c, const String& d) {
  oled.clearDisplay(); oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(2); oled.setCursor(0, 0); oled.print(a);
  oled.setTextSize(1);
  oled.setCursor(0, 24); oled.print(b);
  oled.setCursor(0, 38); oled.print(c);
  oled.setCursor(0, 52); oled.print(d);
  oled.display();
}

void addLog(const String& line) {           // newest first, at most 20 lines
  lastLine = line;
  logHtml = "<li>" + line + "</li>" + logHtml;
  int n = 0, at = 0;
  while ((at = logHtml.indexOf("<li>", at)) >= 0) { if (++n > 20) { logHtml = logHtml.substring(0, at); break; } at += 4; }
}

void showMenu() {
  state = MENU; rgb(0, 0, 0);
  String best = bestAvg[mode] > 0 ? String("Best avg ") + String(bestAvg[mode], 0) + " ms" : String("No record yet");
  screen(MODE_NAME[mode], best, "MODE: next mode", "START: play");
}

void newGame() { round_ = 0; wins1 = wins2 = wrong = 0; for (int i = 0; i < ROUNDS; i++) times[i] = 0; startRound(); }

void startRound() {
  rgb(0, 0, 0);
  pressL = pressR = 0;
  waitUntil = millis() + random(1500, 4500);
  state = WAITING;
  screen("READY", String(MODE_NAME[mode]) + "  round " + String(round_ + 1) + "/" + String(ROUNDS),
         mode == 3 ? String("RED=left  BLUE=right") : String("Wait for the signal"), "Do not press yet");
}

void signalGo() {
  goAt = micros();
  pressL = pressR = 0;                      // only presses after GO count
  if (mode == 3) { target = random(0, 2); rgb(target == 0, 0, target == 1); }
  else if (mode != 2) rgb(0, 1, 0);         // green light (not in SOUND mode)
  if (mode != 1 && mode != 3) tone(BUZZER, 1500, 60);   // beep (not in LIGHT or CHOICE mode)
  state = GO;
}

void endRound(float ms, const String& who, bool falseStart, bool wrongKey) {
  rgb(0, 0, 0); state = RESULT; shownAt = millis();
  if (falseStart || wrongKey) {
    tone(BUZZER, 200, 500);
    if (mode == 0) {                        // DUEL: the other player gets the round
      if (who == "P1") wins2++; else wins1++;
      screen("FOUL!", "False start " + who, "Round to the other one", "P1 " + String(wins1) + " : " + String(wins2) + " P2");
      addLog("DUEL round " + String(round_ + 1) + ": false start " + who);
      round_++; return;
    }
    if (wrongKey) wrong++;
    screen(falseStart ? "TOO SOON" : "WRONG", falseStart ? "Wait for the signal" : "Wrong button", "Round again", "");
    addLog(String(MODE_NAME[mode]) + ": " + (falseStart ? "false start" : "wrong button"));
    return;                                 // the round is played again
  }
  times[round_] = ms;
  tone(BUZZER, 2000, 80);
  if (mode == 0) {
    if (who == "P1") wins1++; else wins2++;
    screen(who + " WINS", String(ms, 1) + " ms", "P1 " + String(wins1) + " : " + String(wins2) + " P2", "");
  } else {
    screen(String(ms, 0) + " ms", "Round " + String(round_ + 1) + "/" + String(ROUNDS),
           "Ruler drops " + String(500.0 * 9.81 * (ms / 1000.0) * (ms / 1000.0), 1) + " cm", "");
  }
  addLog(String(MODE_NAME[mode]) + " round " + String(round_ + 1) + ": " + (mode == 0 ? who + " " : String("")) + String(ms, 1) + " ms");
  round_++;
}

void showSummary() {
  state = SUMMARY;
  int n = 0; float sum = 0, best = 1e9;
  for (int i = 0; i < ROUNDS; i++) if (times[i] > 0) { n++; sum += times[i]; if (times[i] < best) best = times[i]; }
  if (mode == 0) {
    String w = wins1 > wins2 ? "P1 WINS" : wins2 > wins1 ? "P2 WINS" : "DRAW";
    screen(w, "P1 " + String(wins1) + " : " + String(wins2) + " P2", n ? String("Fastest ") + String(best, 1) + " ms" : String(""), "START: menu");
    addLog("DUEL final: " + w + " (" + String(wins1) + ":" + String(wins2) + ")");
    tone(BUZZER, 1200, 400);
    return;
  }
  float avg = sum / n, var = 0;
  for (int i = 0; i < ROUNDS; i++) var += (times[i] - avg) * (times[i] - avg);
  float sd = sqrt(var / n), t = avg / 1000.0;
  float ruler = 500.0 * 9.81 * t * t;       // cm: d = 1/2 g t^2
  float car = (60.0 / 3.6) * t;             // m: d = v t at 60 km/h
  bool record = bestAvg[mode] == 0 || avg < bestAvg[mode];
  if (record) { bestAvg[mode] = avg; memory.putFloat(MODE_NAME[mode], avg); }
  screen(String(avg, 0) + " ms", String(record ? "NEW RECORD! " : "") + "best " + String(best, 0) + " +-" + String(sd, 0),
         "Ruler " + String(ruler, 1) + "cm Car " + String(car, 1) + "m", mode == 3 ? String("Wrong: ") + String(wrong) : String("START: menu"));
  addLog("<b>" + String(MODE_NAME[mode]) + " average " + String(avg, 1) + " ms</b> (best " + String(best, 1) + ", spread " + String(sd, 1) +
         ", ruler " + String(ruler, 1) + " cm, car " + String(car, 2) + " m" + (mode == 3 ? String(", wrong ") + String(wrong) : String("")) + ")" + (record ? " NEW RECORD" : ""));
  tone(BUZZER, record ? 2400 : 1200, 400);
}

/* the phone page: results, records and the formulas */
void pageRoot() {
  String h = F("<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
               "<meta http-equiv='refresh' content='2'><title>Reaction Lab</title><style>body{font-family:sans-serif;margin:16px;background:#111;color:#eee}"
               "h1{color:#ff6b61}li{margin:6px 0}table{border-collapse:collapse}td{border:1px solid #444;padding:6px 10px}</style></head><body>"
               "<h1>Reaction Speed Lab</h1><table><tr><td>Mode</td><td>Best average</td></tr>");
  for (int m = 1; m < 4; m++) h += "<tr><td>" + String(MODE_NAME[m]) + "</td><td>" + (bestAvg[m] > 0 ? String(bestAvg[m], 1) + " ms" : "-") + "</td></tr>";
  h += F("</table><p>Ruler drop: d = &frac12; g t&sup2; (g = 9.81 m/s&sup2;). Car at 60 km/h: d = v t.</p><h2>Results</h2><ol>");
  h += logHtml + F("</ol></body></html>");
  web.send(200, "text/html", h);
}

void setup() {
  pinMode(BTN_L, INPUT_PULLUP); pinMode(BTN_R, INPUT_PULLUP);
  pinMode(BTN_START, INPUT_PULLUP); pinMode(BTN_MODE, INPUT_PULLUP);
  pinMode(LED_R, OUTPUT); pinMode(LED_G, OUTPUT); pinMode(LED_B, OUTPUT); pinMode(BUZZER, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BTN_L), onLeft, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_R), onRight, FALLING);
  randomSeed(esp_random());
  Serial.begin(115200);
  Wire.begin(21, 22);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { Serial.println("OLED not found"); while (true); }
  memory.begin("reaction", false);
  for (int m = 0; m < 4; m++) bestAvg[m] = memory.getFloat(MODE_NAME[m], 0);
  WiFi.softAP("Reaction-Lab", "physics2026");
  web.on("/", pageRoot);
  web.begin();
  showMenu();
}

void loop() {
  web.handleClient();

  switch (state) {
    case MENU:
      if (clicked(BTN_MODE)) { mode = (mode + 1) % 4; tone(BUZZER, 900, 30); showMenu(); }
      if (clicked(BTN_START)) newGame();
      break;

    case WAITING: {                         // a press now is a false start
      unsigned long l = pressL, r = pressR;
      if (l || r) { endRound(0, l && (!r || l <= r) ? "P1" : "P2", true, false); break; }
      if (millis() >= waitUntil) signalGo();
      break;
    }

    case GO: {
      unsigned long l = pressL, r = pressR;
      if (l || r) {
        bool leftFirst = l && (!r || l <= r);
        float ms = ((leftFirst ? l : r) - goAt) / 1000.0;
        if (mode == 3) endRound(ms, "", false, leftFirst != (target == 0));
        else endRound(ms, leftFirst ? "P1" : "P2", false, false);
      } else if (micros() - goAt > 3000000UL) {          // no press in 3 s
        rgb(0, 0, 0); state = RESULT; shownAt = millis();
        screen("TOO SLOW", "No press in 3 s", "Round again", "");
      }
      break;
    }

    case RESULT:                            // show the result 1.5 s, then the next round
      if (millis() - shownAt > 1500) { if (round_ >= ROUNDS) showSummary(); else startRound(); }
      break;

    case SUMMARY:
      if (clicked(BTN_START) || clicked(BTN_MODE)) showMenu();
      break;
  }
}
