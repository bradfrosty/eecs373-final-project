#ifndef MOTORS_H
#define MOTORS_H

#include "CMSIS/a2fxxxm3.h"
#define MOTORS_BASE (FPGA_FABRIC_BASE + 0x100)

#define FORWARD 1
#define STOPPED 3
#define REVERSE 2

typedef struct {
	uint32_t direction_x;
	uint32_t direction_y;
} motors_t;

#define MYMOTORS ((motors_t *) MOTORS_BASE)

#endif
