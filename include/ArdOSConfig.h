/*
 * ArdOSConfig.h
 *
 * Created: 10/4/2013 12:22:11 PM
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


#ifndef ARDOSCONFIG_H_
#define ARDOSCONFIG_H_


// OS Configuration File

/* ------------------------------------------------------------------------

Operating System Configuration: Configure OS clock frequency, scheduler type, etc.


---------------------------------------------------------------------------- */

// OS Scheduler Types
#define OS_PRIORITY	0
#define OS_RR		1
#define OS_RMS		2
#define OS_EDF		3

// Supported CPU Types
#define AT168		0
#define AT328		1
#define AT1280		2
#define AT2560		3

// Configure the CPU Type
#define OSCPU_TYPE	AT2560

// Quantum parameters for OS_RR
#define OSMIN_QUANTUM		5		// Minimum execution time in ms
#define OSMAX_QUANTUM		500		// Maximum execution time in ms
#define OSMAX_PRIOLEVEL		1		// Highest priority level
#define OSQUANTUM_FACTOR	(OSMAX_QUANTUM-OSMIN_QUANTUM)/OSMAX_PRIOLEVEL

// Switch on debug printing
#define OS_DEBUG			0

// Pre-emptive or cooperative
#define OS_PREEMPTIVE		1

// Scheduler type: 0 = Fixed priority, 1 = Priority RR, 2 = EDF
#define OSSCHED_TYPE		OS_PRIORITY

// Parameters for the task manager

// Number of tasks
#define OSTASK_COUNT		2

// Maximum number of tasks. DO NOT MODIFY THIS
#define OSMAX_TASKS			OSTASK_COUNT+1

// Use sleep
#define OSUSE_SLEEP			1

// Use semaphores
#define OSUSE_SEMA			1

// Use queues
#define OSUSE_QUEUES		1

// Use priority queues
#define OSUSE_PRIOQUEUES	1

// Mutexes and conditionals
#define OSUSE_MUTEXES		1
#define OSUSE_CONDITIONALS	1

// Set to 1 if you want to use ArdOS hardware services. Set to 0 if you want to use your own routines.
#define USEIO		1

// GPIO Configuration Section
#define USEGPIO		1	// Enables GPIO
#define USEARDGPIO	1	// Enables pinMode, digitalRead and digitalWrite

// ADC Configuration Section
#define USEADC		1	// Enables ADC
#define USEARDADC	1	// Enables analogRead

// PWM Configuration Section
#define USEPWM		1	// Enables PWM
#define USEARDPWM	1	// Enables analogWrite

// Switch on the PWM pins you intend to use.
// Note that Pin 3 PWM is not available as the OS uses OCR2B for the timer ticks
#define PIN5PWM		1	// Creates ISR for PWM pin 5. Causes code for 8-bit PWM to be built.
#define PIN6PWM		1	// Creates ISR for PWM pin 6. Causes code for 8-bit PWM to be built.
#define PIN9PWM		1	// Creates ISR for PWM pin 9. Causes code for 10-bit PWM to be built.
#define PIN10PWM	1	// Creates ISR for PWM pin 10. Causes code for 10-bit PWM to be built.
#define PIN11PWM	1	// Creates ISR for PWM pin 11. Causes code for 8-bit PWM to be built.

#endif /* ARDOSCONFIG_H_ */
