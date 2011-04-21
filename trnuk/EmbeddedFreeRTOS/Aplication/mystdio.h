#ifndef MYSTIO_H_
#define MYSTIO_H_

#ifdef printf
#undef printf
#endif


/*
 * supported formats:
 *	%d - signed decimal between -32,768 to 32,767
 *	%u - unsigned decimal between 0 to 65,535
 *	%x - unsigned hex between 0x0 to 0xFFFF
 *	%c - single character
 *	%s - null terminated string
 * critical assumption:
 * 	number of formats == number of arguments(not include the format itself)
 *  this function cannot be called in an isr!!!
 */
void myPrintf(char* format,...);
#define printf myPrintf

//#define putchar(c) xSerialPutChar( xPort, (uint8_t)c, 100 )
int putchar(int c);
void zeros(char *buf,int len);
char getchar(void);
int hasRxData();
#endif /* MYSTIO_H_ */
