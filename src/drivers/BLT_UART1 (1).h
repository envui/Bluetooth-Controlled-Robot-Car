// BLT_UART1.h
// Runs on TM4C123
// Low-level UART1 driver for HC-05 in data mode

#ifndef BLT_UART1_H
#define BLT_UART1_H

#include <stdint.h>

void BLT_UART1_InitDataMode(void);
char BLT_UART1_InChar(void);
uint8_t BLT_UART1_Available(void);
void BLT_UART1_Flush(void);

#endif