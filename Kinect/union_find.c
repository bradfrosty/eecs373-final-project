#include "union_find.h"
#include "trajectory.h"
#include <math.h>
#include <assert.h>
#include <opencv/cv.h>

static int y_top = 85;
static int y_bot = HEIGHT - 60;

static int x_left = 20;
static int x_right = WIDTH / 2 - 55;

static int table_top = 45;
static int table_bot = HEIGHT - 33;

static int table_left = 20;
static int table_right = WIDTH - 30;

typedef struct union_str union_t;
struct union_str {
	union_t *parent;
	int num_children;
	int min_x;
	int min_y;
	int max_x;
	int max_y;
};

static int color_error(uint8_t red, uint8_t green, uint8_t blue, uint32_t color);
static void make_set(uint8_t *picture, union_t *udata, uint32_t color);
static void make_blobs(union_t *udata, int im_height, int im_width); // creates blobs from udata
static void link_nodes(union_t *master, union_t *child); //adds child to the blob parent is a part of
static union_t *find_root(union_t *node); //adds child to the blob parent is a part of
int pass_scan(uint8_t *frame, union_t *udata, int im_height, int im_width, trajectory_t* t, uint32_t color); //puts up to num_coords into coords array
int is_green(uint8_t *picture, int h, int w, const int threshold, uint32_t color);

int union_find(uint8_t *im, uint32_t color, trajectory_t *t) {
	int im_height = 480;
	int im_width = 640;
	
	// int im_stride = frame->stride;
	int num_pixels = im_height*im_width;
	union_t *udata = new union_t[num_pixels];

	make_set(im, udata, color);

	make_blobs(udata, im_height, im_width);

	pass_scan(im, udata, im_height, im_width, t, color);

	delete[] udata;

	return 0;

}

void make_set(uint8_t *picture, union_t *udata, uint32_t color) {
	const int threshold = 1;

	int im_height = 480;
	int im_width = 640;
	union_t *iter = udata + (table_top*im_width) + table_left;

	for (int h = table_top; h < table_bot; ++h) {
		for (int w = table_left; w < table_right; ++w) {
			if (is_green(picture, h, w, threshold, color)){ // significant pixel
				//set up union information
				iter->parent = iter;
				iter->min_x = w;
				iter->min_y = h;
				iter->max_x = w;
				iter->max_y = h;
				iter->num_children = 1;
			}
			else {
				iter->parent = NULL;
			}

			++iter;
		}
		iter = udata + (h*im_width) + table_left;
	}
}

int is_green(uint8_t *picture, int h, int w, const int threshold, uint32_t color) {
	//uint8_t red = picture[3 * (h * 640 + w)]; //H
	//uint8_t green = picture[3 * (h * 640 + w) + 1]; //S
	//uint8_t blue = picture[3 * (h * 640 + w) + 2]; //V

	uint8_t gray = picture[h * WIDTH + w];

	//int diff = color_error(red, green, blue, color);
	int diff = color_error(gray, gray, gray, color);
	if (diff < threshold) {
		return 1;
	}
	else
		return 0;
}
void make_blobs(union_t *udata, int im_height, int im_width){

	union_t *iter = udata + (table_top+1)*im_width + table_left+1; //Stride?
	union_t *top_neighbor;
	union_t *left_neighbor;

	for (int h = table_top+1; h < table_bot; ++h) {
		for (int w = table_left+1; w < table_right; ++w) {
			if (iter->parent){
				top_neighbor = iter - im_width; //Stride?
				left_neighbor = iter - 1;

				if (top_neighbor->parent && left_neighbor->parent){
					link_nodes(top_neighbor, left_neighbor);
					link_nodes(top_neighbor, iter);
				}
				else if (left_neighbor->parent) {
					link_nodes(left_neighbor, iter);
				}
				else if (top_neighbor->parent) {
					link_nodes(top_neighbor, iter);
				}
			}

			++iter;
		}
		iter = udata + (h*im_width) + table_left;//Stride?
	}
}

void link_nodes(union_t *master, union_t *child){
	union_t *masterRoot = find_root(master);
	union_t *childRoot = find_root(child);
	if (masterRoot == childRoot) return;

	childRoot->parent = masterRoot;

	if (childRoot->min_x < masterRoot->min_x){
		masterRoot->min_x = childRoot->min_x;
	}
	if (childRoot->min_y < masterRoot->min_y){
		masterRoot->min_y = childRoot->min_y;
	}

	if (childRoot->max_x > masterRoot->max_x){
		masterRoot->max_x = childRoot->max_x;
	}
	if (childRoot->max_y > masterRoot->max_y){
		masterRoot->max_y = childRoot->max_y;
	}

	masterRoot->num_children += childRoot->num_children;
	childRoot->num_children = 0;
}

union_t *find_root(union_t *node){
	/*finds parents and changes parents' parents up the chain*/

	if (node->parent != node) {
		node->parent = find_root(node->parent);
	}
	return node->parent;

}

extern int max_size = 0, min_size = 1000;

int pass_scan(uint8_t *frame, union_t *udata, int im_height, int im_width, trajectory_t *t, uint32_t color) {

	const int upper_pixel_limit = 1100;
	const int lower_pixel_limit = 700;

	union_t *iter = udata + table_top*im_width + table_left;

	//int index = 0;

	for (int h = table_top; h < table_bot; ++h) {
		for (int w = table_left; w < table_right; ++w) {
			//if something is its own parent
			if (iter == iter->parent){

				if (iter->num_children < upper_pixel_limit && iter->num_children > lower_pixel_limit){
					double ratio = (double)(iter->max_x - iter->min_x) / (double)(iter->max_y - iter->min_y);
					if (ratio > 0.9 && ratio < 1.1) {
						int blob_x = (iter->max_x + iter->min_x) / 2;
						int blob_y = (iter->max_y + iter->min_y) / 2;

						int real_x = (blob_x * 125) / im_width;
						int real_y = (blob_y * 94) / im_height;

						// to be added to improve accuracy
						//                        if(is_green(frame, blob_y, blob_x, 82, color)) {
						//printf("x:%d, y:%d, size:%d\n", real_x, real_y, iter->num_children);

						if (iter->num_children > max_size)
							max_size = iter->num_children;
						else if (iter->num_children < min_size)
							min_size = iter->num_children;

						for (int i = blob_y - 5; i < blob_y + 5; ++i) {
							for (int j = blob_x - 5; j < blob_x + 5; ++j) {
								if (i >= 0 && i < im_height && j >= 0 && j < im_width) {
									frame[(i * WIDTH + j)] = 0;
									//frame[3*(i * WIDTH + j) + 1] = 0;
									//frame[3*(i * WIDTH + j) + 2] = 0;
								}
							}
						}

						//t->prev_x = t->cur_x;
						//t->prev_y = t->cur_y;
						t->prev_pix_x = t->cur_pix_x;
						t->prev_pix_y = t->cur_pix_y;

						//t->cur_x = real_x;
						//t->cur_y = real_y;
						t->cur_pix_x = blob_x;
						t->cur_pix_y = blob_y;
						
						//printf("slope:%f\n", t->pix_slope);
					}
				}
			}
			++iter;
		}
		iter = udata + (h*im_width) + table_left;
	}

	//system("pause");

	return 0;

}

int color_error(uint8_t red, uint8_t green, uint8_t blue, uint32_t color) {

	//uint8_t H = (color >> 16) & 0xFF;
	//int8_t S = (color >> 8) & 0xFF;
	//uint8_t V = (color) & 0xFF;

	//int error = abs(S - green) + abs(H - red) + abs(V - blue);

	if ((red < 170) && (red > 40))
		return 0;
	else
		return 1;
}


typedef struct {
	int x;
	int y;
} coord_t;

void paddle_detection(uint8_t *im, trajectory_t *t) {

	coord_t points[10000];

	int start = 0;
	int start_white = 0;
	int end_white = 0;
	int count = 0;

	int x = WIDTH / 3;
	for (int x = x_left; x < x_right; ++x) {
		for (int y = y_top; y < y_bot; ++y) {

			if (im[y * WIDTH + x] < 100 && !start) {
				start = y;
				//printf("x:%d, y:%d\n", x, y);
			}
			if (y - start > 30 && !start_white)
				start = 0;



			if (im[y * WIDTH + x] > 170 && start && !start_white)
				start_white = y;

			if (y - start_white > 50 && start_white && !end_white)
				start = start_white = 0;

			if (im[y * WIDTH + x] < 100 && start_white && !end_white) {
				assert(count < 100);
				end_white = y;
				//im[((start_white + end_white) / 2)* WIDTH + x] = 0;
				points[count].x = x;
				points[count].y = ((start_white + end_white) / 2);
				count++;
				//printf("at y = %d\n", y);
			}
		}
		start = start_white = end_white = 0;
	}

	//find average
	int avg_x = 0;
	int avg_y = 0;
	for (int i = 0; i < count; ++i) {
		avg_x += points[i].x;
		avg_y += points[i].y;
	}

	if (count) {
		avg_x = avg_x / count;
		avg_y = avg_y / count;
	}
	
	//remove outlyers
	int corrected_avg_x = 0;
	int corrected_avg_y = 0;
	int corrected_count = 0;
	for (int i = 0; i < count; ++i) {
		if (abs(points[i].x - avg_x) < 10 && abs(points[i].y - avg_y) < 5) {
			corrected_avg_x += points[i].x;
			corrected_avg_y += points[i].y;
			corrected_count++;
		}
	}

	if (corrected_count) {
		corrected_avg_x = corrected_avg_x / corrected_count;
		corrected_avg_y = corrected_avg_y / corrected_count;
	}

	if (corrected_count > 1) {
		t->paddle_pix_x = corrected_avg_x + 30;
		t->paddle_pix_y = corrected_avg_y;
		/*
		for (int y = t->paddle_pix_y - 2; y <= t->paddle_pix_y; ++y) {
			for (int x = t->paddle_pix_x - 2; x <= t->paddle_pix_x; ++x) {
				im[y* WIDTH + x] = 0;
			}
		}
		*/
	}

	//printf("count:%d \tcorrected count:%d\n", count, corrected_count);
	return;
}