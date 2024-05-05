/*
 * LDC.h
 *
 * Created: 10/03/2024 13:23:13
 *  Author: simon
 */ 

#pragma once

void init_LCD(void);

void LCD_write_4b(unsigned char character);

void LCD_send_command(unsigned char command);

void LCD_send_data(unsigned char znak);

void LCD_clear(void);

void LCD_home(void);

void LCD_set_cursor(uint8_t row, uint8_t col);

void LCD_scroll_left(void);

void LCD_scroll_right(void);

void LCD_shift_cursor_left(void);

void LCD_shift_cursor_right(void);

void LCD_shift_display_left(void);

void LCD_shift_display_right(void);

void LCD_create_char(uint8_t loc, const uint8_t *charmap);

void LCD_write_custom_char(uint8_t location);

void LCD_write_string(const char *str);

int LCD_putchar(char c, FILE *stream);

int LCD_ReadButton();

void LCD_rotate_display_right();

void LCD_rotate_display_left();

void LCD_clear_row(int row);