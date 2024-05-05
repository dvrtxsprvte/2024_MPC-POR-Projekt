/*
 * timer.c
 *
 * Created: 26.02.2024 18:29
 * Author : xprase08
 */ 

#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>

#include "timer.h"

static volatile uint16_t timer1overflowCount = 0; 

void busyDelay(uint32_t us)
{
	us -= 2;
	_NOP();
	_NOP();
	_NOP();
	_NOP();
	_NOP();
	_NOP();
	_NOP();

	for (; us != 0; --us)
	{
		_NOP();
		_NOP();
		_NOP();
		_NOP();
		_NOP();
		_NOP();
	}
}

void delay(uint32_t us)
{
	TCNT1 = 0; 
	TIFR1 = (1<<TOV1) | (1<<OCF1A); 
	TCCR1B |= 0b00000010;
	
	us = (us<<1); 
	uint16_t overflowCount = (uint16_t)(us>>16); 
	OCR1A = (uint16_t)(0xffff & us);

	while (overflowCount != 0)
	{
		while((TIFR1 & (1<<TOV1)) == 0)
		{
		}
		--overflowCount;
		TIFR1 = (1<<TOV1) | (1<<OCF1A);
	}
	while((TIFR1 & (1<<OCF1A)) == 0)
	{
	}
	TCCR1B = 0;
}

void init_Timer()
{
	TCNT1 = 0;
	TIFR1 = (1<<TOV1) | (1<<OCF1A);
	TCCR1B |= 0b00000010;
	TIMSK1 = 1;
	
	sei();
}

uint32_t getTime()
{
	uint32_t t = ((uint32_t)(timer1overflowCount)<<16)+(uint32_t)(TCNT1);
	t = (t>>1);
	return t;
}

ISR(TIMER1_OVF_vect) 
{
	++timer1overflowCount;
}

void delay_u3(uint32_t us)
{
	init_Timer();
	uint32_t timeDifference = getTime();
	
	while(getTime()<(us+timeDifference))
	{}
	
	TCNT1 = 0;
	TIFR1 = (1<<TOV1) | (1<<OCF1A);
	TIMSK1 = 0;

}