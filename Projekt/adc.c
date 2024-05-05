/*
 * cv4.c
 *
 * Created: 28.02.2024 8:20:59
 * Author : xprase08
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "adc.h"


void init_ADC(void)
{
	ADMUX |= (1 << REFS0);									// ref U na AVCC
	ADMUX &= ~(MUX3 | MUX2 | MUX1 | MUX0);					// vynulovani ADC
	ADCSRA |= (1 << ADEN);									// povoleni ADC
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);	// nastaveni preddelicky na 128 (125 kHz) - termistor zvyseni cistlivosti
}

	
uint16_t ADC_res;

uint16_t ADC_measure(uint8_t ADC_pin)
{
	// Nastavení ADMUX pro zvolený kanál ADC
	ADMUX &= ~(MUX3 | MUX2 | MUX1 | MUX0);
	ADMUX |= ADC_pin;
	
	ADCSRA |= (1 << ADSC);	//spustebi prevodu ADC
	
	while (ADCSRA & (1 << ADSC)); // dokonceni prevodu ADC
	{
		ADC_res = ADC;
	}
	
	return ADC_res;
}

const float T0 = 298.15;  // teplota 25 stupnu C v kelvinech
const float R0 = 10000.0; // odpor termistoru pri teplote T0
const float R3 = 1500.0; // odpor R3

float get_Temperature(uint8_t adc_channel, float B)
{
	int adc_value =  ADC_measure(adc_channel); // hodnota z AD prevodniku
	float voltage = adc_value * 5.0 / 1024.0; // prepocet na napeti
	
	float Rth = voltage/(5.0 - voltage) * 20000.0 - R3; // vypocet odporu termistoru
	float T = B/(log(Rth / R0) + B/T0); // vypocet velikost teploty v kelvinech
	
	return T - 273.15; // prevod na stupne C
	
}
/* https://startingelectronics.org/articles/atmel-AVR-8-bit/print-float-atmel-studio-7/	jak printovat float v atmelu*/


