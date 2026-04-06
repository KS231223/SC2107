# RSLK MSP432 Self-Test (UART Menu) – Lab 5

This project is an **edited version of the lab files (Run with Lab 5 UART main file)** for the TI RSLK robot using the MSP432 microcontroller.

It implements a **menu-driven self-test system over UART**, allowing users to test motors, sensors, and robot behaviors interactively.

---

## 📌 Important Note

⚠️ This file **is the Lab 5 UART main file** (modified).

* You should **build and run this file directly**
* No need to switch to another main file
* All Lab 5 testing functionality is integrated here

---

## 📌 Features

* UART command-line menu
* Motor movement tests
* Line-following FSM
* Reflectance sensor usage
* Tachometer-based turning
* Bump sensor reading
* Modular test cases for Lab development

---

## 🧠 System Overview

The program initializes core peripherals and continuously displays a UART menu. Users input commands to trigger different test routines.

### Menu Options

```
[0] RSLK Reset
[1] Motor Test
[2] IR Sensor Test / Line Following
[3] Bumper Test / Turning Test
[4] Reflectance Sensor Test
[5] Tachometer Test
```

---

## ⚙️ Initialization

The following modules are initialized:

* Clock (48 MHz)
* UART (EUSCIA0)
* Motor driver
* LaunchPad I/O
* Tachometer

```c
Clock_Init48MHz();
Motor_Init();
LaunchPad_Init();
Tachometer_Init();
EUSCIA0_Init();
```

---

## 🔄 Main Loop

The system runs an infinite loop:

1. Display menu via UART
2. Wait for user input
3. Execute corresponding test case

---

## 🚗 Key Functionalities

### 1. Motor Test (`CASETWO`)

* Moves forward
* Detects line using reflectance sensors
* Performs corrective maneuvers (left/right/backward)

---

### 2. Line Following FSM (`CASEFOLLOWBLACK`)

Implements a **finite state machine**:

| State  | Action       |
| ------ | ------------ |
| Center | Move forward |
| Left   | Turn left    |
| Right  | Turn right   |

* Input from reflectance sensors
* Output controls motor direction

---

### 3. Turning Control (`TurnAngle`)

Uses tachometer feedback to rotate the robot by a given angle.

#### Key Logic:

* Tracks wheel steps
* Converts steps → distance
* Computes angular rotation

```c
currentAngle = (distanceDifference * 360) / baseCircumference;
```

---

### 4. Reflectance Sensor

```c
Reflectance_Read(1000);
Reflectance_Center(1000);
Reflectance_Number(...);
```

Used for:

* Line detection
* Path correction
* Black line counting

---

### 5. Bump Sensor

```c
bump = Bump_Read();
```

Detects collisions or obstacles.

---

### 6. LED + Switch Test

* Uses onboard switches (P1.1, P1.4)
* Controls RGB LEDs (P2.0–P2.2)

---

## ⏱ Interrupt-Based Sensor Sampling

```c
void SensorRead_ISR(void)
```

* Runs at **2 kHz**
* Reads IR sensors via ADC
* Applies low-pass filtering
* Sets `ADCflag` semaphore

---

## 🔌 UART Interface

Functions used:

* `EUSCIA0_OutString()`
* `EUSCIA0_InUDec()`
* `EUSCIA0_OutChar()`

Provides a simple CLI for user interaction.

---

## 🧪 Example Workflow

1. Connect to UART terminal
2. Flash this program to MSP432
3. Open serial monitor
4. Enter a command number
5. Observe robot behavior

---

## 📁 Dependencies

Header files required:

```
UART0.h
EUSCIA0.h
FIFO0.h
Clock.h
SysTickInts.h
CortexM.h
TimerA1.h
Bump.h
Motor.h
IRDistance.h
ADC14.h
LPF.h
Reflectance.h
TA3InputCapture.h
Tachometer.c
```

---

## ⚠️ Notes

* This is a **modified Lab 5 main file**
* Tachometer calculations use scaled constants:

  * Wheel circumference = 7000
  * Base circumference = 14500
* Some modules are partially implemented or commented out
* Timing uses blocking delays (`Clock_Delay1ms`)

---

## 🧩 Future Improvements

* Add full test coverage for all menu options
* Improve turning accuracy (PID control)
* Replace blocking delays with interrupt-driven logic
* Add obstacle avoidance

---

## 📜 License

BSD License (FreeBSD-style)
© 2017 Jonathan Valvano

---

## 👨‍🏫 Reference

Based on course material from:

**Embedded Systems: Introduction to the MSP432 Microcontroller**
by Jonathan Valvano

http://users.ece.utexas.edu/~valvano/

---
