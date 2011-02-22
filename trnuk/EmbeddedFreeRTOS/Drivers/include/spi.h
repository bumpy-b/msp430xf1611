#ifndef RTOS_SPI_H
#define RTOS_SPI_H


/**************************<<< FROM CONTIKI: contiki-conf.h >>>******************************/
/*
 * SPI bus - CC2420 pin configuration.
 */

#define FIFO_P         0  /* P1.0 - Input: FIFOP from CC2420 */
#define FIFO           3  /* P1.3 - Input: FIFO from CC2420 */
#define CCA            4  /* P1.4 - Input: CCA from CC2420 */

#define SFD            1  /* P4.1 - Input:  SFD from CC2420 */
#define CSN            2  /* P4.2 - Output: SPI Chip Select (CS_N) */
#define VREG_EN        5  /* P4.5 - Output: VREG_EN to CC2420 */
#define RESET_N        6  /* P4.6 - Output: RESET_N to CC2420 */

/* Pin status. */

#define FIFO_IS_1       (!!(P1IN & BV(FIFO)))
#define CCA_IS_1        (!!(P1IN & BV(CCA) ))
#define RESET_IS_1      (!!(P4IN & BV(RESET_N)))
#define VREG_IS_1       (!!(P4IN & BV(VREG_EN)))
#define FIFOP_IS_1      (!!(P1IN & BV(FIFO_P)))
#define SFD_IS_1        (!!(P4IN & BV(SFD)))


/* SPI input/output registers. */
#define SPI_TXBUF U0TXBUF
#define SPI_RXBUF U0RXBUF

				/* USART0 Tx buffer ready? */
#define	SPI_WAITFOREOTx() while ((U0TCTL & TXEPT) == 0)
				/* USART0 Rx buffer ready? */
#define	SPI_WAITFOREORx() while ((IFG1 & URXIFG0) == 0)

#define SCK            1  /* P3.1 - Output: SPI Serial Clock (SCLK) */
#define MOSI           2  /* P3.2 - Output: SPI Master out - slave in (MOSI) */
#define MISO           3  /* P3.3 - Input:  SPI Master in - slave out (MISO) */

#define SPI_ENABLE()    ( P4OUT &= ~BV(CSN) ) /* ENABLE CSn (active low) */
#define SPI_DISABLE()   ( P4OUT |=  BV(CSN) ) /* DISABLE CSn (active low) */
/**************************************************************************************/

extern unsigned char spi_busy;

void spi_init(void);

/******************************************************************************************************
*  TEXAS INSTRUMENTS INC.,                                                                            *
*  MSP430 APPLICATIONS.                                                                               *
*  Copyright Texas Instruments Inc, 2004                                                              *
 *****************************************************************************************************/

/***********************************************************
	FAST SPI: Low level functions
***********************************************************/
// 	  p = pointer to the byte array to be read/written
// 	  c = the number of bytes to read/write
// 	  b = single data byte
// 	  a = register address


// writing byte to the spi(uart0) TX buffer
// the spi(uart0) TX buffer will transmit this byte
// the FASTSPI_TX will wait until the buffer be ready again
#define FASTSPI_TX(x)\
	do {\
		SPI_TXBUF = x;\
		SPI_WAITFOREOTx();\
	} while(0)

// reading byte from the spi(uart0) RX buffer
// the FASTSPI_RX will wait until the buffer be ready again
#define FASTSPI_RX(x)\
    do {\
        SPI_TXBUF = 0;\
	    SPI_WAITFOREORx();\
		x = SPI_RXBUF;\
    } while(0)

// reading byte from the spi(uart0) RX buffer to make it empty
#define FASTSPI_CLEAR_RX(x) do{ SPI_RXBUF; }while(0)

#define FASTSPI_RX_GARBAGE()\
	do {\
    	SPI_TXBUF = 0;\
		SPI_WAITFOREORx();\
		(void)SPI_RXBUF;\
	} while(0)

// writing byte array "p" sized "c" to the spi(uart0) TX buffer
// byte by byte
#define FASTSPI_TX_MANY(p,c)\
	do {\
        u8_t spiCnt;\
        for (spiCnt = 0; spiCnt < (c); spiCnt++) {\
			FASTSPI_TX(((u8_t*)(p))[spiCnt]);\
		}\
	} while(0)

// reading 2 bytes from the spi(uart0) RX buffer
// byte by byte
#define FASTSPI_RX_WORD(x)\
	 do {\
	    SPI_TXBUF = 0;\
        SPI_WAITFOREORx();\
		x = SPI_RXBUF << 8;\
	    SPI_TXBUF = 0;\
		SPI_WAITFOREORx();\
		x |= SPI_RXBUF;\
    } while (0)

// same as FASTSPI_TX
#define FASTSPI_TX_ADDR(a)\
	 do {\
		  SPI_TXBUF = a;\
		  SPI_WAITFOREOTx();\
	 } while (0)


#define FASTSPI_RX_ADDR(a)\
	 do {\
		  SPI_TXBUF = (a) | 0x40;\
		  SPI_WAITFOREOTx();\
	 } while (0)



/***********************************************************
	FAST SPI: Register access
***********************************************************/
// 	  s = command strobe
// 	  a = register address
// 	  v = register value

// the strobe need only one byte - address, no data is needed
// the command s is actually the address
#define FASTSPI_STROBE(s) \
    do {\
		  SPI_ENABLE();\
		  FASTSPI_TX_ADDR(s);\
		  SPI_DISABLE();\
    } while (0)

// set register in address "a"(one byte address) to value "v"(one byte value)
#define FASTSPI_SETREG(a,v)\
	 do {\
		  SPI_ENABLE();\
		  FASTSPI_TX_ADDR(a);\
		  FASTSPI_TX((u8_t) ((v) >> 8));\
		  FASTSPI_TX((u8_t) (v));\
		  SPI_DISABLE();\
	 } while (0)

// get the value stored in register at address "a"(one byte address)
// to the "v" argument ("v" is 2 byte length)
#define FASTSPI_GETREG(a,v)\
	 do {\
		  SPI_ENABLE();\
		  FASTSPI_RX_ADDR(a);\
		  v= (u8_t)SPI_RXBUF;\
		  FASTSPI_RX_WORD(v);\
		  clock_delay(1);\
		  SPI_DISABLE();\
	 } while (0)

// Updates the SPI status byte
// in every access to the cc2420 we by get back the status register
// so, we send nop and as in every other command we get the status register back
#define FASTSPI_UPD_STATUS(s)\
	 do {\
		  SPI_ENABLE();\
		  SPI_TXBUF = CC2420_SNOP;\
		  SPI_WAITFOREOTx();\
		  s = SPI_RXBUF;\
		  SPI_DISABLE();\
	 } while (0)

/***********************************************************
	FAST SPI: FIFO Access
***********************************************************/
// 	  p = pointer to the byte array to be read/written
// 	  c = the number of bytes to read/write
// 	  b = single data byte

// write to the tx fifo the array "p" size "c"
// the cc2420 will transmit it by RF
#define FASTSPI_WRITE_FIFO(p,c)\
	do {\
	    SPI_ENABLE();\
		u8_t i;\
		FASTSPI_TX_ADDR(CC2420_TXFIFO);\
		for (i = 0; i < (c); i++) {\
		    FASTSPI_TX(((u8_t*)(p))[i]);\
		}\
		SPI_DISABLE();\
    } while (0)

// same as FASTSPI_WRITE_FIFO except that no CS - chip select
// is change to low and then to high
#define FASTSPI_WRITE_FIFO_NOCE(p,c)\
	do {\
		FASTSPI_TX_ADDR(CC2420_TXFIFO);\
		for (u8_t spiCnt = 0; spiCnt < (c); spiCnt++) {\
		    FASTSPI_TX(((u8_t*)(p))[spiCnt]);\
		}\
    } while (0)

// read the first byte in the rx fifo to variable "b"
#define FASTSPI_READ_FIFO_BYTE(b)\
	 do {\
		  SPI_ENABLE();\
		  FASTSPI_RX_ADDR(CC2420_RXFIFO);\
		  (void)SPI_RXBUF;\
		  FASTSPI_RX(b);\
  		  clock_delay(1);\
		  SPI_DISABLE();\
	 } while (0)


#define FASTSPI_READ_FIFO_NO_WAIT(p,c)\
	 do {\
		  u8_t spiCnt;\
		  SPI_ENABLE();\
		  FASTSPI_RX_ADDR(CC2420_RXFIFO);\
		  (void)SPI_RXBUF;\
		  for (spiCnt = 0; spiCnt < (c); spiCnt++) {\
				FASTSPI_RX(((u8_t*)(p))[spiCnt]);\
		  }\
		  clock_delay(1);\
		  SPI_DISABLE();\
	 } while (0)



#define FASTSPI_READ_FIFO_GARBAGE(c)\
	 do {\
		  u8_t spiCnt;\
		  SPI_ENABLE();\
		  FASTSPI_RX_ADDR(CC2420_RXFIFO);\
		  (void)SPI_RXBUF;\
		  for (spiCnt = 0; spiCnt < (c); spiCnt++) {\
				FASTSPI_RX_GARBAGE();\
		  }\
  		  clock_delay(1);\
		  SPI_DISABLE();\
	 } while (0)



/***********************************************************
	FAST SPI: CC2420 RAM access (big or little-endian order)
***********************************************************/
//  FAST SPI: CC2420 RAM access (big or little-endian order)
// 	  p = pointer to the variable to be written
// 	  a = the CC2420 RAM address
// 	  c = the number of bytes to write
// 	  n = counter variable which is used in for/while loops (u8_t)
//
//  Example of usage:
// 	  u8_t n;
// 	  u16_t shortAddress = 0xBEEF;
// 	  FASTSPI_WRITE_RAM_LE(&shortAddress, CC2420RAM_SHORTADDR, 2);


#define FASTSPI_WRITE_RAM_LE(p,a,c,n)\
	 do {\
		  SPI_ENABLE();\
		  FASTSPI_TX(0x80 | (a & 0x7F));\
		  FASTSPI_TX((a >> 1) & 0xC0);\
		  for (n = 0; n < (c); n++) {\
				FASTSPI_TX(((u8_t*)(p))[n]);\
		  }\
		  SPI_DISABLE();\
	 } while (0)

#define FASTSPI_READ_RAM_LE(p,a,c,n)\
	 do {\
		  SPI_ENABLE();\
		  FASTSPI_TX(0x80 | (a & 0x7F));\
		  FASTSPI_TX(((a >> 1) & 0xC0) | 0x20);\
		  SPI_RXBUF;\
		  for (n = 0; n < (c); n++) {\
				FASTSPI_RX(((u8_t*)(p))[n]);\
		  }\
		  SPI_DISABLE();\
	 } while (0)

#endif /* SPI_H */
