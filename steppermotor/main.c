#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "drivers/mss_gpio/mss_gpio.h"
#include "drivers/mss_timer/mss_timer.h"
//#include "drivers/mss_uart/mss_uart.h"
#include "mytimer.h"




#define CW 0										//motor direction: clockwise
#define CCW 1										//counterclockwise

#define ANGLEPERSTEP  1.8 							//enter angle per step of motor here
#define T1_FREQ	2000000								//timer freq at 500 kHz
#define FSPR 360/ANGLEPERSTEP						//steps per revolution

#define ALPHA (2*3.14159)/FSPR						//motor step angle
#define A_T_x100 ((uint64_t)(ALPHA*T1_FREQ*100))		//(ALPHA / T1_FREQ)*100
#define T1_FREQ_148 ((uint32_t)((T1_FREQ*0.676)/100)) 	// divided by 100 and scaled by 0.676
#define A_SQ (uint64_t)(ALPHA*2*10000000000)         	// ALPHA*2*10000000000
#define A_x20000 (uint32_t)(ALPHA*20000)              	// ALPHA*20000

typedef struct {
  //! What part of the speed ramp we are in.
  unsigned char run_state : 3;
  //! Direction stepper motor should move.
  unsigned char dir : 1;
  //! Period of next timer delay. At start this value sets the acceleration rate.
  uint32_t step_delay;
  //! What step_pos to start deceleration
  uint32_t decel_start;
  //! Sets deceleration rate.
  int32_t decel_val;
  //! Minimum time delay (max speed)
  int32_t min_delay;
  //! Counter used when accelerating/decelerating to calculate step_delay.
  int32_t accel_count;
} speedRampData;

// Speed ramp states
#define STOP  0
#define ACCEL 1
#define DECEL 2
#define RUN   3



int32_t stepPosition;					//motor position
uint8_t interupt_type = 0;
volatile uint32_t count;
speedRampData srd;

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


//Speed Control Functions
void move(int32_t step,uint32_t accel, uint32_t decel, uint32_t speed);
static uint64_t sqrt(uint64_t x);
uint32_t min(uint32_t x, uint32_t y);





//Driver functions
void initGPIO(void);
void initMotor(void);
unsigned char stepCounter(char dir);
void outputStep(unsigned char stepSeq);

int main()
{
		initGPIO();
		initMotor();

		/* Setup MYTIMER */
		MYTIMER_init();
		//MYTIMER_setOverflowVal(T1_FREQ);
		//MYTIMER_setCompareVal(10000); // 75% duty cycle

		MYTIMER_enable_overflowInt();
		//MYTIMER_enable_compareInt();
		//MYTIMER_enable_allInterrupts();

		NVIC_EnableIRQ(Fabric_IRQn);

		MYTIMER_enable();
		int j = 0, k=0,l=0;

		//Fastest seen: 100 accel, 23 speed, 2000000 Tclk

		move(10000,100, 1,23);

		volatile int y;
		for(j=0;j <1000; ++j) {
			for(k=0; k<10000; ++k) {
					y = 0;
			}
		}


		move(-1000, 1,1,24);

		//count = 1e6;
		//while(1){
		//    	if(!count){ // Global variable checking
			// count decremented inside the interrupt service routine
		//    		MYTIMER_disable(); // Disable after 1e6 interrupts
		//    	}
		//}
		/*
		while(1) {

			uint32_t i;
			printf("Time: %lu\r\n", MYTIMER_getCounterVal()); // Standard printf() now work
			for(i=1e6; i>0; i--); // busy wait
		}*/
		while(1);


		return 0;
}


void initGPIO() {
	//initalize MSS gpio block
	MSS_GPIO_init();

	//configure MSS gpio pins 0 to 3 as outputs
	MSS_GPIO_config(MSS_GPIO_0, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_1, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_2, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_3, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_4, MSS_GPIO_INPUT_MODE);
	MSS_GPIO_config( MSS_GPIO_5, MSS_GPIO_INPUT_MODE);


}







/*********************************************************************************************/
//DRIVER FUNCTIONS
/*********************************************************************************************/


//initializes the motor pins to 0
void initMotor(void) {

	uint32_t gpio_outputs =  MSS_GPIO_get_outputs();
	gpio_outputs &= ~( MSS_GPIO_0_MASK | MSS_GPIO_1_MASK | MSS_GPIO_2_MASK | MSS_GPIO_3_MASK );
	MSS_GPIO_set_outputs(  gpio_outputs );
	srd.run_state = STOP;

}


//keeps track of the step position and increments the counter in the step drive.
//pulls the position of step sequence from the stepOut array.
unsigned char counter = 0;

uint8_t stepCounter(char dir) {


	if(dir == CW) {
		stepPosition++;
		counter++;
	}
	else {
		stepPosition--;
		counter--;
	}
	//keeps the counter within the range of the array
	counter &= 0x03;

	outputStep(counter);
	return counter;

}


//sends the step data to the motor
void outputStep(uint8_t stepSeq) {

	uint8_t temp = stepOut[stepSeq];

	MSS_GPIO_set_outputs(temp);

	return;
}


/*********************************************************************************************/
//Speed Control Functions
/*********************************************************************************************/



void move(int32_t step, uint32_t accel, uint32_t decel, uint32_t speed) {

	//! Number of steps before we hit max speed.
	uint32_t max_s_lim;
	//! Number of steps before we must start deceleration (if accel does not hit max speed).
	uint32_t accel_lim;

	// Set direction from sign on step value.
	if(step < 0){
	srd.dir = CCW;
	step = -step;
	}
	else{
	srd.dir = CW;
	}

	// If moving only 1 step.
	if(step == 1){
		// Move one step...
		srd.accel_count = -1;
		// ...in DECEL state.
		srd.run_state = DECEL;
		// Just a short delay so main() can act on 'running'.
		srd.step_delay = 1000;
		count = 1;
		//MYTIMER_enable_allInterrupts();
		MYTIMER_setOverflowVal(10);
		MYTIMER_enable();
		while(1){
			if(!count){ 			// Global variable checking
									// count decremented inside the interrupt service routine
				MYTIMER_disable();  // Disable after 1 interrupt
			}
		}
	}
	else if(step != 0){
	    // Refer to documentation for detailed information about these calculations.

	    // Set max speed limit, by calc min_delay to use in timer.
	    // min_delay = (alpha / tt)/ w
	    srd.min_delay = A_T_x100 / speed;

	    // Set acceleration by calc the first (c0) step delay .
	    // step_delay = 1/tt * sqrt(2*alpha/accel)
	    // step_delay = ( tfreq*0.676/100 )*100 * sqrt( (2*alpha*10000000000) / (accel*100) )/10000
	    srd.step_delay = (T1_FREQ_148/100) * (sqrt(A_SQ / accel)/100);

	    // Find out after how many steps does the speed hit the max speed limit.
	    // max_s_lim = speed^2 / (2*alpha*accel)
	    max_s_lim = (int64_t)speed*speed/(int64_t)(((int64_t)A_x20000*accel)/100);
	    // If we hit max speed limit before 0,5 step it will round to 0.
	    // But in practice we need to move at least 1 step to get any speed at all.
	    if(max_s_lim == 0){
	      max_s_lim = 1;
	    }

	    // Find out after how many steps we must start deceleration.
	    // n1 = (n1+n2)decel / (accel + decel)
	    accel_lim = ((int64_t)step*decel) / (accel+decel);
	    // We must accelerate at least 1 step before we can start deceleration.
	    if(accel_lim == 0){
	      accel_lim = 1;
	    }

	    // Use the limit we hit first to calc decel.
	    if(accel_lim <= max_s_lim){
	      srd.decel_val = accel_lim - step;
	    }
	    else{
	      srd.decel_val = -((int64_t)max_s_lim*accel)/decel;
	    }
	    // We must decelerate at least 1 step to stop.
	    if(srd.decel_val == 0){
	      srd.decel_val = -1;
	    }

	    // Find step to start deceleration.
	    srd.decel_start = step + srd.decel_val;

	    // If the maximum speed is so low that we don't need to go via acceleration state.
	    if(srd.step_delay <= srd.min_delay){
	      srd.step_delay = srd.min_delay;
	      srd.run_state = RUN;
	    }
	    else{
	      srd.run_state = ACCEL;
	    }

	    // Reset counter.
	    srd.accel_count = 0;

	    count = step;
	    //MYTIMER_enable_allInterrupts();
	    MYTIMER_enable_overflowInt();
	    //MYTIMER_setCompareVal(10);
	    MYTIMER_setOverflowVal(10);
	    MYTIMER_enable();

		while(count){
			if(!count){ 			// Global variable checking
									// count decremented inside the interrupt service routine
				MYTIMER_disable();  // Disable after 1e6 interrupts
			}
		}


	  }
}

__attribute__ ((interrupt)) void Fabric_IRQHandler(void) { //previously void Timer1_IRQHandler( void ), using hardware timer now

	uint32_t time = MYTIMER_getCounterVal();
	uint32_t status = MYTIMER_getInterrupt_status();
	count--;


	// Holds next delay period.
	unsigned int new_step_delay;
	// Remember the last step delay used when accelerating.
	static int last_accel_delay;
	// Counting steps when moving.
	static unsigned int step_count = 0;
	// Keep track of remainder from new_step-delay calculation to increase accuracy
	static unsigned int rest = 0;


	//set pwm value here for acceleration change
	//MYTIMER_setCompareVal(srd.step_delay);
	MYTIMER_setOverflowVal(srd.step_delay);


	switch(srd.run_state) {
	    case STOP:
	      step_count = 0;
	      rest = 0;

	      MYTIMER_disable();

	      break;

	    case ACCEL:
	    /*if(!interupt_type) {
			stepCounter(srd.dir);
			interupt_type = 1;
		}
		else if(interupt_type) {
			uint32_t gpio_outputs =  MSS_GPIO_get_outputs();
			gpio_outputs &= ~( MSS_GPIO_0_MASK | MSS_GPIO_1_MASK | MSS_GPIO_2_MASK | MSS_GPIO_3_MASK );
			MSS_GPIO_set_outputs(  gpio_outputs );
			interupt_type = 0;
		}*/
	      stepCounter(srd.dir);
	      step_count++;
	      srd.accel_count++;
	      new_step_delay = srd.step_delay - (((2 * (int64_t)srd.step_delay) + rest)/(4 * srd.accel_count + 1));
	      rest = ((2 * (int64_t)srd.step_delay)+rest)%(4 * srd.accel_count + 1);
	      // Check if we should start deceleration.
	      if(step_count >= srd.decel_start) {
	        srd.accel_count = srd.decel_val;
	        srd.run_state = DECEL;
	      }
	      // Check if we hit max speed.
	      else if(new_step_delay <= srd.min_delay) {
	        last_accel_delay = new_step_delay;
	        new_step_delay = srd.min_delay;
	        rest = 0;
	        srd.run_state = RUN;
	      }
	      break;

	    case RUN:

		/*if(!interupt_type) {
			stepCounter(srd.dir);
			interupt_type = 1;
		}
		else if(interupt_type) {
				uint32_t gpio_outputs =  MSS_GPIO_get_outputs();
			gpio_outputs &= ~( MSS_GPIO_0_MASK | MSS_GPIO_1_MASK | MSS_GPIO_2_MASK | MSS_GPIO_3_MASK );
			MSS_GPIO_set_outputs(  gpio_outputs );
			interupt_type = 0;
		}*/
		stepCounter(srd.dir);
		step_count++;
		new_step_delay = srd.min_delay;
		// Check if we should start deceleration.
		if(step_count >= srd.decel_start) {
			srd.accel_count = srd.decel_val;
			// Start deceleration with same delay as accel ended with.
			new_step_delay = last_accel_delay;
			srd.run_state = DECEL;
		}
		break;

		case DECEL:
		/*if(!interupt_type) {
			stepCounter(srd.dir);
			interupt_type = 1;
		}
		else if(interupt_type) {
			uint32_t gpio_outputs =  MSS_GPIO_get_outputs();
			gpio_outputs &= ~( MSS_GPIO_0_MASK | MSS_GPIO_1_MASK | MSS_GPIO_2_MASK | MSS_GPIO_3_MASK );
			MSS_GPIO_set_outputs(  gpio_outputs );
			interupt_type = 0;
		}*/
		stepCounter(srd.dir);
		step_count++;
		srd.accel_count++;
		new_step_delay = srd.step_delay - (((2 * (int64_t)srd.step_delay) + rest)/(4 * srd.accel_count + 1));
		rest = ((2 * (int64_t)srd.step_delay)+rest)%(4 * srd.accel_count + 1);
		// Check if we are at last step
		if(srd.accel_count >= 0){
			srd.run_state = STOP;
		}
		break;
	}
	srd.step_delay = new_step_delay;





	NVIC_ClearPendingIRQ( Fabric_IRQn );

	//MSS_TIM1_clear_irq();

}

static uint64_t sqrt(uint64_t x)
{
  register uint64_t xr;  // result register
  register uint64_t q2;  // scan-bit register
  register uint8_t f;   // flag (one bit)

  xr = 0;                     // clear result
  q2 = 0x40000000L;           // highest possible result bit
  do
  {
    if((xr + q2) <= x)
    {
      x -= xr + q2;
      f = 1;                  // set flag
    }
    else{
      f = 0;                  // clear flag
    }
    xr >>= 1;
    if(f){
      xr += q2;               // test flag
    }
  } while(q2 >>= 2);          // shift twice
  if(xr < x){
    return xr +1;             // add for rounding
  }
  else{
    return xr;
  }
}

/*! \brief Find minimum value.
 *
 *  Returns the smallest value.
 *
 *  \return  Min(x,y).
 */
uint32_t min(uint32_t x, uint32_t y)
{
  if(x < y){
    return x;
  }
  else{
    return y;
  }
}
