//stepper.h
//Contains function declarations,globals, and definitions for the stepper motor driver

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "drivers/mss_gpio/mss_gpio.h"
#include "drivers/mss_timer/mss_timer.h"
#include "mytimer.h"

#define CW 0											//motor direction: clockwise
#define CCW 1											//counterclockwise

#define ANGLEPERSTEP  1.8 								//enter angle per step of motor here
#define T1_FREQ	2000000									//timer freq at 500 kHz
#define FSPR 360/ANGLEPERSTEP							//steps per revolution

#define ALPHA (2*3.14159)/FSPR							//motor step angle
#define A_T_x100 ((uint64_t)(ALPHA*T1_FREQ*100))		//(ALPHA / T1_FREQ)*100
#define T1_FREQ_148 ((uint32_t)((T1_FREQ*0.676)/100)) 	// divided by 100 and scaled by 0.676
#define A_SQ (uint64_t)(ALPHA*2*10000000000)         	// ALPHA*2*10000000000
#define A_x20000 (uint32_t)(ALPHA*20000)              	// ALPHA*20000

// Speed ramp states
#define STOP  0
#define ACCEL 1
#define DECEL 2
#define RUN   3


typedef struct {
  unsigned char run_state : 3;						//! What part of the speed ramp we are in.
  unsigned char dir : 1;							//! Direction stepper motor should move.
  uint32_t step_delay;								//! Period of next timer delay. At start this value sets the acceleration rate
  uint32_t decel_start;								//! What step_pos to start deceleration
  int32_t decel_val;								//! Sets deceleration rate.
  int32_t min_delay;								//! Minimum time delay (max speed)
  int32_t accel_count;								//! Counter used when accelerating/decelerating to calculate step_delay.
} speedRampData;


int32_t stepPosition;								//motor position
uint8_t interupt_type = 0;							//interrupt type (compare/overflow), used for pwm
volatile uint32_t count;							//count variable for number of interrupts
speedRampData srd;									//speed ramp data struct declaration							

//array of different positions within a step
//if using full stepping, increment array index by 2
//otherwise, increment array index by 1 for half stepping

unsigned char stepOut[] = { MSS_GPIO_0_MASK |
							MSS_GPIO_3_MASK,
							MSS_GPIO_0_MASK |
							MSS_GPIO_2_MASK,
							MSS_GPIO_1_MASK |
							MSS_GPIO_2_MASK,
							MSS_GPIO_1_MASK |
							MSS_GPIO_3_MASK 
							};

//Speed Control Function Declarations
void move(int32_t step,uint32_t accel, uint32_t decel, uint32_t speed);
static uint64_t sqrt(uint64_t x);
uint32_t min(uint32_t x, uint32_t y);

//Driver Function Declarations
void initStepper(void);
void initGPIO(void);
void initMotor(void);
unsigned char stepCounter(char dir);
void outputStep(unsigned char stepSeq);
void delay(void);



