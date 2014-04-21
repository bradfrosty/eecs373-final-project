#include "N64.h"
#include "lcd.h"
#include "motors.h"

#include <stdint.h>
#include <stdio.h>

static int dir_x = 0;
static int dir_y = 0;

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
		if (dir_y != -1) {
			MYMOTORS->direction_y = REVERSE;
			dir_y = -1;
		}
		if (x < -30 && dir_x != -1) {
			MYMOTORS->direction_x = REVERSE;
			dir_x = -1;
		}
		else if (x > 30 && dir_x != 1) {
			MYMOTORS->direction_x = FORWARD;
			dir_x = 1;
		}
		else if (x > -30 && x < 30 && dir_x != 0) {
			MYMOTORS->direction_x = STOPPED;
			dir_x = 0;
		}
	}
	else if (y > 30) {
			if (dir_y != 1) {
				MYMOTORS->direction_y = FORWARD;
				dir_y = 1;
			}
			if (x < -30 && dir_x != -1) {
				MYMOTORS->direction_x = REVERSE;
				dir_x = -1;
			}
			else if (x > 30 && dir_x != 1) {
				MYMOTORS->direction_x = FORWARD;
				dir_x = 1;
			}
			else if (x > -30 && x < 30 && dir_x != 0) {
				MYMOTORS->direction_x = STOPPED;
				dir_x = 0;
			}
		}
	else {
		if (dir_y != 0) {
			MYMOTORS->direction_y = STOPPED;
			dir_y = 0;
		}
		if (x < -30 && dir_x != -1) {
			MYMOTORS->direction_x = REVERSE;
			dir_x = -1;
		}
		else if (x > 30 && dir_x != 1) {
			MYMOTORS->direction_x = FORWARD;
			dir_x = 1;
			}
		else if (x > -30 && x < 30 && dir_x != 0) {
			MYMOTORS->direction_x = STOPPED;
			dir_x = 0;
		}
	}

	return data;
}

__attribute__ ((interrupt)) void Fabric_IRQHandler(void) {
	N64_get_data();
	NVIC_ClearPendingIRQ(Fabric_IRQn);
}
