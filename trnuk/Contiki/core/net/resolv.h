/**
 * \file
 * uIP DNS resolver code header file.
 * \author Adam Dunkels <adam@dunkels.com>
 */

/*
 * Copyright (c) 2002-2003, Adam Dunkels.
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
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: resolv.h,v 1.3 2006/09/18 23:30:40 oliverschmidt Exp $
 *
 */
#ifndef __RESOLV_H__
#define __RESOLV_H__

#include "contiki.h"

/**
 * Event that is broadcasted when a DNS name has been resolved.
 */
CCIF extern process_event_t resolv_event_found;

/* Callbacks. */
void resolv_found(char *name, u16_t *ipaddr);

/* Functions. */
CCIF void resolv_conf(const uip_ipaddr_t *dnsserver);
CCIF uip_ipaddr_t *resolv_getserver(void);
void resolv_init(char *arg);
CCIF u16_t *resolv_lookup(char *name);
CCIF void resolv_query(char *name);

PROCESS_NAME(resolv_process);

#endif /* __RESOLV_H__ */
