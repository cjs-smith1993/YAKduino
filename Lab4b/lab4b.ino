/* 
File: lab4b_app.c
Revision date: 23 December 2003
Description: Application code for EE 425 lab 4B (Kernel essentials B)
*/

#include <yakk.h>
#include "yaku.h"
#include <SerialGraphicLCD.h>

extern LCD lcd;

#define ASTACKSIZE 256			/* Size of each stack in words */
#define BSTACKSIZE 256
#define CSTACKSIZE 256

int AStk[ASTACKSIZE];			/* Space for each task's stack */
int BStk[BSTACKSIZE];
int CStk[CSTACKSIZE];

void ATask(void);				/* Function prototypes for task code */
void BTask(void);
void CTask(void);

void setup()
{
	delay(2000);
	lcd.clearScreen();

	YKInitialize();
	YKExitMutex();

	delay(500);
	// lcd.printStr("Bananas");
	// lcd.nextLine();
	YKNewTask(ATask, (void *)&AStk[ASTACKSIZE], 5);
	YKExitMutex();

	delay(500);
	lcd.printStr("before");
	lcd.nextLine();
	lcd.printNum(*((int*)(YKReadyList->stack_ptr)));
	lcd.printStr(" ");
	lcd.printNum(YKReadyList->priority);
	lcd.nextLine();

	// delay(500);
	// lcd.printNum(*((int*)(YKCurTask->stack_ptr)));
	// lcd.nextLine();

	delay(500);
	lcd.printStr("Starting kernel");
	lcd.nextLine();
	YKRun();
	YKExitMutex();

	if (YKCurTask == NULL) {
		lcd.printStr("null");
		lcd.nextLine();
	}
	

	delay(500);
	lcd.printStr("after ");
	lcd.nextLine();
	lcd.printNum(*((int*)(YKCurTask->stack_ptr)));
	lcd.printStr(" ");
	lcd.printNum(YKCurTask->priority);
	lcd.nextLine();

	delay(500);
	lcd.printStr("still here?");
	lcd.nextLine();
}

void loop() {
	lcd.printStr("error");
	lcd.nextLine();
	delay(500000);
}

void ATask(void)
{
	delay(500);
	lcd.printStr("Task A started!");
	lcd.nextLine();

	delay(500);
	lcd.printStr("Creating low priority task B...");
	lcd.nextLine();
	YKNewTask(BTask, (void *)&BStk[BSTACKSIZE], 7);

	lcd.printStr("Creating task C...");
	lcd.nextLine();
	YKNewTask(CTask, (void *)&CStk[CSTACKSIZE], 2);

	lcd.printStr("Task A is still running! Oh no! Task A was supposed to stop.");
	lcd.nextLine();
	exit(0);
}

void BTask(void)
{
	lcd.printStr("Task B started! Oh no! Task B wasn't supposed to run.");
	lcd.nextLine();
	exit(0);
}

void CTask(void)
{
	lcd.printStr("start C!");
	lcd.nextLine();

	int count;
	unsigned numCtxSwitches;

	YKEnterMutex();
	numCtxSwitches = YKCtxSwCount;
	YKExitMutex();

	lcd.printStr("Task C started after ");
	lcd.printNum(numCtxSwitches);
	lcd.printStr(" context switches!");
	lcd.nextLine();

	while (1)
	{
		lcd.printStr("Executing in task C.\n");
		for(count = 0; count < 5000; count++);
	}
}

