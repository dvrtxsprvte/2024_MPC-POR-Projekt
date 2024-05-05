/*
 * RTC.h
 *
 * Created: 25/03/2024 21:55:25
 *  Author: simon
 */ 

#pragma once

// SDA PC4
// SCL PC5

#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <avr/interrupt.h>

void RTC_SetDateTime(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t date, uint8_t month, uint8_t year);

void RTC_GetTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds);

void RTC_GetDate_ALL(uint8_t *day, uint8_t *date, uint8_t *month, uint8_t *year);

void RTC_SetSquareWaveOutput(uint16_t freq);

uint8_t char_to_uint8(char c);

void RTC_SetSeconds(uint8_t seconds);

void RTC_SetMinutes(uint8_t minutes);

void RTC_SetHours(uint8_t hours);

void RTC_SetDay(uint8_t day);

void RTC_SetDate(uint8_t date);

void RTC_SetMonth(uint8_t month);

void RTC_SetYear(uint8_t year);

uint8_t RTC_GetSeconds();

uint8_t RTC_GetMinutes();

uint8_t RTC_GetHours();

uint8_t RTC_GetDay();

uint8_t RTC_GetDate();

uint8_t RTC_GetMonth();

uint8_t RTC_GetYear();