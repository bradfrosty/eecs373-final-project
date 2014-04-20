#include "kinect.h"
#include "lcd.h"
#include "N64.h"
#include "motors.h"

#include "drivers/mss_uart/mss_uart.h"

#include <stdio.h>

static uint8_t dir_x = 0;
static uint8_t dir_y = 0;

static void process_camera_data(uint8_t *rx_buff, uint32_t rx_size);
void kinect_data_handler(mss_uart_instance_t *this_uart);

void init_kinect() {
	//diable interupts from N64
	disable_N64();

	//initialize the UART
	MSS_UART_init(&g_mss_uart0, MSS_UART_57600_BAUD, MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT );

	//set irq handler for image data
	MSS_UART_set_rx_handler(&g_mss_uart0, kinect_data_handler, MSS_UART_FIFO_ONE_BYTE);
}

void kinect_data_handler(mss_uart_instance_t *this_uart) {
	uint8_t *rx_buff;
	uint32_t rx_size  = 0;
	rx_size = MSS_UART_get_rx(this_uart, rx_buff, sizeof(rx_buff));
	process_camera_data( rx_buff, rx_size );
}

void process_camera_data(uint8_t *rx_buff, uint32_t rx_size) {
	if (rx_size != 1)
		printf("WTF\n");

	//if we receive x,y data
	if ((*rx_buff & 0xF0) == 0 && (*rx_buff & 0x0F) != dir_x) {
		MYMOTORS->direction_x = *rx_buff & 0x0F;
	}
	else if ((*rx_buff & 0xF0) == 1 && (*rx_buff & 0x0F) != dir_y) {
		MYMOTORS->direction_y = *rx_buff & 0x0F;
	}
	else if ((*rx_buff & 0xF0) == 2) {
		updateScore(BOT);
	}
	else if ((*rx_buff & 0xF0) == 2) {
		updateScore(PLAYER);
	}
}
