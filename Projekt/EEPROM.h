/*
 * EEPROM.h
 *
 * Created: 27/03/2024 08:13:27
 *  Author: simon
 */ 

#pragma once

uint8_t EEPROM_read(uint16_t addr, uint8_t *data, uint8_t len);

uint8_t EEPROM_write(uint16_t addr, uint8_t *data, uint8_t len);