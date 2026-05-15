// LED.c
// TM4C123 onboard LED driver
// PF1 red, PF2 blue, PF3 green

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "LED.h"

// Initialize Port F pins used for the LED
void LED_Init(void){
  volatile uint32_t delay;

  // Turn on clock for Port F
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;
  delay = SYSCTL_RCGC2_R;

  // Unlock Port F and allow changes on PF1-PF3
  GPIO_PORTF_LOCK_R = 0x4C4F434B;
  GPIO_PORTF_CR_R |= 0x0E;

  // Set PF1-PF3 as digital GPIO outputs
  GPIO_PORTF_AMSEL_R &= ~0x0E;
  GPIO_PORTF_PCTL_R &= ~0x0000FFF0;
  GPIO_PORTF_DIR_R |= 0x0E;
  GPIO_PORTF_AFSEL_R &= ~0x0E;
  GPIO_PORTF_DEN_R |= 0x0E;

  // Start with LED off
  GPIO_PORTF_DATA_R &= ~0x0E;
}

// Write color bits to PF1-PF3
void LED_Set(uint8_t color){
  GPIO_PORTF_DATA_R = (GPIO_PORTF_DATA_R & ~0x0E) | (color & 0x0E);
}

// Clear PF1-PF3
void LED_Off(void){
  GPIO_PORTF_DATA_R &= ~0x0E;
}