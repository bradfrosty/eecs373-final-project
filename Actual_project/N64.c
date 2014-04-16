#include "N64.h"
#include "lcd.h"
#include "state.h"

#include <stdint.h>

void init_N64() {
	MYN64->enable = 1;
	NVIC_EnableIRQ(Fabric_IRQn);
}

void disable_N64() {
	MYN64->enable = 0;
}

int counter = 0;

uint32_t N64_get_data() {
	MYN64->data = 0x00000003;

	int i, j;
	volatile int d;
	for (i = 0; i < 100; ++i) {
		for (j = 0; j < 1000; ++j)
			d = 1;
	}

	uint32_t data = MYN64->data;

	printf("data:%d\n\r", data);

	int8_t x = (data & 0x0000FF00) >> 8;
	int8_t y = (data & 0x000000FF);

	if (data & 0x80000000)
		updateScore(BOT);
	else if (data & 0x40000000)
		updateScore(PLAYER);

	if (y < - 16) {
		printf("GOING DOWN\n\r");
		//updateScore(BOT);
		if (x < -16)
			state.N64_dir = DOWN_LEFT;
		else if (x > 16)
			state.N64_dir = DOWN_RIGHT;
		else {
			state.N64_dir = DOWN;
		}
	}
	else if (y > 16) {
		printf("GOING UP\n\r");
		//updateScore(PLAYER);
		if (x < -16)
			state.N64_dir = UP_LEFT;
		else if (x > 16)
			state.N64_dir = UP_RIGHT;
		else {
			state.N64_dir = UP;
		}
	}
	else {
		if (x > 16)
			state.N64_dir = RIGHT;
		else if (x < -16)
			state.N64_dir = LEFT;
		else
			state.N64_dir = NONE;
	}
	return data;
}

__attribute__ ((interrupt)) void Fabric_IRQHandler(void) {
	N64_get_data();
	NVIC_ClearPendingIRQ(Fabric_IRQn);
}
