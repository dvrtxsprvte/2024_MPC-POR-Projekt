#pragma once


void init_ADC(void);

uint16_t ADC_measure(uint8_t ADC_pin);

float get_Temperature(uint8_t adc_channel, float B);
