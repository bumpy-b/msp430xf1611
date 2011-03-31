#include "cc2420.h"
#include "FreeRTOS.h"
#include "spi.h"
#include "radio.h"
#include "msp430def.h"
#include "packebuf.h"
#include "mystdio.h"
#include "debugFunction.h"

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
#define GET_LOCK() locked = 1

/******************/
#define CC2420_IO_INIT() do \
{ \
    P1SEL &= ~(FIFOP_PIN | FIFO_PIN | SFD_PIN | CCA_PIN | RESETn_PIN); \
    P1DIR &= ~(FIFOP_PIN | FIFO_PIN | SFD_PIN | CCA_PIN); \
    P1DIR |= RESETn_PIN; \
    P1OUT &= ~RESETn_PIN; \
    P1IE  &= ~(FIFOP_PIN | FIFO_PIN | SFD_PIN | CCA_PIN | RESETn_PIN); \
    \
    P3SEL &= ~VREGEN_PIN; \
    P3DIR |= VREGEN_PIN; \
    P3OUT &= ~VREGEN_PIN; \
} while (0)
/*****************/

/* declarations */
static void flushrx(void);
void cc2420_set_pan_addr(unsigned pan, unsigned addr, const uint8_t *ieee_addr);
void cc2420_set_txpower(uint8_t power);
void cc2420_set_channel(int c);

/*-------------------------------------------*/

/* global vars */

static uint8_t receive_on;
static uint8_t locked, lock_on, lock_off;
static uint16_t pan_id;
static int channel;
/*------------------------------------------*/

/* simple clock delay */
void clock_delay(unsigned int i) {
	/* volatile int v = 0;
	volatile int j=0;
	for (j=0;j<i;j++)
	{
		v++;
	}*/
     //vTaskDelay(i);
	 asm("add #-1, r15");
	 asm("jnz $-2");
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
// send strobe command named by "enum cc2420_register"
static void strobe(enum cc2420_register regname) {
	FASTSPI_STROBE(regname);
}
// get cc2420 status register
uint8_t cc2420_status(void) {
	uint8_t status;
	FASTSPI_UPD_STATUS(status);
	return status;
}

/*---------------------------------------------------------------------------*/

unsigned cc2420_getstate()
{
	return getreg(CC2420_FSMSTATE & 0x3f);
}

/*---------------------------------------------------------------------------*/


/* Init function - reset the cc2420 component */
void cc2420_init(void) {
	uint16_t reg;
	{
		/* save GIE */
		int s = splhigh();
		spi_init();

		/* all input by default, set these as output */
		P4DIR |= BV(CSN) | BV(VREG_EN) | BV(RESET_N);

		CC2420_DISABLE();            /* Unselect radio. */

		DISABLE_FIFOP_INT();         /* disable interrupts from FIFOP */
		FIFOP_INT_INIT();            /* init the pin registers */
		splx(s);                     /* restore GIE */
	}
	/* Turn on voltage regulator and reset. */
	SET_VREG_ACTIVE();
	clock_delay(250);
	SET_RESET_ACTIVE();
	clock_delay(127);
	SET_RESET_INACTIVE();
	clock_delay(125);

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

	/* This value is from the cc2420 data sheet */
//	setreg(CC2420_MDMCTRL1, CORR_THR(20));

	reg = getreg(CC2420_RXCTRL1);
	/* should be set to 1 by cc2420 data sheet */
	reg |= RXBPF_LOCUR;
	setreg(CC2420_RXCTRL1, reg);

	/* Set the FIFOP threshold to maximum. */
	setreg(CC2420_IOCFG0, FIFOP_THR(127));

	/* Turn off "Security enable" (page 32). */
	reg = getreg(CC2420_SECCTRL0);
	reg &= ~RXFIFO_PROTECTION;
	setreg(CC2420_SECCTRL0, reg);

	/* set the pan address */
//	cc2420_set_pan_addr(0xffff, 0x0000, NULL);

	/* set channel */
//	cc2420_set_channel(12);

}
int cc2420_simplesend()
{
	uint8_t total_len = 1;
	int i;
	char frame[10] = "hello";

	//while (cc2420_status() & CC2420_TX_ACTIVE) {}

	strobe(CC2420_SRFOFF);
	strobe(CC2420_SFLUSHTX);


	FASTSPI_WRITE_FIFO(&total_len, 6);
	FASTSPI_WRITE_FIFO(frame,6);

	strobe(CC2420_STXON);

	for (i=0; i<1000; i++)
	{
		if (SFD_IS_1)
		{
			printf("send: SFD is 1 !!!!!!!!!! \n");
		}
	}
}

static void getrxbyte(uint8_t *byte);

int cc2420_simplerecv()
{
	uint8_t len = 0;

	volatile long i =0;
	//P1IES |= FIFOP_P;
	/* clear SFD flag */
	//P1IFG &= ~SFD;

	/* int enable for SFD */
	//P1IE |= SFD;
	strobe(CC2420_SRFOFF);
	strobe(CC2420_SFLUSHRX);
	strobe(CC2420_SRXON);
	printf("Waiting for packet ...\n");
	while (!FIFOP_IS_1)
	{
		i++;
		if (i>5000)
			return 0;
	}
	printf("Got a packet !!!\n");
//	getrxbyte(&len);

	return len;
}
/*---------------------------------------------------------------------------*/
/* starts the cc2420 - after init was done */
int cc2420_on(void) {
	/* are we already on ? */
	if (receive_on) {
			return 1;
	}

	printf("on\n");

	/* enable fifop interrupts */

	// TODO: check how to handle interrupts

//	ENABLE_FIFOP_INT();
	/* start receive mode */
	strobe(CC2420_SRXON);
	//strobe(CC2420_SRFOFF);
	/* flush the rx */
	flushrx();

	return 1;
}

/* kill the cc2420 */
int cc2420_off(void) {

	/* already off ? */
	if (receive_on == 0) {
		return 1;
	}

	printf("off\n");

	/* Wait for transmission to end before turning radio off. */
	while (cc2420_status() & BV(CC2420_TX_ACTIVE))
		;
	// turning the RF off page 43
	strobe(CC2420_SRFOFF);

	DISABLE_FIFOP_INT();

	return 1;
}
void cc2420_printState()
{
	static volatile unsigned lastState;
	//if (lastState != cc2420_getstate())
		printf("state is %d, status is %x\n",cc2420_getstate(),cc2420_status());
	lastState = cc2420_getstate();

}
/*---------------------------------------------------------------------------*/
int cc2420_send(const void *payload, unsigned short payload_len) {
	int i;
	uint8_t total_len;

	printf("cc2420: sending %d bytes\n", payload_len);



	/* set TX power to max */
	cc2420_set_txpower(CC2420_TXPOWER_MAX);

	/* Wait for any previous transmission to finish. */
	while (cc2420_status() & BV(CC2420_TX_ACTIVE));

	/* Write packet to TX FIFO:
	 * flush
	 * length
	 * payload
	 */
	strobe(CC2420_SFLUSHTX);
	cc2420_printState();
	total_len = payload_len;
	FASTSPI_WRITE_FIFO(&total_len, 1);
	cc2420_printState();
	FASTSPI_WRITE_FIFO(payload, payload_len);
	cc2420_printState();

	/* The TX FIFO can only hold one packet. Make sure to not overrun
	 * FIFO by waiting for transmission to start here and synchronizing
	 * with the CC2420_TX_ACTIVE check in cc2420_send.
	 *
	 * Note that we may have to wait up to 320 us (20 symbols) before
	 * transmission starts.
	 */

	// enable starting transmission
 	strobe(CC2420_STXON);
	cc2420_printState();

	vTaskDelay(100);
	/* now we wait for SFD to start */

	for (i = LOOP_20_SYMBOLS; i > 0; i--) {


		//clock_delay(1);
		//cc2420_printState();
		// the SFD bit will rise up when the Start of Frame Delimiter
		// sent OK, that indicate that the frame is being sending right now
		// this bit must be raised maximum after LOOP_20_SYMBOLS -
		// the preamble (8 symbols) is started 12 symbols period after the command strobe.
		if (SFD_IS_1) {
			debugState(STATE5);
			printf("SFD is 1\n");
			cc2420_printState();
			/* We wait until transmission has ended so that we get an
			 accurate measurement of the transmission time.*/
			while (cc2420_status() & BV(CC2420_TX_ACTIVE)) {}

			return 1;
		}
	}

	/* If we are using WITH_SEND_CCA, we get here if the packet wasn't
	 transmitted because of other channel activity. */

	printf("cc2420: do_send() transmission never started\n");

	return -1; /* Transmission never started! */
}
/*---------------------------------------------------------------------------*/
// flush RX FIFO (when underflow occurs)
static void flushrx(void) {
	uint8_t dummy;

	FASTSPI_READ_FIFO_BYTE(dummy);
	FASTSPI_STROBE(CC2420_SFLUSHRX);
	FASTSPI_STROBE(CC2420_SFLUSHRX);
}

void cc2420_set_pan_addr(unsigned pan, unsigned addr, const uint8_t *ieee_addr) {
	uint16_t f = 0;
	/*
	 * Writing RAM requires crystal oscillator to be stable.
	 */
	while (!(cc2420_status() & (BV(CC2420_XOSC16M_STABLE))))
		;

	/* save the global pan_id */
	pan_id = pan;
	FASTSPI_WRITE_RAM_LE(&pan, CC2420RAM_PANID, 2, f);
	FASTSPI_WRITE_RAM_LE(&addr, CC2420RAM_SHORTADDR, 2, f);
	if (ieee_addr != NULL) {
		FASTSPI_WRITE_RAM_LE(ieee_addr, CC2420RAM_IEEEADDR, 8, f);
	}
}

void cc2420_set_txpower(uint8_t power) {
	uint16_t reg;

	reg = getreg(CC2420_TXCTRL);
	reg = (reg & 0xffe0) | (power & 0x1f);
	setreg(CC2420_TXCTRL, reg);

}
void cc2420_set_channel(int c) {
	uint16_t f;
	/*
	 * Subtract the base channel (11), multiply by 5, which is the
	 * channel spacing.
	 */

	/* save global channel */
	channel = c;

	f = 5 * (c - 11) + 357;
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

// read the rx fifo first byte at rxptr address and proceed the rxptr
static void getrxbyte(uint8_t *byte) {
	FASTSPI_READ_FIFO_BYTE(*byte);
}

//TODO: fix me

int cc2420_read(void *buf, unsigned short bufsize) {
	uint8_t len;


	if (!FIFOP_IS_1) {
		/* If FIFOP is 0, there is no packet in the RXFIFO. */
		return 0;
	}

	// the first byte in a packet, it it's length
	getrxbyte(&len);


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


	return len;
}
