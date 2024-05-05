/*
 * I2C.h
 *
 * Created: 25/03/2024 21:16:20
 *  Author: simon
 */

#pragma once

#define I2C_TIMEOUT 1600
#define F_CPU 16000000UL

#define I2C_START      0x08
#define I2C_RESTART    0x10

#define I2C_ADDR_ACK   0x18
#define I2C_ADDR_NACK  0x20

#define I2C_DATA_ACK   0x28
#define I2C_DATA_NACK  0x30

#define I2C_R_ADDR_ACK  0x40
#define I2C_R_ADDR_NACK 0x48

#define I2C_R_DATA_ACK  0x50
#define I2C_R_DATA_NACK 0x58

#define I2C_ERROR      0x38
#define I2C_NONE       0xF8

enum
{
	I2C_OK,
	I2C_ERROR_START,
	I2C_ERROR_RSTART,
	I2C_ERROR_ADDR_WRITE,
	I2C_ERROR_DATA_WRITE,
	I2C_ERROR_ADDR_READ,
	I2C_ERROR_DATA_READ,
};

void init_I2C(uint32_t speed);

void I2C_write(uint8_t addr,uint8_t reg,uint8_t *data,uint16_t len);

void I2C_read(uint8_t addr,uint8_t reg,uint8_t *data,uint16_t len);


uint8_t twi_start(void);

void twi_stop(void);

uint8_t twi_restart(void);

uint8_t twi_addr_write_ack(void);

uint8_t twi_data_write_ack(void);

uint8_t twi_addr_read_ack(void);

uint8_t twi_data_read_ack(uint8_t ack);












