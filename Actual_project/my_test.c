#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "libfreenect_sync.h"

#include "union_find.h"
#include "rs232.h"
#include "trajectory.h"
#include <Windows.h>

using namespace cv;
using namespace std;

int main()
{
	int i, n,
		cport_nr = 2,        // /dev/ttyS2 (COM3 on windows) 
		bdrate = 57600;       // 9600 baud
	if (RS232_OpenComport(cport_nr, bdrate))
	{
		printf("Can not open comport\n");
		return(0);
	}

	//u8x3_t
	IplImage *im_RGB = cvCreateImageHeader(cvSize(WIDTH, HEIGHT), 8, 3);
	IplImage *im_GRAY = cvCreateImage(cvSize(WIDTH, HEIGHT), 8, 1);
	//IplImage *im_HSV = cvCreateImageHeader(cvSize(WIDTH, HEIGHT), 8, 3);

	//cvNamedWindow("kinect", CV_WINDOW_AUTOSIZE);
	uint8_t *data_RGB;
	//uint8_t *data_HSV;

	trajectory_t trajectory;
	//trajectory.cur_time = 0;
	trajectory.dest_pix_x = 9*WIDTH/10;

	int dir_x = 0;
	int dir_y = 0;
	int prev_dir_x = 0;
	int prev_dir_y = 0;
	int cur_dir_x = 0;
	int cur_dir_y = 0;

	while (cvWaitKey(1) < 0)
	{
		unsigned int timestamp;

		freenect_sync_get_video((void**)(&data_RGB), &timestamp, 0, FREENECT_VIDEO_RGB);
		//freenect_sync_get_video((void**)(&data_HSV), &timestamp, 0, FREENECT_VIDEO_RGB);

		//trajectory.prev_time = trajectory.cur_time;
		//trajectory.cur_time = timestamp;

		uint32_t H = 70;
		uint32_t S = 15;
		uint32_t V = 200;

		uint32_t hsv_color = (H << 16) | (S << 8) | V;

		//union_find(data_RGB, rgb_color, c);

		cvSetData(im_RGB, data_RGB, 640 * 3);
		//cvSetData(im_HSV, data_HSV, 640 * 3);

		//cvCvtColor(im_RGB, im_RGB, CV_RGB2BGR);
		cvCvtColor(im_RGB, im_GRAY, CV_RGB2GRAY);
		//cvCvtColor(im_HSV, im_HSV, CV_RGB2HSV);

		paddle_detection((uint8_t*)im_GRAY->imageData, &trajectory);
		union_find((uint8_t*)im_GRAY->imageData, hsv_color, &trajectory);
		compute_trajectory(&trajectory, (uint8_t*)im_GRAY->imageData);
		draw_walls(im_GRAY);

		trajectory_t *t = &trajectory;

		if (t->paddle_pix_x > 0 && t->paddle_pix_x < WIDTH && t->paddle_pix_y > 0 && t->paddle_pix_y < HEIGHT) {
			for (int y = t->paddle_pix_y - 2; y <= t->paddle_pix_y; ++y) {
				for (int x = t->paddle_pix_x - 2; x <= t->paddle_pix_x; ++x) {
					im_GRAY->imageData[y* WIDTH + x] = 0;
				}
			}
		}

		prev_dir_x = cur_dir_x;
		prev_dir_y = cur_dir_y;
		if (trajectory.dest_pix_y != -1) {
			if (trajectory.paddle_pix_x > 50 &&
				trajectory.paddle_pix_x < (WIDTH / 2 - 75)) {
				if (trajectory.paddle_pix_x - trajectory.dest_pix_x > 5)
					cur_dir_x = 2;
				else if (trajectory.paddle_pix_x - trajectory.dest_pix_x < 5)
					cur_dir_x = 1;
				else
					cur_dir_x = 3;
			}
			else
				cur_dir_x = 3;

			if (trajectory.paddle_pix_y > 70 &&
				trajectory.paddle_pix_y < HEIGHT - 53) {
				if (trajectory.paddle_pix_y - trajectory.dest_pix_y > 5)
					cur_dir_y = 2;
				else if (trajectory.paddle_pix_y - trajectory.dest_pix_y < 5)
					cur_dir_y = 1;
				else
					cur_dir_y = 3;
			}
			else
				cur_dir_y = 3;
		}
		else {
			cur_dir_y = 3;
			cur_dir_x = 3;
		}

		uint8_t data = 0x00;
		//RS232_SendByte(cport_nr, data);


		if (cur_dir_x == prev_dir_x) {
			if (cur_dir_x != dir_x) {
				data = cur_dir_x;
				data = data & 0x0F;
				RS232_SendByte(cport_nr, data);
				printf("going x = %d\n", cur_dir_x);
			}
			dir_x = cur_dir_x;
		}
		
		if (cur_dir_y == prev_dir_y) {
			if (cur_dir_y != dir_y) {
				data = cur_dir_y;
				data = data | 0x10;
				data = data & 0x1F;
				RS232_SendByte(cport_nr, data);
				printf("going y = %d\n", cur_dir_y);
			}
			dir_y = cur_dir_y;
		}

		//cvShowImage("RGB", im_RGB);
		cvShowImage("GRAY", im_GRAY);
		//cvShowImage("HSV", im_HSV);
	}
	freenect_sync_stop();
	cvFree(&im_GRAY);
	cvFree(&im_RGB);
	//cvFree(&im_HSV);

	printf("max:%d, min:%d\n", max_size, min_size);
	system("pause");

	return 0;
}
