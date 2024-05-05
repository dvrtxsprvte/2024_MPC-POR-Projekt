/*
 * ModbusRTU_master.h
 *
 * Created: 22/04/2024 23:21:02
 *  Author: simon
 */ 

#pragma  once

#define MODBUS_ERROR_FUNCTION_NOT_SUPPORTED 0x01
#define MODBUS_ERROR_INVALID_ADDRESS 0x02
#define MODBUS_ERROR_INVALID_VALUE 0x03
#define MODBUS_ERROR_FAILED_TO_EXECUTE_FUNCTION 0x04
#define MODBUS_ERROR_SLAVE_PROCESSING_REQUEST 0x05
#define MODBUS_ERROR_SLAVE_BUSY 0x06
#define MODBUS_ERROR_FUNCTION_CANNOT_BE_EXECUTED 0x07
#define MODBUS_ERROR_PARITY_ERROR_WHEN_READING_MEMORY 0x08

#define MODBUS_READ_COILS 0x01
#define MODBUS_READ_DISCRETE_INPUTS 0x02
#define MODBUS_READ_HOLDING_REGISTERS 0x03
#define MODBUS_READ_INPUT_REGISTERS 0x04
#define MODBUS_WRITE_SINGLE_COIL 0x05
#define MODBUS_WRITE_SINGLE_REGISTER 0x06
#define MODBUS_READ_EXCEPTION_STATUS 0x07
#define MODBUS_WRITE_MULTIPLE_REGISTERS 0x10

#define TEMPERATURE_REGISTER 0
#define UPPER_LIMIT_REGISTER 1
#define LOWER_LIMIT_REGISTER 2
#define STATUS_WORD_REGISTER 3

#define WATCHDOG_BIT       (1 << 0)
#define TEMP_VALID_BIT     (1 << 1)
#define UPPER_LIMIT_BIT    (1 << 2)
#define LOWER_LIMIT_BIT    (1 << 3)

#define NUM_HOLDING_REGISTERS 4

extern uint16_t holdingRegisters[NUM_HOLDING_REGISTERS];

uint16_t CRC16(uint8_t *data, uint8_t dataLength); 

void Modbus_sendException(uint8_t fncCode, uint8_t exceptionCode);

void Modbus_ReadHoldingRegisters(uint8_t *request);

void Modbus_WriteSingleRegister(uint8_t *request);

void Modbus_handleRequest();

uint8_t Modbus_checkStatusBit(uint8_t mask);

void Modbus_clearStatusBit(uint8_t mask);

void Modbus_setStatusBit(uint8_t mask);

void Modbus_toggleWatchdog(); 

void Modbus_updateStatusWord(float temperature, float Hys_val_HI, float Hys_val_LO);

void init_HoldingRegs(); 

void Modbus_initTimer();