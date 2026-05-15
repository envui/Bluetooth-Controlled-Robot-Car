// ModeControl.c
// Stores current mode
// Updates LED to match the mode

#include <stdint.h>
#include "LED.h"
#include "ModeControl.h"

static uint8_t g_mode = MODE_DEMO;

// Start in demo mode
void ModeControl_Init(void){
  g_mode = MODE_DEMO;
  ModeControl_UpdateLED();
}

// Switch between demo mode and free drive mode
void ModeControl_Toggle(void){
  if(g_mode == MODE_DEMO){
    g_mode = MODE_FREE_DRIVE;
  }else{
    g_mode = MODE_DEMO;
  }
  ModeControl_UpdateLED();
}

// Return current mode
uint8_t ModeControl_Get(void){
  return g_mode;
}

// Set LED color for current mode
void ModeControl_UpdateLED(void){
  if(g_mode == MODE_DEMO){
    LED_Set(LED_GREEN);
  }else{
    LED_Set(LED_BLUE);
  }
}