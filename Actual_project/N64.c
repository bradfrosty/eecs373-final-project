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
	uint32_t data = MYN64->data;

	printf("data:%d\n\r", data);

	int8_t x = (data & 0x0001FE00) >> 9;
	int8_t y = (data & 0x000001FE) >> 1;

	if (y < - 16) {
		printf("GOING DOWN\n\r");
		updateScore(BOT);
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
		updateScore(PLAYER);
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
