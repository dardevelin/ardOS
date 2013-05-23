/*
 * task.c
 *
 * Created: 12/4/2013 9:28:57 AM
 *  Author: dcstanc
 *
 * Task handling routines for the kernel. Provides task creation and queue management services.
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

#include "kernel.h"
#include "task.h"
#include "ArdOSConfig.h"
	
// Priority queue routines
void prioEnq(int pid, tTCB *tasklist, tQueue *q)
{
	unsigned char i;
	unsigned int iter=q->head;
	unsigned char flag=0;
	
	OSMakeAtomic();	
	if(q->ctr >= q->len)
		return;
		
	while(iter != q->tail && !flag)
	{
		flag=(tasklist[q->qptr[iter]].prio > tasklist[pid].prio);
			
		if(!flag)
			iter=(iter+1) % q->len;
	}		
		

	// If we have found our spot, shift the rest down and insert. Otherwise insert at the end
	if(flag)
	{
		if(q->tail > q->head)
			for(i=q->tail-1; i>=iter && i != 255; i--)
				q->qptr[(i+1)%q->len]=q->qptr[i];
		else
		{
				for(i=(q->tail > 0 ? q->tail-1 : q->len-1); i!=iter; i=(i>0 ? i-1 : q->len-1))
					q->qptr[(i+1)%q->len]=q->qptr[i];

				// Last case
				q->qptr[(i+1)%q->len]=q->qptr[i];
		}
	}
	else
		iter=q->tail;
		
	q->tail=(q->tail+1)%q->len;	
	q->qptr[iter]=pid;
	q->ctr++;
	OSExitAtomic();
}

void enq(int pid, tQueue *q)
{
	OSMakeAtomic();
	if(q->ctr >= q->len)
		return;
			
	q->qptr[q->tail]=pid;
	q->ctr++;
	q->tail = (q->tail+1) % q->len;
	OSExitAtomic();
}
	
void procEnq(int pid, tTCB *tasklist, tQueue *q)
{
	#if OSSCHED_TYPE==OS_PRIORITY
		prioEnq(pid, tasklist, q);
	#elif OSSCHED_TYPE==OS_RR
		enq(pid,  q);
	#endif
				
}

unsigned char procPeek(tQueue *q)
{
	if(!q->ctr)
		return 255;
	else
		return q->qptr[q->head];	
}

unsigned char procDeq(tQueue *q)
{
	unsigned char ret=255;
	OSMakeAtomic();
	if(q->ctr>0)
	{
		ret=q->qptr[q->head];
		q->head=(q->head+1)%q->len;
		q->ctr--;
	}				
	OSExitAtomic();
	return ret;
}

void initQ(unsigned char *qbuf, unsigned char len, tQueue *q)
{
	unsigned char i;
		
	OSMakeAtomic();
	q->head=0;
	q->tail=0;
	q->qptr=qbuf;
	q->len=len;
	q->ctr=0;

	for(i=0; i<len; i++)
		q->qptr[i]=255;
	OSExitAtomic();
}
	

