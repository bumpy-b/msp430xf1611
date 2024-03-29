/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @(#)$Id: cc2420-arch.c,v 1.4 2009/04/02 22:39:29 joxe Exp $
 */

#include <io.h>
#include <signal.h>

#include "contiki.h"
#include "contiki-net.h"

#include "dev/spi.h"
#include "dev/cc2420.h"

#ifdef CONF_SFD_TIMESTAMPS
uint16_t sfd_start_time;
uint16_t sfd_end_time;
#endif

/*---------------------------------------------------------------------------*/
interrupt(PORT1_VECTOR)
cc24240_port1_interrupt(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  if(cc2420_interrupt()) {
    LPM4_EXIT;
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}

#ifdef CONF_SFD_TIMESTAMPS
/*---------------------------------------------------------------------------*/
/* SFD interrupt for timestamping radio packets */
interrupt(TIMERB1_VECTOR)
cc24240_timerb1_interrupt(void)
{
  int tbiv;
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  /* always read TBIV to clear IFG */
  tbiv = TBIV;
  if(SFD_IS_1) {
    sfd_start_time = TBCCR1;
  } else {
    sfd_end_time = TBCCR1;
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
#endif
/*---------------------------------------------------------------------------*/
void
cc2420_arch_init(void)
{
  spi_init();

  /* all input by default, set these as output */
  P4DIR |= BV(CSN) | BV(VREG_EN) | BV(RESET_N);

#ifdef CONF_SFD_TIMESTAMPS
  /* Need to select the special function! */
  P4SEL = BV(SFD);

  /* start timer B - 32768 ticks per second */
  TBCTL = TBSSEL_1 | TBCLR;

  /* CM_3 = capture mode - capture on both edges */
  TBCCTL1 = CM_3 | CAP | SCS;
  TBCCTL1 |= CCIE;

  /* Start Timer_B in continuous mode. */
  TBCTL |= MC1;
#endif

  SPI_DISABLE();                /* Unselect radio. */
}
/*---------------------------------------------------------------------------*/
