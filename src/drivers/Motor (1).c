// Motor.c
// Motor driver
// Controls motor direction pins and PWM outputs

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "Motor.h"

// Direction control on Port E
#define DIRECTION    (*((volatile unsigned long *)0x4002403C))
#define FORWARD      0x0F
#define BACKWARD     0x0A
#define LEFT_PIVOT   0x06
#define RIGHT_PIVOT  0x09

// Wheel trim values
static int16_t g_leftTrim = 0;
static int16_t g_rightTrim = 0;

// Local setup helpers
static void PortE_Init(void);
static void PWM0A_Init(uint16_t period);
static void PWM0B_Init(uint16_t period);
static uint16_t ApplyTrim(uint16_t duty, int16_t trim);

// Initialize motor direction pins and PWM outputs
void Motor_Init(void){
  PortE_Init();
  PWM0A_Init(MOTOR_PERIOD);
  PWM0B_Init(MOTOR_PERIOD);

  g_leftTrim = 0;
  g_rightTrim = 0;

  Motor_SetDirectionForward();
  Motor_Stop();
}

// Stop both motors
void Motor_Stop(void){
  Motor_SetLeftDuty(MOTOR_STOP);
  Motor_SetRightDuty(MOTOR_STOP);
}

// Set left and right trim values
void Motor_SetTrim(int16_t leftTrim, int16_t rightTrim){
  g_leftTrim = leftTrim;
  g_rightTrim = rightTrim;
}

// Set direction bits for forward motion
void Motor_SetDirectionForward(void){
  DIRECTION = FORWARD;
}

// Set direction bits for backward motion
void Motor_SetDirectionBackward(void){
  DIRECTION = BACKWARD;
}

// Set direction bits for left pivot
void Motor_SetDirectionPivotLeft(void){
  DIRECTION = LEFT_PIVOT;
}

// Set direction bits for right pivot
void Motor_SetDirectionPivotRight(void){
  DIRECTION = RIGHT_PIVOT;
}

// Set left motor PWM duty on PB6
void Motor_SetLeftDuty(uint16_t duty){
  duty = ApplyTrim(duty, g_leftTrim);
  PWM0_0_CMPA_R = duty - 1;
}

// Set right motor PWM duty on PB7
void Motor_SetRightDuty(uint16_t duty){
  duty = ApplyTrim(duty, g_rightTrim);
  PWM0_0_CMPB_R = duty - 1;
}

// Set both motor duties
void Motor_SetDuty(uint16_t leftDuty, uint16_t rightDuty){
  Motor_SetLeftDuty(leftDuty);
  Motor_SetRightDuty(rightDuty);
}

// Apply trim and keep duty in valid range
static uint16_t ApplyTrim(uint16_t duty, int16_t trim){
  int32_t adjusted;

  if(duty <= MOTOR_STOP){
    return MOTOR_STOP;
  }

  adjusted = (int32_t)duty + (int32_t)trim;

  if(adjusted < MOTOR_STOP){
    adjusted = MOTOR_STOP;
  }
  if(adjusted >= MOTOR_PERIOD){
    adjusted = MOTOR_PERIOD - 1;
  }

  return (uint16_t)adjusted;
}

// Initialize PE0-PE3 as direction outputs
static void PortE_Init(void){
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;
  while((SYSCTL_RCGC2_R&SYSCTL_RCGC2_GPIOE) == 0){}

  GPIO_PORTE_AMSEL_R &= ~0x0F;
  GPIO_PORTE_PCTL_R &= ~0x0000FFFF;
  GPIO_PORTE_DIR_R |= 0x0F;
  GPIO_PORTE_AFSEL_R &= ~0x0F;
  GPIO_PORTE_DEN_R |= 0x0F;
}

// Initialize PB6 as M0PWM0
static void PWM0A_Init(uint16_t period){
  SYSCTL_RCGCPWM_R |= 0x01;
  SYSCTL_RCGCGPIO_R |= 0x02;
  while((SYSCTL_RCGCGPIO_R&0x02) == 0){}

  GPIO_PORTB_AFSEL_R |= 0x40;
  GPIO_PORTB_PCTL_R &= ~0x0F000000;
  GPIO_PORTB_PCTL_R |= 0x04000000;
  GPIO_PORTB_AMSEL_R &= ~0x40;
  GPIO_PORTB_DEN_R |= 0x40;
  GPIO_PORTB_DR8R_R |= 0xC0;

  SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV;
  SYSCTL_RCC_R &= ~SYSCTL_RCC_PWMDIV_M;
  SYSCTL_RCC_R += SYSCTL_RCC_PWMDIV_2;

  PWM0_0_CTL_R = 0;
  PWM0_0_GENA_R = 0xC8;
  PWM0_0_LOAD_R = period - 1;
  PWM0_0_CMPA_R = 0;
  PWM0_0_CTL_R |= 0x01;
  PWM0_ENABLE_R |= 0x01;
}

// Initialize PB7 as M0PWM1
static void PWM0B_Init(uint16_t period){
  volatile uint32_t delay;

  SYSCTL_RCGCPWM_R |= 0x01;
  SYSCTL_RCGCGPIO_R |= 0x02;
  delay = SYSCTL_RCGCGPIO_R;

  GPIO_PORTB_AFSEL_R |= 0x80;
  GPIO_PORTB_PCTL_R &= ~0xF0000000;
  GPIO_PORTB_PCTL_R |= 0x40000000;
  GPIO_PORTB_AMSEL_R &= ~0x80;
  GPIO_PORTB_DEN_R |= 0x80;

  SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV;
  SYSCTL_RCC_R &= ~SYSCTL_RCC_PWMDIV_M;
  SYSCTL_RCC_R += SYSCTL_RCC_PWMDIV_2;

  PWM0_0_CTL_R = 0;
  PWM0_0_GENB_R = (PWM_0_GENB_ACTCMPBD_ONE | PWM_0_GENB_ACTLOAD_ZERO);
  PWM0_0_LOAD_R = period - 1;
  PWM0_0_CMPB_R = 0;
  PWM0_0_CTL_R |= 0x01;
  PWM0_ENABLE_R |= 0x02;
}