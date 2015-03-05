#ifndef YAKK_H
#define YAKK_H

#include "yaku.h"

/***********************************************************************
* CONSTANTS/TYPES
***********************************************************************/
#ifndef	NULL
#define NULL 0
#endif

#ifndef	TRUE
#define TRUE 1
#endif

#ifndef	FALSE
#define FALSE 0
#endif

#define YK_STAT_READY 0
#define YK_STAT_DELAYED 1
#define YK_STAT_SEM 2
#define YK_STAT_QUEUE 3
#define YK_STAT_EVENT 4

#define EVENT_WAIT_ANY 0
#define EVENT_WAIT_ALL 1

typedef unsigned char UBYTE;
typedef signed char BYTE;
typedef unsigned int UWORD;
typedef signed int WORD;

/***********************************************************************
* TASK CONTROL BLOCKS
***********************************************************************/
typedef struct taskblock *TCBptr;
typedef struct taskblock {
	void		*stack_ptr;
	UBYTE		status;
	int			priority;
	int			num_delay;
	UBYTE		event_mode;
	UWORD		event_flags;
	TCBptr		next;
	TCBptr		prev;
} YK_TCB;

extern YK_TCB YKTCBArray[MAXTASKS+1];
extern TCBptr YKReadyList;
extern TCBptr YKDelayedList;
extern TCBptr YKTCBFreeList;	
extern TCBptr YKCurTask;

/***********************************************************************
* SEMAPHORE CONTROL BLOCKS
***********************************************************************/

typedef struct sem *YKSemPtr;
typedef struct sem
{
	UWORD  		count;
	TCBptr		pendList;
	YKSemPtr  	nextSem;
} YK_SEM;
typedef	struct sem YKSEM; //alias used by Lab5 task code

extern YK_SEM YKSemArray[MAXSEMS];
extern YKSemPtr YKSemFreeList;

/***********************************************************************
* QUEUE CONTROL BLOCKS
***********************************************************************/

typedef struct queue *YKQueuePtr;
typedef struct queue {
	void		**start;
	void		**end;
	void		**nextEmptySlot;
	void		**nextToRemove;
	UBYTE		size;
	UBYTE		numEntries;
	TCBptr		pendList;
	YKQueuePtr 	nextQueue;
} YK_QUEUE;
typedef struct queue YKQ; //alias used by Lab6 task code

extern YK_QUEUE YKQueueArray[MAXQUEUES];
extern YKQueuePtr YKQueueFreeList;

/***********************************************************************
* EVENT CONTROL BLOCKS
***********************************************************************/

typedef struct event *YKEventPtr;
typedef struct event {
	UWORD		flags;
	TCBptr		pendList;
	YKEventPtr	nextEvent;
} YK_EVENT;
typedef struct event YKEVENT; //alias used by Lab7 task code 

extern YK_EVENT YKEventArray[MAXEVENTS];
extern YKEventPtr YKEventFreeList;

/***********************************************************************
* PRIORITY TABLE
***********************************************************************/
#define NUM_PRIOS 20
#define IDLE_PRIO 19
extern int YKPrioTable[NUM_PRIOS];

/***********************************************************************
* IDLE TASK
***********************************************************************/
#define IDLE_STACK_SIZE 256
extern int YKIdleStack[IDLE_STACK_SIZE];

/***********************************************************************
* KERNEL GLOBALS
***********************************************************************/

extern int YKCtxSwCount;
extern int YKIdleCount;
extern int YKTickNum;
extern int YKNumTasksCreated;
extern int YKInterruptLevel;
extern int YKRunning;

/***********************************************************************
* KERNEL FUNCTIONS
***********************************************************************/

void YKInitialize(void);
void YKNewTask(void (* task)(void), void *taskStack, unsigned char priority);
void YKIdleTask(void *data);
void YKTCBInit(UBYTE priority, void *stack);
void YKAddTCBToList(TCBptr *list, TCBptr tcb);
void YKRemoveTCBFromList(TCBptr *list, TCBptr tcb);
void YKRun(void);
void YKDelayTask(unsigned count);
#ifdef __cplusplus
extern "C" {
#endif
void YKEnterMutex(void);
void YKExitMutex(void);
#ifdef __cplusplus
}
#endif
void YKEnterISR(void);
void YKExitISR(void);
void YKScheduler(void);
#ifdef __cplusplus
extern "C" {
#endif
void YKDispatcher(void);
#ifdef __cplusplus
}
#endif
void YKTickHandler(void);
YKSemPtr YKSemCreate(int count);
void YKSemPend(YKSemPtr sem);
void YKSemPost(YKSemPtr sem);
YKQueuePtr YKQCreate(void **start, unsigned int size);
void* YKQPend(YKQueuePtr q);
int YKQPost(YKQueuePtr q, void *msg);
YKEventPtr YKEventCreate(unsigned int initialValue);
unsigned int YKEventPend(YKEventPtr event, unsigned int eventMask, int waitMode);
void YKEventSet(YKEventPtr event, unsigned int eventMask);
void YKEventReset(YKEventPtr event, unsigned int eventMask);

void YKPrintLists(void);

#endif

