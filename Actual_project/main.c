#include "kinect.h"
#include "N64.h"
#include "lcd.h"
#include "switch.h"
#include "state.h"

#include "drivers/mss_gpio/mss_gpio.h"

#include <stdio.h>
#include <inttypes.h>

void init_gpio_pins();

state_t state;

int main() {
	/*********************** DEBUGGING CODE ***********************/
	//init_N64();
	initLCD();

	int i = 0;
	for (i = 0; i < 10; ++i)
		updateScore(BOT);

	while(1) {}

	/************************* ACTUAL CODE ************************/
	init_gpio_pins();
	initLCD();

	int switch_status = get_switch_position();
	if (switch_status == 0) {
		init_kinect();
		state.input_mode = KINECT;
	}
	else {
		init_N64();
		state.input_mode = N64;
	}

	while(1) {}
	return 0;
}

void init_gpio_pins() {
	MSS_GPIO_init();

	//switch gpio
	MSS_GPIO_config(MSS_GPIO_8, MSS_GPIO_INPUT_MODE | MSS_GPIO_IRQ_EDGE_BOTH);
	MSS_GPIO_enable_irq(MSS_GPIO_8);
}

