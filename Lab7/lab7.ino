/*
File: lab7app.c
Revision date: 10 November 2005
Description: Application code for EE 425 lab 7 (Event flags)
*/

#include <SerialGraphicLCD.h>
#include <TimerThree.h>
#include <mpr121.h>
#include <i2c.h>
#include <digits.h>

#include <yak.h>
#include "lab7defs.h"

extern LCD lcd;

#define TASK_STACK_SIZE	512				/* stack size in words */

YKEVENT *charEvent;
YKEVENT *numEvent;

int CharTaskStk[TASK_STACK_SIZE];		/* a stack for each task */
int AllCharsTaskStk[TASK_STACK_SIZE];
int AllNumsTaskStk[TASK_STACK_SIZE];
int STaskStk[TASK_STACK_SIZE];

void CharTask(void)						/* waits for any events triggered by letter keys */
{
	unsigned events;

	lcd.printStr("Started CharTask     (2)\n");

	while(1) {
		events = YKEventPend(charEvent,
			EVENT_A_KEY | EVENT_B_KEY | EVENT_C_KEY,
			EVENT_WAIT_ANY);

		if(events == 0) {
			lcd.printStr("Oops! At least one event should be set "
				"in return value!\n");
		}

		if(events & EVENT_A_KEY) {
			lcd.printStr("CharTask     (A)\n");
			YKEventReset(charEvent, EVENT_A_KEY);
		}

		if(events & EVENT_B_KEY) {
			lcd.printStr("CharTask     (B)\n");
			YKEventReset(charEvent, EVENT_B_KEY);
		}

		if(events & EVENT_C_KEY) {
			lcd.printStr("CharTask     (C)\n");
			YKEventReset(charEvent, EVENT_C_KEY);
		}
	}
}


void AllCharsTask(void)					/* waits for all events triggered by letter keys */
{
	unsigned events;

	lcd.printStr("Started AllCharsTask (3)\n");

	while(1) {
		events = YKEventPend(charEvent,
			EVENT_A_KEY | EVENT_B_KEY | EVENT_C_KEY,
			EVENT_WAIT_ALL);
		// To be reset by WaitForAny task

		if(events != 0) {
			lcd.printStr("Oops! Char events weren't reset by CharTask!\n");
		}

		lcd.printStr("AllCharsTask (D)\n");
	}
}


void AllNumsTask(void)					/* waits for events triggered by number keys */
{
	unsigned events;

	lcd.printStr("Started AllNumsTask  (1)\n");

	while(1) {
		events = YKEventPend(numEvent,
			EVENT_1_KEY | EVENT_2_KEY | EVENT_3_KEY,
			EVENT_WAIT_ALL);

		if(events != (EVENT_1_KEY | EVENT_2_KEY | EVENT_3_KEY)) {
			lcd.printStr("Oops! All events should be set in return value!\n");
		}

		lcd.printStr("AllNumsTask  (123)\n");

		YKEventReset(numEvent, EVENT_1_KEY | EVENT_2_KEY | EVENT_3_KEY);
	}
}


void STask(void)						/* tracks statistics */
{
	unsigned max, switchCount, idleCount;
	int tmp;

	YKDelayTask(1);
	lcd.printStr("Welcome to the YAK kernel\r\n");
	lcd.printStr("Determining CPU capacity\r\n");
	YKDelayTask(1);
	YKIdleCount = 0;
	YKDelayTask(5);
	max = YKIdleCount / 25;
	YKIdleCount = 0;

	YKNewTask(CharTask, (void *) &CharTaskStk[TASK_STACK_SIZE], 2);
	YKNewTask(AllNumsTask, (void *) &AllNumsTaskStk[TASK_STACK_SIZE], 1);
	YKNewTask(AllCharsTask, (void *) &AllCharsTaskStk[TASK_STACK_SIZE], 3);

	while (1)
	{
		YKDelayTask(20);

		YKEnterMutex();
		switchCount = YKCtxSwCount;
		idleCount = YKIdleCount;
		YKExitMutex();

		lcd.printStr("<<<<< Context switches: ");
		lcd.printNum((int)switchCount);
		lcd.printStr(", CPU usage: ");
		tmp = (int) (idleCount/max);
		lcd.printNum(100-tmp);
		lcd.printStr("% >>>>>\r\n");

		YKEnterMutex();
		YKCtxSwCount = 0;
		YKIdleCount = 0;
		YKExitMutex();
	}
}

void setup()
{

	pinMode(2, INPUT);
	pinMode(13, OUTPUT);

	Serial.begin(115200);

	//output on ADC4 (PC4, SDA)
	DDRC |= 0b00010011;
	// Pull-ups on I2C Bus
	PORTC = 0b00110000;
	// initalize I2C bus. Wiring lib not used.
	i2cInit();

	delay(100);
	// initialize mpr121
	mpr121QuickConfig();

	EICRB = (EICRB & ~((1 << ISC40) | (1 << ISC41))) | (FALLING << ISC40);
	EIMSK |= (1 << INT4);

	delay(2000);
	lcd.clearScreen();
	delay(1000);

	YKInitialize();
	YKTickPeriod = 5000L;

	charEvent = YKEventCreate(0);
	numEvent = YKEventCreate(0);
	YKNewTask(STask, (void *) &STaskStk[TASK_STACK_SIZE], 0);
	YKRun();
}

void loop() {

}