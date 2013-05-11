/*
 * ardio.h
 *
 * Created: 10/4/2013 7:19:46 AM
 *  Author: Colin Tan
 * 
 * This file contains macros for fast access to GPIOs, ADCs and PWM ports
 *
 * 10/4/2013 Initial version
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

#ifndef IODEFS

	#define IODEFS


	#define F_CPU	16000000
	#include "ArdOSConfig.h"
	
	#include <avr/io.h>
	#include <avr/interrupt.h>

#if USEIO == 1
#if USEGPIO == 1
	// GPIO pin defs for masking
	#define PIN0M	0b1
	#define PIN1M	0b10
	#define PIN2M	0b100
	#define PIN3M	0b1000
	#define PIN4M	0b10000
	#define PIN5M	0b100000
	#define PIN6M	0b1000000
	#define PIN7M	0b10000000

	
	// GPIO pin defs for getting inputs / setting outputs
	#define BIT0	0
	#define BIT1	1
	#define BIT2	2
	#define BIT3	3
	#define BIT4	4
	#define BIT5	5
	#define BIT6	6
	#define BIT7	7

	// GPIO port defs for output
	#define OUTB	&PORTB
	#define OUTC	&PORTC
	#define OUTD	&PORTD

	// GPIO port defs for input
	#define INB		&PINB	
	#define INC		&PINC	
	#define IND		&PIND

	// GPIO port defs for directions
	#define DIRB	&DDRB
	#define DIRC	&DDRC
	#define DIRD	&DDRD
	
	#define HIGH	1
	#define LOW		0
	#define INPUT	0
	#define OUTPUT	1
		
	// GPIO macros
	// Raw fast functions
	void makeInputPins(volatile uint8_t *port, uint8_t mask);
	void makeOutputPins(volatile uint8_t *port, uint8_t mask);
	
	int getInput(volatile uint8_t *port, uint8_t pin);
	void setOutput(volatile uint8_t *port, uint8_t pin, uint8_t value);

#if USEARDGPIO == 1
	// Arduino style GPIO
	void pinMode(int pin, int mode);
	void digitalWrite(int pin, int val);
	int digitalRead(int pin);	
	
#endif
#endif

	// ADC support
#if USEADC == 1
	void setupADC();
	int getADC(int portnum);

#if USEARDADC == 1
	int analogRead(int portnum);
#endif
#endif

#if USEPWM == 1

	// PWM support
#if PIN5PWM == 1
extern unsigned int _pin5val;
#endif

#if PIN6PWM == 1
extern unsigned int _pin6val;
#endif

#if PIN9PWM == 1
extern unsigned int _pin9val;
#endif

#if PIN10PWM == 1
extern unsigned int _pin10val;
#endif

#if PIN11PWM == 1
extern unsigned int _pin11val;
#endif

	// PWM constants
	#define PIN5TCCRA		&TCCR0A
	#define PIN5TCNT		&TCNT0
	#define PIN5OCR			&OCR0B
	#define PIN5TCCRB		&TCCR0B
	#define PIN5MSK			&TIMSK0
	#define PIN5DDR			&DDRD
	#define PIN5PIN			5
	#define PIN5CR			1
	#define PIN5SV			&_pin5val
	
	#define PIN6TCCRA		&TCCR0A
	#define PIN6TCNT		&TCNT0
	#define PIN6OCR			&OCR0A
	#define PIN6TCCRB		&TCCR0B
	#define PIN6MSK			&TIMSK0
	#define PIN6DDR			&DDRD
	#define PIN6PIN			6
	#define PIN6CR			0
	#define PIN6SV			&_pin6val

	#define PIN9TCCRA		&TCCR1A
	#define PIN9TCNT		&TCNT1
	#define PIN9OCR			&OCR1A
	#define PIN9TCCRB		&TCCR1B
	#define PIN9MSK			&TIMSK1
	#define PIN9DDR			&DDRB
	#define PIN9PIN			1
	#define PIN9CR			0
	#define PIN9SV			&_pin9val
	
	#define PIN10TCCRA		&TCCR1A
	#define PIN10TCNT		&TCNT1
	#define PIN10OCR		&OCR1B
	#define PIN10TCCRB		&TCCR1B
	#define PIN10MSK		&TIMSK1
	#define PIN10DDR		&DDRB
	#define PIN10PIN		2
	#define PIN10CR			1
	#define PIN10SV			&_pin10val

	#define PIN11TCCRA		&TCCR2A
	#define PIN11TCNT		&TCNT2
	#define PIN11OCR		&OCR2A
	#define PIN11TCCRB		&TCCR2B
	#define PIN11MSK		&TIMSK2
	#define PIN11DDR		&DDRB
	#define PIN11PIN		3
	#define PIN11CR			0
	#define PIN11SV			&_pin11val
	
#if (PIN5PWM == 1) || (PIN6PWM == 1) || (PIN11PWM == 1)
	// compreg == 0 if you are using compare register A and 1 if using compare register B
	void setupPWM(volatile uint8_t *TCNT, volatile uint8_t *TCCRA, volatile uint8_t *TIMSK, volatile uint8_t *DDR, uint8_t pinnum, uint8_t compreg);
	
	// value specified between 0 and 255 or 0 and 65535. storevar = which variable to record PWM value in for the ISR
	void setDutyCycle(volatile uint8_t *OCR, int value, unsigned int *storevar);
	void startPWM(volatile uint8_t *TCCRB);
#endif // PIN5 || PIN6 || PIN 11

#if (PIN9PWM==1) || (PIN10PWM == 1)
	// compreg == 0 if you are using compare register A and 1 if using compare register B
	void setupPWM16(volatile uint16_t *TCNT, volatile uint8_t *TCCRA, volatile uint8_t *TIMSK, volatile uint8_t *DDR, uint8_t pinnum, uint8_t compreg);
	
	// value specified between 0 and 255 or 0 and 65535. storevar = which variable to record PWM value in for the ISR
	void setDutyCycle16(volatile uint16_t *OCR, int value, unsigned int *storevar);
	void startPWM16(volatile uint8_t *TCCRB);
#endif // PIN9 || PIN 10

#if USEARDPWM == 1
	void analogWrite(int pinnum, int value);
#endif // USEARDPWM

#endif // USEPWM

#endif // USEIO
#endif // IODEFS
