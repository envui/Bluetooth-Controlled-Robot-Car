// Motor.h

#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

#define MOTOR_PERIOD   10000U
#define MOTOR_STOP     1U
#define MOTOR_SPEED_LO 3500U
#define MOTOR_SPEED_MD 6000U
#define MOTOR_SPEED_HI 8000U

void Motor_Init(void);
void Motor_Stop(void);

void Motor_SetDirectionForward(void);
void Motor_SetDirectionBackward(void);
void Motor_SetDirectionPivotLeft(void);
void Motor_SetDirectionPivotRight(void);

void Motor_SetLeftDuty(uint16_t duty);
void Motor_SetRightDuty(uint16_t duty);
void Motor_SetDuty(uint16_t leftDuty, uint16_t rightDuty);

void Motor_SetTrim(int16_t leftTrim, int16_t rightTrim);

#endif