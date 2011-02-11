#ifndef CC2420_H_
#define CC2420_H_

#include "FreeRTOS.h"
#include "cc2420_const.h"

/* The CC2420 reset pin. */
#define SET_RESET_INACTIVE()    ( P4OUT |=  BV(RESET_N) )
#define SET_RESET_ACTIVE()      ( P4OUT &= ~BV(RESET_N) )

/* CC2420 voltage regulator enable pin. */
#define SET_VREG_ACTIVE()       ( P4OUT |=  BV(VREG_EN) )
#define SET_VREG_INACTIVE()     ( P4OUT &= ~BV(VREG_EN) )

/* CC2420 rising edge trigger for external interrupt 0 (FIFOP). */
#define FIFOP_INT_INIT() do {\
  P1IES &= ~BV(FIFO_P);\
  CLEAR_FIFOP_INT();\
} while (0)

/* FIFOP on external interrupt 0. */
#define ENABLE_FIFOP_INT()          do { P1IE |= BV(FIFO_P); } while (0)
#define DISABLE_FIFOP_INT()         do { P1IE &= ~BV(FIFO_P); } while (0)
#define CLEAR_FIFOP_INT()           do { P1IFG &= ~BV(FIFO_P); } while (0)

/* Enables/disables CC2420 access to the SPI bus (not the bus).
 *
 * These guys should really be renamed but are compatible with the
 * original Chipcon naming.
 *
 * SPI_CC2420_ENABLE/SPI_CC2420_DISABLE???
 * CC2420_ENABLE_SPI/CC2420_DISABLE_SPI???
 */
//--------------------------------------------------------------------

void cc2420_init(void);

#define CC2420_MAX_PACKET_LEN      127

void cc2420_set_channel(int channel);
int cc2420_get_channel(void);

void cc2420_set_pan_addr(unsigned pan,
				unsigned addr,
				const uint8_t *ieee_addr);

extern signed char cc2420_last_rssi;
extern uint8_t cc2420_last_correlation;

int cc2420_rssi(void);

extern const struct radio_driver cc2420_driver;

/**
 * \param power Between 1 and 31.
 */
void cc2420_set_txpower(uint8_t power);
int cc2420_get_txpower(void);
#define CC2420_TXPOWER_MAX  31
#define CC2420_TXPOWER_MIN   0

/**
 * Interrupt function, called from the simple-cc2420-arch driver.
 *
 */
int cc2420_interrupt(void);

/* XXX hack: these will be made as Chameleon packet attributes */
extern uint8_t cc2420_time_of_arrival, cc2420_time_of_departure;
extern int cc2420_authority_level_of_sender;

int cc2420_on(void);
int cc2420_off(void);




#endif /* CC2420_H_ */
