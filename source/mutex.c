/*
 * mutex.c
 * Provides mutex and conditional variable support
 * Created: 8/5/2013 8:47:44 AM
 *  Author: dcstanc
 */ 

#include "ArdOS.h"

#if OSUSE_MUTEXES==1
void OSCreateMutex(OSMutex *mutex)
{
	OSMakeAtomic();
	mutex->val=1;
	initQ(mutex->procList, OSMAX_TASKS, &mutex->blocked);
	OSExitAtomic();
}

void OSTakeMutex(OSMutex *mutex)
{
	OSMakeAtomic();
	if(!mutex->val)
	{
		procEnq(_running, _tasks, &mutex->blocked);
		_tasks[_running].status |= _OS_BLOCKED;
		OSExitAtomic();
		OSSwap();
	}		
	else
		mutex->val=0;
		
	OSExitAtomic();
}

void OSGiveMutex(OSMutex *mutex)
{
	OSMakeAtomic();
	unsigned char wakeProc=procDeq(&mutex->blocked);
	
	if(wakeProc!=255)
	{
		_tasks[wakeProc].status &= ~_OS_BLOCKED;
		procEnq(wakeProc, _tasks, &_ready);
		OSExitAtomic();
		OSPrioSwap();	
	}	
	else
		mutex->val=1;
	
	OSExitAtomic();
}

#if OSUSE_CONDITIONALS==1
void OSCreateConditional(OSCond *cond)
{
	cond->blockedProcess=255;
}

// Can only be used by one process.
void OSWait(OSCond *cond, OSMutex *mutex)
{
	OSMakeAtomic();
	if(!cond->pendingWake)
	{
		cond->blockedProcess=_running;
		_tasks[_running].status |= _OS_BLOCKED;
		OSGiveMutex(mutex);
		OSMakeAtomic();
		// If val is 1 it means no swap took place.
		if(mutex->val)
		OSSwap();
	
		// Retake the mutex when we resume
		OSTakeMutex(mutex);		
	}
	else
		cond->pendingWake=0;
		
	OSExitAtomic();
}

void OSSignal(OSCond *cond)
{
	OSMakeAtomic();
	if(cond->blockedProcess != 255)
	{
		_tasks[cond->blockedProcess].status &= ~_OS_BLOCKED;
		procEnq(cond->blockedProcess, _tasks, &_ready);
		cond->blockedProcess=255;
		cond->pendingWake=0;
		OSPrioSwap();
	}
	else
		cond->pendingWake=1;
	OSExitAtomic();
}
#endif
#endif