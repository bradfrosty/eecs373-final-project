#ifndef N64_H
#define N64_H

#include "CMSIS/a2fxxxm3.h"
#define N64_BASE (FPGA_FABRIC_BASE + 0x0)

uint32_t N64_get_data();

#endif
