/*
 * EEPROM.c
 *
 * Created: 27/03/2024 08:13:17
 *  Author: simon
 */ 
#include <avr/io.h>
#include <avr/cpufunc.h>
#include <stdio.h>

#include "I2C.h"
#include "timer.h"
#include "EEPROM.h"

#define EEPROM_ADDR 0x50


uint8_t EEPROM_read(uint16_t addr, uint8_t *data, uint8_t len)
{
	if (len > 32)
		return 1;

	uint8_t addr_high_byte = (addr >> 8) & 0x1F; //MSB
	uint8_t addr_low_byte = addr & 0xFF; //LSB
	
	I2C_start();
	
	TWDR = (EEPROM_ADDR << 1) | 0;
	I2C_addr_write_ack();

	TWDR = addr_high_byte;
	I2C_data_write_ack();

	TWDR = addr_low_byte;
	I2C_data_write_ack();

	I2C_restart();

	TWDR = (EEPROM_ADDR << 1) | 1; // | 1 -> READ
	I2C_addr_read_ack();

	
	for (uint8_t i = 0; i < len; i++)
	{
		I2C_data_read_ack(1);
		data[i] = TWDR;
	}
	
	I2C_data_read_ack(0);

	I2C_stop();
	
	return 0;
}

uint8_t EEPROM_write(uint16_t addr, uint8_t *data, uint8_t len)
{
	if (len > 32)
		return I2C_ERROR_START;

	uint8_t addr_high_byte = (addr >> 8) & 0x1F;
	uint8_t addr_low_byte = addr & 0xFF;

	I2C_start();
	
	TWDR = (EEPROM_ADDR << 1) | 0; // | 0 -> WRITE
	I2C_addr_write_ack();

	
	TWDR = addr_high_byte;
	I2C_data_write_ack();

	
	TWDR = addr_low_byte;
	I2C_data_write_ack();

	for (uint8_t i = 0; i < len; i++)
	{
		TWDR = data[i];
		I2C_data_write_ack();
	}

	I2C_stop();
	
	return I2C_OK;
}