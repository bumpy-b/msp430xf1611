#include "cc2420.h"
#include "FreeRTOS.h"
#include "spi.h"
#include "radio.h"
#include "msp430def.h"
#include "packebuf.h"
#include "mystdio.h"


/*---------------------------------------------------------------------------*/
static void (* receiver_callback)(const struct radio_driver *);

void cc2420_arch_init(void);

int cc2420_on(void);
int cc2420_off(void);

int cc2420_read(void *buf, unsigned short bufsize);

int cc2420_send(const void *data, unsigned short len);

void cc2420_set_receiver(void(* recv)(const struct radio_driver *d));
/*---------------------------------------------------------------------------*/

/************************<<< DEFINITION AND TYPES >>>**********************/
#define AUTOACK (1 << 4)
#define ADR_DECODE (1 << 11)
#define RXFIFO_PROTECTION (1 << 9)
#define CORR_THR(n) (((n) & 0x1f) << 6)
#define FIFOP_THR(n) ((n) & 0x7f)
#define RXBPF_LOCUR (1 << 13);

#define WITH_SEND_CCA 0
#define TIMESTAMP_LEN 3
#define FOOTER_LEN 2	// the 2 last bytes of a frame - FCS - Frame Check Sequence
#define CHECKSUM_LEN 0	// no check sum
#define FOOTER1_CRC_OK      0x80	// the first bit of the last byte of a frame - CRC/Corr
#define FOOTER1_CORRELATION 0x7f	// the 7 last bits of the last byte of a frame - CRC/Corr
#define AUX_LEN (CHECKSUM_LEN + TIMESTAMP_LEN + FOOTER_LEN)

#define LOOP_20_SYMBOLS 400	/* 326us (msp430 @ 2.4576MHz) */

struct timestamp {
	uint16_t time;
	uint8_t authority_level;
};

struct rimestats {
	unsigned long tx, rx;
	unsigned long reliabletx, reliablerx, rexmit, acktx, noacktx, ackrx,
			timedout, badackrx;
	unsigned long toolong, tooshort, badsynch, badcrc; /* Reasons for dropping incoming packets: */
	unsigned long contentiondrop; /* Packet dropped due to contention */
	unsigned long sendingdrop; /* Packet dropped when we were sending a packet */
	unsigned long lltx, llrx;
};

enum energest_type {
	//ENERGEST_TYPE_CPU,
	//ENERGEST_TYPE_LPM,
	//ENERGEST_TYPE_IRQ,
	//ENERGEST_TYPE_LED_GREEN,
	//ENERGEST_TYPE_LED_YELLOW,
	//ENERGEST_TYPE_LED_RED,
	ENERGEST_TYPE_TRANSMIT,
	ENERGEST_TYPE_LISTEN,

	//ENERGEST_TYPE_FLASH_READ,
	//ENERGEST_TYPE_FLASH_WRITE,

	//ENERGEST_TYPE_SENSORS,

	//ENERGEST_TYPE_SERIAL,

	ENERGEST_TYPE_MAX
};

typedef struct {
	//unsigned long cumulative[2];
	unsigned long current;
} energest_t;
/**************************************************************************/

/***************************<<< MACROS DEFINITION >>>**********************/
#define RIMESTATS_ADD(x) rimestats.x++
#define rtimer_arch_now() (TAR)
#define RTIMER_NOW() rtimer_arch_now()

#define ENERGEST_ON(type)  do { \
                           /*++energest_total_count;*/ \
                           energest_current_time[type] = RTIMER_NOW(); \
                           energest_current_mode[type] = 1; \
                           } while(0)

#define ENERGEST_OFF(type) do { \
                           energest_total_time[type].current += (uint8_t)(RTIMER_NOW() - \
                           energest_current_time[type]); \
                           energest_current_mode[type] = 0; \
                           } while(0)
/**************************************************************************/

/*********************<<< MODULE INTERNAL VARIABELS >>>********************/
/* XXX hack: these will be made as Chameleon packet attributes */
// the time stamp of the last packet
uint8_t cc2420_time_of_arrival, cc2420_time_of_departure;

static uint8_t setup_time_for_transmission;
static unsigned long total_time_for_transmission, total_transmission_len;
static int num_transmissions;

struct rimestats rimestats;

int cc2420_authority_level_of_sender;

int energest_total_count;

energest_t energest_total_time[ENERGEST_TYPE_MAX];
uint8_t energest_current_time[ENERGEST_TYPE_MAX];
uint8_t energest_current_mode[ENERGEST_TYPE_MAX];

signed char cc2420_last_rssi;
uint8_t cc2420_last_correlation;

const struct radio_driver cc2420_driver = { cc2420_send, cc2420_read,
		cc2420_set_receiver, cc2420_on, cc2420_off, };

static uint8_t receive_on;

/* Radio stuff in network byte order. */
static uint16_t pan_id;

static int channel;

static uint8_t rxptr; /* Pointer to the next byte in the rxfifo. */

static uint8_t locked, lock_on, lock_off;
/**************************************************************************/

/*---------------------------------------------------------------------------*/
//TODO// PROCESS(cc2420_process, "CC2420 driver");

//TODO - added for compilation://
void clock_delay(unsigned int i) {
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int authority_level;
static uint8_t offset;

int timesynch_authority_level(void) {
	return authority_level;
}

uint8_t timesynch_time(void) {
	return rtimer_arch_now() + offset;
}

// pins selects and directions between the cc2420 and msp430
void cc2420_arch_init(void) {
	spi_init();

	/* all input by default, set these as output */
	P4DIR |= BV(CSN) | BV(VREG_EN) | BV(RESET_N);

	CC2420_DISABLE(); /* Unselect radio. */
}
/*---------------------------------------------------------------------------*/
/****************************************************************************/
/*							<<< OPERATIONS >>>								*/
/****************************************************************************/
// read the rx "len" first bytes from RX fifo to "buf" */
// the cc2420 RX fifo pointer will proceed automatically */
static void getrxdata(void *buf, int len) {
	FASTSPI_READ_FIFO_NO_WAIT(buf, len);
	rxptr = (rxptr + len) & 0x7f;
}

// read the rx fifo first byte at rxptr address and proceed the rxptr
static void getrxbyte(uint8_t *byte) {
	FASTSPI_READ_FIFO_BYTE(*byte);
	rxptr = (rxptr + 1) & 0x7f;
}

// flush RX FIFO (when underflow occurs)
static void flushrx(void) {
	uint8_t dummy;

	FASTSPI_READ_FIFO_BYTE(dummy);
	FASTSPI_STROBE(CC2420_SFLUSHRX);
	FASTSPI_STROBE(CC2420_SFLUSHRX);
	rxptr = 0;
}
/*---------------------------------------------------------------------------*/
// send strobe command named by "enum cc2420_register"
static void strobe(enum cc2420_register regname) {
	FASTSPI_STROBE(regname);
}
/*---------------------------------------------------------------------------*/
// get cc2420 status register
uint8_t cc2420_status(void) {
	uint8_t status;
	FASTSPI_UPD_STATUS(status);
	return status;
}
/*---------------------------------------------------------------------------*/
static void on(void) {
	ENERGEST_ON(ENERGEST_TYPE_LISTEN);
	printf("on\n");
	receive_on = 1;

	ENABLE_FIFOP_INT();
	strobe(CC2420_SRXON);
	flushrx();
}

static void off(void) {
	printf("off\n");
	receive_on = 0;

	/* Wait for transmission to end before turning radio off. */
	while (cc2420_status() & BV(CC2420_TX_ACTIVE))
		;

	// turning the RF off page 43
	strobe(CC2420_SRFOFF);
	DISABLE_FIFOP_INT();
	ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
}
/*---------------------------------------------------------------------------*/
#define GET_LOCK() locked = 1
static void RELEASE_LOCK(void) {
	if (lock_on) {
		on();
		lock_on = 0;
	}
	if (lock_off) {
		off();
		lock_off = 0;
	}
	locked = 0;
}
/*---------------------------------------------------------------------------*/
// return the value of register "ragname" from the cc2420
static unsigned getreg(enum cc2420_register regname) {
	unsigned reg;
	FASTSPI_GETREG(regname, reg);
	return reg;
}
/*---------------------------------------------------------------------------*/
// set the value of register "ragname" in the cc2420 to "value"
static void setreg(enum cc2420_register regname, unsigned value) {
	FASTSPI_SETREG(regname, value);
}
/*---------------------------------------------------------------------------*/
void cc2420_set_receiver(void(* recv)(const struct radio_driver *)) {
	receiver_callback = recv;
}
/*---------------------------------------------------------------------------*/
void cc2420_init(void) {
	uint16_t reg;
	{
		int s = splhigh();
		cc2420_arch_init(); /* Initalize ports and SPI. */
		DISABLE_FIFOP_INT(); /* disable interrupts ftom FIFOP */
		FIFOP_INT_INIT(); /* init the pin registers */
		splx(s);
	}

	/* Turn on voltage regulator and reset. */
	SET_VREG_ACTIVE();
	//clock_delay(250); OK
	SET_RESET_ACTIVE();
	clock_delay(127);
	SET_RESET_INACTIVE();
	//clock_delay(125); OK


	/* Turn on the crystal oscillator. */
	strobe(CC2420_SXOSCON);

	/* Turn off automatic packet acknowledgment. */
	reg = getreg(CC2420_MDMCTRL0);
	reg &= ~AUTOACK;
	setreg(CC2420_MDMCTRL0, reg);

	/* Turn off address decoding. */
	reg = getreg(CC2420_MDMCTRL0);
	reg &= ~ADR_DECODE;
	setreg(CC2420_MDMCTRL0, reg);

	/* Change default values as recomended in the data sheet, */
	/* correlation threshold = 20 - threshold for detecting IEEE 804.15.4 Stard of Fram Delimiter (SFD)
	 * RX bandpass filter = 1.3uA. */
	setreg(CC2420_MDMCTRL1, CORR_THR(20));
	reg = getreg(CC2420_RXCTRL1);
	reg |= RXBPF_LOCUR;
	setreg(CC2420_RXCTRL1, reg);

	/* Set the FIFOP threshold to maximum. */
	setreg(CC2420_IOCFG0, FIFOP_THR(127));

	/* Turn off "Security enable" (page 32). */
	reg = getreg(CC2420_SECCTRL0);
	reg &= ~RXFIFO_PROTECTION;
	setreg(CC2420_SECCTRL0, reg);

	cc2420_set_pan_addr(0xffff, 0x0000, NULL);
	cc2420_set_channel(26);

	//TODO//  process_start(&cc2420_process, NULL);
}
/*---------------------------------------------------------------------------*/
/*
 * Interrupt leaves frame intact in FIFO.
 */

static volatile uint8_t interrupt_time;
static volatile int interrupt_time_set;

// FIFOP Interrupt accrued: remember that there is a new packet and it's time stamp
int cc2420_interrupt(void) {
	interrupt_time = timesynch_time(); /* packet time stamp */
	interrupt_time_set = 1; /* remember that there is a new packet */

	CLEAR_FIFOP_INT(); /* clear the interrupts flag */
	//TODO - process the new packet//   process_poll(&cc2420_process);

	return 1;
}
/*---------------------------------------------------------------------------*/
//TODO//  change PROCESS_THREAD to something else
PROCESS_THREAD( cc2420_process, ev, data) {
	//TODO//   PROCESS_BEGIN();

	printf("cc2420_process: started\n");

	while (1) {
		//TODO//     PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

		if (receiver_callback != NULL) {
			printf("cc2420_process: calling receiver callback\n");
			receiver_callback(&cc2420_driver);

		} else {
			printf("cc2420_process not receiving function\n");
			flushrx();
		}
	}

	//TODO//   PROCESS_END();
}
/*---------------------------------------------------------------------------*/
int cc2420_send(const void *payload, unsigned short payload_len) {
	int i;
	uint8_t total_len;
	struct timestamp timestamp;

	GET_LOCK();

	if (packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) > 0) {
		cc2420_set_txpower(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) - 1);
	} else {
		cc2420_set_txpower(CC2420_TXPOWER_MAX);
	}

	printf("cc2420: sending %d bytes\n", payload_len);

	RIMESTATS_ADD(lltx);

	/* Wait for any previous transmission to finish. */
	while (cc2420_status() & BV(CC2420_TX_ACTIVE))
		;

	/* Write packet to TX FIFO:
	 * flush
	 * length
	 * payload
	 */
	strobe(CC2420_SFLUSHTX);
	total_len = payload_len + AUX_LEN;
	FASTSPI_WRITE_FIFO(&total_len, 1);
	FASTSPI_WRITE_FIFO(payload, payload_len);

	timestamp.authority_level = timesynch_authority_level();
	timestamp.time = timesynch_time();
	FASTSPI_WRITE_FIFO(&timestamp, TIMESTAMP_LEN);

	/* The TX FIFO can only hold one packet. Make sure to not overrun
	 * FIFO by waiting for transmission to start here and synchronizing
	 * with the CC2420_TX_ACTIVE check in cc2420_send.
	 *
	 * Note that we may have to wait up to 320 us (20 symbols) before
	 * transmission starts.
	 */

	// enable starting transmission
	strobe(CC2420_STXON);

	for (i = LOOP_20_SYMBOLS; i > 0; i--) {
		// the SFD bit will rise up when the Start of Frame Delimiter
		// sent OK, that indicate that the frame is being sending right now
		// this bit must be raised maximum after LOOP_20_SYMBOLS -
		// the preamble (8 symbols) is started 12 symbols period after the command strobe.
		if (SFD_IS_1) {
			uint8_t txtime = timesynch_time();
			if (receive_on) {
				ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
			}
			ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

			/* We wait until transmission has ended so that we get an
			 accurate measurement of the transmission time.*/
			while (cc2420_status() & BV(CC2420_TX_ACTIVE));

			setup_time_for_transmission = txtime - timestamp.time;

			if (num_transmissions < 10000) {
				total_time_for_transmission += timesynch_time() - txtime;
				total_transmission_len += total_len;
				num_transmissions++;
			}

			ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
			if (receive_on) {
				ENERGEST_ON(ENERGEST_TYPE_LISTEN);
			}

			RELEASE_LOCK();
			return 0;
		}
	}

	/* If we are using WITH_SEND_CCA, we get here if the packet wasn't
	 transmitted because of other channel activity. */
	RIMESTATS_ADD(contentiondrop);
	printf("cc2420: do_send() transmission never started\n");
	RELEASE_LOCK();
	return -3; /* Transmission never started! */
}
/*---------------------------------------------------------------------------*/
// read bufsize bytes to buf from the RX FIFO of the cc2420
int cc2420_read(void *buf, unsigned short bufsize) {
	uint8_t footer[2];
	uint8_t len;
	struct timestamp t;

	if (!FIFOP_IS_1) {
		/* If FIFOP is 0, there is no packet in the RXFIFO. */
		return 0;
	}

	if (interrupt_time_set) {
		cc2420_time_of_arrival = interrupt_time;
		interrupt_time_set = 0;
	} else {
		cc2420_time_of_arrival = 0;
	}
	cc2420_time_of_departure = 0;

	GET_LOCK();

	// the first byte in a packet, it it's length
	getrxbyte(&len);

	if (len > CC2420_MAX_PACKET_LEN) {
		/* Oops, we must be out of sync. */
		flushrx();
		RIMESTATS_ADD(badsynch); /* log reason of dropping */
		RELEASE_LOCK();
		return 0;
	}

	if (len <= AUX_LEN) {
		flushrx();
		RIMESTATS_ADD(tooshort); /* log reason of dropping */
		RELEASE_LOCK();
		return 0;
	}

	if (len - AUX_LEN > bufsize) {
		/* the packet is too long for our buffer */
		flushrx();
		RIMESTATS_ADD(toolong); /* log reason of dropping */
		RELEASE_LOCK();
		return 0;
	}

	getrxdata(buf, len - AUX_LEN);
	getrxdata(&t, TIMESTAMP_LEN);
	getrxdata(footer, FOOTER_LEN);

	if (footer[1] & FOOTER1_CRC_OK) {

		cc2420_last_rssi = footer[0];
		cc2420_last_correlation = footer[1] & FOOTER1_CORRELATION;

		packetbuf_set_attr(PACKETBUF_ATTR_RSSI, cc2420_last_rssi);
		packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, cc2420_last_correlation);

		RIMESTATS_ADD(llrx);

		cc2420_time_of_departure = t.time + setup_time_for_transmission
				+ (total_time_for_transmission * (len - 2))
						/ total_transmission_len;

		cc2420_authority_level_of_sender = t.authority_level;

		packetbuf_set_attr(PACKETBUF_ATTR_TIMESTAMP, t.time);

	} else {
		RIMESTATS_ADD(badcrc);
		len = AUX_LEN;
	}

	/* Clean up in case of FIFO overflow!  This happens for every:
	 *  full length frame and is signaled by -
	 *  FIFOP = 1 and FIFO = 0.
	 */
	if (FIFOP_IS_1 && !FIFO_IS_1) {
		/*    printf("cc2420_read: FIFOP_IS_1 1\n");*/
		flushrx();
	} else if (FIFOP_IS_1) {
		/* Another packet has been received and needs attention. */
		//TODO//  process_poll(&cc2420_process);
	}

	RELEASE_LOCK();

	if (len < AUX_LEN) {
		return 0;
	}

	return len - AUX_LEN;
}
/*---------------------------------------------------------------------------*/
void cc2420_set_txpower(uint8_t power) {
	uint16_t reg;

	GET_LOCK();
	reg = getreg(CC2420_TXCTRL);
	reg = (reg & 0xffe0) | (power & 0x1f);
	setreg(CC2420_TXCTRL, reg);
	RELEASE_LOCK();
}
/*---------------------------------------------------------------------------*/
int cc2420_off(void) {
	/* Don't do anything if we are already turned off. */
	if (receive_on == 0) {
		return 1;
	}

	/* If we are called when the driver is locked, we indicate that the
	 radio should be turned off when the lock is unlocked. */
	if (locked) {
		lock_off = 1;
		return 1;
	}

	/* If we are currently receiving a packet (indicated by SFD == 1),
	 we don't actually switch the radio off now, but signal that the
	 driver should switch off the radio once the packet has been
	 received and processed, by setting the 'lock_off' variable. */
	if (SFD_IS_1) {
		lock_off = 1;
		return 1;
	}

	off();
	return 1;
}
/*---------------------------------------------------------------------------*/
int cc2420_on(void) {
	if (receive_on) {
		return 1;
	}
	if (locked) {
		lock_on = 1;
		return 1;
	}

	on();
	return 1;
}
/*---------------------------------------------------------------------------*/
int cc2420_get_channel(void) {
	return channel;
}
/*---------------------------------------------------------------------------*/
void cc2420_set_channel(int c) {
	uint16_t f;
	/*
	 * Subtract the base channel (11), multiply by 5, which is the
	 * channel spacing. 357 is 2405-2048 and 0x4000 is LOCK_THR = 1.
	 */

	channel = c;

	f = 5 * (c - 11) + 357 + 0x4000;
	/*
	 * Writing RAM requires crystal oscillator to be stable.
	 */
	while (!(cc2420_status() & (BV(CC2420_XOSC16M_STABLE))))
		;

	/* Wait for any transmission to end. */
	while (cc2420_status() & BV(CC2420_TX_ACTIVE))
		;

	setreg(CC2420_FSCTRL, f);

	/* If we are in receive mode, we issue an SRXON command to ensure
	 that the VCO is calibrated. */
	if (receive_on) {
		strobe(CC2420_SRXON);
	}

}
/*---------------------------------------------------------------------------*/
void cc2420_set_pan_addr(unsigned pan, unsigned addr, const uint8_t *ieee_addr) {
	uint16_t f = 0;
	/*
	 * Writing RAM requires crystal oscillator to be stable.
	 */
	while (!(cc2420_status() & (BV(CC2420_XOSC16M_STABLE))))
		;

	pan_id = pan;
	FASTSPI_WRITE_RAM_LE(&pan, CC2420RAM_PANID, 2, f);
	FASTSPI_WRITE_RAM_LE(&addr, CC2420RAM_SHORTADDR, 2, f);
	if (ieee_addr != NULL) {
		FASTSPI_WRITE_RAM_LE(ieee_addr, CC2420RAM_IEEEADDR, 8, f);
	}
}
/*---------------------------------------------------------------------------*/
int cc2420_get_txpower(void) {
	return (int) (getreg(CC2420_TXCTRL) & 0x001f);
}
/*---------------------------------------------------------------------------*/
int cc2420_rssi(void) {
	int rssi;
	int radio_was_off = 0;

	if (!receive_on) {
		radio_was_off = 1;
		cc2420_on();
	}
	while (!(cc2420_status() & BV(CC2420_RSSI_VALID))) {
		/*    printf("cc2420_rssi: RSSI not valid.\n");*/
	}

	rssi = (int) ((signed char) getreg(CC2420_RSSI));

	if (radio_was_off) {
		cc2420_off();
	}
	return rssi;
}

