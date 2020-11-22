/*
 * main.c
 * Referenzcode, zeigt die Empfangene Daten
 * auf einem 4 x 16 Zeichen Display
 * Created: 23.11.2019 07:08:31
 * Author : gfcwfzkm
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include "IR_NEC.h"
#include "display_st.h"

/* Infrarot Struct */
irData_t IR_Receiver;

/* Timer-Interrupt */
ISR(TCB0_INT_vect, ISR_BLOCK)
{	
	IR_NEC_COUNTER_ISR(&IR_Receiver);
	
	TCB0.INTFLAGS = TCB_CAPT_bm;
}

/* External Interrupt */
ISR(PORTF_PORT_vect,ISR_BLOCK)
{	
	IR_NEC_SIGNAL_ISR(&IR_Receiver);
	
	PORTF.INTFLAGS = PORT_INT0_bm;
}

int main(void)
{
	char textBuffer[16];
	uint8_t temp =0;
	uint8_t output=0;
	uint8_t repead=0;
	
	/* 20MHz Clock */
	CCP = 0xD8;
	CLKCTRL.MCLKCTRLB = 0;
	
	/* I/O Setup */
	PORTD.DIRSET = PIN5_bm | PIN6_bm | PIN7_bm;
	PORTA.DIRSET = 0xFF;
	
	PORTF.PIN0CTRL = PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc;
	TIMER_INIT();
	sei();
	
	/* Display Setup */
	display_init();
	display_print("    Infrarot");
	display_position(0,1);
	display_print("    Empf\xE1nger");
	display_position(0,2);
	display_print("      und");
	display_position(0,3);
	display_print("    Dekoder");
	
	_delay_ms(2500);
	
	display_command(0x01);
	display_print("IR-Adr:");
	display_position(0,1);
	display_print("IR-Cmd:");
	display_position(0,2);
	display_print("Taste:");
	
	
	while (1) 
    {
		switch(IR_Receiver.status)	/* Verarbeitung IR Status */
		{
			case DECODING_IDLE:
				// Keine neuen Infos
				break;
			case DECODING_BUSY:
				// IR-Daten werden gerade eingelesen
				break;
			case DECODING_FINISHED:
				IR_Receiver.status &=~ DECODING_FINISHED;
				// IR-Daten verarbeiten
				display_position(8,0);
				sprintf(textBuffer,"0x%02X%02X",IR_Receiver.receivedData[READ_ADDRESS], IR_Receiver.receivedData[READ_INV_ADDRESS]);
				display_print(textBuffer);
				display_position(8,1);
				sprintf(textBuffer,"0x%02X%02X",IR_Receiver.receivedData[READ_COMMAND],IR_Receiver.receivedData[READ_INV_COMMAND]);
				display_print(textBuffer);
				repead = 0;
				output = 0;
				temp = ~IR_Receiver.receivedData[READ_INV_COMMAND];
				if(IR_Receiver.receivedData[READ_COMMAND] == temp)
				{
					display_position(7,2);
					switch(IR_Receiver.receivedData[READ_COMMAND]) {
						case 0x1A:case 0x1B:	output++;
						case 0x03:case 0x09:	output++;
						case 0x02:case 0x08:	output++;
						case 0x01:case 0x07:	output++;
						case 0x0E:case 0x1E:	output++;
						case 0x0c:case 0x06:	output++;
						case 0x19:case 0x05:	output++;
						case 0x1d:case 0x04:	output++;
							sprintf(textBuffer,"%d  ",output);
							break;
						case 0x0F:
							sprintf(textBuffer,"On ");
							break;
						case 0x12:
							sprintf(textBuffer,"Off");
							break;
						default:
							sprintf(textBuffer,"---");
							break;
					}
					display_print(textBuffer);					
				}
				else
				{
					display_position(7,2);
					display_print("Fehler   ");
				}
				display_position(12,2);
				sprintf(textBuffer,"R%d  ",repead);
				display_print(textBuffer);
				display_position(0,3);
				display_print("                      ");
				break;
			case DECODING_REPEATED:
				IR_Receiver.status &=~ DECODING_REPEATED;
				// IR-Wiederholung verarbeiten
				if(repead==255)		repead=0;
				display_position(12,2);
				sprintf(textBuffer,"R%d  ",++repead);
				display_print(textBuffer);
				break;
			case DECODING_START_ERROR:
				display_position(0,3);
				display_print("START ERROR  ");
				
				IR_Receiver.status &=~ DECODING_START_ERROR;
				break;
			case DECODING_RECEIVE_ERROR:
				display_position(0,3);
				display_print("RECEIVE ERROR");
				
				IR_Receiver.status &=~ DECODING_RECEIVE_ERROR;
				break;
			case DECODING_TIMEOUT_ERROR:
				display_position(0,3);
				display_print("TIMEOUT ERROR");
				
				IR_Receiver.status &=~ DECODING_TIMEOUT_ERROR;
				break;
		}
    }
}