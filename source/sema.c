/*
 * sema.c
 *
 * Created: 23/4/2013 4:24:45 PM
 *  Author: dcstanc
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

#include "ardOS.h"
#include "sema.h"

#if OSUSE_SEMA==1

void OSInitSema(TOSSema *sema, unsigned int initval, unsigned char isBinary)
{
	sema->isBinary=isBinary;
	
	if(!isBinary)
		sema->semaval=initval;
	else
		if(initval)
			sema->semaval=1;
		else
			sema->semaval=0;
	
	initQ(sema->tasklist, OSMAX_TASKS, &sema->taskQ);
}

void OSTakeSema(TOSSema *sema)
{
	if(sema->semaval>0)
		sema->semaval--;
	else
	{
		// Block current process
		_tasks[_running].status |= _OS_BLOCKED;
		
		// Enqueue this task
		prioEnq(_running, _tasks, &sema->taskQ);
		// Call scheduler.
		OSSwap();
	}
}

void OSGiveSema(TOSSema *sema)
{
	unsigned char tsk=procDeq(&sema->taskQ);
	
	if(tsk != 255)
	{
		// Removed blocked flag
		_tasks[tsk].status &= ~(_OS_BLOCKED);
		procEnq(tsk, _tasks, &_ready);
		
		// Call scheduler
		OSPrioSwap();
	}
	else
		if(sema->isBinary)
			sema->semaval=1;
		else
			sema->semaval++;
}

#endif