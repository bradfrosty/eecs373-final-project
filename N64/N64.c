#include "N64.h"
#include "state.h"
#include <stdio.h>
#include <stdint.h>

uint32_t N64_get_data() {
	volatile uint32_t *driver = (uint32_t*)N64_BASE;

	*driver = 0x00000003;
	uint32_t data = *driver;

	/*printf("data = %a\n\r", data);

	if (data & 0x80000000)
		printf("a\n\r");
	if (data & 0x40000000)
		printf("b\n\r");
	*/

	int8_t x = (data & 0x0001FE00) >> 9;
	int8_t y = (data & 0x000001FE) >> 1;

	//printf("y:%i, x:%i\n\r", y, x);

	if (y < - 16) {
		if (x < -16)
			state.N64_dir = DOWN_LEFT;
		else if (x > 16)
			state.N64_dir = DOWN_RIGHT;
		else
			state.N64_dir = DOWN;
	}
	else if (y > 16) {
		if (x < -16)
			state.N64_dir = UP_LEFT;
		else if (x > 16)
			state.N64_dir = UP_RIGHT;
		else
			state.N64_dir = UP;
	}
	else {
		if (x > 16)
			state.N64_dir = RIGHT;
		else if (x < -16)
			state.N64_dir = LEFT;
		else
			state.N64_dir = NONE;
	}





	//printf("\n\r");

	return data;
}
