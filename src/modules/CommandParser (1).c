// CommandParser.c
// Maps Bluetooth commands to car actions

#include "CarMotion.h"
#include "DemoMode.h"
#include "BLT_UART1.h"
#include "CommandParser.h"

static void Demo_FinishCommand(void);

// Handle commands in demo mode
void Command_HandleDemo(char cmd){
  switch(cmd){
    case '8':
      Demo_Figure8();
      Demo_FinishCommand();
      break;

    case 'C':
    case 'c':
      Demo_Circle();
      Demo_FinishCommand();
      break;

    case 'S':
    case 's':
      Demo_Square();
      Demo_FinishCommand();
      break;

    case 'Z':
    case 'z':
      Demo_Zigzag();
      Demo_FinishCommand();
      break;

    case 'T':
    case 't':
      Demo_Triangle();
      Demo_FinishCommand();
      break;

    default:
      break;
  }
}

// Handle commands in free-drive mode
void Command_HandleFreeDrive(char cmd){
  switch(cmd){
    case 'F':
    case 'f':
      Car_Forward();
      break;

    case 'B':
    case 'b':
      Car_Backward();
      break;

    case 'L':
    case 'l':
      Car_LeftWide();
      break;

    case 'R':
    case 'r':
      Car_RightWide();
      break;

    case 'S':
    case 's':
      Car_Stop();
      break;

    case 'U':
    case 'u':
      Car_SpeedUp();
      break;

    case 'D':
    case 'd':
      Car_SlowDown();
      break;

    default:
      break;
  }
}

// Stop after demo command and clear extra RX data
static void Demo_FinishCommand(void){
  Car_Stop();
  BLT_UART1_Flush();
}