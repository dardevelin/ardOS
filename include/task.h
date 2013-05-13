/*
 * task.h
 *
 * Created: 12/4/2013 12:40:06 PM
 *  Author: dcstanc
 *
 * Header file for task creation and handling
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


#ifndef TASK_H_
#define TASK_H_

#ifndef uint8_t
	#define uint8_t unsigned char
#endif

#include "ARDOSConfig.h"

// Definition for task control block
typedef struct tc
{
	uint8_t pid;	// Process ID
	uint8_t prio;	// Task priority
	
	// For round robin scheduler
	#if OSSCHED_TYPE==OS_RR
		unsigned int quantum;
		unsigned int timeleft;
	#endif
	
	unsigned char status; // bit 0 = first run flag, bit 1 = blocked flag
	unsigned long sp;	// Stack pointer
	void (*taskptr)(void*); // Task pointer
	void *rarg;
} tTCB;


// Definition for task queues
typedef struct tq
{
	unsigned char *qptr, head, tail, len, ctr;
} tQueue;

#if OS_DEBUG == 1
	void printProcQ(tQueue *q, tTCB *tasklist);
#endif

void initQ(unsigned char *, unsigned char len, tQueue *q);
void prioEnq(int pid, tTCB *tasklist, tQueue *q);
void enq(int pid, tQueue *q);
void procEnq(int pid, tTCB *tasklist, tQueue *q);
unsigned char procPeek(tQueue *q);
unsigned char procDeq(tQueue *q);


#endif /* TASK_H_ */