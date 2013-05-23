/*
 * ardio.c
 *
 * Created: 10/4/2013 9:41:12 AM
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

//#if USEIO == 1
// Need to fix timer2 code

#include "ardio.h"
#include <avr/interrupt.h>
// GPIO routines.

#ifndef NULL
#define NULL (void *) 0
#endif

#if USEGPIO == 1 && USEIO ==1

inline void makeInputPins(volatile uint8_t *port, uint8_t mask)
{
	*port &= ~(mask);
}

inline void makeOutputPins(volatile uint8_t *port, uint8_t mask)
{
	*port |= mask;
}	
	
inline int getInput(volatile uint8_t *port, uint8_t pin)
{
	return (*port & (0b1 << pin));
}

inline void setOutput(volatile uint8_t *port, uint8_t pin, uint8_t value)
{
	if(value)
		*port |= (1 << pin);
	else
		*port &= ~(1 << pin);
}

#if USEARDGPIO == 1 && USEGPIO==1 && USEIO==1
#define getPhysicalPin(PIN) (PIN<8 ? PIN : PIN-8)

inline volatile uint8_t *getDDRPort(int pin)
{
	return (pin<8 ? &DDRD : &DDRB);
}

inline volatile uint8_t *getOutputPort(int pin)
{
	return (pin<8 ? &PORTD : &PORTB);
}

inline volatile uint8_t *getInputPort(int pin)
{
	return (pin<8 ? &PIND : &PINB);
}

void pinMode(int pin, int mode)
{		
	uint8_t myPin=getPhysicalPin(pin);
	volatile uint8_t *port=getDDRPort(pin);
		
	if(mode)
		makeOutputPins(port, 1<<myPin);
	else
		makeOutputPins(port, 1 << myPin);
}	

int digitalRead(int pin)
{
	uint8_t myPin=getPhysicalPin(pin);
	volatile uint8_t *port=getInputPort(pin);
	return getInput(port, myPin);
}

void digitalWrite(int pin, int val)
{
	uint8_t myPin=getPhysicalPin(pin);
	volatile uint8_t *port=getOutputPort(pin);
	setOutput(port, myPin, val);
}
#endif
#endif

// ADC ports
#if USEADC == 1 && USEIO==1

// Initialize the ADC
void initADC()
{
	#if OSCPU_TYPE==AT168 || OSCPU_TYPE==AT328
		// Turn off the power reduction register
		PRR&=~(0b1 << PRADC);
	#elif  OSCPU_TYPE==AT1280 || OSCPU_TYPE==AT2560
	
		PRR0&=~(0b1 << PRADC);
			
	#endif
	// Configure for 125Hz ADC clock, no interrupts.
	ADCSRA=0b10000111;
}	

// Read the ADC using non-interrupt busywait
int getADC(int portNum)
{
	#if OSCPU_TYPE==AT168 || OSCPU_TYPE==AT328
	ADMUX=0b01000000 + (portNum & 0x7);
	ADCSRA|=(1 << ADSC);
	
	while(ADCSRA & (1<<ADSC)); // Wait for ADSC to clear
	
	// Reset ADIF
	ADCSRA |= (1<<ADIF);
	
	// Now read and return value
	int lo=ADCL;
	return (ADCH << 8)+lo;
	#endif
	return 0;
}

#if USEARDADC == 1 && USEADC==1 && USEIO==1
// Arduino style ADC library
int analogRead(int portnum)
{
	initADC();
	return getADC(portnum);
}
#endif
#endif


#if USEPWM == 1 && USEIO==1

// ISRs
#if PIN5PWM == 1 && USEPWM==1 && USEIO==1
unsigned int _pin5val;

ISR(TIMER0_COMPB_vect)
{
	OCR0B=_pin5val;
}
#endif

#if PIN6PWM==1 && USEPWM==1 && USEIO==1
unsigned int _pin6val=0;
ISR(TIMER0_COMPA_vect)
{
	OCR0A=_pin6val;
}

#endif

#if PIN9PWM==1 && USEPWM==1 && USEIO==1
unsigned int _pin9val=0;

ISR(TIMER1_COMPA_vect)
{
	OCR1A=_pin9val;
}

#endif

#if PIN10PWM==1 && USEPWM==1 && USEIO==1
unsigned int _pin10val=0;
ISR(TIMER1_COMPB_vect)
{
	OCR1B=_pin10val;
}

#endif

#if PIN11PWM==1 && USEPWM==1 && USEIO==1
unsigned int _pin11val=0;

ISR(TIMER2_COMPA_vect)
{
	OCR2A=_pin11val;
}

#endif

#if ((PIN5PWM == 1) || (PIN6PWM == 1) || (PIN11PWM == 1)) && USEPWM==1 && USEIO==1

void setupPWM(volatile uint8_t *TCNT, volatile uint8_t *TCCRA, volatile uint8_t *TIMSK, volatile uint8_t *DDR, uint8_t pin, uint8_t compreg)
{
	*TCNT=0;

#if OSCPU_TYPE==AT168 || OSCPU_TYPE==AT328
	#if ((PIN5PWM==1) || (PIN6PWM==1)) && USEPWM==1 && USEIO==1	
		PRR &= ~(1 << PRTIM0);
	#endif

	#if (PIN11PWM==1) && USEPWM==1 && USEIO==1
		PRR &= ~(1 << PRTIM2);
	#endif

#elif OSCPU_TYPE==AT1280 || OSCPU_TYPE==AT2560

	#if ((PIN5PWM==1) || (PIN6PWM==1)) && USEPWM==1 && USEIO==1
	PRR0 &= ~(1 << PRTIM0);
	#endif

	#if (PIN11PWM==1) && USEPWM==1 && USEIO==1
	PRR0 &= ~(1 << PRTIM2);
	#endif

#endif

	if(TCNT==NULL)
		return;

	// If pre-emptive, don't configure timer 2
	#if OS_PREEMPTIVE == 1
		if(TCCRA==PIN11TCCRA)
			return;	
	#endif
	*TCNT=0;
	
	// Set to mode 01
	*TCCRA |=0b1;
	*TCCRA &= ~(0b10);
	
	// Sets mode 10 for pin OCxA or OCxB
	if(!compreg)
	{
		*TCCRA|=   0b10000000;
		*TCCRA &=~(0b01000000);
		*TIMSK|=0b10;		
	}
	else
	{
		*TCCRA|=  0b00100000;
		*TCCRA&=~(0b00010000);
		*TIMSK|=0b100;
	}	
	
	*DDR|=(1 << pin);
}

// value specified between 0 and 255 or 0 and 65535
void setDutyCycle(volatile uint8_t *OCR, int value, unsigned int *storevar)
{
	if(OCR==NULL)
		return;
		
	*OCR=value;
	*storevar=value;	
}

void startPWM(volatile uint8_t *TCCRB)
{
	if(TCCRB==NULL)
		return;
	
	#if OS_PREEMPTIVE==1
		if(TCCRB==PIN11TCCRB)
			return;
	#endif
	*TCCRB=0b00000011;
	sei();
}

#endif

#if ((PIN9PWM == 1) || (PIN10PWM == 1)) && USEPWM==1 && USEIO==1
	// compreg == 0 if you are using compare register A and 1 if using compare register B
	void setupPWM16(volatile uint16_t *TCNT, volatile uint8_t *TCCRA, volatile uint8_t *TIMSK, volatile uint8_t *DDR, uint8_t pinnum, uint8_t compreg)
	{
		if(TCNT==NULL)
			return;
	#if OSCPU_TYPE==AT168 || OSCPU_TYPE==AT328
		PRR &= ~(1 << PRTIM1);
	#elif OSCPU_TYPE==AT1280 || OSCPU_TYPE==AT2560
		PRR0 &= ~(1<<PRTIM1);
	#endif
	
		*TCNT=0;
		// 10 bit phase correct PWM
		if(!compreg)
		{
			*TCCRA=0b10000011;
			*TIMSK|=0b10;
		}			
		else
		{
			*TCCRA=0b00100011;
			*TIMSK|=0b100;
		}			
		
		*DDR|=(1 << pinnum);
	}
	
	// value specified between 0 and 255 or 0 and 65535. storevar = which variable to record PWM value in for the ISR
	void setDutyCycle16(volatile uint16_t *OCR, int value, unsigned int *storevar)
	{
		*OCR=value;
		*storevar=value;	
	}
	void startPWM16(volatile uint8_t *TCCRB)
	{
		*TCCRB=0b00000011;
		sei();
	}	

#endif

#if (USEARDPWM==1) && USEPWM==1 && USEIO==1

// Note: You must ensure that all the PIN?PWM defines for the pins you intend to use are set to 1
void analogWrite(int pinnum, int value)
{
		volatile uint8_t *TCCRA=NULL, *TCCRB=NULL, *TIMSK=NULL, *DDR=NULL;
		
		#if (PIN5PWM==1) || (PIN6PWM==1) || (PIN11PWM==1)
		volatile uint8_t *TCNT=NULL, *OCR=NULL;
		#endif
		
		#if (PIN9PWM==1) || (PIN10PWM==1)
		volatile uint16_t *TCNT16=NULL, *OCR16=NULL;
		#endif
		
		uint8_t CR=0, PIN=0;
		unsigned int *VVar=NULL;
		
		OSMakeAtomic();
		switch(pinnum)
		{
			#if PIN5PWM==1
			case 5: 
				TCNT=PIN5TCNT;
				TCCRA=PIN5TCCRA;
				TCCRB=PIN5TCCRB;
				TIMSK=PIN5MSK;
				OCR=PIN5OCR;
				DDR=PIN5DDR;
				PIN=PIN5PIN;
				CR=PIN5CR;
				VVar=PIN5SV;
				break;
			#endif
			
			#if PIN6PWM==1
			case 6: 
				TCNT=PIN6TCNT;
				TCCRA=PIN6TCCRA;
				TCCRB=PIN6TCCRB;
				TIMSK=PIN6MSK;
				DDR=PIN6DDR;
				PIN=PIN6PIN;
				OCR=PIN6OCR;
				CR=PIN6CR;
				VVar=PIN6SV;
				break;
			#endif

			#if PIN9PWM==1
			case 9: 
				TCNT16=PIN9TCNT;
				TCCRA=PIN9TCCRA;
				TCCRB=PIN9TCCRB;
				TIMSK=PIN9MSK;
				OCR16=PIN9OCR;
				DDR=PIN9DDR;
				PIN=PIN9PIN;
				CR=PIN9CR;
				VVar=PIN9SV;
				break;
			#endif
			
			#if PIN10PWM==1
			case 10: 
				TCNT16=PIN10TCNT;
				TCCRA=PIN10TCCRA;
				TCCRB=PIN10TCCRB;
				TIMSK=PIN10MSK;
				OCR16=PIN10OCR;
				DDR=PIN10DDR;
				PIN=PIN10PIN;
				CR=PIN10CR;
				VVar=PIN10SV;
				break;
			#endif
			
			#if PIN11PWM==1
			case 11: 
				TCNT=PIN11TCNT;
				TCCRA=PIN11TCCRA;
				TCCRB=PIN11TCCRB;
				TIMSK=PIN11MSK;
				DDR=PIN11DDR;
				PIN=PIN11PIN;
				OCR=PIN11OCR;
				CR=PIN11CR;
				VVar=PIN11SV;
				break;
			#endif
		}
		
		if(pinnum==9 || pinnum==10)
		{
			#if (PIN9PWM==1) || (PIN10PWM==1)
			setupPWM16(TCNT16, TCCRA, TIMSK, DDR, PIN, CR);
			setDutyCycle16(OCR16, value, VVar);
			startPWM16(TCCRB);
			#endif
		}
		else
		{
			#if (PIN5PWM==1) || (PIN6PWM==1) || (PIN11PWM==1)
			setupPWM(TCNT, TCCRA, TIMSK, DDR, PIN, CR);
			setDutyCycle(OCR, value, VVar);
			startPWM(TCCRB);			
			#endif
		}
		OSExitAtomic();
}
#endif
#endif

//#endif