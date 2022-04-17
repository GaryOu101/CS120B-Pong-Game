/*	Author: gou004
 *  Partner(s) Name: NA
 *	Lab Section:
 *	Assignment: 
 *	Exercise Description: [optional - include for your own benefit]
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *
 */

#include <avr/io.h> 
#include "timer.h"
#include "bit.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

//global Variables======================================
unsigned char pad1_row = 0xF1;
unsigned char temppad1_row;
unsigned char pad1_col = 0x01;
unsigned char ball_row = 0xFB;
unsigned char ball_col = 0x08;
unsigned char ballDirection = 2;// 1 = diag up, 2 = straight, 3 = diag down
unsigned char pad2_row = 0xF1;
unsigned char temppad2_row;
unsigned char pad2_center = 0xFB;
unsigned char pad2_col = 0x80;
unsigned char randMove;
unsigned short count = 0;
unsigned short speed = 10;
unsigned char choose = 1;
unsigned char direc;	// 1= right, 2 = left
unsigned char hitLoc;
unsigned char diag;
//========================================================
//sm enum declaration
enum pad1_States { pad1_init, On, Off };
enum pad2_States { pad2_init, action, Move }; //controls what AI does
enum Ball_States { Ball_init, countSpeed, movement, win};
enum LED_States { init, s1, s2, s3 }; //this sm sets PORTD and PORTC to display proper LED configuration
enum reset_states { reset_init, reset_wait, reset };
//=======================================================

typedef struct task{
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);
} task;

unsigned short getSpeed(unsigned char hitArea){
	unsigned short ballSpeed = 20;	//speed if hits diagonal  
	if (hitArea == 2){ //speed if hits center . Higher speed # = slower
		ballSpeed = 30;
	}
	return ballSpeed;
}

unsigned char getHitDirec(unsigned char pad, unsigned char ball){
	
	unsigned char hitDirec;
	hitDirec = 3;
	unsigned char temp1 = 0x01 |(ball << 1);
	unsigned char temp2 = 0x03| (ball << 1);
	if ((pad | temp2) == temp2){
		hitDirec = 1;
	}
	else if ((pad | temp1) == temp1){
		hitDirec = 2;
	}
	return hitDirec;
}


int pad1_sm(int state){  //controls the player's paddle
	unsigned char button = ~PINA & 0x03;
	switch (state){
		case pad1_init:
			state = On;
			break;
		
		case Off:
			if (button == 0x00){     //makes sure only 1 move per button press
				state = On;
			}
			else{
				state = Off;
			}
			break;
		case On:
			if ((button & 0x01) == 0x01){
				if (pad1_row != 0xF8){           //move player pad down if not already at bottom
					temppad1_row = pad1_row;
					pad1_row = (pad1_row >> 1) | 0x80;
				}
				state = Off;
			}
			else if ((button & 0x02) == 0x02){
				if (pad1_row != 0xE3){         //move player pad up if not already at top
					temppad1_row = pad1_row;
					pad1_row = (pad1_row << 1) | 0x01;
				}
				state = Off;
			}
			else{
				state = On;
			}
			break;	
		default:
			state = pad1_init;
			break;
	}
	switch(state){
		case pad1_init:
			break;
		case On:
			break;
		case Off:
			break;
		default:
			break;
	}
	return state;
}



				  
int pad2_Tick(int state){ //AI paddle
	switch (state){
		case pad2_init:
			state = action;
			randMove = 0;
			srand(time(NULL)); //random number
			break;
			
		case Move:
			if (randMove == 1){
				if (pad2_row != 0xF8){
					temppad2_row = pad2_row;
					pad2_row = (pad2_row >> 1) | 0x80; // #1 moves the paddle down
					pad2_center = (pad2_center >> 1) | 0x80;
				}
			}
			else if (randMove == 2){
				if (pad2_row != 0xE3){
					temppad2_row = pad2_row;
					pad2_row = (pad2_row << 1) | 0x01; //#2 moves pad up
					pad2_center = (pad2_center << 1) | 0x01;
				}
			}
			else if (randMove == 4 || randMove == 5){ //#4 and #5 makes paddle be on same row of ball
				if (pad2_center < ball_row){ //if ball is below, move down
					if (pad2_row != 0xF8){
						temppad2_row = pad2_row;
						pad2_row = (pad2_row >> 1) | 0x80;
						pad2_center = (pad2_center >> 1) | 0x80;
					}
				}
				else if (pad2_center > ball_row){ //if ball is above, move up
					if (pad2_row != 0xE3){
						temppad2_row = pad2_row;
						pad2_row = (pad2_row << 1) | 0x01;
						pad2_center = (pad2_center << 1) | 0x01;
					}
				}
			}
			break;
		case action:
			randMove = rand() % 5 + 1; 
			state = Move;
			break;
			state = action;
			break;
		default:
			state = pad2_init;
			break;
	}
	
	return state;
}


int Ball_sm(int state){
	switch (state){
		case Ball_init:   //spawns ball at the center moving straight to the right
			direc = 1;
			ballDirection = 2;
			state = movement;
			ball_row = 0xFB;
			ball_col = 0x08;
			break;
		case countSpeed:
			if (count < speed){     //controls speed of ball. lower speed# = faster ball
				count++;
				state = countSpeed;
			}
			else{
				count = 0;
				state = movement;
			}
			break;
		case movement:
			if (ballDirection == 2 && direc == 1){	//Straight right
				if (ball_col != 0x02){ //ball keeps moving right until near paddle
					ball_col >>= 1;
					state = countSpeed;
				}
				else if (ball_col == 0x02 && ((~pad1_row & ~ball_row) == ~ball_row)){ //if paddle is where ball is
					hitLoc = getHitDirec(pad1_row, ball_row);
					speed = getSpeed(hitLoc);
					if (hitLoc == 1){
						ballDirection = 1; //if pad hits the top, ball goes diagonally up
					}
					else if (hitLoc == 2){
						ballDirection = 2; //if ball hits center, go straight 
					}
					else{
						ballDirection = 3; //if ball hits bottom, diagonally down
					}
					direc = 2; //go left
					state = countSpeed;
				}
				else
				{
					ball_col >>= 1; //if paddle misses, ball goes through and player scores
					state = win;
				}
			}
			else if (ballDirection == 2 && direc == 2){ //ball moving left straight
				if (ball_col != 0x40){ //if ball not next to paddle, keep moving                        
					ball_col <<= 1;
					state = countSpeed;
				}
				else if (ball_col == 0x40 && ((~pad2_row & ~ball_row) == ~ball_row)){
					hitLoc = getHitDirec(pad2_row, ball_row);
					speed = getSpeed(hitLoc);
					if (hitLoc == 1){
						ballDirection = 1;
					}
					else if (hitLoc == 2){
						ballDirection = 2;
					}
					else{
						ballDirection = 3;
					}
					direc = 1;
					state = countSpeed;
				}
				else{
					ball_col <<= 1;
					state = win;
				}
			}
			else if (ballDirection == 1 && direc == 1){	
				diag = ~ball_row >> 1;
				if (ball_col != 0x02 && ball_row != 0xFE)
				{
					ball_col >>= 1;
					ball_row = (ball_row >> 1) | 0x80;
					state = countSpeed;
				}
				else if (ball_col == 0x02 && ((~pad1_row & ~ball_row) == ~ball_row)){
					hitLoc = getHitDirec(pad1_row, ball_row);
					speed = getSpeed(hitLoc);
					if (hitLoc == 1){
						ballDirection = 1;
					}
					else if (hitLoc == 2){
						ballDirection = 2;
					}
					else{
						ballDirection = 3;
					}
					direc = 2;
					state = countSpeed;
				}
				else if (ball_col == 0x02 && ((~pad1_row & diag) == diag)){
					ballDirection = 3;
					direc = 2;
					state = countSpeed;
				}
				else if (ball_row == 0xFE && ball_col != 0x02){
					ballDirection = 3;
					state = countSpeed;
				}
				else{
					ball_col >>= 1;
					if (ball_row != 0xFE){
						ball_row = (ball_row >> 1) | 0x80;
					}
					for (int i = 0; i < 500; i++) {}
					state = win;
				}
			}
			else if (ballDirection == 1 && direc == 2){	//Diagonally up left
				diag = ~ball_row >> 1;
				if (ball_col != 0x40 && ball_row != 0xFE){
					ball_col <<= 1;
					ball_row = (ball_row >> 1) | 0x80;
					state = countSpeed;
				}
				else if (ball_col == 0x40 && ((~pad2_row & ~ball_row) == ~ball_row)){
					hitLoc = getHitDirec(pad2_row, ball_row);
					speed = getSpeed(hitLoc);
					if (hitLoc == 1){
						ballDirection = 1;
					}
					else if (hitLoc == 2){
						ballDirection = 2;
					}
					else{
						ballDirection = 3;
					}
					
					direc = 1;
					state = countSpeed;
				}
				else if (ball_col == 0x40 && ((~pad2_row & diag) == diag)){
					ballDirection = 3;
					direc = 1;
					state = countSpeed;
				}
				else if (ball_row == 0xFE && ball_col != 0x40){
					ballDirection = 3;
					state = countSpeed;
				}
				else{
					ball_col <<= 1;
					if (ball_row != 0xFE){
						ball_row= (ball_row >> 1) | 0x80;
					}
					state = win;
				}
			}
			else if (ballDirection == 3 && direc == 1){	//Diagonally down right
				diag = ~ball_row << 1;
				if (ball_col!= 0x02 &&ball_row != 0xEF){
					ball_col >>= 1;
					ball_row = (ball_row << 1) | 0x01;
					state = countSpeed;
				}
				else if (ball_col == 0x02 && ((~pad1_row & ~ball_row) == ~ball_row)){
					hitLoc = getHitDirec(pad1_row,ball_row);
					speed = getSpeed(hitLoc);
					if (hitLoc == 1){
						ballDirection = 1;
					}
					else if (hitLoc == 2){
						ballDirection = 2;
					}
					else{
						ballDirection = 3;
					}
					direc = 2;
					state = countSpeed;
				}
				else if (ball_col == 0x02 && ((~pad1_row & diag) == diag)){
					ballDirection = 1;
					direc = 2;
					state = countSpeed;
				}
				else if (ball_row == 0xEF && ball_col != 0x02){
					ballDirection = 1;
					state = countSpeed;
				}
				else{
					ball_col >>= 1;
					if (ball_row != 0xEF){
						ball_row = (ball_row << 1) | 0x01;
					}
					for (int i = 0; i < 500; i++) {}
					state = win;
				}
			}
			else if (ballDirection == 3 && direc == 2){	//Diagonally down left
				diag = ~ball_row << 1;
				if (ball_col != 0x40 && ball_row != 0xEF){
					ball_col <<= 1;
					ball_row = (ball_row << 1) | 0x01;
					state = countSpeed;
				}
				else if (ball_col== 0x40 && ((~pad2_row & ~ball_row) == ~ball_row)){
					hitLoc = getHitDirec(pad2_row,ball_row);
					speed = getSpeed(hitLoc);
					if (hitLoc == 1){
						ballDirection = 1;
					}
					else if (hitLoc == 2){
						ballDirection = 2;
					}
					else{
						ballDirection = 3;
					}
					direc = 1;
					state = countSpeed;
				}
				else if (ball_col == 0x02 && ((~pad2_row & diag) == diag)){
					ballDirection = 1;
					direc = 1;
					state = countSpeed;
				}
				else if (ball_row == 0xEF && ball_col != 0x40){
					ballDirection = 1;
					state = countSpeed;
				}
				else{
					ball_col <<= 1;
					if (ball_row != 0xEF)
					{
						ball_row = (ball_row << 1) | 0x01;
					}
					state = win;
				}
			}
			break;
		case win:
			if (direc == 1){
				ball_col = 0x10;
				ball_row = 0xFB;
				ballDirection = 2;
				direc = 2;
				speed = 35;
				count = 0;
			}
			else if (direc == 2){
				ball_col = 0x08;
				ball_row = 0xFB;
				ballDirection = 2;
				direc = 1;
				count = 0;
				speed = 35;
			}
			state = countSpeed;
			break;
		default:
			state = Ball_init;
			break;
	}
	
	return state;
}

int led_sm(int state){
	switch (state){
		case init:
			state = s1;
			break;
		case s1:
			state = s2;
			break;
		case s2:
			state = s3;
			break;
		case s3:
			state = s1;
			break;
		default:
			state = init;
			break;
	}
	switch(state){
		case init:
			break;
		case s1:
			PORTD = pad1_row;
			PORTC = pad1_col;
			break;
		case s2:
			PORTD = pad2_row;
			PORTC = pad2_col;
			break;
		case s3:
			PORTD = ball_row;
			PORTC = ball_col;
			break;
		default:
			break;
	}
	return state;
}


int reset_sm(int state){
	unsigned char button = ~PINA & 0x04;
	switch (state){
		case reset_init:
			state = reset_wait;
			break;
		case reset:
			state = reset_wait;
			break;
		case reset_wait:
			if ((button & 0x04) == 0x04){
				state = reset;
			}
			else{
				state = reset_wait;
			}
			break;
		default:
			state = reset_init;
			break;
	}
	switch(state){
		case reset_init:
			break;
		case reset_wait:
			break;
		case reset:
			pad2_row = 0xF1;   //resets to starting conditions
			pad2_center = 0xFB;
			pad2_col = 0x80;
			ball_row = 0xFB;
			ball_col = 0x08;
			direc = 1;
			ballDirection = 2;
			pad1_row = 0xF1;
			pad1_col = 0x01;
			break;
		default:
			break;
	}
			
	return state;
}
	
	
int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; DDRB = 0xFF;
	PORTB = 0x00;DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	static task task1, task2, task3, task4, task5;
	task *tasks[] = { &task1, &task2, &task3, &task4, &task5,  };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	const char start = -1;
	task1.state = start;
	task1.period = 1000;
	task1.elapsedTime = task1.period;
	task1.TickFct = &pad1_sm;
	task2.state = start;
	task2.period = 130000;
	task2.elapsedTime = task2.period;
	task2.TickFct = &pad2_Tick;
	task3.state = start;
	task3.period = 6000;
	task3.elapsedTime = task3.period;
	task3.TickFct = &Ball_sm;
	task4.state = start;
	task4.period = 1000;
	task4.elapsedTime = task4.period;
	task4.TickFct = &led_sm;
	task5.state = start;
	task5.period = 1000;
	task5.elapsedTime = task5.period;
	task5.TickFct = &reset_sm;

	TimerSet(10);
	TimerOn();
	
	unsigned short i;
	while (1){
		for (i = 0; i < numTasks; i++){
			if (tasks[i]->elapsedTime == tasks[i]->period){
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 100;
		}	
		while (!TimerFlag){
			TimerFlag = 0;
		}
	}
	return 0;
}
