
#pragma once
#include <stdio.h>


void init_USART(uint16_t ubrr);

void USART_transmit(unsigned char data);

unsigned char USART_recieve(void);

int USART_putchar(char c, FILE *stream);

int USART_getchar(FILE *stream);

void USART_writeArray(const uint8_t *data, uint16_t length);
