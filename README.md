# Electronic Physics Projects, Fall 2026, ECU

Code and parts lists for Eng. Toqa Hamdy's groups (A3, A4, B3, B4, L3, L4, F1, F2, P1, P2).

**How to use:** install the [Arduino IDE](https://www.arduino.cc/en/software), open your project's `.ino` file (the IDE puts it in a folder for you), install the libraries below from Tools > Manage Libraries, choose the board and port, then press Upload.

**LCD empty?** Turn the blue screw on the back of the I2C module, then change `0x27` to `0x3F` in the code.

| Team | Project | Board | Code |
| --- | --- | --- | --- |
| 1 | [Anti-Theft Flooring System](#team-1-anti-theft-flooring-system) | Arduino Uno | [01_AntiTheft_Flooring.ino](01_AntiTheft_Flooring.ino) |
| 2 | [Mini Racing Game](#team-2-mini-racing-game) | Arduino Uno (or Nano) | [02_Mini_Racing_Game.ino](02_Mini_Racing_Game.ino) |
| 3 | [Mini Solar Power Tracking System](#team-3-mini-solar-power-tracking-system) | Arduino Uno | [03_Mini_Solar_Tracker.ino](03_Mini_Solar_Tracker.ino) |
| 4 | [Ultrasonic Radar Visualization System](#team-4-ultrasonic-radar-visualization-system) | Arduino Uno | [04_Ultrasonic_Radar.ino](04_Ultrasonic_Radar.ino) |
| 5 | [Reaction Speed Game](#team-5-reaction-speed-game) | Arduino Uno | [05_Reaction_Speed_Game.ino](05_Reaction_Speed_Game.ino) |
| 6 | [Solar Powered Phone Charger](#team-6-solar-powered-phone-charger) | Arduino Nano | [06_Solar_Phone_Charger.ino](06_Solar_Phone_Charger.ino) |
| 7 | [Digital Temperature Monitoring Station](#team-7-digital-temperature-monitoring-station) | Arduino Uno | [07_Digital_Temperature_Station.ino](07_Digital_Temperature_Station.ino) |
| 8 | [Automatic Water Tank Level Controller](#team-8-automatic-water-tank-level-controller) | Arduino Uno | [08_Water_Tank_Controller.ino](08_Water_Tank_Controller.ino) |
| 9 | [Fire Fighting Robot](#team-9-fire-fighting-robot) | Arduino Uno | [09_Fire_Fighting_Robot.ino](09_Fire_Fighting_Robot.ino) |
| 10 | [Smart Pill Reminder Box](#team-10-smart-pill-reminder-box) | Arduino Uno | [10_Smart_Pill_Reminder_Box.ino](10_Smart_Pill_Reminder_Box.ino) |

---

## Team 1: Anti-Theft Flooring System

**Board:** Arduino Uno. 6 analog inputs read the 4 piezo tiles directly, and every module works on 5 V. No Wi-Fi is needed.

**Other option:** ESP32, only to send an alarm to a phone; its pins take 3.3 V, so each piezo needs a small protection circuit.

**Code:** [01_AntiTheft_Flooring.ino](01_AntiTheft_Flooring.ino)

**Libraries:** LiquidCrystal I2C

### Components to buy

| Part | Qty | Note |
| --- | --- | --- |
| Arduino Uno + USB cable | 1 |  |
| Piezo disc 27 mm | 4 | one under each tile |
| Resistor 1 MΩ | 4 | one per piezo |
| Push button | 1 | arm / disarm |
| Passive buzzer | 1 | two-tone siren |
| LED red + LED green | 1 + 1 | with 220 Ω resistors |
| LCD 16x2 with I2C module | 1 | address 0x27 (or 0x3F) |
| Breadboard + jumper wires | 1 set |  |
| 4 tiles (MDF or foam board 20 x 20 cm) | 4 | the model floor |

### Connections

| From | To |
| --- | --- |
| Piezo tiles 1 to 4 | A0, A1, A2, A3 (each also to GND through 1 MΩ) |
| Button | D2 and GND |
| Buzzer + | D8 |
| Red / Green LED | D9 / D10 (through 220 Ω) |
| LCD SDA / SCL / VCC / GND | A4 / A5 / 5V / GND |

### Tests, in this order

1. LCD shows "System OFF" (empty screen: turn the I2C screw, or try 0x3F).
2. Tap each tile: its reading passes 100 in the Serial Plotter.
3. Press the button: green LED blinks 10 s, then stays on.
4. Arm, step on a tile: siren and "Tile N stepped!". Press to stop.

---

## Team 2: Mini Racing Game

**Board:** Arduino Uno (or Nano). The game is light for an Uno, and the OLED and buttons plug straight in. A Nano does the same job in a smaller handheld case.

**Other option:** ESP32 only if you want a bigger color screen later; not needed for this game.

**Code:** [02_Mini_Racing_Game.ino](02_Mini_Racing_Game.ino)

**Libraries:** Adafruit SSD1306, Adafruit GFX Library

### Components to buy

| Part | Qty | Note |
| --- | --- | --- |
| Arduino Uno + USB cable | 1 | or Arduino Nano |
| OLED 0.96 inch 128x64 I2C (SSD1306) | 1 | address 0x3C |
| Push button | 2 | left and right |
| Passive buzzer | 1 |  |
| Breadboard + jumper wires | 1 set |  |
| Game case (cardboard or 3D print) | 1 | like a small arcade |

### Connections

| From | To |
| --- | --- |
| Left / Right button | D2 / D3 (other legs to GND) |
| Buzzer + | D8 |
| OLED SDA / SCL / VCC / GND | A4 / A5 / 5V / GND |

### Tests, in this order

1. Upload the library example ssd1306_128x64_i2c: shapes appear on the OLED.
2. Each button moves the car one lane with a beep.
3. A crash shows GAME OVER; a button starts a new game.

---

## Team 3: Mini Solar Power Tracking System

**Board:** Arduino Uno. It reads 4 light sensors and the panel voltage on 5 analog inputs and drives 2 servos. All of it is 5 V and needs no Wi-Fi.

**Other option:** Arduino Nano for a smaller model (same code, same pins).

**Code:** [03_Mini_Solar_Tracker.ino](03_Mini_Solar_Tracker.ino)

**Libraries:** Servo (already in the Arduino IDE)

### Components to buy

| Part | Qty | Note |
| --- | --- | --- |
| Arduino Uno + USB cable | 1 |  |
| LDR (light sensor) | 4 |  |
| Resistor 10 kΩ | 6 | 4 for LDRs, 2 for the divider |
| Servo SG90 or MG90S | 2 | MG90S is stronger |
| Pan-tilt bracket | 1 |  |
| Solar panel 6 V (about 1 W) | 1 |  |
| 5 V supply for the servos (4 x AA) | 1 | GND joined to Arduino GND |
| Card cross wall, 4 cm high | 1 | shade between the LDRs |

### Connections

| From | To |
| --- | --- |
| LDR top-left / top-right / bottom-left / bottom-right | A0 / A1 / A2 / A3 (LDR from 5V, pin to GND through 10 kΩ) |
| Panel + through 10k/10k divider | A4 |
| Horizontal / Vertical servo signal | D9 / D10 |
| Servo power | external 5 V (GND shared) |

### Tests, in this order

1. Cover each LDR: only its number drops in the Serial Monitor.
2. Servos go to 90° and 60° at start (shaking = use the external 5 V).
3. Torch from the left: panel turns to it (if not, flip the sign in the code).
4. Compare the panel voltage facing the torch and turned away.

---

## Team 4: Ultrasonic Radar Visualization System

**Board:** Arduino Uno. The HC-SR04 works on 5 V, and the Uno sends the readings to the laptop over the USB cable for the radar screen.

**Other option:** ESP32 only to show the radar on a phone over Wi-Fi; the sensor echo pin then needs a voltage divider.

**Code:** [04_Ultrasonic_Radar.ino](04_Ultrasonic_Radar.ino) and the radar screen [Radar.pde](Radar.pde) (open it in Processing 4)

**Libraries:** Servo (already in the Arduino IDE) + Processing 4 on the laptop

### Components to buy

| Part | Qty | Note |
| --- | --- | --- |
| Arduino Uno + USB cable | 1 | stays connected to the laptop |
| Ultrasonic sensor HC-SR04 | 1 |  |
| Servo SG90 | 1 |  |
| Passive buzzer | 1 | optional |
| Breadboard + jumper wires + hot glue | 1 set |  |
| Laptop with Processing 4 | 1 | free, processing.org |

### Connections

| From | To |
| --- | --- |
| HC-SR04 TRIG / ECHO | D10 / D11 |
| HC-SR04 VCC / GND | 5V / GND |
| Servo signal | D12 |
| Buzzer + | D8 |

### Tests, in this order

1. Serial Monitor: the distance after each comma changes with your hand.
2. Servo sweeps slowly without shaking.
3. Close the Serial Monitor, set PORT in Radar.pde, Run: a hand shows as red dots.

---

## Team 5: Reaction Speed Game

**Board:** Arduino Uno. It measures time to the millisecond with simple buttons and LEDs. The Uno is the easiest and cheapest choice.

**Other option:** Arduino Nano for a smaller box; ESP32 is not needed.

**Code:** [05_Reaction_Speed_Game.ino](05_Reaction_Speed_Game.ino)

**Libraries:** LiquidCrystal I2C

### Components to buy

| Part | Qty | Note |
| --- | --- | --- |
| Arduino Uno + USB cable | 1 |  |
| Push button (big arcade type) | 3 | Player 1, Player 2, START |
| LED green, red, blue | 1 each | GO, P1, P2 |
| Resistor 220 Ω | 3 |  |
| Passive buzzer | 1 |  |
| LCD 16x2 with I2C module | 1 | address 0x27 (or 0x3F) |
| Breadboard + jumper wires + a box | 1 set |  |

### Connections

| From | To |
| --- | --- |
| Player 1 / Player 2 / START buttons | D2 / D3 / D4 (other legs to GND) |
| Buzzer + | D8 |
| GO / P1 / P2 LEDs | D9 / D10 / D11 (through 220 Ω) |
| LCD SDA / SCL / VCC / GND | A4 / A5 / 5V / GND |

### Tests, in this order

1. LCD shows "Reaction Game / Press START".
2. After START and the wait, the green LED and a beep.
3. Each player button lights its LED and shows a time of about 150-400 ms.
4. A press during "Get ready" gives the round to the other player.

---

## Team 6: Solar Powered Phone Charger

**Board:** Arduino Nano. The charging is done by the modules; the Arduino only watches the voltages. A Nano is small and fits in the charger box (same code as Uno).

**Other option:** Arduino Uno works the same. ESP32 is not recommended: its voltage reading is less accurate.

**Code:** [06_Solar_Phone_Charger.ino](06_Solar_Phone_Charger.ino)

**Libraries:** LiquidCrystal I2C

### Components to buy

| Part | Qty | Note |
| --- | --- | --- |
| Solar panel 6 V, 2 W or more | 1 |  |
| TP4056 charger module with protection | 1 | the 6-pin one (OUT+ / OUT-) |
| 18650 Li-ion battery + holder | 1 |  |
| MT3608 boost module | 1 | set to 5.1 V before connecting a phone |
| USB-A female socket | 1 |  |
| Diode 1N5819 | 1 | stops night leakage into the panel |
| Arduino Nano + USB cable | 1 | or Uno |
| LCD 16x2 with I2C module | 1 | address 0x27 (or 0x3F) |
| Resistor 10 kΩ | 4 | two dividers |
| LED red + 220 Ω | 1 | low battery |
| Switch | 1 | USB output on/off |

### Connections

| From | To |
| --- | --- |
| Panel + → diode → TP4056 IN+ | Panel - → TP4056 IN- |
| TP4056 B+ / B- | Battery + / - |
| TP4056 OUT → switch → MT3608 IN | MT3608 OUT → USB socket |
| Panel + → 10k → A0 → 10k → GND | panel voltage |
| Battery + → 10k → A1 → 10k → GND | battery voltage |
| LED / LCD SDA / SCL | D9 / A4 / A5 |

### Tests, in this order

1. No phone yet: set the MT3608 output to 5.0-5.2 V with a multimeter.
2. Panel in the sun: TP4056 red LED = charging, blue = full.
3. LCD voltages match the multimeter (else set VREF).
4. A phone on the USB socket shows the charging sign.

---

## Team 7: Digital Temperature Monitoring Station

**Board:** Arduino Uno. The DHT22, LCD, buttons and buzzer all work on 5 V, and the code is ready for the Uno.

**Other option:** ESP32, as an upgrade, to see the readings on a phone over Wi-Fi (the DHT22 and LCD also work on it).

**Code:** [07_Digital_Temperature_Station.ino](07_Digital_Temperature_Station.ino)

**Libraries:** DHT sensor library (Adafruit), LiquidCrystal I2C

### Components to buy

| Part | Qty | Note |
| --- | --- | --- |
| Arduino Uno + USB cable | 1 |  |
| DHT22 temperature and humidity sensor | 1 | DHT11 also works |
| Resistor 10 kΩ | 1 | only if the DHT22 has no board |
| LCD 16x2 with I2C module | 1 | address 0x27 (or 0x3F) |
| Push button | 2 | limit up, limit down |
| LED green + LED red | 1 + 1 | with 220 Ω resistors |
| Passive buzzer | 1 |  |
| Breadboard + jumper wires + a box with air holes | 1 set |  |

### Connections

| From | To |
| --- | --- |
| DHT22 VCC / DATA / GND | 5V / D2 / GND |
| Limit UP / DOWN buttons | D3 / D4 (other legs to GND) |
| Buzzer + | D8 |
| Green / Red LED | D9 / D10 (through 220 Ω) |
| LCD SDA / SCL / VCC / GND | A4 / A5 / 5V / GND |

### Tests, in this order

1. LCD shows room temperature ("Sensor error!" = check DATA wire or DHT type).
2. Each button moves the limit by 0.5 °C.
3. Limit 1 °C above room, hold the sensor: red LED and buzzer.
4. Tools > Serial Plotter shows two lines.

---

## Team 8: Automatic Water Tank Level Controller

**Board:** Arduino Uno. The HC-SR04 and the 5 V relay work directly on the Uno, with no level shifting.

**Other option:** ESP32, as an upgrade, to see the tank level on a phone; the sensor echo pin then needs a voltage divider.

**Code:** [08_Water_Tank_Controller.ino](08_Water_Tank_Controller.ino)

**Libraries:** LiquidCrystal I2C

### Components to buy

| Part | Qty | Note |
| --- | --- | --- |
| Arduino Uno + USB cable | 1 |  |
| Ultrasonic sensor HC-SR04 | 1 | at the top, facing down |
| Relay module 5 V, 1 channel | 1 |  |
| Mini water pump 5 V + tube | 1 |  |
| Separate 5 V supply for the pump | 1 |  |
| LCD 16x2 with I2C module | 1 | address 0x27 (or 0x3F) |
| LED red, yellow, green | 1 each | with 220 Ω resistors |
| Passive buzzer + push button | 1 + 1 |  |
| 2 plastic boxes (tank and well) | 2 | about 20 cm deep |

### Connections

| From | To |
| --- | --- |
| HC-SR04 TRIG / ECHO / VCC / GND | D10 / D11 / 5V / GND |
| Relay IN / VCC / GND | D7 / 5V / GND |
| Pump | through relay COM / NO, own supply |
| Manual button | D2 and GND |
| LEDs low / half / full | D3 / D4 / D5 |
| Buzzer + / LCD SDA / SCL | D8 / A4 / A5 |

### Tests, in this order

1. Write the empty and full distances in EMPTY_CM and FULL_CM.
2. Press the button: the relay clicks (wrong way: RELAY_ACTIVE_LOW = false).
3. Tank below 25 %: pump starts, stops at 90 %.
4. Pump out of water: after 60 s it stops, buzzer and FAULT.

---

## Team 9: Fire Fighting Robot

**Board:** Arduino Uno. It gives the PWM pins the L298N motor driver needs, reads the 3 flame sensors and drives a servo, all on 5 V.

**Other option:** Arduino Nano to save space on the chassis (same pins).

**Code:** [09_Fire_Fighting_Robot.ino](09_Fire_Fighting_Robot.ino)

**Libraries:** Servo (already in the Arduino IDE)

### Components to buy

| Part | Qty | Note |
| --- | --- | --- |
| Arduino Uno + USB cable | 1 |  |
| Robot chassis 2WD with 2 DC motors and wheels | 1 |  |
| L298N motor driver | 1 |  |
| Flame sensor module (with AO pin) | 3 | left, center, right |
| Mini water pump 5 V + tube + small bottle | 1 |  |
| Relay module 5 V, 1 channel | 1 | switches the pump |
| Servo SG90 | 1 | moves the nozzle |
| Passive buzzer | 1 |  |
| 18650 battery x2 + holder + switch | 1 set | 7.4 V for the motors |

### Connections

| From | To |
| --- | --- |
| L298N ENA / IN1 / IN2 | D5 / D7 / D8 (remove ENA, ENB jumpers) |
| L298N ENB / IN3 / IN4 | D6 / D12 / D13 |
| L298N 12V / GND / 5V | battery + / battery - and GND / Arduino 5V |
| Flame sensors AO: left / center / right | A0 / A1 / A2 |
| Pump relay IN / Servo / Buzzer | D4 / D3 / A3 |

### Tests, in this order

1. Wheels in the air: both turn forward (else swap that motor's wires).
2. Candle at 30 cm and 10 cm: set SEE_FIRE and NEAR_FIRE from the readings.
3. Cup under the nozzle: the nozzle sweeps and water comes out.
4. With a TA: candle 1 m away; the robot finds it and puts it out.

---

## Team 10: Smart Pill Reminder Box

**Board:** Arduino Uno. An RTC clock keeps the time without internet, and the LCD, LEDs and buttons all fit the Uno pins.

**Other option:** ESP32, as an upgrade, to send a phone notification to the family when a dose is missed.

**Code:** [10_Smart_Pill_Reminder_Box.ino](10_Smart_Pill_Reminder_Box.ino)

**Libraries:** RTClib (Adafruit), LiquidCrystal I2C

### Components to buy

| Part | Qty | Note |
| --- | --- | --- |
| Arduino Uno + USB cable | 1 |  |
| RTC module DS3231 + CR2032 battery | 1 | keeps the time |
| LCD 16x2 with I2C module | 1 | address 0x27 (or 0x3F) |
| LED + 220 Ω | 4 | one per compartment |
| Push button (or IR sensor under the lid) | 4 | one per compartment |
| Passive buzzer | 1 |  |
| Pill box with 4 compartments | 1 |  |
| Breadboard + jumper wires + power bank | 1 set |  |

### Connections

| From | To |
| --- | --- |
| RTC and LCD SDA / SCL / VCC / GND | A4 / A5 / 5V / GND (shared) |
| Compartment LEDs 1 to 4 | D4, D5, D6, D7 (through 220 Ω) |
| Compartment buttons 1 to 4 | D9, D10, D11, D12 (other legs to GND) |
| Buzzer + | D8 |

### Tests, in this order

1. LCD shows the time; unplug and plug again: time still right.
2. Set a dose 2 minutes ahead: its LED blinks and the buzzer beeps.
3. Wrong compartment: warning beep. Right one: "Well done".
4. With WAIT_MS = 60000 and no press: "Dose MISSED!".
