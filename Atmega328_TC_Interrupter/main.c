/*
 * Atmega328_TC_Interrupter.c
 *
 * Created: 5/15/2019 11:11:43 AM
 * Author : Lucas Paiva da Silva 
 */ 

#include "defs.h"

float PW_mult = 1.5;
float PW_mult_limit = 3.0;
int	  Pw_mult_to_display = 1;
unsigned char PW_multStr[4];

int debouncePB3 = 0;
int debouncePB4 = 0;
int debouncePB5 = 0;
int PB3Flag = 0;
int PB4Flag = 1;
int PB5Flag = 0;


int StateSelection = 0;
int DisplaySelectionPosition = 3;

//Menu Variables
unsigned char MenuChar[] = {0x20, 'M', 'I', 'D', 'I', 0x20, 0x20, 0x20, 'F', 'i', 'x', 'e', 'd', 0x20, 0x20, 0x20,
						    0x20, 'N', 'o', 'n', 'e', 0x20, 0x20, 0x20, 'S' , 'e' , 't', 't', 'i', 'n', 'g', 's'};
unsigned char MenuSelectionBar[] = {0, 7, 16, 23};

//MIDI Variables
unsigned char MIDIChar[] = {'E', '5', '-', '4', '4', '0', 'H', 'z', 0x20, 0x20, 'P', 'W', ':', '1', '.', '5',
							0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20};
unsigned char MIDISelectionBar[] = {0, 7, 16, 23};
	
//Fixed Variables
unsigned char FixedChar[] = {0x20, '4', '4', '0', 'H', 'z', 0x20, 0x20, 0x20, 'P', 'W', ':', '1', '.', '5', 0x20,
							0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 , 0x20 , 0x20, 0x20, 0x20, 0x20, 0x20, 0x20};
unsigned char FixedSelectionBar[] = {0, 7, 16, 23};

//None
unsigned char NoneChar[] = {0x20, 'N', 'o', 'n', 'e', 0x20, 0x20, 0x20, 'N', 'o', 'n', 'e', 'N', 'o', 'n', 'e',
							0x20, 'N', 'o', 'n', 'e', 0x20, 0x20, 0x20, 'N', 'o', 'n', 'e', 'N', 'o', 'n', 'e'};
unsigned char NoneSelectionBar[] = {0, 7, 16, 23};
	
//Settings
unsigned char SettingsChar[] = {0x20, 'P' , 'W' , '_' , 'l' , 'i' , 'm' , 'i' , 't', ':', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
								0x20, 'V', 'e', 'r', 's', 'i', 'o', 'n',0x20, '0', '.', '3', '0', 0x20, 0x20, 0x20};
unsigned char SettingsSelectionBar[] = {0, 7, 16, 23};

void InitMessage();
void ChangePWLimit(int operation);
void RefreshDisplay(unsigned char DisplayChar[]);
void ModifyDisplay(unsigned char DisplayChar[], unsigned char DisplaySelectionBar[]);
void ConvertBars(unsigned char DisplayChar[], float PW, float PWMax);
ISR(PCINT0_vect);




int main(void)
{
    
	DDRD = 0xFF; 
	DDRB  = 0x00;	
	PORTB = 0xFF;
	
	//interrupção dos bots
	PCICR = 1<<PCIE0;
	PCMSK0 = (1<<PCINT3) | (1<<PCINT4) | (1<<PCINT5);
	sei();
	
	inic_LCD_4bits();
	InitMessage();
	ModifyDisplay(MenuChar, MenuSelectionBar);
	
    while (1) 
    {
		
		switch (StateSelection)
		{
			case 0:
			ModifyDisplay(MenuChar, MenuSelectionBar);
			break;

			case 1:
			ModifyDisplay(MIDIChar, MIDISelectionBar);
			break;
			
			case 2:
			ModifyDisplay(FixedChar, FixedSelectionBar);
			break;
			
			case 3:
			ModifyDisplay(NoneChar, NoneSelectionBar);
			break;
			
			case 4:
			ModifyDisplay(SettingsChar, SettingsSelectionBar);
			break;
		}
		_delay_ms(500);
		debouncePB3 = 0;
		debouncePB4 = 0;
		debouncePB5 = 0;
    }
}

void ConvertBars(unsigned char DisplayChar[], float PW, float PWMax)
{
	int Nbars = 0, x = 0;
	for (x=0; x<16; x++)
	{
		DisplayChar[16+x] = 0x20;
	}
	Nbars = (16*PW)/PWMax;
	for (x=0; x<Nbars; x++)
	{
		DisplayChar[16+x] = 0xFF;
	}
		
}


void InitMessage()
{
	cmd_LCD(0x80, 0);
	escreve_LCD("Paiva's TC");
	cmd_LCD(0xC0, 0);
	escreve_LCD("328P Interrupter");
	_delay_ms(300);
	cmd_LCD(1, 0);
	cmd_LCD(0x80, 0);
	escreve_LCD("Version 0.1");
	_delay_ms(100);
	cmd_LCD(1, 0);
};

void ChangePWLimit(int operation)
{
	if ((operation == 1)&&(PW_mult_limit<4)){PW_mult_limit = PW_mult_limit + 0.1;}
	if ((operation == 0)&&(PW_mult_limit>0.5)){PW_mult_limit = PW_mult_limit - 0.1;}
	Pw_mult_to_display = (PW_mult_limit * 10);
	ident_num(Pw_mult_to_display, PW_multStr);
	SettingsChar[11] = PW_multStr[1];
	SettingsChar[12] = '.';
	SettingsChar[13] = PW_multStr[0];
}

void RefreshDisplay(unsigned char DisplayChar[])
{
	int x;
	cmd_LCD(1, 0);
	cmd_LCD(0x80, 0);
	for (x=0; x<16;x++)
	{
		cmd_LCD(DisplayChar[x], 1);
	}
	cmd_LCD(0xC0, 0);
	for (x=16; x<32;x++)
	{
		cmd_LCD(DisplayChar[x], 1);
	}
}

void ModifyDisplay(unsigned char DisplayChar[], unsigned char DisplaySelectionBar[])
{
	if ((PB4Flag == 1))
	{
		PB4Flag = 0;
		switch (StateSelection)
		{
			case 0:
			DisplayChar[DisplaySelectionBar[DisplaySelectionPosition]] = 0x20;
			DisplaySelectionPosition++;
			if (DisplaySelectionPosition == 4){DisplaySelectionPosition = 0;}
			DisplayChar[DisplaySelectionBar[DisplaySelectionPosition]] = '>';
			RefreshDisplay(DisplayChar);
			break;

			case 4:
			ChangePWLimit(0);
			RefreshDisplay(SettingsChar);
			break;
		}
	}
	
	if (PB5Flag == 1)
	{
		PB5Flag = 0;
		switch (StateSelection)
		{
			case 0:
			if (DisplaySelectionPosition == 0){StateSelection = 1; ConvertBars(MIDIChar, PW_mult, PW_mult_limit); RefreshDisplay(MIDIChar);}
			if (DisplaySelectionPosition == 1){StateSelection = 2; ConvertBars(FixedChar, PW_mult, PW_mult_limit); RefreshDisplay(FixedChar);}
			if (DisplaySelectionPosition == 2){StateSelection = 3; RefreshDisplay(NoneChar);}
			if (DisplaySelectionPosition == 3){StateSelection = 4; ChangePWLimit(2); RefreshDisplay(SettingsChar);}
			break;

			case 4:
			ChangePWLimit(1);
			RefreshDisplay(SettingsChar);
			break;
		}
	}
	
	if (PB3Flag == 1)
	{
		PB3Flag = 0;
		switch (StateSelection)
		{
			case 0:
			break;

			case 1:
			StateSelection = 0; RefreshDisplay(MenuChar);
			break;
			
			case 2:
			StateSelection = 0; RefreshDisplay(MenuChar);
			break;
			
			case 3:
			StateSelection = 0; RefreshDisplay(MenuChar);
			break;
			
			case 4:
			StateSelection = 0; RefreshDisplay(MenuChar);
			break;
			
		}
	}
}

ISR(PCINT0_vect) //interrupção do TC1
{
	if ((!tst_bit(PINB, PB4))&&(debouncePB4==0))
	{
		PB4Flag = 1;
		debouncePB4 = 1;
	}
	else if ((!tst_bit(PINB, PB5))&&(debouncePB5==0))
	{
		PB5Flag = 1;
		debouncePB5 = 1;
	}
	else if ((!tst_bit(PINB, PB3))&&(debouncePB3==0))
	{
		PB3Flag = 1;
		debouncePB3 = 1;
	}
}



