#include "mss_uart.h"

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

#define BOT 0
#define PLAYER 1


int i=0;
int j=0;

uint8_t charCommand = CHARCOMMAND; 										//for all commands
uint8_t specialCommand = SPECIALCOMMAND; 								//for other commands?
uint8_t clearDispCom = CLEAR; 											//clear display command code
uint8_t homeCom = HOME; 												//set cursor home position command code
uint8_t configureDisp = 0x0E;											//display settings: disp ON, cursor ON, cursor blink OFF
uint8_t dispSettingWide = 0x03;											//set display as 20 characters wide
uint8_t dispSettingLines = 0x05;										//set display as 4 lines long
uint8_t functionSet = 0x28;												//don't worry
uint8_t entryMode = 0x06;												//don't worry
uint8_t defaultCom = 0x12; 												//default settings command, use in loop and sent continously if screen has bugs
//uint8_t score = ZERO;													//ascii code for 0
uint8_t space = SPACE;
uint8_t playerScore = SCORE0;											//player score var
uint8_t botScore = SCORE0;												//bot score var
uint8_t setPlayerScoreCursor = 0x98; 									//set cursor to position 5 on third line
uint8_t setBotScoreCursor = 0xA2; 										//set cursor to position 15 on third line

void delay(void);
void delayLong(void);
void command(void);
void clearDisp(void);
void home(void);
void initScreen(void);
void moveCursor(uint8_t);
void updateScore(uint8_t);


int main()
{

	MSS_UART_init
	     (
	         &g_mss_uart1,
	         MSS_UART_9600_BAUD,
	         MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT
	     );



	//MSS_UART_enable_irq( &g_mss_uart0,( MSS_UART_RBF_IRQ) );

	//uint8_t phrase[12] = "Hello World!";
	
	delay();
	home();
	delay();
	clearDisp();
	delay();
	initScreen();
	delay();

	int l = 0;
	while(l <15) {
		updateScore(BOT);
		delay();
		++l;
	}


	//delayLong();
	updateScore(PLAYER);
	delay();

	//delay();
	//MSS_UART_polled_tx( &g_mss_uart1, phrase, sizeof(phrase) );
	//delay();
	//clearDisp();
	//delay();




	while( 1 )
	{
		
	}
	return 0;
}

//delay loop for screen transactions
volatile int d;
void delay() {
	for(i=0;i<1000;++i) {
		for(j=0; j<1000; ++j) {
			d=0;
		}
	}

}

volatile int e;
void delayLong() {
	for(i=0;i<1000;++i) {
			for(j=0; j<10000; ++j) {
				e=0;
			}
		}

}

void command(void) {
	MSS_UART_polled_tx( &g_mss_uart1, &charCommand, sizeof(charCommand));
	return;
}

//clear screen and move cursor to home position
void clearDisp() {
	command();
	MSS_UART_polled_tx( &g_mss_uart1, &clearDispCom, sizeof(clearDispCom));
	return;
}

//move cursor to home position w/o clearing screen
void home() {
	command();
	MSS_UART_polled_tx( &g_mss_uart1, &homeCom, sizeof(homeCom));
	return;
}

//intialize screen to display game title and teams labels
void initScreen() {
	uint8_t title[15] = "373 Air Hockey";
	uint8_t teams[15] = "Player     Bot";

	uint8_t setTitleCursor = 0x83; //set cursor to position 4 on first line
	uint8_t setTeamCursor = 0xC2; //set cursor to position 3 on second line


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
	MSS_UART_polled_tx( &g_mss_uart1, &setPlayerScoreCursor, sizeof(setPlayerScoreCursor));			//move player score cursor
	MSS_UART_polled_tx( &g_mss_uart1, &playerScore, sizeof(playerScore));							//write 0 as initial score
	command();
	MSS_UART_polled_tx( &g_mss_uart1, &setBotScoreCursor, sizeof(setBotScoreCursor));				//move bot score cursor
	MSS_UART_polled_tx( &g_mss_uart1, &botScore, sizeof(botScore));									//write 0 as initial score

	return;
}

void moveCursor(uint8_t dir) {
	command();
	MSS_UART_polled_tx( &g_mss_uart1, &dir, sizeof(dir));
	return;
}

void delete() {
	moveCursor(CURSORLEFT);
	MSS_UART_polled_tx( &g_mss_uart1, &space, sizeof(&space));
	return;
}

uint8_t prevDigitBot = SCORE0;
uint8_t prevDigitPlayer = SCORE0;
void updateScore(uint8_t player) {

	if(player == BOT) {
		command();
		MSS_UART_polled_tx( &g_mss_uart1, &setBotScoreCursor, sizeof(setBotScoreCursor));
		delay();
		++botScore;
		if(botScore < 0x3A){ //if botscore remains in number range
			//moveCursor(CURSORRIGHT);
			delete();
			//moveCursor(CURSORLEFT);
			command();
			MSS_UART_polled_tx( &g_mss_uart1, &botScore, sizeof(botScore));
			delay();
		}
		else {
			++prevDigitBot;
			++setBotScoreCursor;
			delete();
			delay();
			command();
			delay();
			MSS_UART_polled_tx( &g_mss_uart1, &prevDigitBot, sizeof(prevDigitBot));
			delay();
			moveCursor(CURSORRIGHT);
			delay();
			botScore = SCORE0;
			command();
			delay();
			MSS_UART_polled_tx( &g_mss_uart1, &botScore, sizeof(botScore));
			delay();
		}

	}
	else {
		command();
		MSS_UART_polled_tx( &g_mss_uart1, &setPlayerScoreCursor, sizeof(setPlayerScoreCursor));
		delay();
		++playerScore;
		if(playerScore < 0x3A){ //if botscore remains in number range
			delete();
			command();
			MSS_UART_polled_tx( &g_mss_uart1, &playerScore, sizeof(playerScore));
			delay();
		}
		else {
			++prevDigitPlayer;
			++setPlayerScoreCursor;
			delete();
			command();
			MSS_UART_polled_tx( &g_mss_uart1, &prevDigitPlayer, sizeof(prevDigitPlayer));
			moveCursor(CURSORRIGHT);
			playerScore = SCORE0;
			command();
			MSS_UART_polled_tx( &g_mss_uart1, &playerScore, sizeof(playerScore));
		}
	}

	return;
}

