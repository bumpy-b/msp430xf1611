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
#define MAX_DATA 20
#define GET_LOCK() locked = 1
#define localID 200
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
static uint16_t pan_id;
static int channel;
static volatile uint8_t lock = 0;
static uint8_t map[4] = {0,0,0,0};
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

/* simple send function - sends the buffer of len
 * returns the number of bytes sent, or 0 if failed
 */

int cc2420_simplesend(uint8_t *buf,int len)
{
	int i;
	uint8_t id = localID;



	while (lock == 1) {}
	lock = 1;
	while (cc2420_status() & BV(CC2420_TX_ACTIVE)) {}
	strobe(CC2420_SRFOFF);
	strobe(CC2420_SFLUSHTX);


	FASTSPI_WRITE_FIFO(&len, 1);
	FASTSPI_WRITE_FIFO(&id,1);
	FASTSPI_WRITE_FIFO(buf,len);



	strobe(CC2420_STXON);

	for (i=0; i<20000; i++)
	{
		if (SFD_IS_1)
		{
//			printf("SFD is 1 !\n");
			lock = 0;
			return len;
		}
	}
    lock = 0;
	return 0;
}

static void getrxbyte(uint8_t *byte);
static void getrxdata(void *buf, int len);


/* simple recv function that will wait 50,000 cycles for
 * a mark that a packet is here
 */
int cc2420_simplerecv(uint8_t *buf,uint8_t *who)
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

	while (!FIFOP_IS_1)
	{
		i++;
		/* too much time */
		if (i>50000)
			return 0;
	}

	getrxbyte(&len);

//    printf("Got len %d\n",len);

	if (len >= 200)
	{
		/* this is ID */

		if (map[200 - len] == 0)
			printf("New device on network, ID %d\n",len);
		map[200 - len] = len;
		return 0;
	}
	getrxbyte(who);
	getrxdata(buf,len);

	return len;
}
/*---------------------------------------------------------------------------*/
/* starts the cc2420 - after init was done */
int cc2420_on(void) {
	/* are we already on ? */
	if (receive_on) {
			return 1;
	}

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
static void
getrxdata(void *buf, int len)
{
  FASTSPI_READ_FIFO_NO_WAIT(buf, len);
}
static void
getrxbyte(uint8_t *byte)
{
  FASTSPI_READ_FIFO_BYTE(*byte);
}

void CC2420_printMap()
{
	int i;
	printf("\n\nKnown CC2420 devices: \n");
    printf("Local Device ID: %d\n",localID);
	for (i=0; i<4; i++)
	{
		if (map[i]>0)
			printf("Device ID: %d\n",map[i]);
	}
}

void cc2420_sendID(uint8_t id)
{
	int i;



	while (lock == 1) {}
	lock = 1;

	while (cc2420_status() & BV(CC2420_TX_ACTIVE)) {}
	strobe(CC2420_SRFOFF);
	strobe(CC2420_SFLUSHTX);


	FASTSPI_WRITE_FIFO(&id, 1);
	FASTSPI_WRITE_FIFO(&id, 1);
	FASTSPI_WRITE_FIFO(&id, 1);



	strobe(CC2420_STXON);

	for (i=0; i<20000; i++)
	{
		if (SFD_IS_1)
		{
			lock = 0;
			return;
		}
	}
    lock = 0;
	return;
}
uint8_t cc2420_getID()
{
	return localID;
}
