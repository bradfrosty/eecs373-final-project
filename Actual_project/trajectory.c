#include "trajectory.h"
#include "union_find.h"
#include <opencv/cv.h>
#include <assert.h>
#include <math.h>

//int y_top = 65;
//int y_bot = HEIGHT - 53;

static int y_top = 65;
static int y_bot = HEIGHT - 53;

static int x_left = 20;
static int x_right = WIDTH/2 - 55;

static void compute_delta(trajectory_t *t);

double slopes[3];
int s_count = 0;

typedef struct {
	int x;
	int y;
	int distance;
} dest_t;

void compute_trajectory(trajectory_t *t, uint8_t *frame) {
	double slope = (double(t->cur_pix_y - t->prev_pix_y) / double(t->cur_pix_x - t->prev_pix_x));

	if (t->cur_pix_x == t->prev_pix_x)
		return;

	t->pix_slope = slope;
	
	slopes[2] = slopes[1];
	slopes[1] = slopes[0];
	slopes[0] = slope;

	if (s_count == 3) {
		t->pix_slope = (slopes[0] + slopes[1] + slopes[2]) / 3.0;
	}
	else {
		++s_count;
		return;
		t->pix_slope = slope;
	}

	t->dest_pix_y = -1;

	dest_t dest;
	dest.x = -1;
	dest.y = -1;
	dest.distance = 1000;

	if (t->cur_pix_x - t->prev_pix_x > 0) { // going in +x
		//temporary dest values
		/*
		if (t->cur_pix_x < t->dest_pix_x)
			t->dest_pix_y = (t->dest_pix_x - t->cur_pix_x)*(t->pix_slope) + t->cur_pix_y;
		else
			t->dest_pix_y = -1;
		*/

		//draw slope
		for (int x = t->cur_pix_x; x < WIDTH; ++x) {
			//int dx = abs(x - t->paddle_pix_x);
			int y = (x - t->cur_pix_x)*(t->pix_slope) + t->cur_pix_y;

			while (y < y_top || y > y_bot) {
				if (y < y_top)
					y = y_top + (y_top - y);
				else if (y > y_bot)
					y = y_bot - (y - y_bot);
			}
			/*
			if (abs(y - t->paddle_pix_y) + dx < dest.distance) {
				printf("here\n");
				dest.x = x;
				dest.y = y;
				dest.distance = abs(y - t->paddle_pix_y) + abs(x - t->paddle_pix_x);
			}
			*/

			frame[y * WIDTH + x] = 0;
		}
	}
	else if (t->cur_pix_x - t->prev_pix_x < 0) { //going in -x
		//temporary dest values
		/*
		if (t->cur_pix_x > t->dest_pix_x)
			t->dest_pix_y = (t->cur_pix_x - t->dest_pix_x)*-(t->pix_slope) + t->cur_pix_y;
		else
			t->dest_pix_y = -1;

		*/
		//draw slope
		for (int x = 0; x < t->cur_pix_x; ++x) {
			int dx = abs(x - t->paddle_pix_x);
			int y = (t->cur_pix_x - x)*-(t->pix_slope) + t->cur_pix_y;

			while (y < y_top || y > y_bot) {
				if (y < y_top)
					y = y_top + (y_top - y);
				else if (y > y_bot)
					y = y_bot - (y - y_bot);
			}
			int distance = sqrt(abs(y - t->paddle_pix_y) + dx);

			if (distance < dest.distance) {
				dest.x = x;
				dest.y = y;
				dest.distance = distance;
			}

			frame[y * WIDTH + x] = 0;
		}
	}

	if (dest.x != -1 && dest.y != -1) {
		t->dest_pix_x = dest.x;
		t->dest_pix_y = dest.y;
	}

	if (t->dest_pix_y != -1) {
		for (int i = t->dest_pix_y - 5; i < t->dest_pix_y + 5; ++i) {
			for (int j = t->dest_pix_x - 5; j < t->dest_pix_x + 5; ++j) {
				if (i >= 0 && i < HEIGHT && j >= 0 && j < WIDTH) {
					frame[i * WIDTH + j] = 0;
				}
			}
		}
	}

	return;
}

void draw_walls(IplImage *im) {
	//draw horizontal walls
	for (int x = 0; x < WIDTH; ++x) {
		im->imageData[(y_top - 1) * WIDTH + x] = 0;
		im->imageData[y_top * WIDTH + x] = 0;
		im->imageData[(y_top + 1) * WIDTH + x] = 0;

		im->imageData[(y_bot - 1) * WIDTH + x] = 0;
		im->imageData[y_bot * WIDTH + x] = 0;
		im->imageData[(y_bot + 1) * WIDTH + x] = 0;
	}

	//draw vertical lines
	/*
	for (int y = 0; y < HEIGHT; ++y) {
		im->imageData[y * WIDTH + x_left- 1] = 0;
		im->imageData[y * WIDTH + x_left] = 0;
		im->imageData[y * WIDTH + x_left + 1] = 0;

		im->imageData[y * WIDTH + x_right - 1] = 0;
		im->imageData[y * WIDTH + x_right] = 0;
		im->imageData[y * WIDTH + x_right + 1] = 0;
	}
	*/

	return;
}

static void compute_delta(trajectory_t *t) {
	int pix_delta_x, pix_delta_y;
	 
	pix_delta_x = t->dest_pix_x = t->paddle_pix_x;
	pix_delta_y = t->dest_pix_y = t->paddle_pix_y;

	t->delta_x = (pix_delta_x * 125) / WIDTH;
	t->delta_y = (pix_delta_y * 94) / HEIGHT;

	return;
}

