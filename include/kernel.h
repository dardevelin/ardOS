/*
 * kernel.h
 *
 * Created: 13/4/2013 10:13:52 AM
 *  Author: dcstanc
 *
 * kernel.c - Kernel main routines
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


#ifndef KERNEL_H_
#define KERNEL_H_

#include "task.h"

// Kernel variables
extern unsigned char _procCount;
extern tTCB _tasks[OSMAX_TASKS];

// Task Ready Queue
extern tQueue _ready;

// Task running index
extern unsigned char _running;

// Task Status flags
#define _OS_FIRSTRUN	0b1
#define _OS_BLOCKED		0b10


// Error codes
#define OS_NO_ERR			0	// No error
#define OS_ERR_MAX_PROCS	1	// Reached maximum processes		
#define OS_ERR_BAD_PRIO		2	// Bad priority number (RR)
#define OS_ERR_DUP_PRIO		3	// Duplicate priority number (Fixed Priority)

/* Private OS Functions. Do not call these */

// Push ISR epilog return address onto the active stack

// Function signatures
void OSMakeAtomic();
void OSExitAtomic();
unsigned int OSGetError();
void OSSetError(unsigned int);

/* Public OS Functions */


void OSInit();
void OSRun();
// prio = Task priority, taskStack = stack for task, rptr = Pointer to task routines, rarg = Arguments to pass to task
unsigned int OSCreateTask(int prio, unsigned long *taskStack, void (*rptr)(void *), void *rarg);

// Swaps task. Causes current task to relinquish control of the CPU. Scheduler selects next task to run.
void OSSwap() __attribute__ ((naked));

// Priority swap: Swap takes place only if new task has a higher priority
void OSPrioSwap() __attribute__ ((naked));

// Set idle hook
void OSSetIdleTask(void (*idletask)(void *));

// Makes the current task sleep for the given number of milliseconds
void OSSleep(unsigned int millis);

// Returns the number of milliseconds since the OS was started up
unsigned long OSticks();


// Routines to be called within ISRs
void OSSuspendScheduler();
void OSResumeScheduler();
void OSSwapFromISR() __attribute__ ((naked));
void OSPrioSwapFromISR() __attribute__ ((naked));

#endif /* KERNEL_H_ */