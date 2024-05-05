/*
 * I2C.c
 *
 * Created: 25/03/2024 21:15:43
 *  Author: simon
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "I2C.h"

void init_I2C(uint32_t speed)
{
	uint32_t clock_speed = 0;
	clock_speed = (((F_CPU/speed) - 16) / 2) & 0xFF; //& 0xFF aby to byl uint8
	TWBR = clock_speed & 0xFF;
	TWCR = (1 << TWEN);
	
	PORTC |= (1 << PORTC4 | 1 << PORTC5); // pullup port C4 a C5
}


 uint8_t I2C_start(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	uint16_t i = 0;
	while (!(TWCR & (1 << TWINT)));
	{
		i++;
		if(i >= I2C_TIMEOUT)
		{
			return I2C_ERROR_START;
		}
	}
	
	if ((TWSR & 0xF8) != I2C_START)
	{
		return I2C_ERROR_START;
	}
	
	return I2C_OK;
}
 void I2C_stop(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

 uint8_t I2C_restart(void)
{
	uint16_t i = 0;
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	
	while (!(TWCR & (1 << TWINT)));
	{
		i++;
		if(i >= I2C_TIMEOUT){
			return I2C_ERROR_RSTART;
		}
	}
	
	if ((TWSR & 0xF8) != I2C_RESTART)
	{
		return I2C_ERROR_RSTART;
	}
	return I2C_OK;
}

 uint8_t I2C_addr_write_ack(void)
{
	uint16_t i = 0;
	TWCR = (1 << TWINT) | (1 << TWEN);
	
	while (!(TWCR & (1 << TWINT)));
	{
		i++;
		if(i >= I2C_TIMEOUT)
		{
			return I2C_ERROR_ADDR_WRITE;
		}
	}
	
	if ((TWSR & 0xF8) != I2C_ADDR_ACK)
	{
		return I2C_ERROR_ADDR_WRITE;
	}
	
	return I2C_OK;
}


 uint8_t I2C_data_write_ack(void)
{
	uint16_t i = 0;
	
	TWCR = (1 << TWINT) | (1 << TWEN);
	
	while (!(TWCR & (1 << TWINT)));
	{
		i++;
		if(i >= I2C_TIMEOUT)
		{
			return I2C_ERROR_DATA_WRITE;
		}
	}
	if ((TWSR & 0xF8) != I2C_DATA_ACK)
	{
		return I2C_ERROR_DATA_WRITE;
	}
	return I2C_OK;
}

 uint8_t I2C_addr_read_ack(void)
{
	uint16_t i = 0;
	
	TWCR = (1 << TWINT) | (1 << TWEN);
	
	while (!(TWCR & (1 << TWINT)));
	{
		i++;
		if(i >= I2C_TIMEOUT)
		{
			return I2C_ERROR_ADDR_READ;
		}
	}
	if ((TWSR & 0xF8) != I2C_R_ADDR_ACK)
	{
		return I2C_ERROR_ADDR_READ;
	}
	return I2C_OK;
}

 uint8_t I2C_data_read_ack(uint8_t ack)
{
	uint16_t i = 0;
	if(ack != 0)
	{
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
		
		while (!(TWCR & (1 << TWINT)));
		{
			i++;
			if(i >= I2C_TIMEOUT)
			{
				return I2C_ERROR_DATA_READ;
			}
		}
	}
	else
	{
		TWCR = (1 << TWINT) | (1 << TWEN);
		
		while (!(TWCR & (1 << TWINT)));
		{
			i++;
			if(i >= I2C_TIMEOUT)
			{
				return I2C_ERROR_DATA_READ;
			}
		}
	}
	
	if ((TWSR & 0xF8) != I2C_R_DATA_ACK || (TWSR & 0xF8) != I2C_R_DATA_NACK)
	{
		return I2C_ERROR_DATA_READ;
	}
	
	return I2C_OK;
}

void I2C_read(uint8_t addr,uint8_t reg,uint8_t *data,uint16_t len)
{
	
	uint16_t i = 0;
	I2C_start();
	
	TWDR = (addr << 1) | 0;
	I2C_addr_write_ack();
	
	TWDR = reg;
	I2C_data_write_ack();
	
	I2C_restart();
	
	TWDR = (addr << 1) | 1;
	I2C_addr_read_ack();
	
	for(i = 0 ; i < len ; i++)
	{
		I2C_data_read_ack(1);
		data[i] = TWDR;
	}
	
	I2C_data_read_ack(0);
	
	I2C_stop();
}


void I2C_write(uint8_t addr,uint8_t reg,uint8_t *data,uint16_t len)
{
	
	uint16_t i = 0;
	
	I2C_start();

	TWDR = (addr << 1) | 0;
	I2C_addr_write_ack();
	
	TWDR = reg;
	I2C_data_write_ack();
	
	for(i = 0; i < len; i++)
	{
		TWDR = data[i];
		I2C_data_write_ack();
	}
	
	I2C_stop();
}

