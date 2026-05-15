# Step 1 – Requirements & System Design

*Prepared from the project specification and workflow documents.*

---

## 1. Requirements

**RQ-01**  
Upon power-up, the MCU shall initialize the required GPIO, UART, and PWM resources and display a startup/status message on the PC serial terminal for the HC-05 setup portion of the project.  
**Verification:** Power cycle the system and confirm the startup prompt appears correctly on the serial terminal.  
**Related Modules:** UART0 terminal interface, system initialization, Bluetooth setup UI.

**RQ-02**  
The system shall use the following communication paths:
- **PC Serial Terminal ↔ MCU** for HC-05 setup interaction
- **MCU UART1 ↔ HC-05** for both AT command mode and Bluetooth data mode
- **Bluetooth Terminal App ↔ HC-05** for wireless car control during operation  
**Verification:** Demonstrate successful command/response behavior on each communication link.  
**Related Modules:** UART0 terminal interface, UART1 Bluetooth driver, Bluetooth setup, Bluetooth command parser.

**RQ-03**  
The system shall support configuration of the HC-05 module in **AT Command Mode** when EN/KEY is held high before power-up.  
**Verification:** Send `AT` and verify an `OK` response is received from the module.  
**Related Modules:** Bluetooth setup UI, UART1 Bluetooth driver.

**RQ-04**  
The setup program shall allow configuration of the HC-05 with the following required parameters:
- Bluetooth name = user/team-defined name
- UART = **57600 baud, 8 data bits, 1 stop bit, odd parity**
- Passcode = custom value, not `1234`
- Role = **Slave**  
**Verification:** Use AT query commands and record matching responses in the serial terminal log.  
**Related Modules:** Bluetooth setup UI, UART1 Bluetooth driver.

**RQ-05**  
After initial communication is confirmed, the setup program shall remain in a superloop waiting for user-entered AT commands from the PC terminal and shall display the Bluetooth module responses back to the terminal.  
**Verification:** Enter multiple AT commands from the terminal and confirm the correct response is printed for each.  
**Related Modules:** Bluetooth setup UI, UART0 terminal interface, UART1 Bluetooth driver.

**RQ-06**  
The robot car shall power up in **Mode 1 (Demo)** by default. Pressing **SW1** on the LaunchPad shall toggle between **Mode 1 (Demo)** and **Mode 2 (Free Driving)**.  
**Verification:** Reset the system and confirm it starts in Demo mode; press SW1 and confirm the system changes modes every press.  
**Related Modules:** Mode manager, switch input, LED status.

**RQ-07**  
The active mode shall be indicated using onboard LEDs:
- **Green LED ON** in Mode 1 (Demo)
- **Blue LED ON** in Mode 2 (Free Driving)  
**Verification:** Observe LED state after reset and after each SW1 press.  
**Related Modules:** LED status, mode manager.

**RQ-08**  
In Mode 1 (Demo), the robot car shall accept the following single-character commands and execute the corresponding predefined motion routine:
- `'8'` → figure 8
- `'C'` → circle
- `'S'` → square
- `'Z'` → zigzag with 4 segments  
After the routine completes, the car shall stop.  
**Verification:** Send each command and confirm the expected path is completed and the car stops afterward.  
**Related Modules:** Demo mode controller, motion sequencer, motor control.

**RQ-09**  
While a demo routine is executing, new incoming commands shall either be ignored or held until the current routine has fully completed, so the path is not interrupted mid-execution.  
**Verification:** Send extra commands while a shape is being drawn and confirm the current path completes correctly without corruption.  
**Related Modules:** Demo mode controller, Bluetooth command parser.

**RQ-10**  
In Mode 2 (Free Driving), the robot car shall accept the following single-character commands for real-time movement:
- `'F'` → forward
- `'B'` → reverse
- `'L'` → left wide turn
- `'R'` → right wide turn
- `'S'` → stop  
**Verification:** Send each command individually and observe the matching car movement.  
**Related Modules:** Free driving controller, motor control.

**RQ-11**  
In Mode 2 (Free Driving), the robot car shall support:
- `'U'` → speed up
- `'D'` → slow down  
These commands shall modify motor speed by adjusting PWM duty cycle.  
**Verification:** Send repeated `U` and `D` commands and observe clear speed changes.  
**Related Modules:** Free driving controller, PWM motor control.

**RQ-12**  
Speed control shall remain within a bounded minimum and maximum duty-cycle range so the car does not stall at very low duty cycle or exceed a defined safe operating speed.  
**Verification:** Continue sending `U` and `D` commands and confirm duty cycle clamps at the defined limits.  
**Related Modules:** Free driving controller, PWM motor control.

**RQ-13**  
The current selected speed level in Free Driving mode shall persist across direction changes until changed again by another speed command or a mode reset policy defined by the implementation.  
**Verification:** Increase or decrease speed, then send new directional commands and confirm the updated speed is retained.  
**Related Modules:** Free driving controller, PWM motor control.

**RQ-14**  
Both DC motors shall be controlled using **hardware-generated PWM** through the L298N motor driver. Software-generated PWM shall not be used for normal motor speed control.  
**Verification:** Review implementation and demonstrate that PWM peripheral outputs are used for speed control.  
**Related Modules:** PWM motor control.

**RQ-15**  
The system shall provide a defined stop state in which both motors are disabled or commanded to zero drive so the robot car stops motion safely when:
- the `S` command is received
- a demo routine finishes
- the system transitions to a safe idle state  
**Verification:** Send `S` during Free Driving and verify motors stop; verify stop occurs after demo completion.  
**Related Modules:** Motor control, demo mode, free driving mode.

**RQ-16**  
Invalid, unsupported, or unrecognized Bluetooth command characters shall be ignored without causing unintended motor movement, mode corruption, or system lockup.  
**Verification:** Send unsupported characters and confirm the car maintains its current safe state.  
**Related Modules:** Bluetooth command parser, demo mode, free driving mode.

---

## 2. Constraints

**CN-01 – Required Hardware Platform**  
The system shall use one TM4C123 LaunchPad, one HC-05 Bluetooth module, one L298N motor driver, a two-DC-motor robot chassis, and a battery supply meeting project requirements.  
**Verification:** Hardware inspection.

**CN-02 – UART Assignment**  
UART1 shall be used for communication between the MCU and the HC-05 Bluetooth module.  
**Verification:** Code review and pin-mapping review.

**CN-03 – HC-05 Command Mode Entry**  
To enter HC-05 command mode, the EN/KEY pin must be connected high **before power-on**.  
**Verification:** Hardware setup demonstration and successful AT response.

**CN-04 – HC-05 Baud Rates**  
The HC-05 default baud rate shall be treated as:
- **38400** in command mode
- **9600** in default data mode before reconfiguration  
The project-required configured data mode shall be **57600, 8 data bits, 1 stop bit, odd parity**.  
**Verification:** HC-05 query responses and UART configuration review.

**CN-05 – Data Mode Operation**  
For Bluetooth terminal control of the car, the HC-05 shall be placed in **data mode** by disconnecting the EN/KEY pin from command-mode configuration.  
**Verification:** Hardware setup check and successful Bluetooth terminal control.

**CN-06 – Mode Count**  
The robot car shall operate in exactly **two modes** for the main control program:
- Mode 1: Demo
- Mode 2: Free Driving  
**Verification:** Functional test of both modes.

**CN-07 – LED Mode Indicators**  
The onboard LEDs shall indicate mode exactly as specified:
- Green = Demo
- Blue = Free Driving  
**Verification:** LED observation during mode toggle test.

**CN-08 – PWM Requirement**  
DC motors shall be driven using hardware PWM signals.  
**Verification:** Code review and hardware test.

---

## 3. System Design

### 3.1 Design Overview

The project has two software programs that use the same hardware:

1. **Part 1 – HC-05 Setup Program**  
   This program puts the HC-05 into command mode and sets the Bluetooth name, UART settings, passcode, and slave role.

2. **Part 2 – Bluetooth-Controlled Robot Car Program**  
   This program runs after the HC-05 is switched back to data mode. The Bluetooth terminal sends single-character commands to control the car.

Both programs use the same basic drivers, such as GPIO, UART, and timing, but each one has a different job.

---

### 3.2 Hardware Plan

#### 3.2.1 Main Hardware Blocks

- **TM4C123 LaunchPad**
  - Main controller
  - Runs the setup program and the robot-car control program

- **HC-05 Bluetooth module**
  - Receives wireless commands from the phone or PC
  - Communicates with the MCU through UART1

- **L298N motor driver**
  - Receives direction signals and PWM speed signals from the MCU
  - Drives the two DC motors

- **Robot chassis with 2 DC motors**
  - Mechanical drive system

- **Battery supply**
  - Powers the robot car system

#### 3.2.2 Signal-Level Hardware Responsibilities

**MCU ↔ HC-05**
- UART1 TX on MCU connects to HC-05 RX
- UART1 RX on MCU connects to HC-05 TX
- Common ground is required
- VCC is connected as needed
- EN/KEY is used only to enter AT command mode

**MCU ↔ L298N**
- Two PWM-capable output pins drive the motor enable or speed inputs
- GPIO direction pins control forward and reverse motion for the left and right motors

**MCU ↔ User Inputs/Indicators**
- SW1 is used to toggle between modes
- Onboard LEDs show the active mode and can also be used for simple debug feedback

#### 3.2.3 Functional Hardware Partition

The hardware can be grouped into three parts:

- **Communication**
  - UART0 or terminal connection for setup
  - UART1 and HC-05 for Bluetooth communication

- **Control**
  - TM4C123 firmware
  - command parsing
  - mode control
  - motion sequencing

- **Actuation**
  - PWM outputs
  - direction outputs
  - L298N motor driver
  - DC motors

This keeps communication code separate from motor-control code.

---

### 3.3 Software Architecture

The software is divided into modules, and each module has a specific job.

#### 3.3.1 Proposed Module List

##### 1. `System_Init`
**Purpose:** Initializes clocks, GPIO, UART, PWM, and shared startup state.  
**Responsibilities:**
- Configure the system clock if needed
- Initialize port direction and digital enable settings
- Initialize onboard LEDs and switch input
- Initialize UART modules
- Initialize PWM outputs for the motors

##### 2. `UART0_Terminal`
**Purpose:** Handles PC serial terminal input and output for the HC-05 setup program and debug messages.  
**Responsibilities:**
- Print startup messages and prompts
- Read user-entered AT commands
- Format terminal output if needed
- Display HC-05 responses

##### 3. `UART1_Bluetooth`
**Purpose:** Handles wired serial communication between the MCU and the HC-05.  
**Responsibilities:**
- Initialize UART1 with the required baud rate and format
- Send AT commands to the HC-05
- Receive responses from the HC-05
- Receive live Bluetooth command characters in data mode

##### 4. `Bluetooth_Setup_Manager`
**Purpose:** Runs Part 1 setup behavior for configuring the HC-05 in command mode.  
**Responsibilities:**
- Send the initial `AT` command
- Accept user-entered AT commands from the terminal
- Forward commands to the HC-05 through UART1
- Print HC-05 response strings
- Support configuration check commands such as `AT+NAME?` and `AT+UART?`

##### 5. `Bluetooth_Command_Parser`
**Purpose:** Interprets incoming single-character Bluetooth commands during robot-car operation.  
**Responsibilities:**
- Read command characters from the HC-05 data stream
- Check whether the command is valid
- Send the command to Demo mode or Free Driving mode
- Ignore invalid characters safely

##### 6. `Mode_Manager`
**Purpose:** Keeps track of the current mode and handles mode changes.  
**Responsibilities:**
- Set the default mode at startup
- Detect SW1 press events
- Toggle between Demo and Free Driving
- Notify LED and control modules of mode changes
- Reset or keep mode-specific state based on the design

##### 7. `LED_Status`
**Purpose:** Controls the onboard LEDs for mode indication.  
**Responsibilities:**
- Turn the green LED on in Demo mode
- Turn the blue LED on in Free Driving mode
- Turn off the unused mode LED

##### 8. `Motor_Control`
**Purpose:** Provides the motion-control interface for the robot car.  
**Responsibilities:**
- Set left motor direction
- Set right motor direction
- Set left and right motor speed through PWM
- Implement forward, reverse, left-turn, right-turn, and stop actions
- Enforce safe stop behavior

##### 9. `PWM_Driver`
**Purpose:** Handles PWM peripheral setup and duty-cycle updates.  
**Responsibilities:**
- Configure the PWM hardware
- Set PWM period or frequency as needed
- Update duty cycle for each motor channel
- Enable PWM outputs

##### 10. `Switch_Input`
**Purpose:** Reads SW1 and generates a clean mode-switch event.  
**Responsibilities:**
- Initialize the switch GPIO
- Detect a switch press
- Support polling or interrupt-based input handling
- Reduce false toggles due to bounce if needed

##### 11. `Demo_Mode_Controller`
**Purpose:** Runs the shape-drawing routines for Mode 1.  
**Responsibilities:**
- Decode demo commands (`8`, `C`, `S`, `Z`)
- Execute multi-step movement sequences
- Control timing for turns, straight segments, and stop points
- Prevent new commands from interrupting the current pattern

##### 12. `Free_Driving_Controller`
**Purpose:** Runs real-time movement and speed control for Mode 2.  
**Responsibilities:**
- Decode commands `F`, `B`, `L`, `R`, `S`, `U`, `D`
- Maintain the current speed state
- Update PWM duty cycle for `U` and `D`
- Keep the selected speed across direction changes
- Clamp speed within a valid range

##### 13. `Timing/Delay`
**Purpose:** Provides timing support for demo patterns and startup delays.  
**Responsibilities:**
- Provide fixed delay functions
- Support timing for demo pattern segments
- Provide optional software timer support

---

### 3.4 Data Flow

#### 3.4.1 Part 1 – HC-05 Setup
```text
PC Terminal
   ↓ user types AT command
UART0_Terminal
   ↓ forwards command/request
Bluetooth_Setup_Manager
   ↓ sends AT command
UART1_Bluetooth
   ↓ serial data
HC-05 Module
   ↓ response
UART1_Bluetooth
   ↓ received response string
Bluetooth_Setup_Manager
   ↓ formatted output
UART0_Terminal
   ↓
PC Terminal display
```

#### 3.4.2 Part 2 – Bluetooth Car Control
```text
Bluetooth Terminal App
   ↓ wireless command
HC-05 Module
   ↓ UART1 byte
UART1_Bluetooth
   ↓
Bluetooth_Command_Parser
   ↓
Mode_Manager ───────────────→ LED_Status
   ↓
[Demo_Mode_Controller] or [Free_Driving_Controller]
   ↓
Motor_Control
   ↓
PWM_Driver + GPIO Direction Signals
   ↓
L298N Motor Driver
   ↓
Left/Right DC Motors
```

---

### 3.5 Call/Control Flow at a High Level

```text
main()
 ├─ System_Init()
 ├─ Mode_Manager_Init()
 ├─ LED_Status_Update()
 ├─ if setup program:
 │    └─ Bluetooth_Setup_Manager_Run()
 │         ├─ UART0_Terminal_ReadLine()
 │         ├─ UART1_Bluetooth_SendString()
 │         └─ UART1_Bluetooth_ReadResponse()
 └─ if robot-car program:
      └─ superloop
           ├─ Switch_Input_Check()
           ├─ Mode_Manager_Update()
           ├─ LED_Status_Update()
           ├─ UART1_Bluetooth_ReadChar()
           ├─ Bluetooth_Command_Parser()
           ├─ Demo_Mode_Controller_Run() or
           └─ Free_Driving_Controller_Run()
```

---

## 4. Design Justification

This design was chosen because it separates communication, control, and motor output into different modules. That makes the system easier to test, debug, and explain.

### 4.1 Why This Design Was Chosen
- **Modularity:** Each module has one main job.
- **Testability:** UART, PWM, switch input, and motion control can be tested separately.
- **Reuse:** UART1 is used for both HC-05 setup and Bluetooth data mode.
- **Safety:** Invalid commands are ignored and stop behavior is clearly defined.
- **Maintainability:** Demo logic and free-driving logic are kept separate.

### 4.2 Tradeoffs Considered
- A **single large main program** was rejected because it would be harder to test and debug.
- **Software PWM** was rejected because the project requires hardware PWM and hardware PWM is more stable for motor control.
- A design that mixed setup logic and driving logic together was rejected because AT-command handling and live driving control are different tasks.

### 4.3 Final Design Decision
The final design uses:
- UART1 for HC-05 communication
- a setup manager for AT configuration
- a command parser for live Bluetooth commands
- a mode manager for Demo and Free Driving
- hardware PWM motor control through the L298N

This gives a design that is simple, organized, and easier to verify later.v

## 5. AI Verification Summary

- **What AI was used for:**  
  AI was used to compare the written requirements against the project description, check whether the Step 1 document was complete, help organize the template sections, and help organize the system data flow charts and high-level control flow diagrams. AI was also used to improve wording and clarity in the written document.

- **What was verified:**  
  AI was used to verify that the requirements matched the project description, that both operating modes were included, that the Bluetooth setup requirements were covered, that the hardware PWM requirement was included, and that the system design section was complete and organized correctly.

- **What was accepted/rejected:**  
  Accepted: requirement organization, constraint organization, system design structure, data flow organization, and wording improvements for clarity.  
  Rejected: any requirement, feature, command, or mode not supported by the project description, and any design choice that conflicted with the required hardware PWM motor control.

---
