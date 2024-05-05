/*
 * Relay.c
 *
 * Created: 03/04/2024 13:27:18
 *  Author: simon
 */ 

#include <avr/io.h>
#include "Relay.h"
#include "Encoder.h"

void init_Relay()
{
	DDRD |= (1 << PORTD2); // Topeni
	DDRD |= (1 << PORTD3); // Chlazeni
}

static uint8_t heating = 0;
static uint8_t cooling = 0;

void Temp_Regulation(float temperature, float W_temperature, float Hys_val_HI, float Hys_val_LO)
{
     if (heating && cooling)
     {
	     heating = 0;
	     cooling = 0;
	     PORTD |= (1 << PORTD2) | (1 << PORTD3);
     }
	 
	else if (cooling && temperature > (W_temperature + Hys_val_HI)) // Chlazeni
    {
		PORTD |= (1 << PORTD2);
        PORTD &= ~(1 << PORTD3);
    }
	
    else if (heating && temperature < (W_temperature - Hys_val_LO)) // Topeni
    {
		PORTD &= ~(1 << PORTD2);
        PORTD |= (1 << PORTD3);
    }
	
	else if (temperature < W_temperature)
    {
        heating = 1;
    }
	
    else if (temperature > W_temperature)
    {
        cooling = 1;
    }
 	
}
