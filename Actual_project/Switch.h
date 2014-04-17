#ifndef SWITCH_H
#define SWITCH_H

#define KINECT 0
#define N64 1

void init_switch();
int get_switch_position();
void GPIO8_IRQHandler();

#endif
