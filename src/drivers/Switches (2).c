// Switches.c
// SW1 input driver
// Uses PF4 with negative logic

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "Switches.h"

// Initialize PF4 as input with pull-up resistor
void Switches_Init(void){
  volatile uint32_t delay;

  // Turn on clock for Port F
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;
  delay = SYSCTL_RCGC2_R;

  // Unlock PF4 and allow changes
  GPIO_PORTF_LOCK_R = 0x4C4F434B;
  GPIO_PORTF_CR_R |= 0x10;

  // Set PF4 as digital GPIO input with pull-up
  GPIO_PORTF_AMSEL_R &= ~0x10;
  GPIO_PORTF_PCTL_R &= ~0x000F0000;
  GPIO_PORTF_DIR_R &= ~0x10;
  GPIO_PORTF_AFSEL_R &= ~0x10;
  GPIO_PORTF_PUR_R |= 0x10;
  GPIO_PORTF_DEN_R |= 0x10;
}

// Return 1 when SW1 is pressed, else 0
uint8_t SW1_In(void){
  if((GPIO_PORTF_DATA_R & 0x10) == 0){
    return 1;
  }
  return 0;
}