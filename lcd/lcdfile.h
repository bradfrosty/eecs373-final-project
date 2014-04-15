#ifndef LCDFILE_H
#define LCDFILE_H

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

#define BOT 0
#define PLAYER 1


int i=0;
int j=0;
volatile int d;
volatile int e;

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
uint8_t prevDigitBot = SCORE0;
uint8_t prevDigitPlayer = SCORE0;


//LCD command firmware function declarations
void initLCD(void);
void delay(void);
void delayLong(void);
void command(void);
void clearDisp(void);
void home(void);
void initScreen(void);
void moveCursor(uint8_t);
void backspace(void);
void updateScore(uint8_t);

#endif
