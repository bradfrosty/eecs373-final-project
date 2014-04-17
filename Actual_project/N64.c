#include "N64.h"
#include "lcd.h"
#include "motors.h"

#include <stdint.h>
#include <stdio.h>

void init_N64() {
	MYN64->enable = 1;
	NVIC_EnableIRQ(Fabric_IRQn);
}

void disable_N64() {
	MYN64->enable = 0;
}

int counter = 0;

uint32_t N64_get_data() {
	//send N64 data command
	MYN64->data = 0x00000003;

	//delay
	int i, j;
	volatile int d;
	for (i = 0; i < 100; ++i) {
		for (j = 0; j < 1000; ++j)
			d = 1;
	}

	//grab N64 data
	uint32_t data = MYN64->data;

	//extract analog x and y values
	int8_t x = (data & 0x0000FF00) >> 8;
	int8_t y = (data & 0x000000FF);

	//set motors direction MMIO
	if (y < - 30) {
		MYMOTORS->direction_y = REVERSE;
		if (x < -30)
			MYMOTORS->direction_x = REVERSE;
		else if (x > 30)
			MYMOTORS->direction_x = FORWARD;
		else
			MYMOTORS->direction_x = STOPPED;
	}
	else if (y > 30) {
		MYMOTORS->direction_y = FORWARD;
		if (x < -30)
			MYMOTORS->direction_x = REVERSE;
		else if (x > 30)
			MYMOTORS->direction_x = FORWARD;
		else
			MYMOTORS->direction_x = STOPPED;
	}
	else {
		MYMOTORS->direction_y = STOPPED;
		if (x < -30)
			MYMOTORS->direction_x = REVERSE;
		else if (x > 30)
			MYMOTORS->direction_x = FORWARD;
		else
			MYMOTORS->direction_x = STOPPED;
	}

	return data;
}

__attribute__ ((interrupt)) void Fabric_IRQHandler(void) {
	N64_get_data();
	NVIC_ClearPendingIRQ(Fabric_IRQn);
}
