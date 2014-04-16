#include "lcd.h"

#include "drivers/mss_uart/mss_uart.h"

#define BYTE 8
#define CHARCOMMAND 0xFE
#define SPECIALCOMMAND 0x7C
#define CLEAR 0x01
#define HOME 0x80
#define SCORE0 0x30
#define SPACE 0x20
#define DISPSETTINGS 0x0C
#define DISPTYPEWIDE 0x03
#define DISPTYPELINES 0x05
#define CURSORLEFT 0x10
#define CURSORRIGHT 0x14

player_t Player;
player_t Bot;

void initLCD(void)
{
	Player.score = SCORE0;
	Player.cursor = 0x98;
	Player.prev_Digit = SCORE0;

	Bot.score = SCORE0;
	Bot.cursor = 0xA2;
	Bot.prev_Digit = SCORE0;

	MSS_UART_init
	     (
	         &g_mss_uart1,
	         MSS_UART_9600_BAUD,
	         MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT
	     );

	delay();
	home();
	delay();
	clearDisp();
	delay();
	initScreen();
	delay();

	return;
}

//delay loop for screen transactions

void delay() {
	int i, j;
	volatile int d;
	for(i=0;i<1000;++i) {
		for(j=0; j<1000; ++j) {
			d=0;
		}
	}

}

void delayLong() {
	int i, j;
	volatile int d;
	for(i=0;i<1000;++i) {
		for(j=0; j<10000; ++j) {
			d=0;
		}
	}

}

void command(void) {
	uint8_t charCommand = CHARCOMMAND;
	MSS_UART_polled_tx( &g_mss_uart1, &charCommand, sizeof(charCommand));
	return;
}

//clear screen and move cursor to home position
void clearDisp() {
	uint8_t clearDispCom = CLEAR;
	command();
	MSS_UART_polled_tx( &g_mss_uart1, &clearDispCom, sizeof(clearDispCom));
	return;
}

//move cursor to home position w/o clearing screen
void home() {
	uint8_t homeCom = HOME;
	command();
	MSS_UART_polled_tx( &g_mss_uart1, &homeCom, sizeof(homeCom));
	return;
}

//initialize screen to display game title and teams labels
void initScreen() {
	uint8_t title[15] = "373 Air Hockey";
	uint8_t teams[15] = "Player     Bot";

	uint8_t setTitleCursor = 0x83; //set cursor to position 4 on first line
	uint8_t setTeamCursor = 0xC2; //set cursor to position 3 on second line
	Bot.score = SCORE0;
	Player.score = SCORE0;


	/******************************************/
	//FIGURE OUT IF THIS STUFF IS NECESSARY

	//MSS_UART_polled_tx( &g_mss_uart1, &charCommand, sizeof(charCommand));							//configure #characters in line (20)
	//MSS_UART_polled_tx( &g_mss_uart1, &dispSettingWide, sizeof(dispSettingWide));
	//MSS_UART_polled_tx( &g_mss_uart1, &charCommand, sizeof(charCommand));							//configure #lines on screen (4)
	//MSS_UART_polled_tx( &g_mss_uart1, &dispSettingLines, sizeof(dispSettingLines));
	//MSS_UART_polled_tx( &g_mss_uart1, &charCommand, sizeof(charCommand));							//confugre display settings (defined above)
	//MSS_UART_polled_tx( &g_mss_uart1, &configureDisp, sizeof(configureDisp));
	command();
	MSS_UART_polled_tx( &g_mss_uart1, &setTitleCursor, sizeof(setTitleCursor));						//move cursor to title position on first line
	MSS_UART_polled_tx( &g_mss_uart1, title, sizeof(title)-1);										//write title to screen
	//delete();
	command();
	MSS_UART_polled_tx( &g_mss_uart1, &setTeamCursor, sizeof(setTeamCursor));						//move cursor to team position on second line
	MSS_UART_polled_tx( &g_mss_uart1, teams, sizeof(teams)-1);										//write team names to screen
	command();
	MSS_UART_polled_tx( &g_mss_uart1, &Player.cursor, sizeof(Player.cursor));			//move player score cursor
	MSS_UART_polled_tx( &g_mss_uart1, &Player.score, sizeof(Player.score));							//write 0 as initial score
	command();
	MSS_UART_polled_tx( &g_mss_uart1, &Bot.cursor, sizeof(Bot.cursor));				//move bot score cursor
	MSS_UART_polled_tx( &g_mss_uart1, &Bot.score, sizeof(Bot.score));									//write 0 as initial score

	return;
}

void moveCursor(uint8_t dir) {
	command();
	MSS_UART_polled_tx( &g_mss_uart1, &dir, sizeof(dir));
	return;
}

void backspace() {
	uint8_t spaceChar[1] = " ";
	moveCursor(CURSORLEFT);
	MSS_UART_polled_tx( &g_mss_uart1, spaceChar, sizeof(spaceChar));
	return;
}

uint8_t prevDigitBotCursor = 0xA1;
uint8_t prevDigitPlayerCursor = 0x97;
void updateScore(uint8_t player) {


	if(player == BOT) {
		command();
		MSS_UART_polled_tx( &g_mss_uart1, &(Bot.cursor), sizeof(Bot.cursor));
		delay();
		++Bot.score;
		if(Bot.score < 0x3A){ //if botscore remains in number range
			command();
			MSS_UART_polled_tx( &g_mss_uart1, &Bot.cursor, sizeof(Bot.cursor));
			//delay();
			MSS_UART_polled_tx( &g_mss_uart1, &Bot.score, sizeof(Bot.score));
			//delay();
		}
		else {
			command();
			MSS_UART_polled_tx( &g_mss_uart1, &(prevDigitBotCursor), sizeof(prevDigitBotCursor));
			++Bot.prev_Digit;
			//backspace();
			//delay();
			MSS_UART_polled_tx( &g_mss_uart1, &Bot.prev_Digit, sizeof(Bot.prev_Digit));
			//delay();
			//moveCursor(CURSORRIGHT);
			Bot.score = SCORE0;
			//delay();
			MSS_UART_polled_tx( &g_mss_uart1, &Bot.score, sizeof(Bot.score));
			delay();
		}

	}
	else {
		command();
		MSS_UART_polled_tx( &g_mss_uart1, &Player.cursor, sizeof(Player.cursor));
		delay();
		++Player.score;
		if(Player.score < 0x3A){ //if botscore remains in number range
			command();
			MSS_UART_polled_tx( &g_mss_uart1, &Player.cursor, sizeof(Player.cursor));
			MSS_UART_polled_tx( &g_mss_uart1, &Player.score, sizeof(Player.score));
			delay();
		}
		else {
			command();
			MSS_UART_polled_tx( &g_mss_uart1, &(prevDigitPlayerCursor), sizeof(prevDigitPlayerCursor));
			++Player.prev_Digit;
			//backspace();
			//delay();
			MSS_UART_polled_tx( &g_mss_uart1, &Player.prev_Digit, sizeof(Player.prev_Digit));
			//delay();
			//moveCursor(CURSORRIGHT);
			Player.score = SCORE0;
			//delay();
			MSS_UART_polled_tx( &g_mss_uart1, &Player.score, sizeof(Player.score));
			delay();
		}
	}

	return;
}
