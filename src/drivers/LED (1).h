// LED.h


#ifndef LED_H
#define LED_H

#include <stdint.h>

#define LED_OFF   0x00
#define LED_RED   0x02
#define LED_BLUE  0x04
#define LED_GREEN 0x08

void LED_Init(void);
void LED_Set(uint8_t color);
void LED_Off(void);

#endif