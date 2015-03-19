/*
File: lab6app.c
Revision date: 4 November 2009
Description: Application code for EE 425 lab 6 (Message queues)
*/

#include <yakk.h>
#include <yaku.h>
#include <SerialGraphicLCD.h>
#include <TimerThree.h>

extern LCD lcd;

#define TASK_STACK_SIZE		512		/* stack size in words */
#define MSGQSIZE			10
#define MSGARRAYSIZE      20

struct msg
{
    int tick;
    int data;
};

struct msg MsgArray[MSGARRAYSIZE];	/* buffers for message content */

int ATaskStk[TASK_STACK_SIZE];		/* a stack for each task */
int BTaskStk[TASK_STACK_SIZE];
int STaskStk[TASK_STACK_SIZE];

int GlobalFlag;

void *MsgQ[MSGQSIZE];				/* space for message queue */
YKQ *MsgQPtr;						/* actual name of queue */

void ATask(void)					/* processes data in messages */
{
	struct msg *tmp;
	int min, max, count;

	min = 100;
	max = 0;
	count = 0;

	while (1)
	{
		tmp = (struct msg *) YKQPend(MsgQPtr); /* get next msg */

		/* check sequence count in msg; were msgs dropped? */
		if (tmp->tick != count+1)
		{
			lcd.printStr("\n\n\n----------------");
			lcd.printStr("! Dropped msgs: tick ");
			lcd.printStr("----------------\n\n\n");

			if (tmp->tick - (count+1) > 1) {
				lcd.printNum(count+1);
				lcd.printStr("-");
				lcd.printNum(tmp->tick-1);
				lcd.nextLine();
			}
			else {
				lcd.printNum(tmp->tick-1);
				lcd.nextLine();
			}
		}

		/* update sequence count */
		count = tmp->tick;

		/* process data; update statistics for this sample */
		if (tmp->data < min)
			min = tmp->data;
		if (tmp->data > max)
			max = tmp->data;

		/* output min, max, tick values */
		lcd.printStr("Ticks: ");
		lcd.printNum(count);
		lcd.printStr("\t");
		lcd.printStr("Min: ");
		lcd.printNum(min);
		lcd.printStr("\t");
		lcd.printStr("Max: ");
		lcd.printNum(max);
		lcd.nextLine();
	}
}

void BTask(void)					/* saturates the CPU for 5 ticks */
{
	int busycount, curval, j, flag, chcount;
	unsigned tickNum;

	curval = 1001;
	chcount = 0;

	while (1)
	{
		YKDelayTask(2);

		if (GlobalFlag == 1)
		{							/* flag set -- loop for 5 ticks */
			YKEnterMutex();
			busycount = YKTickNum;
			YKExitMutex();

			while (1)
			{
				YKEnterMutex();
				tickNum = YKTickNum;
				YKExitMutex();
				if(tickNum >= busycount + 5) break;

				curval += 2;		/* evaluate next number */
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
					lcd.printStr("."); /* output a marker for each prime"*/
					if (++chcount > 75)
					{
						lcd.nextLine();
						chcount = 0;
					}
				}
			}
			lcd.nextLine();
			chcount = 0;
			GlobalFlag = 0;			/* clear flag */
		}
	}
}

void STask(void)					/* tracks statistics */
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

	YKNewTask(BTask, (void *) &BTaskStk[TASK_STACK_SIZE], 10);
	YKNewTask(ATask, (void *) &ATaskStk[TASK_STACK_SIZE], 20);

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

	/* create queue, at least one user task, etc. */
	GlobalFlag = 0;
	MsgQPtr = YKQCreate(MsgQ, MSGQSIZE);
	YKNewTask(STask, (void *) &STaskStk[TASK_STACK_SIZE], 30);

	YKRun();
}

void loop(void) {

}

ISR(INT4_vect, ISR_NOBLOCK) {
	GlobalFlag = 1;
}

ISR(TIMER3_OVF_vect, ISR_NOBLOCK) {
	YKEnterISR();
	sei();
	YKTickHandler();
	mytick();
	cli();
	YKExitISR();
}

void resetHandler() {
	exit(0);
}

void mytick(void)
{
	static int next = 0;
	static int data = 0;

	/* create a message with tick (sequence #) and pseudo-random data */
	MsgArray[next].tick = YKTickNum;
	data = (data + 89) % 100;
	MsgArray[next].data = data;
	if (YKQPost(MsgQPtr, (void *) &(MsgArray[next])) == 0)
	lcd.printStr("  TickISR: queue overflow! \n");
	else if (++next >= MSGARRAYSIZE)
	next = 0;
}

void keyboardHandler() {
	GlobalFlag = 1;
}