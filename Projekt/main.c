/*
 * Projekt.c
 *
 * Created: 03/04/2024 12:52:59
 * Author : simon
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>

#include "timer.h"
#include "EEPROM.h"
#include "I2C.h"
#include "LCD.h"
#include "uart.h"
#include "RTC.h"
#include "adc.h"
#include "Encoder.h"
#include "Relay.h"
#include "ModbusRTU_master.h"

static FILE UART_Stream = FDEV_SETUP_STREAM(USART_putchar, NULL, _FDEV_SETUP_WRITE);
static FILE LCD_Stream = FDEV_SETUP_STREAM(LCD_putchar, NULL, _FDEV_SETUP_WRITE);

#define EEPROM_TIME_ADDR 0x00

#define MENU_ITEMS 5
#define TIME_DATE_ITEMS 6
#define HYS_ITEMS 2
#define CALI_ITEMS 2
#define MAIN_ITEMS 2
#define MODBUS_MENU_ITEMS 3
#define MODBUS_HILO_ITEMS 2


const char* menu[MENU_ITEMS] =
{
	"1. Temperature",
	"2. Hysteresis",
	"3. Time & Date",
	"4. Calibration",
	"5. Modbus RTU"
};
const char* Time_Date_menu[TIME_DATE_ITEMS] =
{
	"Seconds",
	"Minutes",
	"Hours",
	"Date",
	"Month",
	"Year"
};
const char* Hys_menu[HYS_ITEMS] =
{
	"Hi value",
	"Lo value"
};
const char* Cali_menu[CALI_ITEMS] =
{
	"B coef",
	"ADC value"
};

const char* Modbus_menu[MODBUS_MENU_ITEMS] =
{
	"Modbus RTU",
	"HI limit",
	"LO limit"
};

const char* Modbus_HILO[MODBUS_HILO_ITEMS] =
{
	"HIGH limit",
	"LOW limit"
};

uint8_t button = 0;
uint8_t signal_state = 0;

uint8_t current_Main_item = 0;
uint8_t current_menu_item = 0;
uint8_t current_hysMenu_item = 0;
uint8_t current_timeMenu_item = 0;
uint8_t current_caliMenu_item = 0;
uint8_t current_ModbusMenu_item = 0;
uint8_t currecnt_ModbusHILO_item = 0;

char degree = 0b11011111;

static uint8_t in_mainDisplay = 0;
static uint8_t in_menu = 0;
static uint8_t in_tempMenu = 0;
static uint8_t in_timeMenu = 0;
static uint8_t in_TimeSubmenu = 0;
static uint8_t in_hysMenu = 0;
static uint8_t in_hysSubmenu = 0;
static uint8_t in_caliMenu = 0;
static uint8_t in_caliSubMenu = 0;
static uint8_t in_ModbusSetmenu = 0;
static uint8_t in_modbusHILOmenu = 0;
static uint8_t in_ModbusMenu = 0;

uint8_t hours = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;
uint8_t day = 0;
uint8_t date = 0;
uint8_t month = 0;
uint8_t year = 0;
uint8_t data[32] = {0};
uint8_t temp_set = 0;

float B = 3895.0;   

static float temperature = 0;
static float W_temperature = 20; 
static float Hys_val_HI = 0;
static float Hys_val_LO = 0;
static float Hys_val = 0;

uint8_t value = 0;
uint8_t value_set = 0;
uint8_t EEPROM_save = 0;
uint8_t W_temp_whole = 0;
uint8_t W_temp_frac = 0;
uint8_t Hys_HI_whole = 0;
uint8_t Hys_HI_frac = 0;
uint8_t Hys_LO_whole = 0;
uint8_t Hys_LO_frac = 0;

uint8_t remote = 0;
uint8_t init_remote = 0;
uint8_t mod_yes = 0;
uint8_t mod_no = 0;
uint8_t modbus_set_yes = 0;
uint8_t modbus_set_no = 0;

uint8_t HI_limit = 0;
uint8_t LO_limit = 0;

uint8_t first_frame = 1;
volatile bool modbus_request_in_progress = false;
		
void init_ALL()
{
	init_LCD();
	init_I2C(100000);
	init_USART(25);
	init_Encoder();
	init_ADC();
	init_Relay();
	init_HoldingRegs();
	RTC_SetSquareWaveOutput(1);
	Modbus_initTimer();
	sei();
}

				
int main(void)
{
	init_ALL();
	in_mainDisplay = 1;

	EEPROM_read(EEPROM_TIME_ADDR, &data, 32);
	seconds = data[0];
	minutes = data[1];
	hours = data[2];
	date = data[3];
	month = data[4];
	year = data[5];
	
	W_temp_whole = data[6];
	W_temp_frac = data[7];
	
	Hys_HI_whole = data[8];
	Hys_HI_frac = data[9];
	
	Hys_LO_whole = data[10];
	Hys_LO_frac = data[11];
	
	HI_limit = data[12];
	LO_limit = data[13];
	
	W_temperature = W_temp_whole + (W_temp_frac / 100.0);
	Hys_val_HI = Hys_HI_whole + (Hys_HI_frac / 100.0);
	Hys_val_LO = Hys_LO_whole + (Hys_LO_frac / 100.0);
	
	RTC_SetDateTime(seconds, minutes, hours, day, date, month, year);
	
	while (1)
	{
		if (in_mainDisplay)
		{
				button = LCD_ReadButton();
				switch (button)
				{
					case 2: //LEFT
					current_Main_item = 0;
					break;

					case 4: //DOWN
					in_mainDisplay = 0;
					in_menu = 1;
					break;

					case 5: //RIGHT
					current_Main_item = 1;
					break;
			
				break;
				}
		
			signal_state = get_btn_count(); // Display value refresh 1s - RTC_SetSquareWaveOutput

			if (signal_state >= 1)
			{
				LCD_home();
				LCD_clear();
				LCD_set_cursor(0, 0);

				if (current_Main_item == 0)
				{
					temperature = get_Temperature(1,B);
					fprintf(&LCD_Stream, "Akt.T: %.2f%cC",temperature,degree);

					LCD_set_cursor(1,0);
					fprintf(&LCD_Stream, "Poz.T: %.2f%cC",W_temperature,degree);
				}
				else if (current_Main_item == 1)
				{
					RTC_GetTime(&hours, &minutes, &seconds);
					fprintf(&LCD_Stream, "Cas: %02d:%02d:%02d", hours, minutes, seconds);

					LCD_set_cursor(1,0);
					RTC_GetDate_ALL(&day, &date, &month, &year);
					fprintf(&LCD_Stream, "Datum: %02d.%02d.%02d", date, month, year);
				}
				reset_btn_count(); 
			}
		}

		else if (in_menu)
		{
			signal_state = get_btn_count(); 

			if(signal_state >= 1)
			{
				LCD_clear();
				LCD_home();

				LCD_set_cursor(0,0);
				fprintf(&LCD_Stream,"Menu");

				LCD_set_cursor(1,0);
				fprintf(&LCD_Stream,"%s", menu[current_menu_item]);

				reset_btn_count();
			}
			
			button = LCD_ReadButton();
			switch (button)
			{
				case 1: //SELECT
				break;

				case 2: //LEFT
				Menu_move_left();
				break;

				case 3: //UP
				in_menu = 0;
				current_Main_item = 0;
				in_mainDisplay = 1;
				break;

				case 4: //DOWN
				switch (current_menu_item)
				{
					case 0: // Temperature
					in_menu = 0;
					in_tempMenu = 1;
					break;

					case 1: // Hysteresis
					in_menu = 0;
					in_hysMenu = 1;
					break;

					case 2: // Time & Date
					in_menu = 0;
					in_timeMenu = 1;
					break;

					case 3: // Calibration
					in_menu = 0;
					in_caliMenu = 1;
					break;
					
					case 4:
					in_menu = 0;
					in_ModbusMenu = 1;
				}
				break;

				case 5: //RIGHT
				Menu_move_right();
				break;
			}
		}

		else if (in_tempMenu)
		{
			signal_state = get_btn_count();
			
			if (signal_state >= 1)
			{
				LCD_clear();
				LCD_home();

				LCD_set_cursor(0,0);
				fprintf(&LCD_Stream, "Set new T");

				LCD_set_cursor(1,0);
				fprintf(&LCD_Stream, "Teplota: %.2f", W_temperature);

				reset_btn_count();
			}

			button = LCD_ReadButton();
			switch(button)
			{
				case 1: // SELECT
				temp_set = 1;
				break;

				case 2: // LEFT
				W_temperature -= 0.5;
				break;

				case 3: //UP
				if (temp_set == 1)
				{
					in_tempMenu = 0;
					in_menu = 1;
					temp_set = 0;
				}
				break;

				case 5: // RIGHT
				W_temperature += 0.5;
				break;
			}
		}

		else if(in_hysMenu)
		{
			signal_state = get_btn_count();

			if (signal_state >= 1)
			{
				LCD_clear();
				LCD_home();

				LCD_set_cursor(0,0);
				fprintf(&LCD_Stream,"%s", Hys_menu[current_hysMenu_item]);
				LCD_set_cursor(1,0);
				if (current_hysMenu_item == 0)
				{
				fprintf(&LCD_Stream,"%.2f", Hys_val_HI);			
				}
				if (current_hysMenu_item == 1)
				{
				fprintf(&LCD_Stream,"%.2f", Hys_val_LO);				
				}
				reset_btn_count();
			}
			
			button = LCD_ReadButton();
			switch(button)
			{
				case 1: // SELECT
				break;

				case 2: // LEFT
				Menu_move_left();
				break;

				case 3: //UP
				in_hysMenu = 0;
				in_menu = 1;
				break;

				case 4: // DOWN
				in_hysMenu = 0;
				in_hysSubmenu = 1;
				break;

				case 5: // RIGHT
				Menu_move_right();
				break;
			}
		}
		else if (in_hysSubmenu)
		{
			signal_state = get_btn_count();

			if (signal_state >= 1)
			{
				LCD_clear_row(0);

				LCD_set_cursor(0,0);
				fprintf(&LCD_Stream,"Set %s", Hys_menu[current_hysMenu_item]);
			}
			if (signal_state > 0)
			{
				LCD_clear_row(1);
				LCD_set_cursor(1,0);
				
				fprintf(&LCD_Stream,"%.2f", Hys_val);
			}
			reset_btn_count();
			
			button = LCD_ReadButton();
			switch(button)
			{
				case 1: // SELECT
				if (current_hysMenu_item == 0)
				{
					Hys_val_HI = Hys_val;
					value_set = 1;
				}
				else if (current_hysMenu_item == 1)
				{
					Hys_val_LO = Hys_val;
					value_set = 1;
				}
				break;
				
				case 2: // LEFT
				if (Hys_val > 0)
				{
					Hys_val -= 0.5;
				}
				break;

				case 3: // UP
				if (value_set == 1)
				{
				in_hysSubmenu = 0;
				in_hysMenu = 1;
				value_set = 0;
				Hys_val = 0;	
				}
				break;

				case 5: // RIGHT
				Hys_val += 0.5;
				break;
			}
		}
		
		else if(in_timeMenu)
		{
			signal_state = get_btn_count();
			if (signal_state >= 1)
			{
				LCD_clear();
				LCD_home();

				LCD_set_cursor(0,0);
				fprintf(&LCD_Stream,"%s", Time_Date_menu[current_timeMenu_item]);

				LCD_set_cursor(1,0);
				if (current_timeMenu_item == 0) // Seconds
				{
					seconds = RTC_GetSeconds();
					fprintf(&LCD_Stream,"%02d", seconds);
				}
				else if (current_timeMenu_item == 1) // Minutes
				{
					minutes = RTC_GetMinutes();
					fprintf(&LCD_Stream,"%02d", minutes);
				}
				else if (current_timeMenu_item == 2) // Hours
				{
					hours = RTC_GetHours();
					fprintf(&LCD_Stream,"%02d", hours);
				}
				else if (current_timeMenu_item == 3) // Date
				{
					date = RTC_GetDate();
					fprintf(&LCD_Stream,"%02d", date);
				}
				else if (current_timeMenu_item == 4) // Month
				{
					month = RTC_GetMonth();
					fprintf(&LCD_Stream,"%02d", month);
				}
				else if (current_timeMenu_item == 5) // Year
				{
					year = RTC_GetYear();
					fprintf(&LCD_Stream,"%02d", year);
				}

				reset_btn_count();
			}

			button = LCD_ReadButton();
			switch(button)
			{
				case 2:
				Menu_move_left();
				break;

				case 3:
				in_timeMenu = 0;
				in_menu = 1;
				break;

				case 4:
				in_timeMenu = 0;
				in_TimeSubmenu = 1;
				break;

				case 5:
				Menu_move_right();
				break;
			}
		}

		else if(in_TimeSubmenu)
		{
			
			signal_state = get_btn_count();
			if (signal_state >= 1)
			{
				LCD_clear_row(0);
				LCD_set_cursor(0, 0);
				
				fprintf(&LCD_Stream,"Set %s", Time_Date_menu[current_timeMenu_item]);
			}
			
			if (signal_state > 0)
			{
				LCD_clear_row(1);
				LCD_set_cursor(1,0);
				
				fprintf(&LCD_Stream,"%02d", value);
			}
			
			reset_btn_count();

			button = LCD_ReadButton();
			switch(button)
			{
				case 1:
				switch (current_timeMenu_item)
				{
					case 0: // Seconds
					if (value >= 0 && value < 60)
					RTC_SetSeconds(value);
					value_set = 1;
					break;

					case 1: // Minutes
					if (value >= 0 && value < 60)
					RTC_SetMinutes(value);
					value_set = 1;
					break;

					case 2: // Hours
					if (value >= 0 && value < 24)
					RTC_SetHours(value);
					value_set = 1;
					break;

					case 3: // Date
					if (value >= 1 && value <= 31)
					RTC_SetDate(value);
					value_set = 1;
					break;

					case 4: // Month
					if (value >= 1 && value <= 12)
					RTC_SetMonth(value);
					value_set = 1;
					break;

					case 5: // Year
					if (value >= 0 && value <= 99)
					RTC_SetYear(value);
					value_set = 1;
					break;
				}

				break;

				case 2: // LEFT
				if (value > 0)
				{
					value--;
				}
				break;

				case 3: // UP
				if (value_set)
				{
				in_TimeSubmenu = 0;
				in_timeMenu = 1;
				value_set = 0;
				value = 0; 	
				}
				break;

				case 5: // RIGHT
				switch (current_timeMenu_item)
				{
					case 0: // Seconds
					if (value < 59)
					value++;
					break;

					case 1: // Minutes
					if (value < 59)
					value++;
					break;

					case 2: // Hours
					if (value < 23)
					value++;
					break;

					case 3: // Date
					if (value >= 0 && value < 31)
					value++;
					break;

					case 4: // Month
					if (value >= 0 && value < 12)
					value++;
					break;

					case 5: // Year
					if (value < 99)
					value++;
					break;
				}
				break;
			}
		}
		else if (in_caliMenu)
		{
			signal_state = get_btn_count();

			if (signal_state >= 1)
			{
				LCD_clear();
				LCD_home();
				
				LCD_set_cursor(0,0);
				fprintf(&LCD_Stream,"%s", Cali_menu[current_caliMenu_item]);
				
				LCD_set_cursor(1,0);
				if (current_caliMenu_item == 0)
				{
					fprintf(&LCD_Stream,"%.2f", B);
				}
				if (current_caliMenu_item == 1)
				{
					uint16_t ADC_val = ADC_measure(1);
					fprintf(&LCD_Stream,"%d", ADC_val);
				}
				
				reset_btn_count();
			}
			
			
			button = LCD_ReadButton();
			switch(button)
			{				
				case 2:
				Menu_move_left();
				break;

				case 3:
				in_caliMenu = 0;
				in_menu = 1;
				break;
				
				case 4:
				if (current_caliMenu_item == 0)
				{
					in_caliMenu = 0;
					in_caliSubMenu = 1;
				}
				break;
				
				case 5:
				Menu_move_right();
				break;
			}
		}
		else if (in_caliSubMenu)
		{
			signal_state = get_btn_count();

			if (signal_state >= 1)
			{
				LCD_clear_row(0);

				LCD_set_cursor(0,0);
				fprintf(&LCD_Stream,"Set %s", Cali_menu[current_caliMenu_item]);
			}
					
			if (signal_state > 0)
			{
				LCD_clear_row(1);
				LCD_set_cursor(1,0);
						
				fprintf(&LCD_Stream,"%.2f", B);
			}
			reset_btn_count();
					
			button = LCD_ReadButton();
			switch(button)
			{
				case 1:
				value_set = 1;	
				break;
						
				case 2:
				B -= 1;
				break;

				case 3:
				if (value_set)
				{
					in_caliSubMenu = 0;
					in_caliMenu = 1;
					value_set = 0;
				}
					
				break;

				case 5:
				B += 1;
				break;
			}	
		}
		else if (in_ModbusMenu)
		{
			signal_state = get_btn_count();
			if (signal_state >= 1)
			{
				LCD_clear();
				LCD_home();

				LCD_set_cursor(0,0);
				fprintf(&LCD_Stream, "%s", Modbus_menu[current_ModbusMenu_item]);
				
				LCD_set_cursor(1,0);
				if (current_ModbusMenu_item == 0)
				{
					if (mod_yes)
					{
					fprintf(&LCD_Stream, "YES");	
					}
					else
					{
					fprintf(&LCD_Stream, "NO");	
					}
				}	
				if (current_ModbusMenu_item == 1)
				{
					fprintf(&LCD_Stream,"%d", HI_limit);
				}
				if (current_ModbusMenu_item == 2)
				{
					fprintf(&LCD_Stream,"%d", LO_limit);
				}
				reset_btn_count();	
			}
			 button = LCD_ReadButton();
			 switch (button)
			 {
				case 2: // LEFT
				Menu_move_left();
				break;

				case 3: // UP
				in_ModbusMenu = 0;
				in_menu = 1;
				break;
				
				case 4: // DOWN
				switch (current_ModbusMenu_item) 
				{
					case 0: // Modbus Set Menu
					in_ModbusSetmenu = 1;
					in_ModbusMenu = 0;
					break;
					case 1: // Modbus HILO Menu
					in_modbusHILOmenu = 1;
					in_ModbusMenu = 0;
					currecnt_ModbusHILO_item = 0;
					break;
					case 2:
					in_modbusHILOmenu = 1;
					in_ModbusMenu = 0;
					currecnt_ModbusHILO_item = 1;
					break;
				}
				break;
		
				 case 5: // RIGHT
				 Menu_move_right();
				 break;
			 }	
		}
		else if (in_ModbusSetmenu)
		{
			signal_state = get_btn_count();
			if (signal_state >= 1)
			{
				LCD_clear();
				LCD_home();

				LCD_set_cursor(0,0);
				fprintf(&LCD_Stream, "Modbus RTU on?");

				LCD_set_cursor(1,0);
				if (mod_no == 1)
				{
				fprintf(&LCD_Stream, "NO");
				}
				else
				{
				fprintf(&LCD_Stream, "YES");
				}
				reset_btn_count();
			}			
			button = LCD_ReadButton();
			switch (button)
			{
				case 1: //SELECT
				if (mod_yes)
				{
					modbus_set_yes = 1;
					value_set = 1;
				} 
				else
				{
					modbus_set_no = 1;	
					value_set = 1;	
				}				
				break;
				
				case 2: //LEFT
				mod_yes = 1;
				mod_no = 0;
				
				case 3: //UP
				if(value_set)
				{
				in_ModbusSetmenu = 0;
				in_ModbusMenu = 1;
				value_set = 0;	
				}
				break;

				case 5: 
				mod_yes = 0;
				mod_no = 1;
				break;
			}
		}
		else if (in_modbusHILOmenu)
		{
			signal_state = get_btn_count();

			if (signal_state >= 1)
			{
				LCD_clear_row(0);

				LCD_set_cursor(0,0);
				fprintf(&LCD_Stream,"Set %s", Modbus_HILO[currecnt_ModbusHILO_item]);
			}
			if (signal_state > 0)
			{
				LCD_clear_row(1);
				LCD_set_cursor(1,0);
		
				fprintf(&LCD_Stream,"%d", value);
			}
			reset_btn_count();
	
			button = LCD_ReadButton();
			switch(button)
			{
				case 1: // SELECT
				if (currecnt_ModbusHILO_item == 0)
				{
					HI_limit = value;
					value_set = 1;
				}
				else if (currecnt_ModbusHILO_item == 1)
				{
					LO_limit = value;
					value_set = 1;
				}
				break;
		
				case 2: // LEFT
				if (value > 0)
				{
					value -= 1;
				}
				break;

				case 3: // UP
				if (value_set == 1)
				{
					in_modbusHILOmenu = 0;
					in_ModbusMenu = 1;
					value_set = 0;
					value = 0;
				}
				break;

				case 5: // RIGHT
				value += 1;
				break;
			}
		}
		
		if (modbus_set_yes)
		{
			UCSR0B |= (1 << RXCIE0);
			
			temperature = get_Temperature(1,3895);
			holdingRegisters[TEMPERATURE_REGISTER] = (uint16_t)(temperature);
						
			holdingRegisters[UPPER_LIMIT_REGISTER] = HI_limit;
			holdingRegisters[LOWER_LIMIT_REGISTER]= LO_limit;
	
			Modbus_handleRequest();
			
			Modbus_updateStatusWord(temperature,HI_limit,LO_limit);
			
			Modbus_toggleWatchdog();

		}
		
		if(modbus_set_no)
		{
			UCSR0B &= ~(1 << RXCIE0);
			modbus_set_no = 0;
		}
		
		
		if (!(PINB & (1<<PINB7)))
		{
			remote = 1;
			init_remote = 1;
		}
		
		else if(remote == 1)
		{
			LCD_home();
			LCD_clear();
			LCD_set_cursor(0, 0);
			fprintf(&LCD_Stream,"Remote Control");

			
			if (init_remote == 1)
			{
				fprintf(&UART_Stream,"Help:\n");
				fprintf(&UART_Stream,"q - Zobrazeni aktualni teploty, pozadovane teploty, datumu a casu\n");
				fprintf(&UART_Stream,"w - Nastaveni pozadovane teploty\n");
				fprintf(&UART_Stream,"e - Nastaveni casu a datumu\n");
				fprintf(&UART_Stream,"r - Zobrazeni hodnoty koeficientu B a ADC value\n");
				fprintf(&UART_Stream,"s - Nastaveni nove hodnoty koeficientu B \n");
				fprintf(&UART_Stream,"x - Ukonceni remote control\n");
				fprintf(&UART_Stream,"h - Zobrazeni help\n");
				init_remote = 0;
			}
			
			char c = USART_recieve();
			if (c == 104) // h
			{
				fprintf(&UART_Stream,"Help:\n");
				fprintf(&UART_Stream,"q - Zobrazeni aktualni teploty, pozadovane teploty, datumu a casu\n");
				fprintf(&UART_Stream,"w - Nastaveni pozadovane teploty\n");
				fprintf(&UART_Stream,"e - Nastaveni casu a datumu\n");
				fprintf(&UART_Stream,"r - Zobrazeni hodnoty koeficientu B a ADC value\n");
				fprintf(&UART_Stream,"s - Nastaveni nove hodnoty koeficientu B\n");
				fprintf(&UART_Stream,"x - Ukonceni remote control\n");
				fprintf(&UART_Stream,"h - Zobrazeni help\n");
			}
			else if(c == 113) // q
			{
				fprintf(&UART_Stream,"Akt.T: %.2f C\n",temperature);
				fprintf(&UART_Stream,"Poz.T: %.2f C\n",W_temperature);
				fprintf(&UART_Stream,"Datum: %02d.%02d.%02d\n", date, month, year);
				fprintf(&UART_Stream,"Cas: %02d:%02d:%02d\n", hours, minutes, seconds);
			}
			
			else if (c == 119) // w
			{
				char input[5];
				float new_temp;
				uint8_t i = 0;

				fprintf(&UART_Stream,"Nastav pozadovanou teplotu:\n");

				i = 0;
				while(i < 4)
				{
					char c = USART_recieve();
					if (c == '\n')
					{
						break;
					}
					
					input[i] = c;
					fprintf(&UART_Stream, "%c", input[i]);
					i++;
				}
				int temp = ((int)input[0] - '0')*10 + ((int)input[1] - '0');
				float temp_dec = ((int)input[3] - '0');
				W_temperature = (float)temp + temp_dec/10;
				fprintf(&UART_Stream,"\n Nova pozadovana teplota:  %0.2f\n",W_temperature);
			}
			
			else if (c == 101) //e
			{
				fprintf(&UART_Stream,"Nastav sekundy (ss):\n");
				char seconds_input[3];
				uint8_t i = 0;
				while(i < 2)
				{
					char c = USART_recieve();
					if (c == '\n')
					{
						break;
					}
					seconds_input[i] = c;
					i++;
					fprintf(&UART_Stream,"%c",seconds_input[i]);
				}
				seconds = (seconds_input[0]-'0')*10 + (seconds_input[1]-'0');

				fprintf(&UART_Stream,"Nastav minuty (mm):\n");
				char minutes_input[3];
				i = 0;
				while(i < 2)
				{
					char c = USART_recieve();
					if (c == '\n')
					{
						break;
					}
					minutes_input[i] = c;
					fprintf(&UART_Stream,"%c",minutes_input[i]);

					i++;
				}
				minutes = (minutes_input[0]-'0')*10 + (minutes_input[1]-'0');

				fprintf(&UART_Stream,"Nastav hodiny (hh):\n");
				char hours_input[3];
				i = 0;
				while(i < 2)
				{
					char c = USART_recieve();
					if (c == '\n')
					{
						break;
					}
					hours_input[i] = c;
					fprintf(&UART_Stream,"%c",hours_input[i]);
					i++;
				}
				hours = (hours_input[0]-'0')*10 + (hours_input[1]-'0');

				fprintf(&UART_Stream,"Nastav den (dd):\n");
				char date_input[3];
				i = 0;
				while(i < 2)
				{
					char c = USART_recieve();
					if (c == '\n')
					{
						break;
					}
					date_input[i] = c;
					fprintf(&UART_Stream,"%c",date_input[i]);
					i++;
				}
				date = (date_input[0]-'0')*10 + (date_input[1]-'0');

				fprintf(&UART_Stream,"Nastav mesic (mm):\n");
				char month_input[3];
				i = 0;
				while(i < 2)
				{
					char c = USART_recieve();
					if (c == '\n')
					{
						break;
					}
					month_input[i] = c;
					fprintf(&UART_Stream,"%c",month_input[i]);
					i++;
				}
				month = (month_input[0]-'0')*10 + (month_input[1]-'0');

				fprintf(&UART_Stream,"Nastav rok (yy):\n");
				char year_input[3];
				i = 0;
				while(i < 2)
				{
					char c = USART_recieve();
					if (c == '\n')
					{
						break;
					}
					year_input[i] = c;
					fprintf(&UART_Stream,"%c",year_input[i]);
					i++;
				}
				year = (year_input[0]-'0')*10 + (year_input[1]-'0');

				RTC_SetDateTime(seconds,minutes,hours,day,date,month,year);
			}
			
			else if (c == 114) // r
			{
				fprintf(&UART_Stream,"Hodnota koeficientu B: %.1f\n", B);
				fprintf(&UART_Stream,"Hodnota ADC value: %d\n", ADC_measure(1));
			}
			
			else if (c == 115) // s
			{
				fprintf(&UART_Stream,"Nastav novou hodnotu koeficientu B (xxxx.x):\n");
				char B_input[7];
				uint8_t i = 0;
				while(i < 6)
				{
					char c = USART_recieve();
					if (c == '\n')
					{
						break;
					}
					B_input[i] = c;
					fprintf(&UART_Stream,"%c",B_input[i]);
					i++;
				}
				int B_temp = ((int)B_input[0] - '0')*1000 + ((int)B_input[1] - '0')*100 + ((int)B_input[2] - '0')*10 + ((int)B_input[3] - '0');
				float B_dec = ((int)B_input - '0');
				B = B_temp + B_dec/10;
				fprintf(&UART_Stream,"\n Nove nastavena hodnota B: %.02f\n", B);
			}
			
			else if (c == 120) // x
			{
				remote = 0;
				in_mainDisplay = 1;
			}
			
		}
			
		// Ulozeni na EEPROM, co cca 5 sekund
		EEPROM_save = get_btn2_count();
		if (EEPROM_save > 10)
		{
			RTC_GetTime(&hours, &minutes, &seconds);
			data[0] = seconds;
			data[1] = minutes;
			data[2] = hours;
			RTC_GetDate_ALL(&day, &date, &month, &year);
			data[3] = date;
			data[4] = month;
			data[5] = year;
			
			W_temp_whole = (uint8_t)W_temperature;
			W_temp_frac = (uint8_t)((W_temperature - W_temp_whole) * 100);
			
			data[6] = W_temp_whole;
			data[7] = W_temp_frac;
			
			Hys_HI_whole = (uint8_t)Hys_val_HI;
			Hys_HI_frac = (uint8_t)((Hys_val_HI - Hys_HI_whole) * 100);
			
			data[8] = Hys_HI_whole;
			data[9] = Hys_HI_frac;
			
			Hys_LO_whole = (uint8_t)Hys_val_LO;
			Hys_LO_frac = (uint8_t)((Hys_val_LO - Hys_LO_whole) * 100);
			
			data[10] = Hys_LO_whole;
			data[11] = Hys_LO_frac;
			
			data[12] = HI_limit;
			data[13] = LO_limit;
			
			EEPROM_write(EEPROM_TIME_ADDR, &data, sizeof(data));

			reset_btn2_count();
		}
		

		Temp_Regulation(temperature,W_temperature,Hys_val_HI,Hys_val_LO);

	}
	return 0;
}

void Menu_move_right()
{
	if (in_menu)
	{
		current_menu_item = (current_menu_item + 1) % MENU_ITEMS;
	}
	else if (in_hysMenu)
	{
		current_hysMenu_item = (current_hysMenu_item + 1) % HYS_ITEMS;
	}
	else if (in_timeMenu || in_TimeSubmenu)
	{
		current_timeMenu_item = (current_timeMenu_item + 1) % TIME_DATE_ITEMS;
	}
	else if (in_caliMenu)
	{
		current_caliMenu_item = (current_caliMenu_item + 1) % CALI_ITEMS;
	}
	else if (in_ModbusMenu)
	{
		current_ModbusMenu_item = (current_ModbusMenu_item + 1) % MODBUS_MENU_ITEMS;
	}
	else if (in_modbusHILOmenu)
	{
		currecnt_ModbusHILO_item = (currecnt_ModbusHILO_item + 1) % MODBUS_HILO_ITEMS;
	}
}

void Menu_move_left()
{
	if (in_menu)
	{
		current_menu_item = (current_menu_item - 1 + MENU_ITEMS) % MENU_ITEMS;
	}
	else if (in_hysMenu)
	{
		current_hysMenu_item = (current_hysMenu_item - 1 + HYS_ITEMS) % HYS_ITEMS;
	}
	else if (in_timeMenu || in_TimeSubmenu)
	{
		current_timeMenu_item = (current_timeMenu_item - 1 + TIME_DATE_ITEMS) % TIME_DATE_ITEMS;
	}
	else if (in_caliMenu)
	{
		current_caliMenu_item = (current_caliMenu_item - 1 + CALI_ITEMS) % CALI_ITEMS;
	}
	else if (in_ModbusMenu)
	{
		current_ModbusMenu_item = (current_ModbusMenu_item - 1 + MODBUS_MENU_ITEMS) % MODBUS_MENU_ITEMS;
	}
	else if (in_modbusHILOmenu)
	{
		currecnt_ModbusHILO_item = (currecnt_ModbusHILO_item - 1 + MODBUS_HILO_ITEMS) % MODBUS_HILO_ITEMS;
	}
	
}



