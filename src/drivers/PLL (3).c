// PLL.c
// System clock setup
// Sets the TM4C123 clock to 50 MHz

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "PLL.h"

// Configure PLL for 50 MHz system clock
void PLL_Init(void){
  SYSCTL_RCC2_R |= SYSCTL_RCC2_USERCC2;     // use RCC2
  SYSCTL_RCC2_R |= SYSCTL_RCC2_BYPASS2;     // bypass PLL during setup
  SYSCTL_RCC_R &= ~SYSCTL_RCC_XTAL_M;       // clear crystal field
  SYSCTL_RCC_R += SYSCTL_RCC_XTAL_16MHZ;    // 16 MHz crystal
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_OSCSRC2_M;  // main oscillator source
  SYSCTL_RCC2_R += SYSCTL_RCC2_OSCSRC2_MO;
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_PWRDN2;     // turn on PLL
  SYSCTL_RCC2_R |= SYSCTL_RCC2_DIV400;      // use 400 MHz PLL
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~0x1FC00000) + (7<<22); // divide to 50 MHz

  // Wait for PLL to lock
  while((SYSCTL_RIS_R&SYSCTL_RIS_PLLLRIS)==0){}

  // Enable PLL output
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_BYPASS2;
}