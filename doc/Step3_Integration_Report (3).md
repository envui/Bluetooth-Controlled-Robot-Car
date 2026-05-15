# Step 3 – Integration & Validation

## 1. Integration Order

The modules were integrated in the following order, building from low-level hardware drivers up to the full application:

1. **PLL** – Integrated first because every other module depends on a stable 50 MHz system clock for correct UART baud rates, PWM timing, and software delays.

2. **LED and Switches** – Integrated next because they provide immediate visual feedback and input capability with no external hardware dependencies beyond the LaunchPad itself. This allowed early verification that Port F GPIO was configured correctly.

3. **Motor (PWM + direction)** – Integrated after LED/Switches to bring up the hardware PWM on PB6/PB7 and direction control on PE0–PE3. Verified by spinning wheels at known duty cycles and confirming forward/backward/pivot direction patterns on the L298N.

4. **CarMotion** – Built on top of Motor to verify high-level motion commands (forward, backward, wide turn, sharp turn, pivot) and the three-level speed system (LO/MD/HI) with differential turn deltas.

5. **BLT_UART1** – Integrated once the motor subsystem was working so that Bluetooth characters from the HC-05 could be received and tested against known motor responses. Verified by pairing a smartphone Bluetooth terminal and confirming single characters were received correctly at 57600 baud with odd parity.

6. **DemoMode** – Integrated after CarMotion and BLT_UART1 so that blocking shape routines could be triggered by Bluetooth commands and observed on the physical car.

7. **ModeControl and CommandParser** – Integrated together because CommandParser routes commands based on the current mode returned by ModeControl. Verified by toggling modes with SW1 and confirming that the same character triggered different behavior in Demo vs Free Driving.

8. **main** – Final integration of all modules into the superloop with SW1 edge detection, debounce, Bluetooth polling, command dispatch, and LED updates.

This bottom-up order was chosen because each layer could be tested independently before adding the next, making it easier to isolate bugs. Hardware drivers were verified before any higher-level logic depended on them.

---

## 2. System Architecture

### 2.1 Final Call Graph

```text
main()
├─ PLL_Init()
├─ LED_Init()
├─ Switches_Init()
├─ BLT_UART1_InitDataMode()
├─ Motor_Init()
│  ├─ PortE_Init()              (PE0–PE3 direction outputs)
│  ├─ PWM0A_Init(10000)         (PB6 M0PWM0 left motor)
│  ├─ PWM0B_Init(10000)         (PB7 M0PWM1 right motor)
│  ├─ Motor_SetDirectionForward()
│  └─ Motor_Stop()
├─ Motor_SetTrim(0, 700)        (right motor compensation)
├─ CarMotion_Init()
│  └─ Car_Stop() → Motor_Stop()
├─ ModeControl_Init()
│  └─ ModeControl_UpdateLED() → LED_Set(LED_GREEN)
│
└─ while(1)
   ├─ SW1_In()
   │  └─ if pressed (edge detected):
   │     ├─ Car_Stop() → Motor_Stop()
   │     ├─ ModeControl_Toggle()
   │     │  └─ ModeControl_UpdateLED() → LED_Set()
   │     └─ DelayMs(100)           (debounce)
   │
   ├─ BLT_UART1_Available()
   │  └─ if data ready:
   │     ├─ BLT_UART1_InChar()
   │     │
   │     ├─ if MODE_DEMO:
   │     │  └─ Command_HandleDemo(cmd)
   │     │     ├─ Demo_Figure8()  → Car_TurnLeft/Right() + DelayMs()
   │     │     ├─ Demo_Circle()   → Car_TurnLeft() + DelayMs()
   │     │     ├─ Demo_Square()   → Car_Forward() + Car_PivotRight() + DelayMs()
   │     │     ├─ Demo_Zigzag()   → Car_PivotLeft/Right() + Car_Forward() + DelayMs()
   │     │     ├─ Demo_Triangle() → Car_Backward/Forward() + Car_PivotLeft/Right() + DelayMs()
   │     │     └─ Demo_FinishCommand()
   │     │        ├─ Car_Stop() → Motor_Stop()
   │     │        └─ BLT_UART1_Flush()
   │     │
   │     └─ if MODE_FREE_DRIVE:
   │        └─ Command_HandleFreeDrive(cmd)
   │           ├─ Car_Forward()    → Motor_SetDirectionForward() + Motor_SetDuty()
   │           ├─ Car_Backward()   → Motor_SetDirectionBackward() + Motor_SetDuty()
   │           ├─ Car_LeftWide()   → Motor_SetDirectionForward() + Motor_SetDuty(hi, lo)
   │           ├─ Car_RightWide()  → Motor_SetDirectionForward() + Motor_SetDuty(lo, hi)
   │           ├─ Car_Stop()       → Motor_Stop()
   │           ├─ Car_SpeedUp()    → update g_speed + Car_ApplyMotion()
   │           └─ Car_SlowDown()   → update g_speed + Car_ApplyMotion()
   │
   └─ ModeControl_UpdateLED()
```

### 2.2 Data Flow Diagram

```text
+-------------------------+                         +-------------------+
| Smartphone/PC Bluetooth |  --- 2.4GHz wireless -->| HC-05 (data mode) |
| Terminal App            |                         +--------+----------+
+-------------------------+                                  |
                                                             | UART1 (57600, odd parity)
                                                             v
                                                  +----------+----------+
                                                  | BLT_UART1 (PB0 RX) |
                                                  | Available() / InChar()|
                                                  +----------+----------+
                                                             |
                                                             | single char command
                                                             v
                                                  +----------+----------+
                                                  |       main()        |
                                                  | mode check + dispatch|
                                                  +---+------------+----+
                                                      |            |
                                      MODE_DEMO       |            |    MODE_FREE_DRIVE
                                                      v            v
                                          +-----------+--+  +------+-----------+
                                          | CommandParser|  | CommandParser     |
                                          | HandleDemo() |  | HandleFreeDrive()|
                                          +-----------+--+  +------+-----------+
                                                      |            |
                                                      v            v
                                          +-----------+--+  +------+-----------+
                                          | DemoMode     |  | CarMotion        |
                                          | (blocking    |  | Forward/Back/    |
                                          |  sequences)  |  | Turn/Speed       |
                                          +-----------+--+  +------+-----------+
                                                      |            |
                                                      +------+-----+
                                                             |
                                                             v
                                                  +----------+----------+
                                                  |     CarMotion       |
                                                  | Car_ApplyMotion()   |
                                                  | g_speed, g_motion   |
                                                  +----------+----------+
                                                             |
                                                             v
                                                  +----------+----------+
                                                  |       Motor         |
                                                  | Direction: PE0–PE3  |
                                                  | PWM: PB6 (L), PB7 (R)|
                                                  | Trim: (0, 700)      |
                                                  +----------+----------+
                                                             |
                                                             v
                                                  +----------+----------+
                                                  |   L298N → DC Motors |
                                                  +---------------------+


+-------------------+
| SW1 (PF4, polled) |-----> main() edge detect + 100ms debounce
+-------------------+           |
                                v
                     +----------+----------+
                     |    ModeControl      |
                     | Toggle() + Get()    |
                     +----------+----------+
                                |
                                v
                     +----------+----------+
                     |       LED           |
                     | Green=Demo          |
                     | Blue=Free Driving   |
                     +---------------------+
```

---

## 3. System-Level Validation

| Requirement | Test Method | Evidence | Pass/Fail |
|-------------|------------|----------|-----------|
| RQ-01 (Startup initialization) | Power cycle the board and observe LED and motor state | Green LED turns on, motors are off, system is responsive to Bluetooth | Pass |
| RQ-06 (Power up in Demo, SW1 toggles) | Reset system, observe LED color, press SW1 multiple times | Green LED on at startup; toggles to blue then back to green on each SW1 press | Pass |
| RQ-07 (Green = Demo, Blue = Free Driving) | Observe LED after each mode toggle | LED color matches mode after every SW1 press | Pass |
| RQ-08 (Demo: '8', 'C', 'S', 'Z' then stop) | Send each demo command from Bluetooth terminal and observe car path | Car traces the expected shape and stops after each routine | Pass |
| RQ-09 (No mid-routine interruption) | Send extra characters during a demo routine | Extra characters are ignored; car completes the shape, then UART RX is flushed | Pass |
| RQ-10 (Free Driving: 'F', 'B', 'L', 'R', 'S') | Send each Free Driving command and observe car movement | Car moves in the correct direction for each command; stops on 'S' | Pass |
| RQ-11 (Speed up 'U', slow down 'D') | Send 'F' then 'U' repeatedly, then 'D' repeatedly | Visible speed increase and decrease in wheel rotation | Pass |
| RQ-12 (Speed bounded LO/MD/HI) | Send 'U' many times beyond HI or 'D' many times beyond LO | Speed does not exceed HI (8000) or drop below LO (3500); no stall or runaway | Pass |
| RQ-13 (Speed persists across direction changes) | Speed up to HI, then send 'L' for wide left turn | Car turns at HI speed without resetting to MD | Pass |
| RQ-14 (Hardware PWM) | Measure PB6 and PB7 with oscilloscope at known duty cycle | PWM waveform matches expected period (10000) and duty cycle; hardware-generated | Pass |
| RQ-15 (Defined stop state) | Send 'S' in Free Driving; observe stop after demo; press SW1 during motion | Motors stop in all cases; direction pins go low or duty goes to MOTOR_STOP (1) | Pass |
| RQ-16 (Invalid commands ignored) | Send 'X', '#', '!' in both modes | No motor movement or mode change; system continues operating normally | Pass |

---

## 4. Robustness Test

### Test 1: Rapid repeated SW1 presses during motion
- **What was tested:** SW1 was pressed rapidly many times while the car was moving forward in Free Driving mode, to check for debounce failures or mode corruption.
- **Expected behavior:** Each press should produce exactly one mode toggle, the car should stop on each toggle, and the LED should alternate green/blue cleanly.
- **Actual result:** The 100 ms software debounce and edge detection (`lastSW1` variable) prevented double toggles. The car stopped on each press, mode toggled once per press, and the LED updated correctly. No mode corruption observed.

### Test 2: Bluetooth commands sent during a blocking demo routine
- **What was tested:** While the car was executing a square demo ('S'), multiple characters ('F', 'B', 'U') were sent rapidly from the Bluetooth terminal.
- **Expected behavior:** The demo routine should complete without interruption. Characters received during the blocking routine should accumulate in the UART1 hardware FIFO and be flushed by `Demo_FinishCommand` after the routine finishes.
- **Actual result:** The square pattern completed normally. After the routine finished, `BLT_UART1_Flush` cleared all buffered characters. The car stopped cleanly and waited for the next command. No stray movement occurred from the buffered characters.

### Test 3: Invalid and mixed-case characters
- **What was tested:** Characters not in the command set ('X', '0', '!', '@') and lowercase versions of valid commands ('f', 'b', 'l') were sent in both modes.
- **Expected behavior:** Invalid characters should be silently ignored. Lowercase valid commands should work identically to uppercase.
- **Actual result:** Invalid characters produced no motor movement or state change. Lowercase commands ('f', 'b', 'l', 'r', 's', 'u', 'd', 'c', 's', 'z', 't') were accepted and produced the same behavior as their uppercase counterparts. The `default` case in both switch statements handled unknown characters safely.

---

## 5. Claim–Evidence–Reasoning (CER)

### Claim 1: The system correctly uses hardware PWM to control motor speed.
- **Evidence:** The Motor module configures PWM Module 0 Generator 0 on PB6 (M0PWM0, GENA) and PB7 (M0PWM1, GENB) with a period of 10000 and a clock divider of 2. Oscilloscope measurements on PB6 and PB7 show hardware-generated PWM waveforms at the expected frequency and duty cycle. No software toggle loops are used for speed control.
- **Reasoning:** The TM4C123 PWM peripheral generates the signals autonomously after the comparator registers are written, confirming hardware PWM per RQ-14. The `PWM0_0_CMPA_R` and `PWM0_0_CMPB_R` registers directly control duty cycle without CPU intervention during each PWM cycle.

### Claim 2: Demo routines complete without mid-execution command interruption.
- **Evidence:** Demo functions (`Demo_Figure8`, `Demo_Circle`, `Demo_Square`, `Demo_Zigzag`, `Demo_Triangle`) are blocking — they do not return until the full shape sequence is finished. After each demo routine, `Demo_FinishCommand` calls `Car_Stop()` and `BLT_UART1_Flush()` to stop the car and discard any characters that arrived during execution. Testing confirmed that sending extra commands during a demo had no effect on the running pattern.
- **Reasoning:** Because the demo functions block the main loop, `BLT_UART1_Available` is never checked and `BLT_UART1_InChar` is never called until the routine returns. Any characters that arrive during the demo sit in the UART1 hardware FIFO and are explicitly flushed before the next command is read, satisfying RQ-09.

### Claim 3: Speed persists across direction changes in Free Driving mode.
- **Evidence:** CarMotion maintains a static `g_speed` variable that is only modified by `Car_SpeedUp` and `Car_SlowDown`. All direction functions (`Car_Forward`, `Car_Backward`, `Car_LeftWide`, `Car_RightWide`) call `Car_ApplyMotion`, which uses the current `g_speed` value to set motor duty cycles. Testing confirmed that after increasing speed to HI and then sending 'L' for a wide left turn, the car turned at HI speed.
- **Reasoning:** The speed state is decoupled from the motion state. Changing direction updates `g_motion` but not `g_speed`, so the previously selected speed level is reapplied automatically by `Car_ApplyMotion`, satisfying RQ-13.

### Claim 4: The mode toggle system correctly switches between Demo and Free Driving with accurate LED indication.
- **Evidence:** `ModeControl_Init` sets the default mode to `MODE_DEMO` (0) and calls `ModeControl_UpdateLED` which sets the green LED. `ModeControl_Toggle` flips the mode between `MODE_DEMO` and `MODE_FREE_DRIVE` (1) and immediately updates the LED. The main loop detects SW1 presses using edge detection (`lastSW1` transitions from 0 to 1) with a 100 ms debounce delay. Testing confirmed green LED at startup and correct green/blue alternation on each SW1 press.
- **Reasoning:** The edge detection prevents repeated toggles while SW1 is held, and the debounce delay prevents bounce-triggered double toggles. The car is stopped before each toggle (`Car_Stop` is called before `ModeControl_Toggle`) so the mode change does not leave motors in an inconsistent state, satisfying RQ-06 and RQ-07.

### Claim 5: Invalid Bluetooth commands are safely ignored in both modes.
- **Evidence:** `Command_HandleDemo` and `Command_HandleFreeDrive` both use switch statements with explicit cases for valid commands and a `default` case that does nothing (empty `break`). Testing with characters 'X', '#', '!', and '0' in both modes produced no motor movement, no mode change, and no system lockup.
- **Reasoning:** The switch-case structure ensures that only recognized command characters trigger any action. All other characters fall through to `default` and are discarded, leaving `g_motion`, `g_speed`, and `g_mode` unchanged, satisfying RQ-16.

---

## 6. Final Demo Summary

- **What was demonstrated:**
  The complete Part 2 Bluetooth Controlled Robot Car was demonstrated with both operating modes. In Mode 1 (Demo), all five shape commands were executed: figure 8 ('8'), circle ('C'), square ('S'), zigzag ('Z'), and the bonus triangle ('T'). In Mode 2 (Free Driving), all directional commands ('F', 'B', 'L', 'R', 'S') and speed commands ('U', 'D') were demonstrated. Mode toggling with SW1 and LED indication (green for Demo, blue for Free Driving) were shown. The motor trim compensation (`Motor_SetTrim(0, 700)`) corrected a right motor imbalance so the car drove straight.

- **Key results:**
  All demo shapes completed and stopped correctly. Free Driving commands responded immediately with visible speed changes across the three levels (LO=3500, MD=6000, HI=8000). Speed persisted across direction changes. Invalid commands were ignored. Mid-demo commands were flushed after routine completion. SW1 toggling was clean with no double-toggle or mode corruption.

- **Observations:**
  Demo shape accuracy depends on surface friction and battery voltage — timing values were tuned empirically and may need adjustment on different surfaces. The right motor trim of 700 was specific to the hardware used and would need recalibration for a different chassis. The busy-wait delay loop (13333 iterations per ms at 50 MHz) provided adequate timing for demo patterns but is not precise enough for time-critical applications.

---

## 7. AI Verification Summary

- **Debugging assistance:**
  AI was used to review the Motor module's PWM initialization sequence and confirm that the CMPA/CMPB register writes, generator control register values (GENA=0xC8, GENB using ACTCMPBD_ONE and ACTLOAD_ZERO), and clock divider settings were correct for the intended PWM behavior. AI also helped verify that the bit-banded direction address `0x4002403C` correctly maps to PE0–PE3 for the L298N direction patterns.

- **Verification support:**
  AI was used to trace the full call graph from `main` through `CommandParser`, `DemoMode`, `CarMotion`, and `Motor` to verify that every Bluetooth command reaches the correct motor output. AI also verified that the `Demo_FinishCommand` flush path correctly prevents stale commands from affecting the system after a demo routine. The traceability table mapping RQ-01 through RQ-16 to modules and tests was generated with AI assistance.

- **One rejected AI suggestion:**
  AI suggested replacing the polling-based UART1 reception with an interrupt-driven approach using a software FIFO to eliminate the risk of missing characters during the main loop. This was rejected because the polling design is simpler, the Bluetooth commands are single characters sent at human speed, and the blocking demo routines intentionally prevent new command processing during execution. The explicit `BLT_UART1_Flush` call after each demo already handles buffered characters safely. Adding UART interrupts would increase complexity without a meaningful benefit for this application.

---
