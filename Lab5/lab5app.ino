/*
File: lab5app.c
Revision date: 13 November 2009
Description: Application code for EE 425 lab 5 (Semaphores)
*/

#include <yakk.h>
#include <yaku.h>
#include <SerialGraphicLCD.h>
#include <TimerThree.h>

extern LCD lcd;

#define TASK_STACK_SIZE 512				/* stack size in words */

int TaskWStk[TASK_STACK_SIZE];			/* stacks for each task */
int TaskSStk[TASK_STACK_SIZE];
int TaskPStk[TASK_STACK_SIZE];
int TaskStatStk[TASK_STACK_SIZE];
int TaskPRMStk[TASK_STACK_SIZE];

YKSEM *PSemPtr;							/* YKSEM must be defined in yakk.h */
YKSEM *SSemPtr;
YKSEM *WSemPtr;
YKSEM *NSemPtr;

void TaskWord(void)
{
	while (1)
	{
		YKSemPend(WSemPtr);
		lcd.printStr("Hey");
		YKSemPost(PSemPtr);

		YKSemPend(WSemPtr);
		lcd.printStr("it");
		YKSemPost(SSemPtr);

		YKSemPend(WSemPtr);
		lcd.printStr("works");
		YKSemPost(PSemPtr);
	}
}

void TaskSpace(void)
{
	while (1)
	{
		YKSemPend(SSemPtr);
		lcd.printStr(" ");
		YKSemPost(WSemPtr);
	}
}

void TaskPunc(void)
{
	while (1)
	{
		YKSemPend(PSemPtr);
		lcd.printStr("\"");
		YKSemPost(WSemPtr);

		YKSemPend(PSemPtr);
		lcd.printStr(",");
		YKSemPost(SSemPtr);

		YKSemPend(PSemPtr);
		lcd.printStr("!\"\r\n");
		YKSemPost(PSemPtr);

		YKDelayTask(6);
	}
}

void TaskPrime(void)					/* task that actually computes primes */
{
	int curval = 1001;
	int j,flag,lncnt;
	int endval;

	while (1)
	{
		YKSemPend(NSemPtr);

		/* compute next range of primes */
		lncnt = 0;
		endval = curval + 500;
		for ( ; curval < endval; curval += 2)
		{
			flag = 0;
			for (j = 3; (j*j) < curval; j += 2)
			{
				if (curval % j == 0)
				{
					flag = 1;
					break;
				}
			}
			if (!flag)
			{
				lcd.printStr(" ");
				lcd.printNum(curval);
				lncnt++;
				if (lncnt > 9)
				{
					lcd.nextLine();
					lncnt = 0;
				}
			}
		}
		lcd.nextLine();
	}
}

void TaskStat(void)						/* a task to track statistics */
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

	YKNewTask(TaskPrime, (void *) &TaskPRMStk[TASK_STACK_SIZE], 12);
	YKNewTask(TaskWord,  (void *) &TaskWStk[TASK_STACK_SIZE], 0);
	YKNewTask(TaskSpace, (void *) &TaskSStk[TASK_STACK_SIZE], 1);
	YKNewTask(TaskPunc,  (void *) &TaskPStk[TASK_STACK_SIZE], 2);

	while (1)
	{
		YKDelayTask(20);

		YKEnterMutex();
		switchCount = YKCtxSwCount;
		idleCount = YKIdleCount;
		YKExitMutex();

		lcd.printStr ("<<<<< Context switches: ");
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

// void test() {
ISR(INT4_vect, ISR_NOBLOCK) {
	digitalWrite(13, HIGH);
	lcd.clearScreen();
	Serial.print("test\n");
	YKSemPost(NSemPtr);
}

ISR(TIMER3_OVF_vect, ISR_NOBLOCK) {
	YKEnterISR();
	sei();
	YKTickHandler();
	cli();
	YKExitISR();
}

void setup(void)
{
	EICRB = (EICRB & ~((1 << ISC40) | (1 << ISC41))) | (RISING << ISC40);
	EIMSK |= (1 << INT4);

	pinMode(2, INPUT);
	pinMode(13, OUTPUT);

	Serial.begin(115200);

	delay(2000);
	lcd.clearScreen();
	delay(1000);

	YKInitialize();
	YKTickPeriod = 50000L;

	/* create all semaphores, at least one user task, etc. */
	PSemPtr = YKSemCreate(1);
	SSemPtr = YKSemCreate(0);
	WSemPtr = YKSemCreate(0);
	NSemPtr = YKSemCreate(0);

	YKNewTask(TaskStat, (void *) &TaskStatStk[TASK_STACK_SIZE], 10);
	YKRun();
}

void loop(void) {

}