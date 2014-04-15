#include "Switch.h"
#include "N64.h"
#include "kinect.h"
#include "state.h"

#include "drivers/mss_gpio/mss_gpio.h"

int get_switch_position() {
	int switch_status = MSS_GPIO_get_inputs() >> 8;
	return switch_status & 0x1;

}

void GPIO8_IRQHandler() {
	int switch_status = get_switch_position();
	if (switch_status == 0) {
		init_kinect();
		state.input_mode = KINECT;
	}
	else
		state.input_mode = N64;

	MSS_GPIO_clear_irq(MSS_GPIO_8);
}
