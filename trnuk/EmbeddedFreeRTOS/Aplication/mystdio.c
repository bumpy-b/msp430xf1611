#include "FreeRTOS.h"
#include "serial.h"
#include "semphr.h"

//include for function with unknown number of arguments
#include <stdarg.h>
#include "mystdio.h"
/* serial port */
extern xComPortHandle xPort;

static char DigitToChar[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

int putchar(int c)
{
	return xSerialPutChar( xPort, (uint8_t)c, 100 );
}
char getchar(void)
{
	char ch;
	xSerialGetChar( xPort, &ch, 100);
	return ch;
}
int hasRxData(void)
{
	char ch;
	return xSerialPeek ( xPort,&ch, 0);
}
void printUInt(uint16_t n){
	if (n){
		printUInt(n/10);
		putchar(DigitToChar[n%10]);
	}
}

void printHex(uint16_t n){
	putchar('0');
	putchar('x');
	putchar(DigitToChar[n>>12]);
	putchar(DigitToChar[n>>8 & 0xf]);
	putchar(DigitToChar[n>>4 & 0xf]);
	putchar(DigitToChar[n & 0xf]);
}

void myPrintf(char* format,...){
	/* semaphore for printf */
	static xSemaphoreHandle xSemaphore;
	static uint8_t initialized = 0;
	if (!initialized){
		vSemaphoreCreateBinary( xSemaphore );
		initialized = 1;
	}

	if (pdTRUE != xSemaphoreTake( xSemaphore, 1000 ))
		return;
	va_list ap;
	char* arg;

	va_start(ap, format);
	while ((*format)){
		if (*format == '%'){
			format++;
			arg =  va_arg(ap, char*);
			switch (*format++){
				case 'c':
					putchar((char)arg);
					if ((char)arg == '\n')
						putchar('\r');
					break;
				case 's':
					while ((*arg)){
						if (*arg == '\n')
							putchar('\r');
						putchar(*arg++);
					}
					break;
				case 'd':
					if ((int)arg < 0){
						putchar('-');
						printUInt((int16_t)arg*-1);
					}
					else{
						if ((int16_t)arg == 0)
							putchar('0');
						else
							printUInt((int16_t)arg);
					}
					break;
				case 'u':
					if ((int16_t)arg == 0)
						putchar('0');
					else
						printUInt((int16_t)arg);
					break;
				case 'x':
					if ((int16_t)arg == 0){
						putchar('0');
						putchar('x');
						putchar('0');
					}
					else
						printHex((uint16_t)arg);
					break;
			}
		}
		else{
			if (*format == '\n')
				putchar('\r');
			putchar(*format++);
		}
	}
	va_end(ap);

	xSemaphoreGive( xSemaphore );
}
