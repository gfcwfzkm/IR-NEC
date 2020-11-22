/*
 * IR_NEC.c
 *
 * Created: 14.11.2019 08:05:51
 *  Author: gfcwfzkm
 */ 

#include "IR_NEC.h"

/* Ein 'Tick' ist 1/10 von der kleinsten Übertragungszeit
 * Die kürzeste Übertragung ist laut Spezifikation 562,5 Mikrosekunden lang.
 * Dadurch ist ein Tick, welcher auch der Timer / Counter zählt, 56,25 Mikrosekunden lang.
 */
#define NEC_TICK	1	

/* Alle Signale haben ±5% Toleranz
 * Ein Startsignal geht 13.5ms, geteilt durch die TICK Zeit (56,25us) geht 240.
 * Bei ±5% Toleranz muss das Startsignal zwischen 228 und 252 Ticks lang sein.
 */
#define NEC_START_LOWERLIMIT	228
#define NEC_START_UPPERLIMIT	252

/* Ein Repeatsignal geht 11.25ms, geteilt durch die Tick Zeit(56,25us) geht 
 * Bei ±5% Toleranz muss das Repeatsignal zwischen 190 und 210 Ticks lang sein.
 */
#define NEC_REPEAT_LOWERLIMIT	190
#define NEC_REPEAT_UPPERLIMIT	210

/* Minumumzeit die vergehen muss bis die Library IR Startsignale verarbeitet */
#define NEC_MIN_START			10

/* Ein LOW-Signal geht 1.125ms, geteilt durch die Tick Zeit(56,25us) geht
 * Bei ±5% Toleranz muss das LOW-Signal zwischen 19 und 21 Ticks lang sein.
 */
#define NEC_LOW_LOWERLIMIT	19
#define NEC_LOW_UPPERLIMIT	21

/* Ein HIGH-Signal geht 2.25ms, geteilt durch die Tick Zeit(56,25us) geht
 * Bei ±5% Toleranz muss das HIGH-Signal zwischen 38 und 42 Ticks lang sein.
 */
#define NEC_HIGH_LOWERLIMIT 38
#define NEC_HIGH_UPPERLIMIT 42

#define TRUE		1
#define FALSE		0

void IR_NEC_SIGNAL_ISR(irData_t *ptr)
{
	uint8_t tCounter = ptr->_timeCounter;
	ptr->_timeCounter = 0;
	
	TIMER_RESET();
	
	if ((ptr->status & DECODING_BUSY) == FALSE)	//Überprüfen, ob ein Start schon erkannt wurde
	{
		if (tCounter > NEC_MIN_START)		
		{	// Es wir abgefragt ob die 13,5ms für das Start Bit gemessen wurde
			if ( (tCounter >= NEC_START_LOWERLIMIT) && (tCounter <= NEC_START_UPPERLIMIT) )
			{
				ptr->status = DECODING_BUSY;		// Start wurde erkannt
				ptr->_bitCounter = 0;				// Der Bit Zähler wird zurückgesetzt
				
				// Das Array für die empfangenen Daten wird zurückgesetzt
				ptr->receivedData[0] = 0;
				ptr->receivedData[1] = 0;
				ptr->receivedData[2] = 0;
				ptr->receivedData[3] = 0;
			}
			// Es wird abgefragt ob die 11.25 ms für das Repead Signal gemessen wurde
			else if ((tCounter >= NEC_REPEAT_LOWERLIMIT) && (tCounter <= NEC_REPEAT_UPPERLIMIT) )
			{
				TIMER_STOP();				//Der Timer wird angehalten
				ptr->status = DECODING_REPEATED;
			}
			else
			{
				ptr->status = DECODING_START_ERROR;
				TIMER_STOP();				//Der Timer wird angehalten
			}
		}
		else
		{
			TIMER_START();		//Der Timer wird gestartet
		}
	}
	else
	{	//Ein gültiges Startsignal wurde erkannt. Nun wird abgefragt ob ein High Signal gemessen wurde
		if ( (tCounter >= NEC_HIGH_LOWERLIMIT) && (tCounter <= NEC_HIGH_UPPERLIMIT) )
		{
			ptr->receivedData[ptr->_bitCounter / 8] |= (1<< (ptr->_bitCounter % 8) );
		}
		// Wenn das Bit weder High noch Low ist, ist es ein Fehler
		else if ( (tCounter < NEC_LOW_LOWERLIMIT) || (tCounter > NEC_LOW_UPPERLIMIT) )
		{
			ptr->status = DECODING_RECEIVE_ERROR;
			TIMER_STOP();				// Der Timer wird gestoppt
		}
		
		// Der Bit Zähler um 1 erhöht
		ptr->_bitCounter++;	
		// Nun wird abgefragt ob alle 32 bit erkannt und gültig waren		
		if (ptr->_bitCounter >= 32)
		{
			ptr->status = DECODING_FINISHED;
			TIMER_STOP();					// Der Timer TCB wird gestoppt
		}
	}
}

void IR_NEC_COUNTER_ISR(irData_t *ptr)
{
	if (ptr->_timeCounter == 255)	/* Overflow? Timer stop + error! */
	{
		TIMER_STOP();
		ptr->status = DECODING_TIMEOUT_ERROR;
		ptr->_timeCounter = 0;
	}
	else	/* Timer erhöhen */
	{
		ptr->_timeCounter++;
	}
}