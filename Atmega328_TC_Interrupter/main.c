/*
 * Atmega328_TC_Interrupter.c
 *
 * Created: 5/15/2019 11:11:43 AM
 * Author : Lucas Paiva da Silva 
 */ 

#include "defs.h"
#define OutputPin PB0
#define RelePin PB1

float Vcc;
float VB1;
float VB2;
int NewBatReading = 0;

unsigned char CanOutput = 1;

int Timer0Division = 0;
int Timer2Division = 0;

float PW_mult = 1.0;
float PW_mult_limit = 2.0;
int	  Pw_mult_to_display = 1;
unsigned char PW_multStr[4];

int FixedFreq = 220;
int FixedFrqLimit = 500;
unsigned char FixedFreqStr[4];

volatile int ON_TIME = 200;

int NewSerial = 0;

int debouncePB2 = 0;
int debouncePB3 = 0;
int debouncePB4 = 0;
int debouncePB5 = 0;
int PB2Flag = 0;
int PB3Flag = 0;
int PB4Flag = 1;
int PB5Flag = 0;

int StateSelection = 0;
int FixedModeSubStateSelection = 0;

char note_srt[3]; // recebe o valor da nota e o estado da mesma
unsigned char freqstr[4];


//Menu Variables
unsigned char MenuChar[] = {0x20, 'M', 'I', 'D', 'I', 0x20, 0x20, 0x20, 'F', 'i', 'x', 'e', 'd', 0x20, 0x20, 0x20,
						    0x20, 'T', 'e', 's', 't', 0x20, 0x20, 0x20, 'S' , 'e' , 't', 't', 'i', 'n', 'g', 's'};
unsigned char MenuSelectionBar[] = {0, 7, 16, 23};
int MenuSelectionPosition = 3;

//MIDI Variables
unsigned char MIDIChar[] = {0x20, 0x20,0x20, 0x20, 0x20, 'H', 'z', 0x20, 0x20, 0x20, 'P', 'W', ':', '1', '.', '7',
							0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20};
	
//Fixed Variables
unsigned char FixedChar[] = {'>', '4', '4', '0', 'H', 'z', 0x20, 0x20, 0x20,  0x20, 'P', 'W', ':', '1', '.', '2',
							0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 , 0x20 , 0x20, 0x20, 0x20, 0x20, 0x20, 0x20};
unsigned char FixedSelectionBar[] = {0, 9};
int FixedSelectionPosition = 0;

//Test
unsigned char NoneChar[] = {0x20, 'B', '1', ':',  0x20, '1', '0', '0', 'H', 'z', '@', '5', '0', '0', 'm', 'S',
							0x20, 'B', '2', ':',  0x20, '2', '0', '0', 'H', 'z', '@', '5', '0', '0', 'm', 'S',};
	
//Settings
unsigned char SettingsChar[] = {0x20, 'P' , 'W' , '_' , 'l' , 'i' , 'm' , 'i' , 't', ':', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
								0x20, 'R', 'e', 'a', 'd', 'i', 'n', 'g',0x20, 'B', 'a', 't', '1', '/', '2', 0x20};

void InitMessage();
void ChangePWLimit(int operation);
void RefreshDisplay(unsigned char DisplayChar[]);
void ModifyDisplay(unsigned char DisplayChar[], unsigned char DisplaySelectionBar[]);
void ConvertBars(unsigned char DisplayChar[], float PW, float PWMax);
void ChangeFixedFreq(int operation, unsigned char DisplayChar[]);
void ChangePW(int operation, unsigned char DisplayChar[]);
int GetOnTime(int freq);
void NoteOnOff(int frequency);
int NoteToFreq();
void NoteToDisplay();
void DisableOutput();
void ReEnableOutput();
void TurnOff(int ExitStatus);
void BatToDisplay();
ISR(PCINT0_vect);
ISR(TIMER1_COMPA_vect);




int main(void)
{
    
	DDRD = 0xFF;
	DDRB  = 0b00000011;
	PORTB = 0b11111100;
	
	//interrupção dos bots
	PCICR = 1<<PCIE0;
	PCMSK0 = (1<<PCINT2) | (1<<PCINT3) | (1<<PCINT4) | (1<<PCINT5);
	
	//Timer 0
	TCCR0B = (1<<CS02) | (1<<CS00);	
	
	//Timer 1
	TCCR1A = 0x00;                        
	TCCR1B = (1 << CS11) | (1 << WGM12); 
	
	//Timer 2
	TCCR2B = (1<<CS22) | (1<<CS21) | (1<<CS20);
	set_bit(TIMSK2,TOIE2);
	
	ADMUX = (1<<REFS0);
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	
	set_bit(PORTB, RelePin);
	USART_Inic(MYUBRR);
	set_bit(UCSR0B, RXCIE0);
	inic_LCD_4bits();
	InitMessage();
	ModifyDisplay(MenuChar, MenuSelectionBar);
	ChangeFixedFreq(2, FixedChar);							
	ChangePW(2, MIDIChar);		
	sei();	
								
    while (1) 
    {
		
		switch (StateSelection)
		{
			case 0:
			ModifyDisplay(MenuChar, MenuSelectionBar);
			break;

			case 1:
			if (NewSerial == 1)
			{
				NewSerial = 0;
				NoteOnOff(NoteToFreq());
				NoteToDisplay();
				if (note_srt[0] == 'L')
				{
					MIDIChar[0] = '|';
				}
				else
				{
					MIDIChar[0] = '/';
				}
				RefreshDisplay(MIDIChar);
			}
			ModifyDisplay(MIDIChar, MenuSelectionBar);
			break;
			
			case 2:
			ModifyDisplay(FixedChar, FixedSelectionBar);
			break;
			
			case 3:
			ModifyDisplay(NoneChar, MenuSelectionBar);
			break;
			
			case 4:
			if (NewBatReading == 1)
			{
				NewBatReading = 0;
				BatToDisplay();
				RefreshDisplay(SettingsChar);
			}
			ModifyDisplay(SettingsChar, MenuSelectionBar);
			break;
		}
    }
}

void TurnOff(int ExitStatus)
{
	TIMSK1 &= ~(1 << OCIE1A);
	clr_bit(PORTB, PB0);
	cmd_LCD(1, 0);
	cmd_LCD(0x80, 0);
	if (ExitStatus>=1)//Por causa da bateria
	{
		escreve_LCD("BATERIA BAIXA");
		_delay_ms(1000);
		cmd_LCD(1, 0);
		escreve_LCD("desligando");
		_delay_ms(500);
		cmd_LCD(1, 0);
		escreve_LCD("desligando.");	
		_delay_ms(500);
		cmd_LCD(1, 0);
		escreve_LCD("desligando..");	
		_delay_ms(500);
		cmd_LCD(1, 0);
		escreve_LCD("desligando...");	
		_delay_ms(500);	
	}
	else
	{
		escreve_LCD("desligando");
		_delay_ms(500);
		cmd_LCD(1, 0);
		escreve_LCD("desligando.");
		_delay_ms(500);
		cmd_LCD(1, 0);
		escreve_LCD("desligando..");
		_delay_ms(500);
		cmd_LCD(1, 0);
		escreve_LCD("desligando...");
		_delay_ms(500);
	}
	clr_bit(PORTB, RelePin);
}

void InitMessage()
{
	cmd_LCD(0x80, 0);
	escreve_LCD("Paiva's TC");
	cmd_LCD(0xC0, 0);
	escreve_LCD("328P Interrupter");
	_delay_ms(2500);
	cmd_LCD(1, 0);
	cmd_LCD(0x80, 0);
	escreve_LCD("Version 1");
	_delay_ms(1000);
	cmd_LCD(1, 0);
};

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
			DisplayChar[DisplaySelectionBar[MenuSelectionPosition]] = 0x20;
			MenuSelectionPosition++;
			if (MenuSelectionPosition == 4){MenuSelectionPosition = 0;}
			DisplayChar[DisplaySelectionBar[MenuSelectionPosition]] = '>';
			RefreshDisplay(DisplayChar);
			break;
			
			case 1:
			ChangePW(0, MIDIChar);
			ConvertBars(MIDIChar, PW_mult, PW_mult_limit);
			RefreshDisplay(MIDIChar);
			break;
			
			case 2:
			if (FixedModeSubStateSelection == 0)
			{
				DisplayChar[DisplaySelectionBar[FixedSelectionPosition]] = 0x20;
				FixedSelectionPosition++;
				if (FixedSelectionPosition == 2){FixedSelectionPosition = 0;};
				DisplayChar[DisplaySelectionBar[FixedSelectionPosition]] = '>';
				RefreshDisplay(DisplayChar);
			}
			else if (FixedModeSubStateSelection == 1)
			{
				ChangeFixedFreq(0, FixedChar);
				NoteOnOff(FixedFreq);
				RefreshDisplay(FixedChar);
			}
			else if (FixedModeSubStateSelection == 2)
			{
				ChangePW(0, FixedChar);
				NoteOnOff(FixedFreq);
				ConvertBars(FixedChar, PW_mult, PW_mult_limit);
				RefreshDisplay(FixedChar);
			}
			break;
			
			case 3:
			NoneChar[0] = '>';
			RefreshDisplay(NoneChar);
			note_srt[0]= 'L';
			NoteOnOff(100);
			_delay_ms(500);
			note_srt[0]= 'D';
			NoteOnOff(100);	
			NoneChar[0] = 0x20;
			RefreshDisplay(NoneChar);		
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
			if (MenuSelectionPosition == 0)
			{
				StateSelection = 1; 
				ConvertBars(MIDIChar, PW_mult, PW_mult_limit); 
				RefreshDisplay(MIDIChar);
			}
			
			if (MenuSelectionPosition == 1)
			{
				StateSelection = 2; 
				ChangePW(2, FixedChar); 
				ConvertBars(FixedChar, PW_mult, PW_mult_limit); 
				RefreshDisplay(FixedChar);
				note_srt[0]= 'L';
				NoteOnOff(FixedFreq);
			}
			
			if (MenuSelectionPosition == 2)
			{
				StateSelection = 3; 
				RefreshDisplay(NoneChar);
			}
			
			if (MenuSelectionPosition == 3)
			{
				StateSelection = 4; 
				ChangePWLimit(2); 
				RefreshDisplay(SettingsChar);
			}
			break;
			
			case 1:
			ChangePW(1, MIDIChar);
			ConvertBars(MIDIChar, PW_mult, PW_mult_limit);
			RefreshDisplay(MIDIChar);
			break;
			
			case 2:
			if (FixedModeSubStateSelection == 0)
			{
				if (FixedSelectionPosition == 0)
				{
					FixedModeSubStateSelection = 1;
				}
				if (FixedSelectionPosition == 1)
				{
					FixedModeSubStateSelection = 2;
				}
				DisplayChar[DisplaySelectionBar[FixedSelectionPosition]] = '|';
				RefreshDisplay(FixedChar);
			}
			else if (FixedModeSubStateSelection == 1)
			{
				ChangeFixedFreq(1, FixedChar);
				NoteOnOff(FixedFreq);
				RefreshDisplay(FixedChar);
			}
			else if (FixedModeSubStateSelection == 2)
			{
				ChangePW(1, FixedChar);
				NoteOnOff(FixedFreq);
				ConvertBars(FixedChar, PW_mult, PW_mult_limit);
				RefreshDisplay(FixedChar);
			}
			break;
			
			case 3:
			NoneChar[16] = '>';
			RefreshDisplay(NoneChar);
			note_srt[0]= 'L';
			NoteOnOff(200);
			_delay_ms(500);
			note_srt[0]= 'D';
			NoteOnOff(200);
			NoneChar[16] = 0x20;
			RefreshDisplay(NoneChar);
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
			StateSelection = 0; 
			RefreshDisplay(MenuChar);
			note_srt[0]= 'D';
			NoteOnOff(0);
			break;
			
			case 2:
			if (FixedModeSubStateSelection == 0)
			{
				StateSelection = 0;
				RefreshDisplay(MenuChar);
				note_srt[0]= 'D';
				NoteOnOff(FixedFreq);
			}
			else if (FixedModeSubStateSelection == 1)
			{
				FixedModeSubStateSelection = 0;
				DisplayChar[DisplaySelectionBar[FixedSelectionPosition]] = '>';
				RefreshDisplay(FixedChar);
			}
			else if (FixedModeSubStateSelection == 2)
			{
				FixedModeSubStateSelection = 0;
				DisplayChar[DisplaySelectionBar[FixedSelectionPosition]] = '>';
				RefreshDisplay(FixedChar);
			}
			break;
			
			case 3:
			StateSelection = 0;
			RefreshDisplay(MenuChar);
			break;
			
			case 4:
			StateSelection = 0; 
			RefreshDisplay(MenuChar);
			break;
			
		}
	}
	
	if (PB2Flag == 1)
	{
		PB2Flag = 0;
		switch (StateSelection)
		{
			case 0:
			TurnOff(0);			
			break;

			case 1:
			DisableOutput();
			break;
			
			case 2:
			DisableOutput();
			break;
			
			case 3:
			DisableOutput();
			break;
			
			case 4:
			ReEnableOutput();
			break;
			
		}
	}
	
}

void ChangePW(int operation, unsigned char DisplayChar[])
{
	if ((operation == 1)&&(PW_mult<PW_mult_limit)){PW_mult = PW_mult + 0.1;}
	if ((operation == 0)&&(PW_mult>0.1)){PW_mult = PW_mult - 0.1;}
	if (PW_mult>=PW_mult_limit){PW_mult = PW_mult_limit;}
	Pw_mult_to_display = (PW_mult * 10);
	ident_num(Pw_mult_to_display, PW_multStr);
	DisplayChar[13] = PW_multStr[1];
	DisplayChar[14] = '.';
	DisplayChar[15] = PW_multStr[0];
}

void ChangeFixedFreq(int operation, unsigned char DisplayChar[])
{
	if ((operation == 1)&&(FixedFreq<FixedFrqLimit)){FixedFreq = FixedFreq + 10;}
	if ((operation == 0)&&(FixedFreq>50)){FixedFreq = FixedFreq - 10;}
	ident_num(FixedFreq, FixedFreqStr);
	DisplayChar[1] = FixedFreqStr[2];
	DisplayChar[2] = FixedFreqStr[1];
	DisplayChar[3] = FixedFreqStr[0];
}


void ChangePWLimit(int operation)
{
	if ((operation == 1)&&(PW_mult_limit<4)){PW_mult_limit = PW_mult_limit + 0.1;}
	if ((operation == 0)&&(PW_mult_limit>0.5)){PW_mult_limit = PW_mult_limit - 0.1;}
		
	if (PW_mult>=PW_mult_limit)
	{
		PW_mult = PW_mult_limit;
		ChangePW(2, MIDIChar);
	}
	
	Pw_mult_to_display = (PW_mult_limit * 10);
	ident_num(Pw_mult_to_display, PW_multStr);
	SettingsChar[11] = PW_multStr[1];
	SettingsChar[12] = '.';
	SettingsChar[13] = PW_multStr[0];
}

int GetOnTime(int freq)
{
	int on_time = 10;
	if (freq < 700)  {on_time = 20;}
	if (freq < 600)  {on_time = 23;}
	if (freq < 500)  {on_time = 27;}
	if (freq < 400)  {on_time = 30;}
	if (freq < 300)  {on_time = 35;}
	if (freq < 200)  {on_time = 40;}
	if (freq < 100)  {on_time = 45;}
	on_time = on_time * PW_mult;
	return on_time;
}

void ConvertBars(unsigned char DisplayChar[], float V, float Vmax)
{
	int Nbars = 0, x = 0;
	for (x=0; x<16; x++)
	{
		DisplayChar[16+x] = 0x20;
	}
	Nbars = (16*V)/Vmax;
	for (x=0; x<Nbars; x++)
	{
		DisplayChar[16+x] = 0xFF;
	}
		
}

int NoteToFreq()
{
	int pitch;
	pitch = (int)((note_srt[1]-'0')*10) + (int)(note_srt[2]-'0');
	
	return (int) (220.0 * pow(pow(2.0, 1.0/12.0), pitch - 57) + 0.5);
}

void NoteOnOff(int frequency)
{
	int period; 
	if (note_srt[0] == 'L')
	{
		period = 1000000 / frequency;
		ON_TIME = GetOnTime(frequency); 
		OCR1A   = 2 * period;     
		TCNT1   = 0;              
		TIMSK1 |= (1 << OCIE1A); 
	}
	if (note_srt[0] == 'D')
	{
		TIMSK1 &= ~(1 << OCIE1A);
	}
}

void NoteToDisplay()
{
	int freq;
	freq = NoteToFreq();
	ident_num(freq, freqstr);
	MIDIChar[2] = freqstr[2];
	MIDIChar[3] = freqstr[1];
	MIDIChar[4] = freqstr[0];
}

void BatToDisplay()
{
	float temp;
	unsigned char TempStr[3];
	temp = VB1 * 500/1024;
	ident_num(temp, TempStr);
	SettingsChar[17] = 'B';
	SettingsChar[18] = '1';
	SettingsChar[19] = ':';
	SettingsChar[20] = TempStr[2];
	SettingsChar[21] = '.';
	SettingsChar[22] = TempStr[1];
	SettingsChar[23] = 0x20;
	temp = VB2 * 500/1024;
	ident_num(temp, TempStr);
	SettingsChar[24] = 'B';
	SettingsChar[25] = '2';
	SettingsChar[26] = ':';
	SettingsChar[27] = TempStr[2];
	SettingsChar[28] = '.';
	SettingsChar[29] = TempStr[1];
}

void DisableOutput()
{
	CanOutput = 0;
	TIMSK1 &= ~(1 << OCIE1A);
	clr_bit(PORTB, OutputPin);
	cmd_LCD(1,0);
	escreve_LCD("Disabled out");
	_delay_ms(3000);
}

void ReEnableOutput()
{
	if (CanOutput == 0)
	{
		CanOutput = 1;
		cmd_LCD(1,0);
		escreve_LCD("Enabled out");
		_delay_ms(3000);
		
	}
}

uint16_t ReadADC(uint8_t ch)
{
	// select the corresponding channel 0~7
	// ANDing with ’7? will always keep the value
	// of ‘ch’ between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
	
	// start single convertion
	// write ’1? to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes ’0? again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}

ISR(USART_RX_vect)
{
	char status;
	int recived = 0;
	while(recived == 0)
	{
		status = USART_Recebe();
		if (status == 'L')
		{
			note_srt[0]= 'L';
			note_srt[1]= USART_Recebe();
			note_srt[2]= USART_Recebe();
			recived = 1;
		}
		if (status == 'D')
		{
			note_srt[0]= 'D';
			note_srt[1]= USART_Recebe();
			note_srt[2]= USART_Recebe();
			recived = 1;
		}
	}
	NewSerial = 1;
}

ISR(PCINT0_vect)
{
	set_bit(TIMSK0,TOIE0);
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
	else if ((!tst_bit(PINB, PB2))&&(debouncePB2==0))
	{
		PB2Flag = 1;
		debouncePB2 = 1;
	}
}

ISR(TIMER1_COMPA_vect)
{
	if (CanOutput == 1)
	{
		int x;
		set_bit(PORTB, PB0);
		for (x=0;x<ON_TIME;x++)
		{
			_delay_us(1);
		}
		clr_bit(PORTB, PB0);
	}   
	else
	{}     
}

ISR(TIMER0_OVF_vect)
{
	Timer0Division++;
	if (Timer0Division >= 17)
	{
		debouncePB3 = 0;
		debouncePB4 = 0;
		debouncePB5 = 0;
		debouncePB2 = 0;
		Timer0Division = 0;
		clr_bit(TIMSK0,TOIE0);
	}
}ISR(TIMER2_OVF_vect)
{
	Timer2Division++;
	if (Timer2Division >= 300)
	{
		Timer2Division = 0;
		NewBatReading = 1;
		Vcc = ReadADC(1)*2;
		VB1 = ReadADC(0)*2;
		VB2 = Vcc - VB1;
		if (Vcc<=1230)
		{
			TurnOff(1);
		}
		if (VB1<=615)
		{
			TurnOff(1);
		}
		if (VB2<=615)
		{
			TurnOff(1);
		}		
	}
}



