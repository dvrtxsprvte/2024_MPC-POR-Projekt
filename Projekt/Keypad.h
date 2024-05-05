/*
 * Keypad.h
 *
 * Created: 19/03/2024 16:44:01
 *  Author: xprase08
 */ 

#pragma once

#define ROWS 4
#define COLS 4

void init_MatrixKeypad(void);

char MatrixKeypad_getkey(void);