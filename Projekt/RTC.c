#include <avr/io.h>
#include <avr/interrupt.h>

#include "I2C.h"
#include "uart.h"

#define DS1307_ADDR 0x68 // Adresa I2C pro DS1307

// Prevod z DEC na BCD
uint8_t dec_to_bcd(uint8_t val)
{
	return ((val / 10 * 16) + (val % 10));
}

// Prevod z BCD na DEC
uint8_t bcd_to_dec(uint8_t val)
{
	return ((val / 16 * 10) + (val % 16));
}

void RTC_SetDateTime(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t date, uint8_t month, uint8_t year)
{
	uint8_t data[7] = {dec_to_bcd(seconds), dec_to_bcd(minutes), dec_to_bcd(hours), dec_to_bcd(day), dec_to_bcd(date), dec_to_bcd(month), dec_to_bcd(year)};
	I2C_write(DS1307_ADDR, 0x00, data, 7);
}

void RTC_GetTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{
	uint8_t data[3];
	I2C_read(DS1307_ADDR, 0x00, data, 3);
	//fprintf(&UART_Stream,"h %d m %d s %d\n", bcd_to_dec(data[2]), bcd_to_dec(data[1]), bcd_to_dec(data[0]));
	
	*seconds = bcd_to_dec(data[0]);
	*minutes = bcd_to_dec(data[1]);
	*hours = bcd_to_dec(data[2]);

}

void RTC_GetDate_ALL(uint8_t *day, uint8_t *date, uint8_t *month, uint8_t *year)
{
	uint8_t data[4];
	I2C_read(DS1307_ADDR, 0x03, data, 4);
	*day = bcd_to_dec(data[0]);
	*date = bcd_to_dec(data[1]);
	*month = bcd_to_dec(data[2]);
	*year = bcd_to_dec(data[3]);
}

void RTC_SetSquareWaveOutput(uint16_t freq)
{
	uint8_t control_reg = 0x10; // Enable square wave output
	switch(freq)
	{
		case 1: control_reg |= 0x00; break; // 1Hz
		case 4096: control_reg |= 0x01; break; // 4.096kHz
		case 8192: control_reg |= 0x02; break; // 8.192kHz
		case 32768: control_reg |= 0x03; break; // 32.768kHz
		default: control_reg = 0x00; break; // Disable square wave output
	}
	I2C_write(DS1307_ADDR, 0x07, &control_reg, 1);
}

uint8_t char_to_uint8(char c)
{
	if (c >= '0' && c <= '9')
	{
		return (uint8_t)(c - '0');
	}
	else
	{
		return 0;
	}
}

void RTC_SetSeconds(uint8_t seconds)
{
	uint8_t data = dec_to_bcd(seconds);
	I2C_write(DS1307_ADDR, 0x00, &data, 1);
}

void RTC_SetMinutes(uint8_t minutes)
{
	uint8_t data = dec_to_bcd(minutes);
	I2C_write(DS1307_ADDR, 0x01, &data, 1);
}

void RTC_SetHours(uint8_t hours)
{
	uint8_t data = dec_to_bcd(hours);
	I2C_write(DS1307_ADDR, 0x02, &data, 1);
}

void RTC_SetDay(uint8_t day)
{
	uint8_t data = dec_to_bcd(day);
	I2C_write(DS1307_ADDR, 0x03, &data, 1);
}

void RTC_SetDate(uint8_t date)
{
	uint8_t data = dec_to_bcd(date);
	I2C_write(DS1307_ADDR, 0x04, &data, 1);
}

void RTC_SetMonth(uint8_t month)
{
	uint8_t data = dec_to_bcd(month);
	I2C_write(DS1307_ADDR, 0x05, &data, 1);
}

void RTC_SetYear(uint8_t year)
{
	uint8_t data = dec_to_bcd(year);
	I2C_write(DS1307_ADDR, 0x06, &data, 1);
}

uint8_t RTC_GetSeconds()
{
	uint8_t data;
	I2C_read(DS1307_ADDR, 0x00, &data, 1);
	return bcd_to_dec(data);
}

uint8_t RTC_GetMinutes()
{
	uint8_t data;
	I2C_read(DS1307_ADDR, 0x01, &data, 1);
	return bcd_to_dec(data);
}

uint8_t RTC_GetHours()
{
	uint8_t data;
	I2C_read(DS1307_ADDR, 0x02, &data, 1);
	return bcd_to_dec(data);
}

uint8_t RTC_GetDay()
{
	uint8_t data;
	I2C_read(DS1307_ADDR, 0x03, &data, 1);
	return bcd_to_dec(data);
}

uint8_t RTC_GetDate()
{
	uint8_t data;
	I2C_read(DS1307_ADDR, 0x04, &data, 1);
	return bcd_to_dec(data);
}

uint8_t RTC_GetMonth()
{
	uint8_t data;
	I2C_read(DS1307_ADDR, 0x05, &data, 1);
	return bcd_to_dec(data);
}

uint8_t RTC_GetYear()
{
	uint8_t data;
	I2C_read(DS1307_ADDR, 0x06, &data, 1);
	return bcd_to_dec(data);
}
