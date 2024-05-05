/*
 * Relay.h
 *
 * Created: 03/04/2024 13:28:09
 *  Author: simon
 */ 

#pragma once

void init_Relay();

void Temp_Regulation(float temperature, float W_temperature, float Hys_val_HI, float Hys_val_LO);