// Switches.h


#ifndef SWITCHES_H
#define SWITCHES_H

#include <stdint.h>

#define SW1 0x10   // PF4, negative logic

void Switches_Init(void);
uint8_t SW1_In(void);

#endif