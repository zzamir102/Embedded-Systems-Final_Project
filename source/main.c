/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
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
unsigned char character[8] = { 0x04,0x04,0x04,0x04,0x15,0x0e,0x04,0x00 };


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
			LCD_DisplayString(1, "Game started"); //testing states
			break;
		case toggleScore_press:
			LCD_ClearScreen();
			break;
		case toggleScore_wait:
			LCD_CustomChar(0, character);
			LCD_WriteData(0x00);
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
	static task task1;
	task *tasks[] = {&task1};
	const unsigned short numTasks = sizeof(tasks) / sizeof(task*);
	const char start = 0;

	task1.state = start;
	task1.period = 1000;
	task1.elapsedTime = task1.period;
	task1.TickFct = &toggleStartTick;

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
