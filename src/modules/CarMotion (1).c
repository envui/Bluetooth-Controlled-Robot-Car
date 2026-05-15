// CarMotion.c
// Runs on TM4C123
// High-level motion control for robot car
// Uses Motor driver for actual PWM and direction output

#include <stdint.h>
#include "Motor.h"
#include "CarMotion.h"

#define TURN_WIDE_DELTA   2000U
#define TURN_SHARP_DELTA  3500U
#define PIVOT_SPEED       MOTOR_SPEED_HI

static uint16_t g_speed = MOTOR_SPEED_MD;
static uint8_t g_motion = 0;

#define MOTION_STOP        0U
#define MOTION_FORWARD     1U
#define MOTION_BACKWARD    2U
#define MOTION_LEFT_WIDE   3U
#define MOTION_RIGHT_WIDE  4U
#define MOTION_LEFT_TURN   5U
#define MOTION_RIGHT_TURN  6U
#define MOTION_BACK_LEFT   7U
#define MOTION_BACK_RIGHT  8U
#define MOTION_PIVOT_LEFT  9U
#define MOTION_PIVOT_RIGHT 10U

static void Car_ApplyMotion(void);
static uint16_t ClampLow(uint16_t speed, uint16_t delta);

//------------CarMotion_Init------------
// Initialize motion state
// Input: none
// Output: none
void CarMotion_Init(void){
  g_speed = MOTOR_SPEED_MD;
  g_motion = MOTION_STOP;
  Car_Stop();
}

//------------Car_Stop------------
// Stop the car
// Input: none
// Output: none
void Car_Stop(void){
  g_motion = MOTION_STOP;
  Motor_Stop();
}

//------------Car_Forward------------
// Move car forward at current speed
// Input: none
// Output: none
void Car_Forward(void){
  g_motion = MOTION_FORWARD;
  Car_ApplyMotion();
}

//------------Car_Backward------------
// Move car backward at current speed
// Input: none
// Output: none
void Car_Backward(void){
  g_motion = MOTION_BACKWARD;
  Car_ApplyMotion();
}

//------------Car_LeftWide------------
// Wide left turn while moving forward
// Input: none
// Output: none
void Car_LeftWide(void){
  g_motion = MOTION_LEFT_WIDE;
  Car_ApplyMotion();
}

//------------Car_RightWide------------
// Wide right turn while moving forward
// Input: none
// Output: none
void Car_RightWide(void){
  g_motion = MOTION_RIGHT_WIDE;
  Car_ApplyMotion();
}

//------------Car_TurnLeft------------
// Sharper left turn while moving forward
// Input: none
// Output: none
void Car_TurnLeft(void){
  g_motion = MOTION_LEFT_TURN;
  Car_ApplyMotion();
}

//------------Car_TurnRight------------
// Sharper right turn while moving forward
// Input: none
// Output: none
void Car_TurnRight(void){
  g_motion = MOTION_RIGHT_TURN;
  Car_ApplyMotion();
}

//------------Car_BackLeft------------
// Left turn while moving backward
// Input: none
// Output: none
void Car_BackLeft(void){
  g_motion = MOTION_BACK_LEFT;
  Car_ApplyMotion();
}

//------------Car_BackRight------------
// Right turn while moving backward
// Input: none
// Output: none
void Car_BackRight(void){
  g_motion = MOTION_BACK_RIGHT;
  Car_ApplyMotion();
}

//------------Car_PivotLeft------------
// Tight pivot left turn
// Input: none
// Output: none
void Car_PivotLeft(void){
  g_motion = MOTION_PIVOT_LEFT;
  Car_ApplyMotion();
}

//------------Car_PivotRight------------
// Tight pivot right turn
// Input: none
// Output: none
void Car_PivotRight(void){
  g_motion = MOTION_PIVOT_RIGHT;
  Car_ApplyMotion();
}

//------------Car_SpeedUp------------
// Increase drive speed by one step and reapply current motion
// Input: none
// Output: none
void Car_SpeedUp(void){
  if(g_speed == MOTOR_SPEED_LO){
    g_speed = MOTOR_SPEED_MD;
  }else if(g_speed == MOTOR_SPEED_MD){
    g_speed = MOTOR_SPEED_HI;
  }
  Car_ApplyMotion();
}

//------------Car_SlowDown------------
// Decrease drive speed by one step and reapply current motion
// Input: none
// Output: none
void Car_SlowDown(void){
  if(g_speed == MOTOR_SPEED_HI){
    g_speed = MOTOR_SPEED_MD;
  }else if(g_speed == MOTOR_SPEED_MD){
    g_speed = MOTOR_SPEED_LO;
  }
  Car_ApplyMotion();
}

//------------Car_GetSpeed------------
// Return current speed setting
// Input: none
// Output: speed duty value
uint16_t Car_GetSpeed(void){
  return g_speed;
}

//------------ClampLow------------
// Reduce speed by delta with floor at MOTOR_STOP
// Input: speed, delta
// Output: reduced speed
static uint16_t ClampLow(uint16_t speed, uint16_t delta){
  if(speed > delta){
    return speed - delta;
  }
  return MOTOR_STOP;
}

//------------Car_ApplyMotion------------
// Apply current motion state using current speed
// Input: none
// Output: none
static void Car_ApplyMotion(void){
  uint16_t leftDuty;
  uint16_t rightDuty;

  switch(g_motion){
    case MOTION_FORWARD:
      Motor_SetDirectionForward();
      Motor_SetDuty(g_speed, g_speed);
      break;

    case MOTION_BACKWARD:
      Motor_SetDirectionBackward();
      Motor_SetDuty(g_speed, g_speed);
      break;

    case MOTION_LEFT_WIDE:
      Motor_SetDirectionForward();
      leftDuty = g_speed;
      rightDuty = ClampLow(g_speed, TURN_WIDE_DELTA);
      Motor_SetDuty(leftDuty, rightDuty);
      break;

    case MOTION_RIGHT_WIDE:
      Motor_SetDirectionForward();
      leftDuty = ClampLow(g_speed, TURN_WIDE_DELTA);
      rightDuty = g_speed;
      Motor_SetDuty(leftDuty, rightDuty);
      break;

    case MOTION_LEFT_TURN:
      Motor_SetDirectionForward();
      leftDuty = g_speed;
      rightDuty = ClampLow(g_speed, TURN_SHARP_DELTA);
      Motor_SetDuty(leftDuty, rightDuty);
      break;

    case MOTION_RIGHT_TURN:
      Motor_SetDirectionForward();
      leftDuty = ClampLow(g_speed, TURN_SHARP_DELTA);
      rightDuty = g_speed;
      Motor_SetDuty(leftDuty, rightDuty);
      break;

    case MOTION_BACK_LEFT:
      Motor_SetDirectionBackward();
      leftDuty = ClampLow(g_speed, TURN_SHARP_DELTA);
      rightDuty = g_speed;
      Motor_SetDuty(leftDuty, rightDuty);
      break;

    case MOTION_BACK_RIGHT:
      Motor_SetDirectionBackward();
      leftDuty = g_speed;
      rightDuty = ClampLow(g_speed, TURN_SHARP_DELTA);
      Motor_SetDuty(leftDuty, rightDuty);
      break;

    case MOTION_PIVOT_LEFT:
      Motor_SetDirectionPivotLeft();
      Motor_SetDuty(PIVOT_SPEED, PIVOT_SPEED);
      break;

    case MOTION_PIVOT_RIGHT:
      Motor_SetDirectionPivotRight();
      Motor_SetDuty(PIVOT_SPEED, PIVOT_SPEED);
      break;

    default:
      Motor_Stop();
      break;
  }
}