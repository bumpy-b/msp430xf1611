/*
    FreeRTOS V6.1.0 - Copyright (C) 2010 Real Time Engineers Ltd.

    ***************************************************************************
    *                                                                         *
    * If you are:                                                             *
    *                                                                         *
    *    + New to FreeRTOS,                                                   *
    *    + Wanting to learn FreeRTOS or multitasking in general quickly       *
    *    + Looking for basic training,                                        *
    *    + Wanting to improve your FreeRTOS skills and productivity           *
    *                                                                         *
    * then take a look at the FreeRTOS books - available as PDF or paperback  *
    *                                                                         *
    *        "Using the FreeRTOS Real Time Kernel - a Practical Guide"        *
    *                  http://www.FreeRTOS.org/Documentation                  *
    *                                                                         *
    * A pdf reference manual is also available.  Both are usually delivered   *
    * to your inbox within 20 minutes to two hours when purchased between 8am *
    * and 8pm GMT (although please allow up to 24 hours in case of            *
    * exceptional circumstances).  Thank you for your support!                *
    *                                                                         *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    ***NOTE*** The exception to the GPL is included to allow you to distribute
    a combined work that includes FreeRTOS without being obliged to provide the
    source code for proprietary components outside of the FreeRTOS kernel.
    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public 
    License and the FreeRTOS license exception along with FreeRTOS; if not it 
    can be viewed here: http://www.freertos.org/a00114.html and also obtained 
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/


/* BASIC INTERRUPT DRIVEN SERIAL PORT DRIVER.   
 * 
 * This file only supports UART 1
 */

/* Standard includes. */
#include <stdlib.h>
#include <signal.h>
#include "debugFunction.h"
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/* Demo application includes. */
#include "serial.h"

/* Constants required to setup the hardware. */
#define serTX_AND_RX			( ( unsigned char ) 0xC0 )

/* Misc. constants. */
#define serNO_BLOCK				( ( portTickType ) 0 )

/* Enable the UART Tx interrupt. */
#define vInterruptOn() IFG2 |= UTXIFG1

/* The queue used to hold received characters. */
static xQueueHandle xRxedChars; 

/* The queue used to hold characters waiting transmission. */
static xQueueHandle xCharsForTx; 

static volatile short sTHREEmpty;

/* Interrupt service routines. */
interrupt (UART1RX_VECTOR) wakeup vRxISR( void );
interrupt (UART1TX_VECTOR) wakeup vTxISR( void );

/*-----------------------------------------------------------*/

xComPortHandle xSerialPortInitMinimal( eBaud ulWantedBaud, unsigned portBASE_TYPE uxQueueLength )
{
	/* Initialise the hardware. */

	portENTER_CRITICAL();
	{
		/* Create the queues used by the com test task. */
		xRxedChars = xQueueCreate( uxQueueLength, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
		xCharsForTx = xQueueCreate( uxQueueLength, ( unsigned portBASE_TYPE ) sizeof( signed char ) );

		/* Reset UART. */
		P3DIR &= ~BIT7;			/* Select P3.7 for input (UART1RX) */
		P3DIR |= BIT6;			/* Select P3.6 for output (UART1TX) */
		P3SEL |= serTX_AND_RX;  /* Select P3.6,P3.7 for UART1{TX,RX} */

		/* Set pin function. */


		/* All other bits remain at zero for n, 8, 1 interrupt driven operation. 
		LOOPBACK MODE!*/
		U1CTL |= CHAR;

		/* baud rate calculations */

		U1TCTL |= SSEL0; /* use ACLK (32 kHz on TelosB)           */
		U1BR1   = 0x00;  /* Setup baud rate high byte. */

		switch (ulWantedBaud){
		case 	ser1200:
			U1BR0   = 0x1B;   /* bit-rate generator setup              */
			U1MCTL  = 0x03;
			break;
		case ser4800:
			U1BR0   = 0x06;   /* bit-rate generator setup              */
			U1MCTL  = 0x6F;
			break;
		case ser9600:
			U1BR0   = 0x03;   /* bit-rate generator setup              */
			U1MCTL  = 0x4A;
			break;
		default: /*ser2400*/
			UBR01   = 0x0D;   /* bit-rate generator setup              */
			U1MCTL  = 0x6B;

		}

		ME2 &= ~USPIE1;		   	 /* USART1 SPI module disable */
	    ME2 |= (UTXE1 | URXE1);  /* Enable USART1 TXD/RXD */


		/* Set. */
		UCTL1 &= ~SWRST;

		/* XXX Clear pending interrupts before enable!!! */
		IFG2 &= ~URXIFG1;
		U1TCTL |= URXSE;
		//IE2 |= URXIE1;

		/* Nothing in the buffer yet. */
		sTHREEmpty = pdTRUE;

		/* Enable interrupts. */
		IE2 |= URXIE1 | UTXIE1;

	}
	portEXIT_CRITICAL();

	/* Unlike other ports, this serial code does not allow for more than one
	com port.  We therefore don't return a pointer to a port structure and can
	instead just return NULL. */
	return NULL;
}
/* this function peeks the rx buffer and return the data */
signed portBASE_TYPE xSerialPeek
	( xComPortHandle pxPort,
	  signed char *pcRxedChar,
	  portTickType xBlockTime)
{
	return
			xQueueGenericReceive( xRxedChars, pcRxedChar, xBlockTime, pdTRUE);
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialGetChar( xComPortHandle pxPort, signed char *pcRxedChar, portTickType xBlockTime )
{
	/* Get the next character from the buffer.  Return false if no characters
	are available, or arrive before xBlockTime expires. */
	if( xQueueReceive( xRxedChars, pcRxedChar, xBlockTime ) )
	{
		return pdTRUE;
	}
	else
	{
		return pdFALSE;
	}
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialPutChar( xComPortHandle pxPort, signed char cOutChar, uint32_t xBlockTime )
{
signed portBASE_TYPE xReturn;

	/* Transmit a character. */

	portENTER_CRITICAL();
	{
		if( sTHREEmpty == pdTRUE )
		{
			/* If sTHREEmpty is true then the UART Tx ISR has indicated that 
			there are no characters queued to be transmitted - so we can
			write the character directly to the shift Tx register. */
			sTHREEmpty = pdFALSE;
			U1TXBUF = cOutChar;
			xReturn = pdPASS;
		}
		else
		{
			/* sTHREEmpty is false, so there are still characters waiting to be
			transmitted.  We have to queue this character so it gets 
			transmitted	in turn. */

			/* Return false if after the block time there is no room on the Tx 
			queue.  It is ok to block inside a critical section as each task
			maintains it's own critical section status. */
			xReturn = xQueueSend( xCharsForTx, &cOutChar, xBlockTime );

			/* Depending on queue sizing and task prioritisation:  While we 
			were blocked waiting to post on the queue interrupts were not 
			disabled.  It is possible that the serial ISR has emptied the 
			Tx queue, in which case we need to start the Tx off again
			writing directly to the Tx register. */
			if( ( sTHREEmpty == pdTRUE ) && ( xReturn == pdPASS ) )
			{
				/* Get back the character we just posted. */
				xQueueReceive( xCharsForTx, &cOutChar, serNO_BLOCK );
				sTHREEmpty = pdFALSE;
				U1TXBUF = cOutChar;
			}
		}
	}
	portEXIT_CRITICAL();

	return pdPASS;
}
/*-----------------------------------------------------------*/

/*
 * UART RX interrupt service routine.
 */
interrupt (UART1RX_VECTOR) wakeup vRxISR( void )
{
signed char cChar;
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	ledFlip(BLUE);

	 if(!(URXIFG1 & IFG2)) {
	    /* Edge detect if IFG not set? */
	    U1TCTL &= ~URXSE; /* Clear the URXS signal */
	    U1TCTL |= URXSE;  /* Re-enable URXS - needed here?*/

	  } else {
	    /* Check status register for receive errors. */
	    if(URCTL1 & RXERR) {
	      cChar = RXBUF1;   /* Clear error flags by forcing a dummy read. */
	    } else {

	    	cChar = RXBUF1;
	    	xQueueSendFromISR( xRxedChars, &cChar, &xHigherPriorityTaskWoken );

	    	if( xHigherPriorityTaskWoken )
	    	{
	    		/*If the post causes a task to wake force a context switch
	    		as the woken task may have a higher priority than the task we have
	    		interrupted. */
	    		taskYIELD();
	    	}

	    }
	   }
	 return;


}
/*-----------------------------------------------------------*/

/*
 * UART Tx interrupt service routine.
 */
interrupt (UART1TX_VECTOR) wakeup vTxISR( void )
{
signed char cChar;
portBASE_TYPE xTaskWoken = pdFALSE;

// toggle the blue led on ISR
//P5OUT ^= (1 << 6);

	/* The previous character has been transmitted.  See if there are any
	further characters waiting transmission. */

	if( xQueueReceiveFromISR( xCharsForTx, &cChar, &xTaskWoken ) == pdTRUE )
	{
		/* There was another character queued - transmit it now. */
		U1TXBUF = cChar;
	}
	else
	{
		/* There were no other characters to transmit. */
		sTHREEmpty = pdTRUE;
	}
}

/*-----------------------------------------------------------*/

void vSerialPutString( xComPortHandle pxPort, const char * const pcString, unsigned short usStringLength )
{
char * pcNextChar;
const portTickType xNoBlock = ( portTickType ) 0;

	/* Stop warnings. */
	( void ) usStringLength;

	pcNextChar = ( char * ) pcString;
	while( *pcNextChar )
	{
		xSerialPutChar( pxPort, *pcNextChar, xNoBlock );
		pcNextChar++;
	}
}
/*-----------------------------------------------------------*/
