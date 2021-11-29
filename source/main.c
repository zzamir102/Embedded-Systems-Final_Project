/*	Author: Zamir Noor
 *  Partner(s) Name: 
 *	Lab Section:21
 *	Assignment: Custom Lab Assignment
 *	Exercise Description: [optional - include for your own benefit]
 *	Video Link: https://www.youtube.com/watch?v=-z5c4OJ9VMc
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <timer.h>
#include <keypad.h>
#include <io.h>
#include <scheduler.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#define startButton (~PINA & 0x01)
#define scoreButton (~PINA & 0x02)
#define resetButton (~PINA & 0x04)
unsigned char startGame = 0x00;
unsigned char bestScore = 8;
unsigned char character1[8] = { 0x00,0x04,0x0A,0x11,0x0A,0x04,0x04,0x00 };
unsigned char character2[8] = { 0x1F,0x00,0x0A,0x00,0x11,0x0E,0x00,0x1F };
unsigned char character3[8] = { 0x11,0x0E,0x0A,0x04,0x15,0x0E,0x04,0x04 };
unsigned char character4[8] = { 0x04,0x04,0x1F,0x15,0x04,0x0E,0x11,0x11 };

enum toggleStart_States {toggleStart_Init, toggleStart_press, toggleScore_press, toggleStart_wait, toggleScore_wait};

int toggleStartTick (int state) {
	switch(state) {
		case toggleStart_Init:
			if (startButton && !scoreButton) {
				state = toggleStart_press;
			}
			else if (scoreButton && !startButton) {
				state = toggleScore_press;
			}
			else {
				state = toggleStart_Init;
			}
			break;
		case toggleStart_press:
			if (startButton) {
				state = toggleStart_press;
			}
			else {
				state = toggleStart_wait;
			}
			break;
		case toggleStart_wait:
			if (resetButton) {
				state = toggleStart_Init;
			}
			else {
				state = toggleStart_wait;
			}
			break;
		case toggleScore_press:
			if (scoreButton) {
				state = toggleScore_press;
			}
			else {
				state = toggleScore_wait;
			}
			break;
		case toggleScore_wait:
			if (resetButton) {
				state = toggleStart_Init;
			}
			else {
				state = toggleScore_wait;
			}
			break;
		default:
			LCD_DisplayString(1, "Error");
			break;
	}
	switch(state) {
		case toggleStart_Init:
			startGame = 0x00;
			LCD_DisplayString(1, "Menu: 1 to Play or 2 for Score");
			break;
		case toggleStart_press:
			LCD_ClearScreen();	
			break;
		case toggleStart_wait:
			startGame = 0x01;
			//LCD_DisplayString(1, "Game started"); //testing states
			break;
		case toggleScore_press:
			LCD_ClearScreen();
			break;
		case toggleScore_wait:
			LCD_CustomChar(0, character1);
			LCD_WriteData(0x00);
			break;
	}
	return state;
}

enum chooseChar_States {charInit, charSelect};

int chooseChar(int state) {

	switch(state) {
		case charInit:
			if (startGame == 0x00) {
				state = charInit;
			}
			else if (startGame == 0x01) {
				state = charSelect;
			}
			break;
		case charSelect:
			if (startGame == 0x01) {
				state = charSelect;
			}
			else {
				state = charInit;
			}
			break;
		default:
			break;
	}
	switch(state) {
		case charInit:
			break;
		case charSelect:
			LCD_DisplayString(17, "Character Select");
			LCD_CustomChar(0, character1);
			LCD_CustomChar(1, character2);
			LCD_CustomChar(2, character3);
			LCD_CustomChar(3, character4);
			LCD_Cursor(0x01);
			LCD_WriteData(0x00);
			LCD_Cursor(0x03);
			LCD_WriteData(0x01);
			LCD_Cursor(0x05);
			LCD_WriteData(0x02);
			LCD_Cursor(0x07);
			LCD_WriteData(0x03);
			break;
		default:
			break;
	}

	return state;
}


int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00;	PORTA = 0xFF;
	DDRC = 0xFF;	PORTC = 0x00;
	DDRD = 0xFF;	PORTD = 0x00;
    /* Insert your solution below */
	LCD_init();
	static task task1, task2;
	task *tasks[] = {&task1, &task2};
	const unsigned short numTasks = sizeof(tasks) / sizeof(task*);
	const char start = 0;

	task1.state = start;
	task1.period = 1000;
	task1.elapsedTime = task1.period;
	task1.TickFct = &toggleStartTick;

	task2.state = start;
	task2.period = 1000;
	task2.elapsedTime = task2.period;
	task2.TickFct = &chooseChar;

	TimerSet(1000);
	TimerOn();


	unsigned short i;
	while (1) {
	for (i = 0; i < numTasks; i++) {
		if (tasks[i]->elapsedTime == tasks[i]->period) {
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += 1000;
	}
	while (!TimerFlag);
	TimerFlag = 0;
    }
    return 0;
}
