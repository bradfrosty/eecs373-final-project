#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <opencv\cv.h>
#include <stdint.h>

typedef struct trajectory_t {
	//int cur_x;
	//int cur_y;
	int cur_pix_x;
	int cur_pix_y;
	//unsigned int cur_time;
	//int prev_x;
	//int prev_y;
	int prev_pix_x;
	int prev_pix_y;
	//unsigned int prev_time;
	//double slope;
	double pix_slope;
	int dest_pix_x;
	int dest_pix_y;
	int paddle_pix_x;
	int paddle_pix_y;
	int delta_x;
	int delta_y;
} trajectory_t;

void compute_trajectory(trajectory_t *t, uint8_t *frame);

void draw_walls(IplImage *im);

#endif