#ifndef __UNION_FIND_H__
#define __UNION_FIND_H__

#include <stdint.h>
#include <opencv/cv.h>

#define WIDTH 640
#define HEIGHT 480

typedef struct coords_t coords_t;
struct coords_t {
	int x;
	int y;
};

struct trajectory_t;

extern int max_size, min_size;

int union_find(uint8_t *im, uint32_t color, trajectory_t *t);

void paddle_detection(uint8_t *im, trajectory_t *t);

#endif