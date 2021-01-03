/*
 * PROJECT1_DIEUHOA.cpp
 *
 * Created: 1/3/2021 12:07:42 AM
 * Author : blackhorn
 */ 
#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/delay.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>

void send( unsigned char c )
{
	while (bit_is_clear(UCSRA,UDRE));
	UDR = c;
}
unsigned char receive()
{
	while(bit_is_clear(UCSRA,RXC));
	return UDR;
}


int main(void)
{
  
   DDRA = 0b00000011;
   PORTA = 0b00000000;
   UBRRL = 51;
   UCSRC = (1<<UCSZ1)|(1<<UCSZ0);
   UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE); 
    while (1) 
    {
		send('5');
		_delay_ms(1000);
		if (receive()=='1') {
			PORTA = 0x01;
			_delay_ms(1000);
		}
		else if (receive()=='2')
		{
			PORTA=0b00000010;
			_delay_ms(1000);
		}
		PORTA=0x00;
		UDR=0x00;
	}
}
	
	



