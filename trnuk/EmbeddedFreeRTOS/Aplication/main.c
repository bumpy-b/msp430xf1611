//#define SENDER

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
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

#define MAX_MSG 20
/*
* The LEDs flashing tasks
*/
static void vTaskRx         ( void *pvParameters );
static void vTaskSHELL      ( void *pvParameters );
static void vTaskCC2420_send( void *pvParameters );
static void vTaskBROADCAST  ( void *pvParameters );
/*
* Perform Hardware initialization.
*/
static void prvSetupHardware( void );

/* serial uart device */
static uint16_t uxBufferLength = 255;
static eBaud eBaudRate = ser2400;
uint8_t msg[MAX_MSG];
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
  xTaskCreate( vTaskRx, "RX", configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY, NULL );
  xTaskCreate( vTaskSHELL, "SHELL", configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY, NULL );
  xTaskCreate( vTaskBROADCAST, "BROADCAST", configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY, NULL );

#ifdef SENDER
    xTaskCreate( vTaskCC2420_send, "CC2420_send", configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY, NULL );
#endif

  /* Start the scheduler. */
  vTaskStartScheduler();

  /* As the scheduler has been started the demo application
     tasks will be executing and we should never get here! */

  return 0;
}

/* Second LED flash task */
static void vTaskBROADCAST ( void *pvParameters)
{
		while (1)
		{
			//printf("Sending BROADCAST ID\n");
			cc2420_sendID(cc2420_getID());
			vTaskDelay(7000);

		}
}
static void vTaskCC2420_send( void *pvParameters )
{
    char b = 'a';
    uint8_t msg[20] = "Hello!";

    printf("starting task send\n");
    printf("buffer is %c\n",b);

	while (1)
	{
	//	printf("Sending ..\n");
		cc2420_simplesend(msg,10);
		vTaskDelay(2500);
	}
}

/* This task is an echo repeater
 * it will show all the chars inputed
 * to putty
 */
static void vTaskRx( void *pvParameters )
{
  int len;
  uint8_t who;
  while (1)
  {

	  zeros(msg,20);
	  len = cc2420_simplerecv(msg,&who);
	  if (len == 0)
		  continue;

	  msg[len-2] = 0;
	  printf("\nCC2420 incoming message from device %d: %s\n",who,msg);
	  ledFlip(BLUE);
	  printf("cmd : > ");
	  vTaskDelay(2000);
  }
}

void process_cmd(char *msg);
/* Second LED flash task */
static void vTaskSHELL( void *pvParameters )
{
	char ch;

    int index = 0;

	printf("Starting FreeRTOS Shell for MSP430 V1.0\n"
           "Initialization of UART ...\n"
		   "Initialization of CC2420 ...\n");
	printf("For Help type 'help' \n");
	printf("cmd : > ");

  while (1)
  {

    /* Toggle green LED and wait 1000 ticks */
    if (hasRxData())
    {
	 	  ch = getchar();
        //  printf("Index is %d\n",index);

          if (ch == 127)
          {

        	  if (index <= 0)
        		  continue;
        	  index--;
        	  msg[index]=0;
        	  putchar(ch);
        	  continue;

          }


	 	  putchar(ch);

	 	 if (ch == 13)
	  	 {
			 process_cmd(msg);
	         printf("\ncmd : > ");
	         msg[0] = 0;
	  	     index = 0;
	  	     continue;
	 	  }

	 	  msg[index++] = ch;
	 	  if (index > MAX_MSG)
	 	 {
	 		 printf("\ncmd too long !\ncmd: > ");
	 		 index = 0;
	 	  }



    }

    vTaskDelay(50);
  }
}

/* Some Processing functions */

void process_cmd(char *msg)
{
	if (msg[0]==0)
		return;

	if (strncmp(msg,"help",strlen("help")) == 0)
	{
		printf("\n\nstatus           - shows the status of the platform\n"
			   "map              - shows others CC2420 MSP active\n"
			   "send <str>       - sends a message in brodcast\n");
		return;
	}

	if (strncmp(msg,"status",strlen("status")) == 0)
	{
		printf("\n\n - MSP430 status - \n"
			   "Red Led is %s\n"
			   "Blue Led is %s\n"
			   "Green Led is %s\n"
			   "CC2420 Status register is %x\n"
			   "CC2420 Receiver is %s\n"
			   ,ledState(RED)
			   ,ledState(BLUE)
			   ,ledState(GREEN)
			   ,cc2420_status()
			   ,"On"
			   );
		return;
	}

	if (strncmp(msg,"map",strlen("map")) == 0)
	{
		CC2420_printMap();
		return;
	}
	printf("\nUnknown Command %s \n",msg);

}

static void prvSetupHardware( void )
{
//  int i;

  /* Stop the watchdog timer. */
  WDTCTL = WDTPW | WDTHOLD;
  /* initial UART device */
  xPort = xSerialPortInitMinimal(eBaudRate, uxBufferLength );

  /* Configure IO for LED use */
  P5SEL &= ~(BIT_BLUE | BIT_GREEN | BIT_RED);
  P5OUT &= ~(BIT_BLUE | BIT_GREEN | BIT_RED);
  P5DIR |= (BIT_BLUE | BIT_GREEN | BIT_RED);

  /* CC2420 init */
  cc2420_init();
  cc2420_on();
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
