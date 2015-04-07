#include "yakk.h"
#include "yaku.h"
#include <SerialGraphicLCD.h>
#include <TimerThree.h>

LCD lcd;

/***********************************************************************
* GLOBAL VARS
***********************************************************************/
YK_TCB YKTCBArray[MAXTASKS+1];
TCBptr YKReadyList;
TCBptr YKDelayedList;
TCBptr YKTCBFreeList;
TCBptr YKCurTask;

YK_SEM YKSemArray[MAXSEMS];
YKSemPtr YKSemFreeList;

YK_QUEUE YKQueueArray[MAXQUEUES];
YKQueuePtr YKQueueFreeList;

YK_EVENT YKEventArray[MAXEVENTS];
YKEventPtr YKEventFreeList;

int YKPrioTable[NUM_PRIOS];
int YKIdleStack[IDLE_STACK_SIZE];

int YKCtxSwCount;
int YKIdleCount;
int YKTickNum;
long long YKTickPeriod;
int YKNumTasksCreated;
int YKInterruptLevel;
int YKRunning;

/***********************************************************************
* YAK KERNEL FUNCTIONS
***********************************************************************/
void YKInitialize(void) {
	int i;

	YKEnterMutex();

	YKCtxSwCount = 0;
	YKIdleCount = 0;
	YKTickNum = 0;
	YKTickPeriod = 0L;
	YKNumTasksCreated = 0;
	YKRunning = 0;
	YKInterruptLevel = 0;
	YKCurTask = NULL;

	//Initialize TCB list
	YKTCBFreeList = &(YKTCBArray[0]);
	for (i = 0; i < MAXTASKS; i++) {
		YKTCBArray[i].next = &(YKTCBArray[i+1]);
	}

	//Initialize ready queue
	YKReadyList = NULL;

	//Initialize blocked queue
	YKDelayedList = NULL;

	//Initialize semaphore list
	YKSemFreeList = &(YKSemArray[0]);
	for (i = 0; i < MAXSEMS-1; i++) {
		YKSemArray[i].nextSem = &(YKSemArray[i+1]);
	}
	YKSemArray[MAXSEMS-1].nextSem = NULL;

	//Initialize queue list
	YKQueueFreeList = &(YKQueueArray[0]);
	for (i = 0; i < MAXQUEUES-1; i++) {
		YKQueueArray[i].nextQueue = &(YKQueueArray[i+1]);
	}
	YKQueueArray[MAXQUEUES-1].nextQueue = NULL;

	//Initialize event list
	YKEventFreeList = &(YKEventArray[0]);
	for (i = 0; i < MAXEVENTS-1; i++) {
		YKEventArray[i].nextEvent = &(YKEventArray[i+1]);
	}
	YKEventArray[MAXEVENTS-1].nextEvent = NULL;

	//Clear priority table
	for (i = 0; i < NUM_PRIOS; i++) {
		YKPrioTable[i] = 0;
	}

	//Create idle task
	YKNewTask((void *)YKIdleTask, (void *)&YKIdleStack[IDLE_STACK_SIZE], IDLE_PRIO);
}

void YKNewTask(void (* task)(void), void *taskStack, UBYTE priority) {
	UBYTE *stack;
	UWORD taskAddr = (UWORD)task;
	UBYTE task_hi8 = (UBYTE)((taskAddr >> 16) & 0x00FF);
	UBYTE task_mid8 = (UBYTE)((taskAddr >> 8) & 0x00FF);
	UBYTE task_lo8 = (UBYTE)((taskAddr >> 0) & 0x00FF);

	YKEnterMutex();
	if (!YKPrioTable[priority]) {
		YKPrioTable[priority] = 1;

		if (YKNumTasksCreated <= MAXTASKS) {
			YKNumTasksCreated++;

			stack = (UBYTE *)taskStack;
			stack--;

			*(stack--) = task_lo8;
			*(stack--) = task_mid8;
			*(stack--) = task_hi8;

			*(stack--) = (UBYTE)0x80;		//flags

			*(stack--) = (UBYTE)0x00;		//R0	= 0
			*(stack--) = (UBYTE)0x00;		//R1	= 0
			*(stack--) = (UBYTE)0x00;		//R2	= 0
			*(stack--) = (UBYTE)0x00;		//R3	= 0
			*(stack--) = (UBYTE)0x00;		//R4	= 0
			*(stack--) = (UBYTE)0x00;		//R5	= 0
			*(stack--) = (UBYTE)0x00;		//R6	= 0
			*(stack--) = (UBYTE)0x00;		//R7	= 0
			*(stack--) = (UBYTE)0x00;		//R8	= 0
			*(stack--) = (UBYTE)0x00;		//R9	= 0
			*(stack--) = (UBYTE)0x00;		//R10	= 0
			*(stack--) = (UBYTE)0x00;		//R11	= 0
			*(stack--) = (UBYTE)0x00;		//R12	= 0
			*(stack--) = (UBYTE)0x00;		//R13	= 0
			*(stack--) = (UBYTE)0x00;		//R14	= 0
			*(stack--) = (UBYTE)0x00;		//R15	= 0
			*(stack--) = (UBYTE)0x00;		//R16	= 0
			*(stack--) = (UBYTE)0x00;		//R17	= 0
			*(stack--) = (UBYTE)0x00;		//R18	= 0
			*(stack--) = (UBYTE)0x00;		//R19	= 0
			*(stack--) = (UBYTE)0x00;		//R20	= 0
			*(stack--) = (UBYTE)0x00;		//R21	= 0
			*(stack--) = (UBYTE)0x00;		//R22	= 0
			*(stack--) = (UBYTE)0x00;		//R23	= 0
			*(stack--) = (UBYTE)0x00;		//R24	= 0
			*(stack--) = (UBYTE)0x00;		//R25	= 0
			*(stack--) = (UBYTE)0x00;		//R26	= 0
			*(stack--) = (UBYTE)0x00;		//R27	= 0
			*(stack--) = (UBYTE)0x00;		//R28	= 0
			*(stack--) = (UBYTE)0x00;		//R29	= 0
			*(stack--) = (UBYTE)0x00;		//R30	= 0
			*(stack--) = (UBYTE)0x00;		//R31	= 0

			YKTCBInit(priority, (void *)stack);

			if (YKRunning) {
				YKScheduler();
				YKExitMutex();
			}
		}
		else {
			if (YKRunning) {
				YKExitMutex();
			}
			lcd.printStr("\nERROR.\nNO MORE TASKS AVAILABLE.\nTERMINATING\n");
			// exit(0);
			delay(10000);
		}

	}
	else {
		if (YKRunning) {
			YKExitMutex();
		}
		lcd.printStr("\nERROR.\nPRIORITY IN USE.\nTERMINATING.\n");
		// exit(0);
		delay(10000);
	}
}

void YKIdleTask(void *data) {
	data = data;
	while (1) {
		YKEnterMutex();
		YKIdleCount++;
		YKExitMutex();
	}
}

void YKTCBInit(UBYTE priority, void *stack) {
	TCBptr tcb;

	tcb = YKTCBFreeList;
	YKTCBFreeList = YKTCBFreeList->next;

	tcb->stack_ptr = stack;
	tcb->status = YK_STAT_READY;
	tcb->priority = priority;
	tcb->num_delay = 0;
	tcb->next = NULL;
	tcb->prev = NULL;

	YKAddTCBToList(&YKReadyList, tcb);
}

void YKAddTCBToList(TCBptr *list, TCBptr tcb) {
	TCBptr curTCB = *list;
	TCBptr nextTCB = NULL;

	if (!curTCB || tcb->priority < curTCB->priority) { //insert at head
		tcb->prev = NULL;
		tcb->next = curTCB;
		curTCB->prev = tcb;
		*list = tcb;
	}
	else { //insert at middle/end
		nextTCB = curTCB->next;
		while (nextTCB && tcb->priority > nextTCB->priority) {
			curTCB = nextTCB;
			nextTCB = curTCB->next;
		}

		tcb->next = nextTCB;
		tcb->prev = curTCB;
		curTCB->next = tcb;
		if (nextTCB) {
			nextTCB->prev = tcb;
		}
	}
}

void YKRemoveTCBFromList(TCBptr *list, TCBptr tcb) {
	TCBptr prevTCB;
	TCBptr nextTCB;

	prevTCB = tcb->prev;
	nextTCB = tcb->next;

	if (prevTCB) { //If not removing from the head
		prevTCB->next = tcb->next;
	}
	else { //If removing from the head, set head to our next
		*list = tcb->next;
	}

	if (nextTCB) { //If not removing from the tail
		nextTCB->prev = tcb->prev;
	}
	else if (prevTCB) { //If removing from the tail AND not from the head
		prevTCB->next = NULL;
	}

	tcb->prev = NULL;
	tcb->next = NULL;
}

void YKRun(void) {
	if (YKNumTasksCreated > 1) {
		YKRunning = TRUE;
		YKExitMutex();
		Timer3.initialize(YKTickPeriod);
		Timer3.attachInterrupt(YKTickHandler);
		YKScheduler();
	}
	else {
		lcd.printStr("ERROR.\nMUST DEFINE AT LEAST ONE USER TASK BEFORE STARTING KERNEL.\nTERMINATING.\n");
		// exit(0);
		delay(10000);
	}
}

void YKDelayTask(unsigned count) {
	if (count > 0) {
		YKEnterMutex();

		YKCurTask->num_delay = count;
		YKCurTask->status = YK_STAT_DELAYED;
		YKRemoveTCBFromList(&YKReadyList, YKCurTask);
		YKAddTCBToList(&YKDelayedList, YKCurTask);

		YKExitMutex();
		YKScheduler();
	}
}

void YKEnterISR(void) {
	YKEnterMutex();
	YKInterruptLevel++;
	YKExitMutex();
}

void YKExitISR(void) {
	YKEnterMutex();
	YKInterruptLevel--;
	YKExitMutex();
	if (YKInterruptLevel == 0) {
		YKScheduler();
	}
}

void YKScheduler(void) {
	YKEnterMutex();
	if (YKCurTask != YKReadyList) {
		YKCtxSwCount++;
		YKDispatcher();
	}
	YKExitMutex();
}

void YKTickHandler(void) {
	TCBptr curTCB = YKDelayedList;
	TCBptr nextTCB;

	YKEnterMutex();
	YKTickNum++;

	while (curTCB) {
		nextTCB = curTCB->next;
		if (curTCB->num_delay > 0) {
			if (--curTCB->num_delay == 0) {
				if (curTCB->status == YK_STAT_DELAYED) {
					curTCB->status = YK_STAT_READY;
					YKRemoveTCBFromList(&YKDelayedList, curTCB);
					YKAddTCBToList(&YKReadyList, curTCB);
				}
			}
		}
		curTCB = nextTCB;
	}
	YKExitMutex();
}

YKSemPtr YKSemCreate(int count) {
	YKSemPtr newSem;

	YKEnterMutex();
	newSem = YKSemFreeList; //Get next available semaphore block
	if (YKSemFreeList != NULL) {
		YKSemFreeList = (YKSemPtr) YKSemFreeList->nextSem;
	}
	if (YKRunning) {
		YKExitMutex();
	}
	if (newSem != NULL) { //Initialize values
		newSem->count = count;
		newSem->pendList = NULL;
	}
	return newSem;
}

void YKSemPend(YKSemPtr sem) {
	YKEnterMutex();
	if (sem->count > 0) {
		sem->count = 0;
		YKExitMutex();
	}
	else {
		YKCurTask->status = YK_STAT_SEM;
		YKRemoveTCBFromList(&YKReadyList, YKCurTask);
		YKAddTCBToList(&(sem->pendList), YKCurTask);
		YKExitMutex();
		YKScheduler();
	}
}

void YKSemPost(YKSemPtr sem) {
	TCBptr pendHead = sem->pendList;

	YKEnterMutex();
	if (pendHead != NULL) {
		YKRemoveTCBFromList(&(sem->pendList), pendHead);
		YKAddTCBToList(&YKReadyList, pendHead);
		YKExitMutex();
		if (YKInterruptLevel == 0) {
			YKScheduler();
		}
	}
	else {
		sem->count = 1;
		YKExitMutex();
	}
}

YKQueuePtr YKQCreate(void **start, unsigned int size) {
	YKQueuePtr newQ;

	YKEnterMutex();
	newQ = YKQueueFreeList;
	if (YKQueueFreeList != NULL) {
		YKQueueFreeList = (YKQueuePtr) YKQueueFreeList->nextQueue;
	}
	if (YKRunning) {
		YKExitMutex();
	}
	if (newQ != NULL) {
		newQ->start = start;
		newQ->end = &start[size];
		newQ->nextEmptySlot = start;
		newQ->nextToRemove = start;
		newQ->size = size;
		newQ->numEntries = 0;
		newQ->pendList = NULL;
	}
	return newQ;
}

void* YKQPend(YKQueuePtr q) {
	void *msg;
	int *message;

	YKEnterMutex();
	if (q->numEntries > 0) {
		msg = *(q->nextToRemove);
		q->nextToRemove++;
		if (q->nextToRemove == q->end) {
			q->nextToRemove = q->start;
		}
		q->numEntries--;
		YKExitMutex();
	}
	else {
		YKCurTask->status = YK_STAT_QUEUE;
		YKRemoveTCBFromList(&YKReadyList, YKCurTask);
		YKAddTCBToList(&(q->pendList), YKCurTask);
		YKExitMutex();
		YKScheduler();
		YKEnterMutex();
		msg = *(q->nextToRemove);
		q->nextToRemove++;
		if (q->nextToRemove == q->end) {
			q->nextToRemove = q->start;
		}
		q->numEntries--;
		YKExitMutex();
	}
	return msg;
}

int YKQPost(YKQueuePtr q, void *message) {
	TCBptr pendHead = q->pendList;
	int ret = 0;

	YKEnterMutex();
	if (q->numEntries < q->size) {
		*(q->nextEmptySlot) = message;
		q->nextEmptySlot++;
		if (q->nextEmptySlot == q->end) {
			q->nextEmptySlot = q->start;
		}
		q->numEntries++;
		ret = 1;

		if (pendHead != NULL) {
			YKRemoveTCBFromList(&(q->pendList), pendHead);
			YKAddTCBToList(&YKReadyList, pendHead);
			YKExitMutex();
			if (YKInterruptLevel == 0) {
				YKScheduler();
			}
		}
		else {
			YKExitMutex();
		}
	}

	return ret;
}

YKEventPtr YKEventCreate(unsigned int initialValue) {
	YKEventPtr newEvent;

	YKEnterMutex();
	newEvent = YKEventFreeList;
	if (YKEventFreeList != NULL) {
		YKEventFreeList = YKEventFreeList->nextEvent;
	}
	if (YKRunning) {
		YKExitMutex();
	}
	if (newEvent != NULL) {
		newEvent->flags = initialValue;
		newEvent->pendList = NULL;
	}
	return newEvent;
}

unsigned int YKEventPend(YKEventPtr event, unsigned int eventMask, int waitMode) {
	UWORD group_flags;
	UWORD ready;

	YKEnterMutex();
	group_flags = event->flags;
	ready = (waitMode == EVENT_WAIT_ANY) && ((group_flags & eventMask) > 0);
	ready |= (waitMode == EVENT_WAIT_ALL) && ((group_flags & eventMask) == eventMask);

	if (ready) {
		YKExitMutex();
	}
	else {
		YKCurTask->status = YK_STAT_EVENT;
		YKCurTask->event_mode = waitMode;
		YKCurTask->event_flags = eventMask;
		YKRemoveTCBFromList(&YKReadyList, YKCurTask);
		YKAddTCBToList(&(event->pendList), YKCurTask);
		YKExitMutex();
		YKScheduler();
		group_flags = event->flags;
	}
	return group_flags;
}

void YKEventSet(YKEventPtr event, unsigned int eventMask) {
	UWORD group_flags;
	UWORD req_flags;
	UBYTE mode;
	UWORD ready;
	TCBptr curPend;
	TCBptr nextPend;

	YKEnterMutex();
	event->flags |= eventMask;
	group_flags = event->flags;
	curPend = event->pendList;
	while (curPend != NULL) {
		nextPend = curPend->next;
		req_flags = curPend->event_flags;
		mode = curPend->event_mode;

		ready = (mode == EVENT_WAIT_ANY) && ((group_flags & req_flags) > 0);
		ready |= (mode == EVENT_WAIT_ALL) && ((group_flags & req_flags) == req_flags);

		if (ready) {
			YKRemoveTCBFromList(&(event->pendList), curPend);
			YKAddTCBToList(&YKReadyList, curPend);
		}

		curPend = nextPend;
	}
	if (YKInterruptLevel == 0) {
		YKExitMutex();
		YKScheduler();
	}
	YKExitMutex();
}

void YKEventReset(YKEventPtr event, unsigned int eventMask) {
	event->flags &= ~eventMask;
}


void YKPrintLists(void) {
	// YKExitMutex();
	// delay(1000);
	lcd.printStr("test");
	// delay(1000);
}