#ifndef STATE_H
#define STATE_H

#define KINECT 0
#define N64 1

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define UP_RIGHT 4
#define UP_LEFT 5
#define DOWN_RIGHT 6
#define DOWN_LEFT 7
#define NONE 8

typedef struct state_t {
	int input_mode;
	int N64_dir;

} state_t;

extern state_t state;

#endif
