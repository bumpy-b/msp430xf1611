/*
 * radio.h
 *
 *  Created on: 11/02/2011
 *      Author: Lior
 */

#ifndef RADIO_H_
#define RADIO_H_

struct radio_driver {
  /** Send a packet */
  int (* send)(const void *payload, unsigned short payload_len);

  /** Read a received packet into a buffer. */
  int (* read)(void *buf, unsigned short buf_len);

  /** Set a function to be called when a packet has been received. */
  void (* set_receive_function)(void (*f)(const struct radio_driver *d));

  /** Turn the radio on. */
  int (* on)(void);

  /** Turn the radio off. */
  int (* off)(void);
};

#endif /* RADIO_H_ */
