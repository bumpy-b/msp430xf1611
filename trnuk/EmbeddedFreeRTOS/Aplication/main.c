/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "serial.h"
#include "io.h"
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* debugging */
#include "debugFunction.h"
#include "mystdio.h"

/* CC2420 include */
#include "cc2420.h"

/* LEDs config */
#define mainLED_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
/*
* The LEDs flashing tasks
*/
//static void vTaskLED0( void *pvParameters );
//static void vTaskLED1( void *pvParameters );
//static void vTaskLED2( void *pvParameters );
static void vTaskCC2420_send( void *pvParameters );
static void vTaskCC2420_recv( void *pvParameters );
/*
* Perform Hardware initialization.
*/
static void prvSetupHardware( void );

/* serial uart device */
static uint16_t uxBufferLength = 255;
static eBaud eBaudRate = ser2400;
xComPortHandle xPort;

/*---------------------------------------------------*/


int main( void )
{
  /* Setup the hardware ready for the demo. */
  prvSetupHardware();
  ledOff(GREEN);
  ledOff(BLUE);
  ledOff(RED);

  /* Start the LEDs tasks */
//  xTaskCreate( vTaskLED0, "LED0", configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY, NULL );
//  xTaskCreate( vTaskLED1, "LED1", configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY, NULL );
//  xTaskCreate( vTaskLED2, "LED2", configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY, NULL );
    xTaskCreate( vTaskCC2420_send, "CC2420_send", configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY, NULL );
 // xTaskCreate( vTaskCC2420_recv, "CC2420_recv", configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY, NULL );


  /* Start the scheduler. */
  vTaskStartScheduler();

  /* As the scheduler has been started the demo application
     tasks will be executing and we should never get here! */

  return 0;
}

/* Second LED flash task */
static void vTaskCC2420_recv ( void *pvParameters)
{
	    char b = 'a';
	    int size;
	    char temp = 0;

	    printf("starting task recv\n");
	    printf("buffer is %c\n",b);
		cc2420_init();
		cc2420_printState();
		cc2420_on();
		cc2420_printState();
		debugState(STATE0);
		cc2420_printState();
		printf("status is %x\n",cc2420_status());
		while (1)
		{
			cc2420_printState();
	    	size = cc2420_simplerecv();
	        printf("got %d size\n",size);
			vTaskDelay(2000);
		//	size = cc2420_simplerecv();
	    //    printf("got %d size\n",size);
		}
}
static void vTaskCC2420_send( void *pvParameters )
{
    char b = 'a';
    int size;
    char temp = 0;

    printf("starting task send\n");
    printf("buffer is %c\n",b);
	cc2420_init();
	cc2420_printState();
	cc2420_on();
	cc2420_printState();
	debugState(STATE0);
	cc2420_printState();
	printf("status is %x\n",cc2420_status());
	while (1)
	{
		cc2420_printState();
		cc2420_simplesend();
		vTaskDelay(2000);
	//	size = cc2420_simplerecv();
    //    printf("got %d size\n",size);
	}
}

/* First LED flash task */
static void vTaskLED0( void *pvParameters )
{

  while (1)
  {
	  if (hasRxData())
		  putchar(getchar());

    /* Toggle blue LED and wait 500 ticks */
	//  printf("BIT BLUE %s number with hex number \n","hello");

	//  printf("hello\n");
//	 if (U1RXBUF != ch)
 // 	{
  //		ledFlip(BIT_BLUE);
  //	}
    vTaskDelay(500);
  }
}

/* Second LED flash task */
static void vTaskLED1( void *pvParameters )
{
  while (1)
  {
    /* Toggle green LED and wait 1000 ticks */
  	ledFlip(GREEN);
    vTaskDelay(1000);
  }
}

/* Third LED flash task */
static void vTaskLED2( void *pvParameters )
{
  while (1)
  {
    /* Toggle red LED and wait 2000 ticks */
		ledFlip(RED);
		vTaskDelay(2000);
  }
}

static void prvSetupHardware( void )
{
//  int i;

  /* Stop the watchdog timer. */
  WDTCTL = WDTPW|WDTHOLD;
  /* initial UART device */
  xPort = xSerialPortInitMinimal(eBaudRate, uxBufferLength );

  /* Setup MCLK 8MHz and SMCLK 1MHz */
//  DCOCTL = 0;
//  BCSCTL1 = 0;
//  BCSCTL2 = SELM_2 | (SELS | DIVS_3) ;

  /* Wait for cristal to stabilize */
//  do {
    /* Clear OSCFault flag */
//    IFG1 &= ~OFIFG;
    /* Time for flag to set */
//    for (i = 0xff; i > 0; i--) nop();
//  } while ((IFG1 & OFIFG) != 0);

  /* Configure IO for LED use */
  P5SEL &= ~(BIT_BLUE | BIT_GREEN | BIT_RED);
  P5OUT &= ~(BIT_BLUE | BIT_GREEN | BIT_RED);
  P5DIR |= (BIT_BLUE | BIT_GREEN | BIT_RED);

}


/*-----------------------------------------------------------*/
/* Used to detect the idle hook function stalling. */
static volatile unsigned long ulIdleLoops = 0UL;

void vApplicationIdleHook( void );
void vApplicationIdleHook( void )
{
	/* Simple put the CPU into lowpower mode. */
	_BIS_SR( LPM3_bits );
	ulIdleLoops++;
}
/*-----------------------------------------------------------*/
