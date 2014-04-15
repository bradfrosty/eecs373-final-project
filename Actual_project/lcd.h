#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define BOT 0
#define PLAYER 1

typedef struct {
	uint8_t score;
	uint8_t cursor;
	uint8_t prev_Digit;
} player_t;

//LCD command firmware function declarations
void initLCD();
void delay();
void delayLong();
void command();
void clearDisp();
void home();
void initScreen();
void moveCursor(uint8_t);
void backspace();
void updateScore(uint8_t);

#endif
