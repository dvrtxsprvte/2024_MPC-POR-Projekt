/*
 * Keypad.c
 *
 * Created: 19/03/2024 16:43:16
 *  Author: xprase08
 */ 

#include <avr/io.h>
#include "keypad.h"
#include "timer.h"
#include "uart.h"

// cela klavesnica
char keys[ROWS][COLS] = {
	{'1', '2', '3', 'A'},
	{'4', '5', '6', 'B'},
	{'7', '8', '9', 'C'},
	{'*', '0', '#', 'D'}
};

void init_MatrixKeypad(void) 
{
	// ROwS vystup + pullup
	DDRD |= (1 << PIND2) | (1 << PIND3);
	PORTD |= (1 << PIND2) | (1 << PIND3);
	
	DDRB |= (1 << PINB3) | (1 << PINB4);
	PORTB |= (1 << PINB3) | (1 << PINB4);
	
	// COLS vstup + pullup
	DDRC &= ~((1 << PINC1) | (1 << PINC2) | (1 << PINC3));
	PORTC |= (1 << PINC1) | (1 << PINC2) | (1 << PINC3);
}

char MatrixKeypad_getkey(void) 
{
	char key = '\0';
	
	for (int i = 0; i < ROWS; i++)
	{
		if(i < 2)
		{
			PORTD &= ~(1 << (PIND2 + i)); // ROW -> LOW
		}
		if (i >= 2)
		{
			PORTB &= ~(1 << (PINB3 + i - 2)); // ROW -> LOW
		}
		delay_u3(15000);
		
		for (int j = 0; j < COLS - 1; j++)
		{
			if (!(PINC & (1 << (PINC1 + j)))) // Pokud je tlacitko zmacknute
			{
				key = keys[i][j];
				while(!(PINC & (1 << (PINC1 + j))))//rozepnuti tlacitka
				{
					delay_u3(15000); // Osetreni zakmitu
				}
				break; // vyskoceni z foru
			}
		}
					
		if(i < 2)
		{
			PORTD |= (1 << (PIND2 + i)); 
		}
		if (i >= 2)
		{
			PORTB |= (1 << (PINB3 + i - 2)); 
		}

		delay_u3(15000);
		
		if (key != '\0')  // pokud bylo tlacitko stiskle -> break
		{
			break;
		}
	}
	return key;
}





