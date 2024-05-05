/*
 * Encoder.c
 *
 * Created: 17/03/2024 18:00:46
 *  Author: xprase08
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "timer.h"

void init_Encoder()
{
	// PD2, PD3 a PB3 jako vstup
	DDRD &= ~((1 << PORTD2) | (1 << PORTD3));
	DDRB &= ~(1 << PORTB3);
	
	// Povoleni pullup rezistoru
	PORTD |= (1 << PORTD2) | (1 << PORTD3);
	PORTB |= (1 << PORTB3);

	// Povoleni PCINTP0-7 a PCINT16-23 interupt
	PCICR |= (1 << PCIE0) | (1 << PCIE2);
	
	// Povoleni interrupt pD2 a PD3
	PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
	
	// Povoleni interrupt PB3
	PCMSK0 |= (1 << PCINT3);
	
}

volatile int encoder_counter = 0;
volatile int btn_counter = 0;
volatile int btn_counter2 = 0;

ISR(PCINT0_vect)
{
	PCICR &= ~(1 << PCIE0);
	
	if (!(PINB & (1 << PORTB3)))
	{
		btn_counter++;
		btn_counter2++;
	}
	
	PCICR |= (1 << PCIE0);
}


ISR(PCINT2_vect)
{
	PCICR &= ~(1 << PCIE2);
	
	static int A_prev = 0;
	static int B_prev = 0;
	
	int A_current = (PIND & (1 << PORTD2)) >> PORTD2;
	int B_current = (PIND & (1 << PORTD3)) >> PORTD3;
	
	if (A_prev == 0 && A_current == 1)
	{
		if (B_current == 1)
		{
			encoder_counter++;
		}
	}
	
	if (B_prev == 0 && B_current == 1)
	{
		if (A_current == 1)
		{
			encoder_counter--;
		}
	}	
	A_prev = A_current;
	B_prev = B_current;
	
	PCICR |= (1 << PCIE2);
}



int get_btn_count()
{
	return btn_counter;
}

int get_encoder_count()
{
	return encoder_counter;
}

void reset_btn_count()
{
	btn_counter = 0;
}

void reset_enc_count() 
{
	encoder_counter = 0;
}

int get_btn2_count()
{
	return btn_counter2;
}

void reset_btn2_count()
{
	btn_counter2 = 0;
}