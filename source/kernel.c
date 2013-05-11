/*
 * kernel.c
 *
 * Created: 13/4/2013 10:13:40 AM
 *  Author: dcstanc
 *
 * Contains main OS codes
 *
 *
 *
   	Copyright (C) 2013 Colin Tan
   	
   	
   	This file is part of ArdOS.

   	ArdOS is free software: you can redistribute it and/or modify
   	it under the terms of the GNU Lesser General Public License as published by
   	the Free Software Foundation, either version 3 of the License, or
   	(at your option) any later version.

   	ArdOS is distributed in the hope that it will be useful,
   	but WITHOUT ANY WARRANTY; without even the implied warranty of
   	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   	GNU Lesser General Public License for more details.

   	You should have received a copy of the GNU Lesser General Public License
   	along with ArdOS.  If not, see <http://www.gnu.org/licenses/>.

 */ 

#include <stdlib.h>
#include "ArdOS.h"

// Stack pointer temp variable
unsigned long pxCurrentTCB;

// Pointer to routine start
unsigned long pxFuncPtr, pxFuncArg;

// Task lists
unsigned char _procCount=0;
tTCB _tasks[OSMAX_TASKS];

// Task Ready Queue
static unsigned char _readybuf[OSMAX_TASKS];
tQueue _ready;

// Tick counter
unsigned long _osticks=0;

// Task running index
unsigned char _running=255;
unsigned int _errno=0;

// Forced Swap flag
unsigned char _forcedSwap=0;

// Scheduler suspension flag
unsigned char _suspend=0;

// Macros to save and restore context
#define portSAVE_CONTEXT()\
asm volatile (\
"push r0	\n\t"\
"in r0, __SREG__	\n\t"\
"cli	\n\t"\
"push r0	\n\t"\
"push r1	\n\t"\
"clr r1	\n\t"\
"push r2	\n\t"\
"push r3	\n\t"\
"push r4	\n\t"\
"push r5	\n\t"\
"push r6	\n\t"\
"push r7	\n\t"\
"push r8	\n\t"\
"push r9	\n\t"\
"push r10	\n\t"\
"push r11	\n\t"\
"push r12	\n\t"\
"push r13	\n\t"\
"push r14	\n\t"\
"push r15	\n\t"\
"push r16	\n\t"\
"push r17	\n\t"\
"push r18	\n\t"\
"push r19	\n\t"\
"push r20	\n\t"\
"push r21	\n\t"\
"push r22	\n\t"\
"push r23	\n\t"\
"push r24	\n\t"\
"push r25	\n\t"\
"push r26	\n\t"\
"push r27	\n\t"\
"push r28	\n\t"\
"push r29	\n\t"\
"push r30	\n\t"\
"push r31	\n\t"\
"in r26, __SP_L__	\n\t"\
"in r27, __SP_H__	\n\t"\
"sts pxCurrentTCB+1, r27	\n\t"\
"sts pxCurrentTCB, r26	\n\t"\
"sei	\n\t" : : :\
);

#define portRESTORE_CONTEXT()\
asm volatile (\
"out __SP_L__, %A0	\n\t"\
"out __SP_H__, %B0	\n\t"\
"pop r31	\n\t"\
"pop r30	\n\t"\
"pop r29	\n\t"\
"pop r28	\n\t"\
"pop r27	\n\t"\
"pop r26	\n\t"\
"pop r25	\n\t"\
"pop r24	\n\t"\
"pop r23	\n\t"\
"pop r22	\n\t"\
"pop r21	\n\t"\
"pop r20	\n\t"\
"pop r19	\n\t"\
"pop r18	\n\t"\
"pop r17	\n\t"\
"pop r16	\n\t"\
"pop r15	\n\t"\
"pop r14	\n\t"\
"pop r13	\n\t"\
"pop r12	\n\t"\
"pop r11	\n\t"\
"pop r10	\n\t"\
"pop r9	\n\t"\
"pop r8	\n\t"\
"pop r7	\n\t"\
"pop r6	\n\t"\
"pop r5	\n\t"\
"pop r4	\n\t"\
"pop r3	\n\t"\
"pop r2	\n\t"\
"pop r1	\n\t"\
"pop r0	\n\t"\
"out __SREG__, r0\n\t"\
"pop r0	\n\t": : "r" (pxCurrentTCB):\
);

// Sets up SP to point to the thread stack
#define portSetStack()\
asm volatile(\
	"cli	\n\t"\
	"OUT __SP_L__, %A0	\n\t"\
	"OUT __SP_H__, %B0	\n\t"\
	"sei": : "r" (pxCurrentTCB):\
)

// Loads the starting address of the thread function onto the stack and
// puts in the passed parameter into R25 and R24 as expected by the function.
#define portPushRetAddress()\
asm volatile(\
	"cli	\n\t"\
	"mov r0, %A0	\n\t"\
	"push r0	\n\t"\
	"mov r0, %B0	\n\t"\
	"push r0	\n\t"\
	"mov R25, %B1	\n\t"\
	"mov R24, %A1	\n\t"\
	"sei": : "r" (pxFuncPtr), "r" (pxFuncArg):\
)
// Error handling
unsigned int OSGetError()
{
	return _errno;
}

void OSSetError(unsigned int errno)
{
	_errno=errno;
}

// Atomicity Control
inline void OSMakeAtomic()
{
	cli();
}

inline void OSExitAtomic()
{
	sei();
}

#if OSUSE_SLEEP==1

	// Sleep routine. Records number of milliseconds to sleep. sleep_flag tells you if process is indeed sleeping. 1 bit per process
	// maximum 16 processes.
	unsigned int _sleepTime[OSMAX_TASKS];
	int _sleepFlag=0;

	// Sleep routine. Sleeps in milliseconds
	void OSSleep(unsigned int millis)
	{
		// Set sleep time
		_sleepTime[_running]=millis;
		_sleepFlag |= (1<<_running);
	
		// Set blocked flag
		_tasks[_running].status|=_OS_BLOCKED;
	
		// Note: No need to remove from READY queue because a _running process would have already been de-queued from there.
		// So just call scheduler to swap.
		
		OSSwap();
	}
#endif

// OS Task Management Routines
// Task handling

// prio = Task priority, taskStack = stack for task, rptr = Pointer to task routines, rarg = Arguments to pass to task
unsigned int OSCreateTask(int prio, unsigned long *taskStack, void (*rptr)(void *), void *rarg)
{
	if(_procCount==OSMAX_TASKS)
	{
		OSSetError(OS_ERR_MAX_PROCS);
		return OSGetError();
	}

	// For fixed priority we must have unique priorities and priorities must be between 0 and n-1 for n processes.
	#if OSSCHED_TYPE == OS_PRIORITY
	
		// Check if current priority level already exists
		uint8_t i, found=0;
	
		for(i=0; i<_procCount && !found; i++)
			found=(_tasks[i].prio==prio);
	
		if(found)
		{
			OSSetError(OS_ERR_DUP_PRIO);
			return OSGetError();
		}
		// Check for BAD_PRIO. Should be between 0 and _procCount-1
		if(prio != 255 && (prio <0 || prio > _procCount))
		{
			OSSetError(OS_ERR_BAD_PRIO);
			return OSGetError();
		}
		
	#endif
	
	// For round robin we cannot have negative priority or priority above OSMAX_PRIOLEVEL
	#if OSSCHED_TYPE == OS_RR
	{
		if(prio != 255 && (prio < 0 || prio > OSMAX_PRIOLEVEL))
		{
			OSSetError(OS_ERR_BAD_PRIO);
			return OSGetError();
		}	
	}
	#endif
	
	// Insert task
	_tasks[_procCount].prio=prio;
	_tasks[_procCount].taskptr=rptr;
	_tasks[_procCount].rarg=rarg;
	_tasks[_procCount].sp=(unsigned long) taskStack;
	_tasks[_procCount].pid=_procCount;
	_tasks[_procCount].status|=_OS_FIRSTRUN;

	// For round robin we must calculate time quantum
	#if OSSCHED_TYPE==OS_RR
	
		// If prio is 255 we have the idle task, which we set to have a time quantum of 1 ms
		if(prio!=255)
			_tasks[_procCount].quantum=(OSMAX_PRIOLEVEL-prio) * OSQUANTUM_FACTOR + OSMIN_QUANTUM;
		else
			_tasks[_procCount].quantum=1;
		
		// Initialze the time left
		_tasks[_procCount].timeleft=_tasks[_procCount].quantum;
	#endif
	
	// Insert into ready queue	
	procEnq(_procCount, _tasks, &_ready);
	_procCount++;

	return OS_NO_ERR;
}

void OSScheduler() 
{
	// Remove first item from queue
	unsigned char _nextRun=procPeek(&_ready);
	
	// Check to see that it is a proper process
	if(_nextRun != 255)
#if OSSCHED_TYPE==OS_PRIORITY
	if(_tasks[_nextRun].prio < _tasks[_running].prio || _forcedSwap)
#endif
	{
		_nextRun=procDeq(&_ready);
		if(_running!=255 && _nextRun != _running)
		{
			_tasks[_running].sp=pxCurrentTCB;
			
			// Push to READY queue if not blocked
			if(!(_tasks[_running].status & _OS_BLOCKED))
				procEnq(_running, _tasks, &_ready);
		}

		pxCurrentTCB=_tasks[_nextRun].sp;
		_running=_nextRun;
		
	}
	
//	_running=0;

}

inline void runTask() 
{
	// Check if this is the first run of this process
	if(_tasks[_running].status & _OS_FIRSTRUN)
	{
		_tasks[_running].status &= ~(_OS_FIRSTRUN);
		portSetStack();
		// Run the function by setting (R25,R24) = function argument and pushing the start
		// address of the function onto the stack
		pxFuncPtr=(unsigned long) _tasks[_running].taskptr;
		pxFuncArg=(unsigned long) _tasks[_running].rarg;
		portPushRetAddress();
	}
	else
		portRESTORE_CONTEXT();	
}

// Swaps task. Causes current task to relinquish control of the CPU. Scheduler selects next task to run.
void OSSwap() 
{
	portSAVE_CONTEXT();
	if(!_suspend)
	{
		_forcedSwap=1;
		OSScheduler();		
	}
	
	runTask();
	asm("ret");
}

void OSSwapFromISR() 
{
	portSAVE_CONTEXT();
	if(!_suspend)
	{
		_forcedSwap=1;
		OSScheduler();		
	}
	
	runTask();	
	asm("reti");
}

void OSPrioSwap()
{
	portSAVE_CONTEXT();
	
	if(!_suspend)
	{
		_forcedSwap=1;
		OSScheduler();
	}

	runTask();		
	asm("ret");	
}

void OSPrioSwapFromISR()
{
	portSAVE_CONTEXT();
	if(!_suspend)
	{
		_forcedSwap=0;
		OSScheduler();
	}

	runTask();	
	asm("reti");	
}

void OSSuspendScheduler()
{
	_suspend=1;
}

void OSResumeScheduler()
{
	_suspend=0;
	OSPrioSwapFromISR();
}

ISR(TIMER2_OVF_vect, ISR_NAKED)
{
	//portSAVE_CONTEXT();
	// We don't call the scheduler for fixed priority.
	// Increment tick counter
	_osticks++;
	
	OSSuspendScheduler();
	#if OSUSE_SLEEP==1
		// Decrement wait counters
		unsigned char i;
	
		for(i=0; i<OSMAX_TASKS; i++)
			if(_sleepTime[i]>0)
				_sleepTime[i]--;
			else
				if(_sleepFlag & (0b1 << i))
				{
					// Clear the flag
					_sleepFlag &= ~(0b1 << i);
				
					// Unblock the task
					_tasks[i].status &= ~(_OS_BLOCKED);
					// Put onto ready queue, call scheduler
					procEnq(i, _tasks, &_ready);
				
					// Call scheduler if this is fixed priority
				
					#if OSSCHED_TYPE==OS_PRIORITY && OS_PREEMPTIVE==1
						OSPrioSwapFromISR();
					#endif
				}
	#endif
	
	// Round Robin scheduler
	#if OSSCHED_TYPE==OS_RR && OS_PREEMPTIVE==1
		// Check if timeleft is 0. If so restore time_left and insert into ready queue. Otherwise decrement timeleft
		if(!_tasks[_running].timeleft)
		{
			// Restore time left
			_tasks[_running].timeleft=_tasks[_running].quantum;
			
			// And call swap. Note if the queue is empty and the current task is the idle task it will be re-executed immediately
			OSSwapFromISR();
		}
		else
			_tasks[_running].timeleft--;
	#endif	
	
	//portRESTORE_CONTEXT();
	OSResumeScheduler();
	asm("reti");
}

unsigned long OSticks()
{
	return _osticks;
}

void configureTimer()
{
	// Set fast PWM, OC2A and OC2B disconnected.
	TCCR2A=0b00000011;
	OCR2B=255;
	TCNT2=0;
	
	// Enable TOV2
	TIMSK2|=0b1;
	
}

void startTimer()
{
	// Start timer giving frequency of approx 1000 Hz
	TCCR2B=0b00000100;
	sei();
}


// OS Initialization and Starting Routines
void OSInit()
{
	configureTimer();

	_suspend=0;
	// Initialization code for fixed priority. +1 for idle task
	initQ(_readybuf, OSMAX_TASKS, &_ready);
	
}


// The idle task. Just wastes CPU cycles
void _OSIdle(void *p)
{
	while(1);
}

void (*OSIdletask)(void *)=_OSIdle;
unsigned long _idleStack[20];

void OSSetIdleTask(void (*idletask)(void *))
{
	OSIdletask=idletask;
}

void OSRun()
{
	cli();
	// Create idle task
	OSCreateTask(255, &_idleStack[19], OSIdletask, NULL);

	startTimer();	
	
	sei();
	OSSwap();
}		