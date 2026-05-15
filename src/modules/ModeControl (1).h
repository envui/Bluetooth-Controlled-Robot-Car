// ModeControl.h


#ifndef MODECONTROL_H
#define MODECONTROL_H

#include <stdint.h>

#define MODE_DEMO       0U
#define MODE_FREE_DRIVE 1U

void ModeControl_Init(void);
void ModeControl_Toggle(void);
uint8_t ModeControl_Get(void);
void ModeControl_UpdateLED(void);

#endif