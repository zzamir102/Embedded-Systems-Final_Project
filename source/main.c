/*	Author: Zamir Noor
 *  Partner(s) Name: 
 *	Lab Section:21
 *	Assignment: Custom Lab Assignment
 *	Website Used: https://scienceprog.com/using-analog-joystick-in-avr-projects/
 *		      https://www.electronicwings.com/avr-atmega/lcd-custom-character-display-using-atmega-16-32-
 *	Exercise Description: [optional - include for your own benefit]
 *	Video Link: https://www.youtube.com/watch?v=tRqaPzXBC9E
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <avr/eeprom.h>
#include <timer.h>
#include <keypad.h>
#include <io.h>
#include <scheduler.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#define startButton (~PINA & 0x08)
#define scoreButton (~PINA & 0x10)
#define resetButton (~PINA & 0x20)
uint8_t highScore;
uint8_t characterChose;
unsigned char startGame = 0x00;
int position;
unsigned char character1[8] = { 0x00,0x04,0x0A,0x11,0x0A,0x04,0x04,0x00 };
unsigned char character2[8] = { 0x1F,0x00,0x0A,0x00,0x11,0x0E,0x00,0x1F };
unsigned char character3[8] = { 0x11,0x0E,0x0A,0x04,0x15,0x0E,0x04,0x04 };
unsigned char character4[8] = { 0x04,0x04,0x1F,0x15,0x04,0x0E,0x11,0x11 };

//helper functions if needed
void eeprom_Write(unsigned char addr, unsigned char data) {
	eeprom_write_byte(addr, data);
}

unsigned char eeprom_Read(unsigned char addr) {
	return eeprom_read_byte(addr);
}

void setHighScore() {
	eeprom_write_word((uint8_t*)20, 4);
	highScore = eeprom_read_byte((uint8_t*)20);
	//characterChose = eeprom_read_byte((uint8_t*)22);
}

void InitADC(void)
{
    ADMUX |= (1<<REFS0);
  
    ADCSRA |= (1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);
}

uint16_t ReadADC(uint8_t ADCchannel)
{
    //select ADC channel with safety mask
    ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
    //single conversion mode
    ADCSRA |= (1<<ADSC);
    // wait until ADC conversion is complete
    while( ADCSRA & (1<<ADSC) );
    return ADC;
}

int joystick() {
	int pos;
	unsigned short x;

	x = ReadADC(0);

	if (x >= 600) {
		pos = 1;
	}
	else if (x <= 500) {
		pos = 2;
	}
	else {
		pos = 0;
	}
	return pos;
}

unsigned int curr = 1;
int move() {
	position = joystick();
	if (position == 1 && curr < 3) {
		curr++;
	}
	else if (position == 2 && curr > 0) {
		curr--;
	}
	LCD_Cursor(curr);
	delay_ms(250);
	return curr;
}

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
			LCD_DisplayString(1, "Highest Score: ");
			LCD_Cursor(15);
			LCD_WriteData(highScore + '0');
			characterChose = eeprom_Read(0x00);
			if (characterChose == 1) {
				LCD_CustomChar(0, character1);
				LCD_Cursor(16);
				LCD_WriteData(0x00);	
			}
			else if (characterChose == 2) {
				LCD_CustomChar(0, character2);
				LCD_Cursor(16);
				LCD_WriteData(0x01);
			}
			break;
	}
	return state;
}

enum chooseChar_States {charInit, charSelect, charWait, initGame, waitGame};

int pos1 = 0;

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
				state = charWait;
			}
			else {
				state = charInit;
			}
			break;
		case charWait:
			if (startGame == 0x01) {
				if (startButton && pos1 == 1) {
					state = initGame;
					eeprom_Write(0x00, 1);
				}
				else if (startButton && pos1 == 3) {
					state = initGame;
					eeprom_Write(0x00, 2);
				}
				else {
					state = charWait;	
				}
				break;
			}
			else {
				state = charInit;
			}
			break;
		case initGame:
			if (startGame == 0x01) {
				state = waitGame;
			}
			else {
				state = charInit;
			}
			break;
		case waitGame:
			if (startGame == 0x01) {
				state = waitGame;
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
			LCD_Cursor(0x01);
			LCD_WriteData(0x00);
			LCD_Cursor(0x03);
			LCD_WriteData(0x01);
			LCD_Cursor(0x01);
			break;
		case charWait:
			pos1 = move();
			break;
		case initGame:
			LCD_DisplayString(1, "Game in progress");
			break;
		case waitGame:
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
	InitADC();
	setHighScore();
	static task task1, task2;
	task *tasks[] = {&task1, &task2};
	const unsigned short numTasks = sizeof(tasks) / sizeof(task*);
	const char start = 0;

	task1.state = start;
	task1.period = 1000;
	task1.elapsedTime = task1.period;
	task1.TickFct = &toggleStartTick;

	task2.state = start;
	task2.period = 10;
	task2.elapsedTime = task2.period;
	task2.TickFct = &chooseChar;

	TimerSet(10);
	TimerOn();

	unsigned short i;
	while (1) {
	for (i = 0; i < numTasks; i++) {
		if (tasks[i]->elapsedTime == tasks[i]->period) {
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += 10;
	}
	while (!TimerFlag);
	TimerFlag = 0;
    }
    return 0;
}
