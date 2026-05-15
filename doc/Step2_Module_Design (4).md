# Step 2 – Module Design & Implementation

## 1. Module Decomposition

| Module Name | Responsibility | Inputs | Outputs |
|-------------|---------------|--------|---------|
| PLL | Configures system clock to 50 MHz using the PLL | Power-on/reset | 50 MHz system clock for all peripherals |
| LED | Onboard RGB LED driver (PF1 red, PF2 blue, PF3 green) | Color code (LED_RED, LED_BLUE, LED_GREEN, LED_OFF) | LED color output on PF1–PF3 |
| Switches | SW1 input driver on PF4 with pull-up (negative logic) | Physical SW1 press | Returns 1 when pressed, 0 when released |
| BLT_UART1 | UART1 driver for HC-05 Bluetooth module in data mode (PB0 RX, PB1 TX) | Bluetooth serial bytes from HC-05 at 57600 baud, odd parity | Received characters; data-available flag; RX FIFO flush |
| Motor | Low-level motor driver: direction pins on Port E (PE0–PE3) and hardware PWM on PB6/PB7 (M0PWM0/M0PWM1) | Direction commands; duty cycle values (0–10000); trim offsets | PWM signals to L298N ENA/ENB; GPIO direction signals to L298N IN1–IN4 |
| CarMotion | High-level motion control: translates motion commands into Motor calls with differential speed for turns | Motion type (forward, backward, wide turn, sharp turn, pivot, etc.); speed up/down requests | Motor direction and duty cycle settings; maintains current speed state (LO/MD/HI) |
| DemoMode | Blocking shape-drawing routines for Mode 1 | Demo command trigger from CommandParser | Sequences of CarMotion calls with software delays to trace figure 8, circle, square, zigzag, triangle |
| CommandParser | Maps single-character Bluetooth commands to CarMotion or DemoMode actions based on current mode | Command character; current mode (via caller in main) | Calls to DemoMode routines or CarMotion functions; flushes UART RX after demo completion |
| ModeControl | Stores current mode (MODE_DEMO or MODE_FREE_DRIVE) and updates LED to match | Toggle request from main loop | Current mode value; LED color update (green = Demo, blue = Free Driving) |
| main | Top-level application: initializes all modules, runs superloop with SW1 polling, Bluetooth command dispatch | All module outputs; SW1 state; Bluetooth characters | Calls to CommandParser and ModeControl based on input |

---

## 2. Implementation Summary

### PLL
- **Key functions:** `PLL_Init(void)`
- **Logic:** Uses RCC2 register to configure the TM4C123 PLL with a 16 MHz crystal input, 400 MHz PLL output divided to 50 MHz (divisor 7+1=8 in SYSDIV2 field). Bypasses PLL during setup, waits for PLLRIS lock bit, then enables PLL output.
- **Assumptions:** 16 MHz external crystal is present on the LaunchPad.

### LED
- **Key functions:** `LED_Init(void)`, `LED_Set(uint8_t color)`, `LED_Off(void)`
- **Logic:** Initializes PF1–PF3 as digital GPIO outputs. Unlocks Port F and enables changes on PF1–PF3 via the commit register. `LED_Set` writes the color bits to PF1–PF3 using a read-modify-write with mask `0x0E`. Defines constants `LED_RED` (0x02), `LED_BLUE` (0x04), `LED_GREEN` (0x08).
- **Assumptions:** Port F clock is enabled during init. Only PF1–PF3 are used for LED output.

### Switches
- **Key functions:** `Switches_Init(void)`, `SW1_In(void)`
- **Logic:** Initializes PF4 as a digital input with internal pull-up resistor. Unlocks Port F and enables changes on PF4. `SW1_In` returns 1 when PF4 reads low (negative logic, switch pressed) and 0 otherwise.
- **Assumptions:** SW1 is the onboard switch on PF4 with negative logic. No hardware debounce; debounce is handled in main with a 100 ms software delay.

### BLT_UART1
- **Key functions:** `BLT_UART1_InitDataMode(void)`, `BLT_UART1_InChar(void)`, `BLT_UART1_Available(void)`, `BLT_UART1_Flush(void)`
- **Logic:** Initializes UART1 on PB0 (RX) and PB1 (TX) for 57600 baud with a 50 MHz clock (IBRD=54, FBRD=16). Configures 8-bit data, odd parity, and FIFO enabled. All reception is polling-based: `Available` checks the RXFE flag, `InChar` busy-waits until a character arrives, and `Flush` drains the RX FIFO by reading and discarding all pending bytes.
- **Assumptions:** HC-05 has already been configured to 57600 baud, odd parity, 1 stop bit using the Part 1 setup program. EN/KEY pin is disconnected for data mode.

### Motor
- **Key functions:** `Motor_Init(void)`, `Motor_Stop(void)`, `Motor_SetDirectionForward(void)`, `Motor_SetDirectionBackward(void)`, `Motor_SetDirectionPivotLeft(void)`, `Motor_SetDirectionPivotRight(void)`, `Motor_SetLeftDuty(uint16_t)`, `Motor_SetRightDuty(uint16_t)`, `Motor_SetDuty(uint16_t, uint16_t)`, `Motor_SetTrim(int16_t, int16_t)`
- **Logic:** Direction control uses PE0–PE3 mapped to L298N IN1–IN4 via bit-banded address `0x4002403C`. Direction patterns: forward=0x0F, backward=0x0A, left pivot=0x06, right pivot=0x09. PWM uses Module 0 Generator 0: PB6 as M0PWM0 (left motor, CMPA) and PB7 as M0PWM1 (right motor, CMPB). PWM period is 10000 with a PWM clock divider of 2. Trim values are added to duty cycle to compensate for motor imbalance, clamped between `MOTOR_STOP` (1) and `MOTOR_PERIOD-1` (9999). Defines three speed presets: `MOTOR_SPEED_LO` (3500), `MOTOR_SPEED_MD` (6000), `MOTOR_SPEED_HI` (8000).
- **Assumptions:** L298N ENA connected to PB6, ENB connected to PB7. L298N IN1–IN4 connected to PE0–PE3.

### CarMotion
- **Key functions:** `CarMotion_Init(void)`, `Car_Stop(void)`, `Car_Forward(void)`, `Car_Backward(void)`, `Car_LeftWide(void)`, `Car_RightWide(void)`, `Car_TurnLeft(void)`, `Car_TurnRight(void)`, `Car_BackLeft(void)`, `Car_BackRight(void)`, `Car_PivotLeft(void)`, `Car_PivotRight(void)`, `Car_SpeedUp(void)`, `Car_SlowDown(void)`, `Car_GetSpeed(void)`
- **Logic:** Maintains a speed state `g_speed` (initialized to `MOTOR_SPEED_MD`) and a motion state `g_motion`. Wide turns use a delta of 2000 subtracted from one wheel; sharp turns use a delta of 3500. Pivot turns use `MOTOR_SPEED_HI` on both wheels with opposing directions. `Car_SpeedUp` steps through LO→MD→HI; `Car_SlowDown` steps HI→MD→LO. Speed is clamped to three discrete levels. `Car_ApplyMotion` reapplies the current motion type whenever speed or direction changes, ensuring speed persists across direction changes.
- **Assumptions:** Motor module is initialized before CarMotion_Init is called.

### DemoMode
- **Key functions:** `Demo_Figure8(void)`, `Demo_Circle(void)`, `Demo_Square(void)`, `Demo_Zigzag(void)`, `Demo_Triangle(void)`
- **Logic:** Each routine is a blocking function that calls CarMotion direction functions with software delays (`DelayMs`) between segments. Figure 8: alternates left/right sharp turns (750 ms each) for 2 iterations. Circle: left sharp turn for 2200 ms. Square: forward 600 ms then pivot right 44 ms, repeated 5 times. Zigzag: alternating pivot turns and forward segments. Triangle (bonus): backward, pivot right, forward, pivot left, forward sequence. `DelayMs` uses a busy-wait loop calibrated for a 50 MHz clock (loop count 13333 per ms).
- **Assumptions:** Timing values are tuned empirically on the physical car. Blocking execution naturally prevents new commands during a demo.

### CommandParser
- **Key functions:** `Command_HandleDemo(char cmd)`, `Command_HandleFreeDrive(char cmd)`
- **Logic:** `Command_HandleDemo` maps '8', 'C'/'c', 'S'/'s', 'Z'/'z', 'T'/'t' to their corresponding DemoMode routines, then calls `Demo_FinishCommand` which stops the car and flushes the UART1 RX FIFO to discard any characters received during the blocking demo. `Command_HandleFreeDrive` maps 'F'/'f', 'B'/'b', 'L'/'l', 'R'/'r', 'S'/'s', 'U'/'u', 'D'/'d' to CarMotion functions. Both functions accept uppercase and lowercase. Unrecognized characters fall through the default case and are silently ignored.
- **Assumptions:** Mode checking is done by the caller (main); CommandParser only handles dispatch.

### ModeControl
- **Key functions:** `ModeControl_Init(void)`, `ModeControl_Toggle(void)`, `ModeControl_Get(void)`, `ModeControl_UpdateLED(void)`
- **Logic:** Maintains a static `g_mode` variable, initialized to `MODE_DEMO` (0). `Toggle` switches between `MODE_DEMO` and `MODE_FREE_DRIVE` (1) and updates the LED. `UpdateLED` sets green LED for Demo mode and blue LED for Free Driving mode via `LED_Set`.
- **Assumptions:** LED module is initialized before ModeControl_Init is called.

### main
- **Key functions:** `main(void)`
- **Logic:** Calls all init functions in order: `PLL_Init`, `LED_Init`, `Switches_Init`, `BLT_UART1_InitDataMode`, `Motor_Init`, `Motor_SetTrim(0, 700)` (right motor trim compensation), `CarMotion_Init`, `ModeControl_Init`. Enters superloop that: (1) polls SW1 with edge detection using `lastSW1` variable and 100 ms debounce delay, stops the car and toggles mode on press; (2) checks `BLT_UART1_Available` and reads a character; (3) dispatches to `Command_HandleDemo` or `Command_HandleFreeDrive` based on `ModeControl_Get`; (4) calls `ModeControl_UpdateLED` after each command.
- **Assumptions:** All modules are initialized before the superloop begins. Motor trim is hardware-specific (right motor reduced by 700).

---

## 3. Module Testing

Each module must have at least **1 valid test**.

### Test Cases

| Module | Test ID | Description | Evidence |
|--------|---------|-------------|----------|
| PLL | T-01 | Power cycle the board and verify the system runs at 50 MHz by confirming UART communication works correctly at 57600 baud. | Bluetooth commands received and processed correctly, confirming clock accuracy. |
| LED | T-02 | Call `LED_Set(LED_GREEN)` then `LED_Set(LED_BLUE)` and confirm the correct LED color appears on the LaunchPad. | Visual observation of green then blue LED on the board. |
| Switches | T-03 | Press and release SW1 and confirm `SW1_In()` returns 1 when pressed and 0 when released. | Debug observation or LED toggle confirming correct press detection. |
| BLT_UART1 | T-04 | Pair smartphone Bluetooth terminal with HC-05, send a character, and confirm `BLT_UART1_Available` returns 1 and `BLT_UART1_InChar` returns the correct character. | Correct character received and car responds to the command. |
| Motor | T-05 | Call `Motor_SetDirectionForward()` and `Motor_SetDuty(6000, 6000)` and verify both wheels spin forward; call `Motor_Stop()` and verify both wheels stop. | Video of car wheels spinning forward then stopping. |
| CarMotion | T-06 | Call `Car_Forward()`, then `Car_SpeedUp()` twice, then `Car_SlowDown()` once, and verify speed changes are visible in wheel rotation speed. | Video showing speed increase (MD→HI) and decrease (HI→MD). |
| DemoMode | T-07 | Send 'S' from Bluetooth terminal in Demo mode and verify the car traces a square path (5 forward+pivot iterations) and stops; send another character mid-routine and verify it has no effect (flushed after completion). | Video of square path; no response to mid-demo commands. |
| CommandParser | T-08 | In Free Driving mode, send 'F', 'S', 'X' from Bluetooth terminal and verify car moves forward on 'F', stops on 'S', and ignores 'X'. Also verify lowercase 'f' works identically to 'F'. | Observation: car moves, stops, ignores invalid; lowercase accepted. |
| ModeControl | T-09 | Reset the system and confirm green LED is on (Demo mode). Press SW1 and confirm blue LED turns on (Free Driving). Press SW1 again and confirm green returns. | Photo or video of LED states matching mode on each SW1 press. |
| main | T-10 | Full integration test: power on, confirm green LED. Send '8' and verify figure-8 completes and car stops. Press SW1 (blue LED). Send 'F' then 'U' then 'L' and verify forward, speed increase, and wide left turn. Send 'S' to stop. | Video demonstrating both modes, mode switching, and multiple commands. |

---

## 4. Traceability Summary

Refer to Step 1 Requirements (RQ-01 through RQ-16) and CECS 447 Project 3 description.

| Requirement | Modules Involved | Test IDs |
|-------------|-----------------|----------|
| RQ-01 (Startup initialization and status) | PLL, LED, Switches, BLT_UART1, Motor, CarMotion, ModeControl, main | T-01, T-09 |
| RQ-02 (Communication paths) | BLT_UART1 | T-04 |
| RQ-06 (Power up in Demo, SW1 toggles modes) | ModeControl, Switches, main | T-03, T-09, T-10 |
| RQ-07 (Green LED = Demo, Blue LED = Free Driving) | LED, ModeControl | T-02, T-09 |
| RQ-08 (Demo commands: '8', 'C', 'S', 'Z' then stop) | DemoMode, CommandParser, CarMotion | T-07, T-10 |
| RQ-09 (No mid-routine interruption; UART flush after demo) | DemoMode, CommandParser (Demo_FinishCommand), BLT_UART1 | T-07 |
| RQ-10 (Free Driving: 'F', 'B', 'L', 'R', 'S') | CommandParser, CarMotion | T-08, T-10 |
| RQ-11 (Speed up 'U', slow down 'D' via PWM) | CommandParser, CarMotion, Motor | T-06, T-10 |
| RQ-12 (Speed bounded LO/MD/HI) | CarMotion (Car_SpeedUp, Car_SlowDown) | T-06 |
| RQ-13 (Speed persists across direction changes) | CarMotion (Car_ApplyMotion reuses g_speed) | T-06, T-10 |
| RQ-14 (Hardware PWM for DC motors) | Motor (M0PWM0 on PB6, M0PWM1 on PB7) | T-05 |
| RQ-15 (Defined stop state) | Motor (Motor_Stop), CarMotion (Car_Stop), CommandParser (Demo_FinishCommand) | T-05, T-07, T-08 |
| RQ-16 (Invalid commands ignored) | CommandParser (default case in both handlers) | T-08 |

**Note:** RQ-03, RQ-04, and RQ-05 (HC-05 AT command setup) relate to the Part 1 setup program, which is a separate project. The code files provided cover Part 2 only.

---