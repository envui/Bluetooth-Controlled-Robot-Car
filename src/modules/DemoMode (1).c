// DemoMode.c
// Demo movement patterns
// Uses CarMotion functions

#include <stdint.h>
#include "CarMotion.h"
#include "DemoMode.h"

static void DelayLoop(volatile uint32_t count);
static void DelayMs(uint32_t ms);

// Run figure-8 pattern
void Demo_Figure8(void){
  uint8_t i;

  for(i = 0; i < 2; i++){
    Car_TurnLeft();
    DelayMs(750);

    Car_TurnRight();
    DelayMs(750);

    Car_Stop();
  }
}

// Run triangle pattern
void Demo_Triangle(void){
  Car_Backward();
  DelayMs(200);

  Car_PivotRight();
  DelayMs(44);

  Car_Forward();
  DelayMs(200);

  Car_PivotLeft();
  DelayMs(66);

  Car_Forward();
  DelayMs(400);
}

// Run circle pattern
void Demo_Circle(void){
  Car_TurnLeft();
  DelayMs(2200);
  Car_Stop();
}

// Run square pattern
void Demo_Square(void){
  uint8_t i;

  for(i = 0; i < 5; i++){
    Car_Forward();
    DelayMs(600);

    Car_PivotRight();
    DelayMs(44);
  }

  Car_Stop();
}

// Run zigzag pattern
void Demo_Zigzag(void){
  Car_PivotRight();
  DelayMs(22);

  Car_Forward();
  DelayMs(600);

  Car_PivotLeft();
  DelayMs(44);

  Car_Forward();
  DelayMs(600);

  Car_PivotRight();
  DelayMs(44);

  Car_Forward();
  DelayMs(600);

  Car_Stop();
}

// Delay in milliseconds using a software loop
static void DelayMs(uint32_t ms){
  while(ms){
    DelayLoop(13333);
    ms--;
  }
}

// Inner busy-wait loop
static void DelayLoop(volatile uint32_t count){
  while(count){
    count--;
  }
}