/*
 * Atmega328_TC_Interrupter.c
 *
 * Created: 5/15/2019 11:11:43 AM
 * Author : Lucas Paiva da Silva 
 */ 

#include "defs.h"

int debouncePB4 = 0;
int debouncePB5 = 0;
int PB4Flag = 1;
int PB5Flag = 0;

//Menu Variables
unsigned char MenuChar[] = {0x20, 'M', 'I', 'D', 'I', 0x20, 0x20, 0x20, 'F', 'i', 'x', 'e', 'd', 0x20, 0x20, 0x20,
						    0x20, 'N', 'o', 'n', 'e', 0x20, 0x20, 0x20, 's' , 'e' , 't', 't', 'i', 'n', 'g', 's'};
unsigned char MenuSelectionBar[] = {0, 7, 16, 23};
int MenuSelectionPosition = -1;

//MIDI Variables
unsigned char MIDIChar[] = {0x20, 'M', 'I', 'D', 'I', 0x20, 0x20, 0x20, 'F', 'i', 'x', 'e', 'd', 0x20, 0x20, 0x20,
							0x20, 'N', 'o', 'n', 'e', 0x20, 0x20, 0x20, 's' , 'e' , 't', 't', 'i', 'n', 'g', 's'};
unsigned char MIDISelectionBar[] = {0, 7, 16, 23};
int MIDISelectionPosition = -1;

	
void RefreshDisplay()
{
	int x;
	cmd_LCD(1, 0);
	cmd_LCD(0x80, 0);
	for (x=0; x<16;x++)
	{
		cmd_LCD(MenuChar[x], 1);
	}
	cmd_LCD(0xC0, 0);
	for (x=16; x<32;x++)
	{
		cmd_LCD(MenuChar[x], 1);
	}
}	

void InitMessage()
{
	cmd_LCD(0x80, 0);
	escreve_LCD("Paiva's TC");
	cmd_LCD(0xC0, 0);
	escreve_LCD("328P Interrupter");
	_delay_ms(3000);
	cmd_LCD(1, 0);
	cmd_LCD(0x80, 0);
	escreve_LCD("Version 0.1");
	_delay_ms(1000);
	cmd_LCD(1, 0);
};

void ModifyDisplay()
{
	if (PB4Flag == 1)
	{
		MenuChar[MenuSelectionBar[MenuSelectionPosition]] = 0x20;
		MenuSelectionPosition++;
		if (MenuSelectionPosition == 4){MenuSelectionPosition = 0;}
		MenuChar[MenuSelectionBar[MenuSelectionPosition]] = '>';
		PB4Flag = 0;
		RefreshDisplay();
		
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
}


int main(void)
{
    
	DDRD = 0xFF; 
	DDRB  = 0x00;	
	PORTB = 0xFF;
	
	//interrupção dos bots
	PCICR = 1<<PCIE0;
	PCMSK0 = (1<<PCINT4) | (1<<PCINT5);
	sei();
	
	inic_LCD_4bits();
	InitMessage();
	ModifyDisplay();
	RefreshDisplay();
	
    while (1) 
    {
		ModifyDisplay();
		_delay_ms(500);
		debouncePB4 = 0;
    }
}

