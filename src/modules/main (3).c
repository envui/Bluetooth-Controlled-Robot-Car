// main_car.c
// Project 3 Part 2: Bluetooth Controlled Robot Car
// Mode 1: Demo
// Mode 2: Free Driving
//
// Bluetooth terminal commands:
// Demo mode:
//   '8' = figure 8
//   'C' = circle
//   'S' = square
//   'Z' = zigzag 
//   'T' = triangle(bonus) 
//
// Free Driving mode:
//   'F' = forward
//   'B' = reverse
//   'L' = left wide turn
//   'R' = right wide turn
//   'S' = stop
//   'U' = speed up
//   'D' = slow down

#include <stdint.h>
#include "PLL.h"
#include "LED.h"
#include "Switches.h"
#include "BLT_UART1.h"
#include "Motor.h"
#include "CarMotion.h"
#include "ModeControl.h"
#include "CommandParser.h"

static void DelayMs(uint32_t ms);

int main(void){
  uint8_t lastSW1;
  char cmd;

  PLL_Init();
  LED_Init();
  Switches_Init();
  BLT_UART1_InitDataMode();
  Motor_Init(); 
	Motor_SetTrim(0 , 700);   // right motor slightly reduced
  CarMotion_Init();
  ModeControl_Init();

  lastSW1 = 0;

  while(1){
    if(SW1_In()){
      if(lastSW1 == 0){
        Car_Stop();
        ModeControl_Toggle();
        DelayMs(100);           // simple debounce
      }
      lastSW1 = 1;
    }else{
      lastSW1 = 0;
    }

    if(BLT_UART1_Available()){
      cmd = BLT_UART1_InChar();

      if(ModeControl_Get() == MODE_DEMO){
        Command_HandleDemo(cmd);
      }else{
        Command_HandleFreeDrive(cmd);
      }

      ModeControl_UpdateLED();
    }
  }
}


// Simple software delay for switch debounce

static void DelayMs(uint32_t ms){
  volatile uint32_t count;

  while(ms){
    count = 13333;
    while(count){
      count--;
    }
    ms--;
  }
}