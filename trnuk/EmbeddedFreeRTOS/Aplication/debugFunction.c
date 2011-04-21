#include "FreeRTOS.h"
#include "debugFunction.h"


void ledOn(eColor_t colore){
	LED_OUT &= ~colore;
}

void ledOff(eColor_t colore){
	LED_OUT |= colore;
}

void ledFlip(eColor_t colore){
	LED_OUT ^= colore;
}

void debugState(eDebugState state) {
	LED_OUT = state;
}
char *ledState(eColor_t color) {
	if (LED_OUT & color)
			return "Off";
		else
			return "On";
}
