#include <stdio.h>
#include <inttypes.h>
#include "kinect.h"
#include "N64.h"
#include "state.h"
#include "drivers/mss_gpio/mss_gpio.h"

void init_gpio_pins();

state_t state;

int main() {

	init_gpio_pins();

	while(1) {
		uint32_t data = N64_get_data();
		//printf("received data:%i\n\r", (int)data);
	}

	uint32_t switch_status = MSS_GPIO_get_inputs() >> 8;
	if (switch_status == 0) {
		init_kinect();
		state.input_mode = KINECT;
	}
	else
		state.input_mode = N64;

	while(1) {}
	return 0;
}

void init_gpio_pins() {
	MSS_GPIO_init();

	//motor gpio
	MSS_GPIO_config(MSS_GPIO_0, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_1, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_2, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_3, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_4, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_5, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_6, MSS_GPIO_OUTPUT_MODE);
	MSS_GPIO_config(MSS_GPIO_7, MSS_GPIO_OUTPUT_MODE);

	//switch gpio
	MSS_GPIO_config(MSS_GPIO_8, MSS_GPIO_INPUT_MODE | MSS_GPIO_IRQ_EDGE_BOTH);
	MSS_GPIO_enable_irq(MSS_GPIO_8);
}

