/*
 * queue.c
 *
 * Created: 4/5/2013 9:36:36 AM
 *  Author: dcstanc
 
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

#include "queue.h"

#if OSUSE_QUEUES==1 || OSUSE_PRIOQUEUES==1

// Unblock a blocked process and call the scheduler
void OSQueueUnblock(TMsgQ *queue)
{
	_tasks[queue->blockproc].status &= ~_OS_BLOCKED;

	// Enqueue blocked process
	procEnq(queue->blockproc, _tasks, &_ready);
	queue->blockproc=255;
	
	OSExitAtomic();

	// Make a priority swap	
	OSPrioSwap();
}

int OSDequeue(TMsgQ *queue)
{
	int ret=255;
	OSMakeAtomic();

	if(!queue->count)
	{
		// Block the current task
		queue->blockproc=_running;
		_tasks[_running].status|=_OS_BLOCKED;
		
		OSExitAtomic();
		
		// Make a task swap
		OSSwap();
	}
	
	OSMakeAtomic();
	#if OSUSE_QUEUES==1
	
		if(!queue->prioQ)
			ret=queue->qbuf[queue->head];

	#endif

	#if OSUSE_PRIOQUEUES==1

		if(queue->prioQ)
			ret=queue->pqbuf[queue->head].data;

	#endif

	queue->head = (queue->head+1) % queue->len;
	queue->count--;
	
	OSExitAtomic();
	return ret;
}
#endif

#if OSUSE_QUEUES==1
void OSMakeQueue(int *buffer, unsigned char length, TMsgQ *queue)
{
	OSMakeAtomic();
	queue->blockproc=255;
	queue->count=0;
	queue->head=0;
	queue->tail=0;
	queue->qbuf=buffer;
	queue->len=length;
	queue->prioQ=0;
	OSExitAtomic();	
}

void OSEnqueue(int data, TMsgQ *queue)
{
	OSMakeAtomic();
	if(queue->count==queue->len)
		return;
	
	if(!queue->prioQ)
	{
		queue->qbuf[queue->tail]=data;
		queue->tail = (queue->tail+1)  % queue->len;
		queue->count++;		
	}
	
	// Check if any process is blocked. Unblock if there is
	if(queue->blockproc!=255)
		OSQueueUnblock(queue);
		
	OSExitAtomic();
}

#endif

#if OSUSE_PRIOQUEUES==1
void OSMakePrioQueue(TPrioNode *buffer, unsigned char length, TMsgQ *queue)
{
	OSMakeAtomic();
	queue->blockproc=255;
	queue->count=0;
	queue->head=0;
	queue->len=length;
	queue->pqbuf=buffer;
	queue->prioQ=1;
	queue->tail=0;
	OSExitAtomic();
}

void OSPrioEnqueue(int data, unsigned char prio, TMsgQ *queue)
{
		unsigned char i;
		unsigned int iter=queue->head;
		unsigned char flag=0;
		
		OSMakeAtomic();
		if(queue->count >= queue->len)
			return;
		
		while(iter != queue->tail && !flag)
		{
			flag=(queue->pqbuf[iter].prio > prio);
			
			if(!flag)
				iter=(iter+1) % queue->len;
		}
		

		// If we have found our spot, shift the rest down and insert. Otherwise insert at the end
		if(flag)
		{
			if(queue->tail > queue->head)
				for(i=queue->tail-1; i>=iter && i != 255; i--)
					queue->pqbuf[(i+1)%queue->len]=queue->pqbuf[i];
			else
			{
				for(i=(queue->tail > 0 ? queue->tail-1 : queue->len-1); i!=iter; i=(i>0 ? i-1 : queue->len-1))
					queue->pqbuf[(i+1)%queue->len]=queue->pqbuf[i];

				// Last case
				queue->pqbuf[(i+1)%queue->len]=queue->pqbuf[i];
			}
		}
		else
			iter=queue->tail;
		
		queue->tail=(queue->tail+1)%queue->len;
		queue->pqbuf[iter].data=data;
		queue->pqbuf[iter].prio=prio;
		queue->count++;
		
		if(queue->blockproc!=255)
			OSQueueUnblock(queue);
			
		OSExitAtomic();

}
#endif



