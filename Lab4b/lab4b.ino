/*
File: lab4b_app.c
Revision date: 23 December 2003
Description: Application code for EE 425 lab 4B (Kernel essentials B)
*/

#include <yakk.h>
#include <yaku.h>
#include <SerialGraphicLCD.h>
#include <TimerThree.h>

extern LCD lcd;

#define ASTACKSIZE 512			/* Size of each stack in words */
#define BSTACKSIZE 512
#define CSTACKSIZE 512

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
	delay(1000);

	lcd.printStr("init");
	lcd.nextLine();

	YKInitialize();
	YKNewTask(ATask, (void *)&AStk[ASTACKSIZE], 5);
	YKRun();

	delay(200);
	lcd.printStr("Failed");
	lcd.nextLine();
}

void loop() {

}

void ATask(void)
{
	delay(200);
	lcd.printStr("A started!");
	lcd.nextLine();

	delay(200);
	lcd.printStr("Create B");
	lcd.nextLine();

	YKNewTask(BTask, (void *)&BStk[BSTACKSIZE], 7);

	delay(200);
	lcd.printStr("Create C");
	lcd.nextLine();

	YKNewTask(CTask, (void *)&CStk[CSTACKSIZE], 2);

	delay(200);
	lcd.printStr("Still A!");
	lcd.nextLine();
	exit(0);
}

void BTask(void)
{
	delay(200);
	lcd.printStr("B started :(");
	lcd.nextLine();
	exit(0);
}

void CTask(void)
{
	delay(200);
	lcd.printStr("C started!");
	lcd.nextLine();

	digitalWrite(13, HIGH);


	int count;
	unsigned numCtxSwitches;

	YKEnterMutex();
	numCtxSwitches = YKCtxSwCount;
	YKExitMutex();

	lcd.printNum(numCtxSwitches);
	lcd.printStr(" ctxSwitches!");
	lcd.nextLine();

	while (1)
	{
		delay(200);
		lcd.printStr("running task C\n");
		lcd.nextLine();
		for(count = 0; count < 5000; count++);
	}
}

