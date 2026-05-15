// BLT_UART1.c
// UART1 driver for HC-05 Bluetooth module
// PB0 U1Rx, PB1 U1Tx

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "BLT_UART1.h"

// Set up UART1 for Bluetooth data mode
void BLT_UART1_InitDataMode(void){
  // Turn on UART1 and Port B clocks
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART1;
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB;
  while((SYSCTL_RCGC2_R&SYSCTL_RCGC2_GPIOB)==0){}

  // Disable UART1 during setup
  UART1_CTL_R &= ~UART_CTL_UARTEN;

  // Set baud rate for 57600 with 50 MHz clock
  UART1_IBRD_R = 54;
  UART1_FBRD_R = 16;

  // 8-bit data, odd parity, FIFO enabled
  UART1_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_PEN | UART_LCRH_FEN;

  // Enable UART1, RX, and TX
  UART1_CTL_R |= UART_CTL_UARTEN | UART_CTL_RXE | UART_CTL_TXE;

  // Set PB0 and PB1 for UART
  GPIO_PORTB_AFSEL_R |= 0x03;
  GPIO_PORTB_DEN_R |= 0x03;
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFFFF00) + 0x00000011;
  GPIO_PORTB_AMSEL_R &= ~0x03;
}

// Wait for one received character
char BLT_UART1_InChar(void){
  while((UART1_FR_R&UART_FR_RXFE) != 0){}
  return (char)(UART1_DR_R&0xFF);
}

// Return 1 if RX FIFO is not empty
uint8_t BLT_UART1_Available(void){
  if((UART1_FR_R&UART_FR_RXFE) == 0){
    return 1;
  }
  return 0;
}

// Remove all characters waiting in RX FIFO
void BLT_UART1_Flush(void){
  while((UART1_FR_R&UART_FR_RXFE) == 0){
    (void)UART1_DR_R;
  }
}