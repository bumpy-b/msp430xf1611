/*
 * Copyright (c) 2001, Adam Dunkels.
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
 * $Id: vnc-server.h,v 1.1 2006/06/17 22:41:16 adamdunkels Exp $
 *
 */

#ifndef __VNC_SERVER_H__
#define __VNC_SERVER_H__


/*struct vnc_server_updatearea {
  u8_t active;
  u8_t x, y;
  u8_t w, h;
  };*/

struct vnc_server_update {
  struct vnc_server_update *next;

#define VNC_SERVER_UPDATE_NONE  0
#define VNC_SERVER_UPDATE_PARTS 1
#define VNC_SERVER_UPDATE_FULL  2

  u8_t type;

  u8_t x, y;
  u8_t w, h;  
};

struct vnc_server_state {
  u16_t counter;
  u8_t type;
  u8_t state;
  u16_t height, width;

  u8_t update_requested;
  
  /* Variables used when sending screen updates. */
  u8_t x, y, x1, y1, x2, y2;
  u8_t w, h;

  

  u16_t readlen;
  u8_t sendmsg;
  u8_t button;

  
  struct vnc_server_update *updates_current;
  struct vnc_server_update *updates_pending;
  struct vnc_server_update *updates_free;

#define VNC_SERVER_MAX_UPDATES 8  
  struct vnc_server_update updates_pool[VNC_SERVER_MAX_UPDATES];

};

struct vnc_server_update *
     vnc_server_update_alloc(struct vnc_server_state *vs);
void vnc_server_update_free(struct vnc_server_state *vs,
			    struct vnc_server_update *a);
void vnc_server_update_remove(struct vnc_server_state *vs,
			      struct vnc_server_update *a);

void vnc_server_update_add(struct vnc_server_state *vs,
			   struct vnc_server_update *a);
struct vnc_server_update *
     vnc_server_update_dequeue(struct vnc_server_state *vs);




void vnc_server_init(void);
void vnc_server_appcall(struct vnc_server_state *state);


extern struct vnc_server_state *vs;

enum {
  VNC_DEALLOCATED,
  VNC_VERSION,
  VNC_VERSION2,
  VNC_AUTH,
  VNC_AUTH2,
  VNC_INIT,
  VNC_INIT2,
  VNC_RUNNING
};

/* Sendmsg */
enum {
  SEND_NONE,
  SEND_BLANK,
  SENT_BLANK,
  SEND_SCREEN,
  SEND_UPDATE
};


/* Definitions of the RFB (Remote Frame Buffer) protocol
   structures and constants. */

#include "contiki-net.h"

void vnc_server_send_data(struct vnc_server_state *vs);
u8_t vnc_server_draw_rect(u8_t *ptr, u16_t x, u16_t y, u16_t w, u16_t h, u8_t c);


/* Generic rectangle - x, y coordinates, width and height. */
struct rfb_rect {
  u16_t x;
  u16_t y;
  u16_t w;
  u16_t h;
};

/* Pixel format definition. */
struct rfb_pixel_format {
  u8_t bps;       /* Bits per pixel: 8, 16 or 32. */
  u8_t depth;     /* Color depth: 8-32 */
  u8_t endian;    /* 1 - big endian (motorola), 0 - little endian
		     (x86) */
  u8_t truecolor; /* 1 - true color is used, 0 - true color is not used. */

  /* The following fields are only used if true color is used. */
  u16_t red_max, green_max, blue_max;
  u8_t red_shift, green_shift, blue_shift;
  u8_t pad1;
  u16_t pad2;
};


/* RFB authentication constants. */

#define RFB_AUTH_FAILED      0
#define RFB_AUTH_NONE        1
#define RFB_AUTH_VNC         2

#define RFB_VNC_AUTH_OK      0
#define RFB_VNC_AUTH_FAILED  1
#define RFB_VNC_AUTH_TOOMANY 2

/* RFB message types. */

/* From server to client: */
#define RFB_FB_UPDATE            0
#define RFB_SET_COLORMAP_ENTRIES 1
#define RFB_BELL                 2
#define RFB_SERVER_CUT_TEXT      3

/* From client to server. */
#define RFB_SET_PIXEL_FORMAT     0
#define RFB_FIX_COLORMAP_ENTRIES 1
#define RFB_SET_ENCODINGS        2
#define RFB_FB_UPDATE_REQ        3
#define RFB_KEY_EVENT            4
#define RFB_POINTER_EVENT        5
#define RFB_CLIENT_CUT_TEXT      6

/* Encoding types. */
#define RFB_ENC_RAW      0
#define RFB_ENC_COPYRECT 1
#define RFB_ENC_RRE      2
#define RFB_ENC_CORRE    3
#define RFB_ENC_HEXTILE  4

/* Message definitions. */

/* Server to client messages. */

struct rfb_server_init {
  u16_t width;
  u16_t height;
  struct rfb_pixel_format format;
  u8_t namelength[4];
  /* Followed by name. */
};

struct rfb_fb_update {
  u8_t type;
  u8_t pad;
  u16_t rects; /* Number of rectanges (struct rfb_fb_update_rect_hdr +
		  data) that follows. */
};

struct rfb_fb_update_rect_hdr {
  struct rfb_rect rect;
  u8_t encoding[4];
};

struct rfb_copy_rect {
  u16_t srcx;
  u16_t srcy;
};

struct rfb_rre_hdr {
  u16_t subrects[2];  /* Number of subrectangles (struct
			 rfb_rre_subrect) to follow. */
  u8_t bgpixel;
};

struct rfb_rre_subrect {
  u8_t pixel;
  struct rfb_rect rect;
};

struct rfb_corre_rect {
  u8_t x;
  u8_t y;
  u8_t w;
  u8_t h;
};

/* Client to server messages. */

struct rfb_set_pixel_format {
  u8_t type;
  u8_t pad;
  u16_t pad2;
  struct rfb_pixel_format format;
};

struct rfb_fix_colormap_entries {
  u8_t type;
  u8_t pad;
  u16_t firstcolor;
  u16_t colors;
};

struct rfb_set_encoding {
  u8_t type;
  u8_t pad;
  u16_t encodings;
};

struct rfb_fb_update_request {
  u8_t type;
  u8_t incremental;
  u16_t x;
  u16_t y;
  u16_t w;
  u16_t h;
};

struct rfb_key_event {
  u8_t type;
  u8_t down;
  u16_t pad;
  u8_t key[4];
};

#define RFB_BUTTON_MASK1 1
#define RFB_BUTTON_MASK2 2
#define RFB_BUTTON_MASK3 4
struct rfb_pointer_event {
  u8_t type;
  u8_t buttonmask;
  u16_t x;
  u16_t y;
};

struct rfb_client_cut_text {
  u8_t type;
  u8_t pad[3];
  u8_t len[4];
};

#endif /* __VNC_SERVER_H__ */
