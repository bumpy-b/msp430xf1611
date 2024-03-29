/**
 * \addtogroup rimeabc
 * @{
 */


/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 * This file is part of the Contiki operating system.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: abc.c,v 1.19 2009/03/12 21:58:20 adamdunkels Exp $
 */

/**
 * \file
 *         Anonymous best-effort local area Broad Cast (abc)
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki-net.h"
#include "net/rime.h"


#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static const struct packetbuf_attrlist attributes[] =
  { ABC_ATTRIBUTES PACKETBUF_ATTR_LAST };

/*---------------------------------------------------------------------------*/
void
abc_open(struct abc_conn *c, uint16_t channelno,
	  const struct abc_callbacks *callbacks)
{
  channel_open(&c->channel, channelno);
  c->u = callbacks;
  channel_set_attributes(channelno, attributes);
}
/*---------------------------------------------------------------------------*/
void
abc_close(struct abc_conn *c)
{
  channel_close(&c->channel);
}
/*---------------------------------------------------------------------------*/
int
abc_send(struct abc_conn *c)
{
  PRINTF("%d.%d: abc: abc_send on channel %d\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	 c->channel.channelno);
  return chameleon_output(&c->channel);
}
/*---------------------------------------------------------------------------*/
void
abc_input(struct channel *channel)
{
  struct abc_conn *c = (struct abc_conn *)channel;
  PRINTF("%d.%d: abc: abc_input_packet on channel %d\n",
	 rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],
	 channel->channelno);

  c->u->recv(c);
}
/*---------------------------------------------------------------------------*/

/** @} */
