/*
 * IR_NEC.h
 * Version 1.0
 * Getestet und fertig
 * Created: 14.11.2019 08:06:59
 *  Author: gfcwfzkm
 */ 


#ifndef IR_NEC_H_
#define IR_NEC_H_

#include <avr/io.h>

/* Byte-Position in
 * receivedData[4]
 */
#define READ_ADDRESS		0
#define READ_INV_ADDRESS	1
#define READ_COMMAND		2
#define READ_INV_COMMAND	3

/* Hardware-Timer initialization für einen 
 * Interrupt alle 56,25us
 */
#define TIMER_INIT()	TCB0_INTCTRL=TCB_CAPT_bm;TCB0_CCMP=0x0464
/* Hardware-Timer Start */
#define TIMER_START()	TCB0_CTRLA=TCB_ENABLE_bm
/* Hardware-Timer Stop */
#define TIMER_STOP()	TCB0_CTRLA=0
/* Hardware-Timer Reset */
#define TIMER_RESET()	TCB0_CNT=0

/* Infrarot-Library Status */
enum ir_status {
	DECODING_IDLE=0,		/* Idle / No work */
	DECODING_BUSY,			/* Signal wird gerade dekodiert */
	DECODING_FINISHED,		/* Signal fertig empfangen */
	DECODING_REPEATED,		/* Signal Repeated Empfangen */
	DECODING_START_ERROR,	/* Start Signal Error */
	DECODING_RECEIVE_ERROR,	/* Receiving Error */
	DECODING_TIMEOUT_ERROR	/* Timeout / Timer-Overflow Error */
};

typedef struct _irData_t_
{
	volatile enum ir_status status;			/* Infrarot Bibliothek Status, Fehlerbits müsse vom User gelöscht werden */
	
	volatile uint8_t receivedData[4];		/* Alle empfangene Daten: 0 ist die Adresse, 1 die invertierte Adresse, 2 der Befehl, 3 der invertierte Befehl */	
	volatile uint8_t _bitCounter;			/* Interne Variable, Bit-Zähler */
	volatile uint8_t _timeCounter;			/* Anzahl Schritte in 56,25 uS */		
}irData_t;

void IR_NEC_SIGNAL_ISR(irData_t *ptr);

void IR_NEC_COUNTER_ISR(irData_t *ptr);

#endif /* IR_NEC_H_ */