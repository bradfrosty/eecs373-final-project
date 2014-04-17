#include "Switch.h"
#include "N64.h"
#include "kinect.h"

#include "drivers/mss_gpio/mss_gpio.h"

void init_switch() {
	MSS_GPIO_init();

	//switch gpio
	MSS_GPIO_config(MSS_GPIO_8, MSS_GPIO_INPUT_MODE | MSS_GPIO_IRQ_EDGE_BOTH);
	MSS_GPIO_enable_irq(MSS_GPIO_8);

	int switch_status = get_switch_position();
	if (switch_status == 0)
		init_kinect();
	else
		init_N64();

}

int get_switch_position() {
	int switch_status = MSS_GPIO_get_inputs() >> 8;
	return switch_status & 0x1;

}

void GPIO8_IRQHandler() {
	int switch_status = get_switch_position();
	if (switch_status == 0)
		init_kinect();
	else
		init_N64();

	MSS_GPIO_clear_irq(MSS_GPIO_8);
}
