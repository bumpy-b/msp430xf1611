#include "spi.h"
#include "FreeRTOS.h"
/*
 * Initialize SPI bus.
 */
void
spi_init(void)
{
  static unsigned char spi_inited = 0;

  if (spi_inited)
    return;

  /* Initalize ports for communication with SPI units. */

  U0CTL  = CHAR + SYNC + MM + SWRST; 	  /* SW  reset,8-bit transfer, SPI master */
  U0TCTL = CKPH + SSEL1 + STC;			  /* Data on Rising Edge, SMCLK, 3-wire. */

  U0BR0  = 0x02;						  /* SPICLK set baud. */
  U0BR1  = 0;  							  /* Dont need baud rate control register 2 - clear it */
  U0MCTL = 0;						  	  /* Dont need modulation control. */

  P3SEL |= BV(SCK) | BV(MOSI) | BV(MISO); /* Select Peripheral functionality and not I/O */
  P3DIR |= BV(SCK) | BV(MISO);	          /* Configure as outputs(SIMO,CLK). */
  	  	  	  	  	  	  	  	  	  	  /*//TODO: check this names (opposite ?)
  	  	  	  	  	  	  	  	  	  	  /* that don't seem right, maybe should be MOSI ? */


  ME1   |= USPIE0;	   					  /* Module enable ME1 --> U0ME? xxx/bg */
  U0CTL &= ~SWRST;						  /* Remove RESET */

  	  	  	  	  	  	  	  	  	  	  /* here we enable interrupts */
  IE1 = UTXIE0 | URXIE0;
  IFG1 &= ~URXIFG0 & ~UTXIFG0;

  spi_inited = 1; 						/* finish init */
}

