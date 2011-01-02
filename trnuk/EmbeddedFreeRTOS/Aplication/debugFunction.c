#include "FreeRTOS.h"
#include "debugFunction.h"

void ledOn(uint8_t colore){
	LED_OUT &= ~colore;
}

void ledOff(uint8_t colore){
	LED_OUT |= colore;
}

void ledFlip(uint8_t colore){
	LED_OUT ^= colore;
}
