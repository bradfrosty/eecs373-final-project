#ifndef N64_H
#define N64_H

#include "CMSIS/a2fxxxm3.h"
#define N64_BASE FPGA_FABRIC_BASE

typedef struct {
    uint32_t data; // Offset 0x0
    uint32_t enable; // Offset 0x4
} N64_t;

#define MYN64 ((N64_t *) N64_BASE)

void init_N64();
void disable_N64();
uint32_t N64_get_data();

#endif
