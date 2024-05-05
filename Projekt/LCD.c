/*
 * LCD.c
 *
 * Created: 10/03/2024 13:22:35
 *  Author: xprase08
 */ 
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "timer.h"
#include "adc.h"

void init_LCD(void)
{
	// Power on delay - more than 40ms
	delay_u3(41000);

	// D4-D7 output
	DDRD |= (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7);

	//  RS a E output
	DDRB |= (1 << DDB0) | (1 << DDB1);

	// Clear PORTD
	PORTD &= 0;

	// Clear E and RS
	PORTB &= ~(1 << PINB1);
	PORTB &= ~(1 << PINB0);

	// Software reset
	LCD_write_4b(0x03);
	delay_u3(5000); // wait more than 4.1ms
	LCD_write_4b(0x03);
	delay_u3(110); // wait more than 100us
	LCD_write_4b(0x03);
	delay_u3(50);

	// Set 4-bit mode
	LCD_write_4b(0x02);
	delay_u3(110); // wait more than 100us
	LCD_send_command(0x28);
	delay_u3(50);

	// Display OFF
	LCD_send_command(0x08);
	delay_u3(50);

	// Clear display
	LCD_send_command(0x01);
	delay_u3(2000);

	// Entry mode
	LCD_send_command(0x06);
	delay_u3(50);

	// Display ON, Cursor OFF
	LCD_send_command(0x0C);
	delay_u3(50);	
}

void LCD_write_4b(unsigned char character)
{
	
	PORTD = (PORTD & 0x0F) | (character << 4);
	PORTB |= (1 << PINB1);                    //EN = 1
	delay_u3(10);
	PORTB &= ~(1 << PINB1);					 //EN = 0
	delay_u3(100);
}

void LCD_send_command(unsigned char command)
{
	PORTB &= ~(1 << PINB0);    //RS = 0
	LCD_write_4b(command >> 4);
	LCD_write_4b(command & 0x0F);
}

void LCD_send_data(unsigned char znak)
{
	PORTB |= (1 << PINB0);    //RS = 1
	LCD_write_4b(znak >> 4);
	LCD_write_4b(znak & 0x0F);
}

void LCD_clear(void)
{
	LCD_send_command(0x01);
	delay_u3(2000);
}

void LCD_home(void)
{
	LCD_send_command(0x02);
	delay_u3(2000);
}

void LCD_set_cursor(uint8_t row, uint8_t col)
{
	const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54}; // address of rows
	LCD_send_command(0x80 | (row_offsets[row] + col)); // set row and column address
	delay_u3(50);
}

void LCD_scroll_left(void)
{
	LCD_send_command(0x18);
	delay_u3(50);
}

void LCD_scroll_right(void)
{
	LCD_send_command(0x1C);
	delay_u3(50);
}

void LCD_shift_cursor_left(void)
{
	LCD_send_command(0x10);
	delay_u3(50);
}

void LCD_shift_cursor_right(void)
{
	LCD_send_command(0x14);
	delay_u3(50);
}

void LCD_shift_display_left(void)
{
	LCD_send_command(0x18);
	delay_u3(50);
}

void LCD_shift_display_right(void)
{
	LCD_send_command(0x1C);
	delay_u3(50);
}

void LCD_create_char(uint8_t loc, const uint8_t *charmap)
{
	loc &= 0x7; // limit character location to 8 positions
	LCD_send_command(0x40 | (loc << 3));
	for (int i = 0; i < 8; i++) {
		LCD_send_data(charmap[i]);
	}
}

void LCD_write_custom_char(uint8_t location)
{
	LCD_send_data(location);
}

void LCD_write_string(const char *str)
{
	while (*str) {
		LCD_send_data(*str++);
	}
}

int LCD_putchar(char c, FILE *stream)
{
	if (c == '\n') {
		} else {
		LCD_send_data(c);
	}
	return 0;
}

int LCD_ReadButton()
{
	uint16_t result_adc0 = ADC_measure(0); 
	
	int ret_value = 0;
	
	if (result_adc0 >= 0 && result_adc0 < 50) ret_value =  5; // RIGHT - 0
	
	else if (result_adc0 > 75 && result_adc0 < 150) ret_value = 3; // UP - 99
	
	else if (result_adc0 > 200 && result_adc0 < 300) ret_value = 4; // DOWN - 255
	
	else if (result_adc0 > 350 && result_adc0 < 500) ret_value = 2; //LEFT - 409
	
	else if (result_adc0 > 550 && result_adc0 < 750) ret_value = 1; // SELECT - 640
	
	else ret_value = 0;
	
	while(result_adc0 < 750)
	{
		delay_u3(20000);
		result_adc0 = ADC_measure(0);	
	}
	
	return ret_value;
}

void LCD_rotate_display_right()
{
	int i = 0;
	while (i < 16)
	{
		LCD_shift_display_right();
		i++;
	}
	
}

void LCD_rotate_display_left()
{
	int i = 0;
	while (i < 16)
	{
		LCD_shift_display_left();
		i++;
	}
	
}

void LCD_clear_row(int row) 
{
	LCD_set_cursor(row, 0);

	for (int col = 0; col < 15; col++)
		LCD_send_data(' ');
}



