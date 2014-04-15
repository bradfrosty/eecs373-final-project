#include "Switch.h"
#include "N64.h"
#include "kinect.h"
#include "state.h"
#include "drivers/mss_gpio/mss_gpio.h"
#include "drivers/mss_uart/mss_uart.h"
#include <stdio.h>

void GPIO8_IRQHandler() {
	uint32_t switch_status = MSS_GPIO_get_inputs() >> 8;
	if (switch_status == 0) {
		init_kinect();
		state.input_mode = KINECT;
	}
	else
		state.input_mode = N64;

	MSS_GPIO_clear_irq(MSS_GPIO_8);
}
