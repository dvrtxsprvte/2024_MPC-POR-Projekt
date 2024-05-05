#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"

// UART se vsim moznym nastavenim
#define ASYNCHRONOUS (0<<UMSEL00) // USART Mode Selection

#define DISABLED    (0<<UPM00)
#define EVEN_PARITY (2<<UPM00)
#define ODD_PARITY  (3<<UPM00)
#define PARITY_MODE  DISABLED // USART Parity Bit Selection

#define ONE_BIT (0<<USBS0)
#define TWO_BIT (1<<USBS0)
#define STOP_BIT ONE_BIT      // USART Stop Bit Selection

#define FIVE_BIT  (0<<UCSZ00)
#define SIX_BIT   (1<<UCSZ00)
#define SEVEN_BIT (2<<UCSZ00)
#define EIGHT_BIT (3<<UCSZ00)
#define DATA_BIT   EIGHT_BIT  // USART Data Bit Selection

//^= XOR


void init_USART(uint16_t ubrr)
{
	// Nastaveni baudrate, chci baudrate 38400 -> 25
	UBRR0H = (uint8_t)(ubrr>>8);
	UBRR0L = (uint8_t)(ubrr);
	
	UCSR0C = ASYNCHRONOUS | PARITY_MODE | STOP_BIT | DATA_BIT;
	//UCSR0C = (0<<UMSEL00) | (0<<UPM00) | (0<<USBS0) | (3<<UCSZ00) // dalsi obdobna moznost
	
	// Povoleni TX a RX
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
}

// Sending frames w 5-8 data bit
void USART_transmit(unsigned char data)
{
	while (!(UCSR0A & (1<<UDRE0))); // cekani az bude transmit buffik prazdny
	
	UDR0 = data; // buchnut data na transmitor
}

unsigned char USART_recieve(void)
{
	while(!(UCSR0A & (1 << RXC0))); // cekani na data

	return UDR0; // vraceni dat
}

int USART_putchar(char c, FILE *stream)
{
	if (c == '\n')
	{
		USART_putchar('\r', stream);
	}
	
	while (!(UCSR0A & (1<<UDRE0)));

	UDR0 = c; // poslat znak
	
	return 0;
}

int USART_getchar(FILE *stream)
{
	while (!(UCSR0A & (1<<UDRE0)));
	
	return UDR0;
}

void USART_writeArray(const uint8_t *data, uint16_t length)
{
	for (uint16_t i = 0; i < length; i++)
	{
		USART_transmit(data[i]);
	}
}