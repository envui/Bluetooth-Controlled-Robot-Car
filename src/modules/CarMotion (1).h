// CarMotion.h

#ifndef CARMOTION_H
#define CARMOTION_H

#include <stdint.h>

void CarMotion_Init(void);

void Car_Stop(void);
void Car_Forward(void);
void Car_Backward(void);

void Car_LeftWide(void);
void Car_RightWide(void);

void Car_TurnLeft(void);
void Car_TurnRight(void);

void Car_BackLeft(void);
void Car_BackRight(void);

void Car_PivotLeft(void);
void Car_PivotRight(void);

void Car_SpeedUp(void);
void Car_SlowDown(void);

uint16_t Car_GetSpeed(void);

#endif