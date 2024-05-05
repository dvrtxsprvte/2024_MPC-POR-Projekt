/*
 * ModbusRTU_master.c
 *
 * Created: 22/04/2024 23:17:47
 *  Author: simon
 */ 
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "ModbusRTU_master.h"
#include "uart.h"

#define SLAVE_ADD 0x01
#define NUM_HOLDING_REGISTERS 4
#define BUFFER_SIZE 8

volatile uint8_t rx_buffer[BUFFER_SIZE];
volatile uint8_t rx_pos = 0;
volatile uint8_t rx_count = 0;
volatile uint8_t rx_flag = 0;
volatile uint8_t rx_hold = 0;

extern uint16_t holdingRegisters[NUM_HOLDING_REGISTERS] = {0};

ISR(USART_RX_vect)
{	
	char received = UDR0;

	rx_buffer[rx_pos++] = received;

	if (rx_pos >= BUFFER_SIZE)
	{
		rx_pos = 0; 
	}
	
	++rx_count;
	
	if (rx_count == 8)
	{
		rx_flag = 1;
	}
	
	TCNT0 = 0;
		
}

ISR(TIMER0_COMPA_vect)
{
	if(rx_count != 0)
	{
		rx_hold = 1;
	}
	TCNT0 = 0;
}


void Modbus_initTimer()
{
	TCCR0A = (1 << WGM01); 
	TCCR0B = (1 << CS02) | (1 << CS00);  // Prescaler 1024
	OCR0A = 25;  
	TIMSK0 = (1 << OCIE0A);  
}

void Modbus_ReadHoldingRegisters(uint8_t *request)
{
	//MODBUS Application Protocol Specification V1.1b3
	// inspired by - Figure 13: Read Holding Registers state diagram, page 16/50
	
	// Request
	uint8_t device_addr = request[0];
	uint8_t fncCode = request[1];
	uint8_t start_Add_Hi = request[2];
	uint8_t start_Add_Lo = request[3];
	uint8_t reg_count_Hi = request[4];
	uint8_t reg_count_Lo = request[5];
	
	if (fncCode != MODBUS_READ_HOLDING_REGISTERS)
	{
		return Modbus_sendException(fncCode, MODBUS_ERROR_FUNCTION_NOT_SUPPORTED);
	}
	
	uint16_t firstReg = (start_Add_Hi << 8) | (start_Add_Lo & 0xFF);
	uint16_t regCount = ((reg_count_Hi << 8) | (reg_count_Lo & 0xFF));
	
	if (regCount < 1 || regCount > 125)
	{
		return Modbus_sendException(fncCode, MODBUS_ERROR_INVALID_VALUE);
	}
	
	if (firstReg >= NUM_HOLDING_REGISTERS || firstReg + regCount > NUM_HOLDING_REGISTERS) 
	{
		return Modbus_sendException(fncCode, MODBUS_ERROR_INVALID_ADDRESS);
	}
	
	// Response
	uint8_t response[256] = {0};
	uint8_t index = 0;
	
	response[index++] = device_addr;
	response[index++] = fncCode;
	response[index++] = regCount * 2;
	
	for (uint8_t i = firstReg; i < (firstReg + regCount); i++)
	{
		response[index++] = (holdingRegisters[i] >> 8) & 0xFF;  // Upper Byte
		response[index++] = holdingRegisters[i] & 0xFF;         // low Byte
	}
	
	uint16_t crc = CRC16(response, index);
	response[index++] = crc & 0xFF;
	response[index++] = (crc >> 8) & 0xFF;
	
	USART_writeArray(response, index);
}
	
void Modbus_WriteSingleRegister(uint8_t *request)
{
	//MODBUS Application Protocol Specification V1.1b3
	// inspired by - Figure 13: Read Holding Registers state diagram, page 20/50
	
	// Request
	uint8_t device_addr = request[0];
	uint8_t fncCode = request[1];
	uint8_t start_Add_Hi = request[2];
	uint8_t start_Add_Lo = request[3];
	uint8_t reg_value_Hi = request[4];
	uint8_t reg_value_Lo = request[5];
	
	if (fncCode != MODBUS_WRITE_SINGLE_REGISTER)
	{
		return Modbus_sendException(fncCode, MODBUS_ERROR_FUNCTION_NOT_SUPPORTED);
	}
	
	uint16_t firstReg = (start_Add_Hi << 8) | (start_Add_Lo & 0xFF);
	uint16_t regVal = ((reg_value_Hi << 8) | (reg_value_Lo & 0xFF));

	holdingRegisters[firstReg] = regVal;
	
	if (firstReg < 0 || firstReg >= 255) // Correctly should be 0xFFFF instead of 0x00FF 
	{
		return Modbus_sendException(fncCode, MODBUS_ERROR_INVALID_VALUE);
	}
	
	uint8_t response[8] = {0};
	uint8_t index = 0;
	
	if (holdingRegisters[firstReg] == regVal)
	{
		response[index++] = device_addr;
		response[index++] = fncCode;
		response[index++] = start_Add_Hi;
		response[index++] = start_Add_Lo;
		response[index++] = reg_value_Hi;
		response[index++] = reg_value_Lo;
	
		uint16_t crc = CRC16(response, index);
		response[index++] = crc & 0xFF;
		response[index++] = crc >> 8;
			
		USART_writeArray(response, index);	
	}
	else
	{
		return Modbus_sendException(fncCode,MODBUS_ERROR_FAILED_TO_EXECUTE_FUNCTION);
	}
}

extern volatile bool modbus_request_in_progress;

void Modbus_handleRequest()
{
	static uint8_t buffer[8];
	static uint8_t fncCode, slaveID;

	if (rx_flag != 0 && rx_hold == 1)
	{
		modbus_request_in_progress = true;
		for (int i = 0; i < 8; i++)
		{
			buffer[i] = rx_buffer[i];
		}

		uint16_t crc = CRC16(buffer, 6);

		if (crc != ((uint8_t)buffer[6] | ((uint8_t)buffer[7] << 8)))
		{
			Modbus_sendException(buffer[1], MODBUS_ERROR_INVALID_VALUE);
		}
		else
		{
			slaveID = buffer[0];
			fncCode = buffer[1];

			switch(fncCode)
			{
				case MODBUS_READ_HOLDING_REGISTERS:
				Modbus_ReadHoldingRegisters(buffer);
				break;

				case MODBUS_WRITE_SINGLE_REGISTER:
				Modbus_WriteSingleRegister(buffer);
				break;

				default:
				Modbus_sendException(slaveID, MODBUS_ERROR_FUNCTION_NOT_SUPPORTED);
				break;
			}
		}
		rx_pos = 0;
		rx_flag = 0;
		rx_hold = 0;
		rx_count = 0;
	}
}

void Modbus_sendException(uint8_t fncCode, uint8_t exceptionCode)
{
	uint16_t buffer[5];
	
	buffer[0] = SLAVE_ADD;
	buffer[1] = fncCode | 0x80; // set the highest bit to indicate an exception response
	buffer[2] = exceptionCode;
	
	uint16_t crc = CRC16(buffer,3);
	buffer[3] = crc & 0xFF;
	buffer[4] = (crc >> 8) & 0xFF;
	
	USART_writeArray(buffer,5);
}


uint16_t CRC16(uint8_t *data, uint8_t dataLength) 
{
	uint16_t crc = 0xFFFF;
	for (uint8_t i = 0; i < dataLength; i++) 
	{
		crc ^= data[i];
		for (uint8_t j = 0; j < 8; j++) 
		{
			if (crc & 0x0001)
			 {
				crc >>= 1;
				crc ^= 0xA001;
			} 
			else 
			{
				crc >>= 1;
			}
		}
	}
	return crc;
}


#define TEMPERATURE_REGISTER 0
#define UPPER_LIMIT_REGISTER 1
#define LOWER_LIMIT_REGISTER 2
#define STATUS_WORD_REGISTER 3

void init_HoldingRegs() 
{
	holdingRegisters[TEMPERATURE_REGISTER] = 0;
	holdingRegisters[UPPER_LIMIT_REGISTER] = 0;
	holdingRegisters[LOWER_LIMIT_REGISTER] = 0;
	holdingRegisters[STATUS_WORD_REGISTER] = 0;  // Clear status word initially
}

void Modbus_updateStatusWord(float temperature, float Hys_val_HI, float Hys_val_LO) 
{	
	Modbus_clearStatusBit(UPPER_LIMIT_BIT | LOWER_LIMIT_BIT | TEMP_VALID_BIT); //Clearing of Status word bits before change

	if (temperature > Hys_val_HI) 
	{
		Modbus_setStatusBit(UPPER_LIMIT_BIT);
	}
	else if (temperature < Hys_val_LO) 
	{
		Modbus_setStatusBit(LOWER_LIMIT_BIT);
	} 
	else 
	{
		Modbus_setStatusBit(TEMP_VALID_BIT);
	}
}

void Modbus_toggleWatchdog() 
{
	if (Modbus_checkStatusBit(WATCHDOG_BIT)) 
	{
		Modbus_clearStatusBit(WATCHDOG_BIT);
	} 
	else 
	{
		Modbus_setStatusBit(WATCHDOG_BIT);
	}
}

void Modbus_setStatusBit(uint8_t mask) 
{
	holdingRegisters[STATUS_WORD_REGISTER] |= mask;
}

void Modbus_clearStatusBit(uint8_t mask)
{
	holdingRegisters[STATUS_WORD_REGISTER] &= ~mask;
}

uint8_t Modbus_checkStatusBit(uint8_t mask)
{
	return (holdingRegisters[STATUS_WORD_REGISTER] & mask) != 0;
}

