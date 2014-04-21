#include "kinect.h"
#include "N64.h"
#include "lcd.h"
#include "switch.h"

#include <stdio.h>
#include <inttypes.h>

int main() {
	/*********************** DEBUGGING CODE ***********************/
	//init_N64();
	init_kinect();

	while(1) {}
	/************************* ACTUAL CODE ************************/
	init_switch(); // also initializes N64 or Kinect
	initLCD();


	while(1) {}
	return 0;
}


