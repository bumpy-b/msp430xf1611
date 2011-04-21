#ifndef DEBUGFUNCTION_H_
#define DEBUGFUNCTION_H_

/* LEDS config */

#define LED_OUT P5OUT

#define BIT_BLUE (1 << 6)
#define BIT_GREEN (1 << 5)
#define BIT_RED (1 << 4)


typedef enum{
	GREEN  = (1 << 5),
	BLUE  = (1 << 6),
	RED  = (1 << 4),
}eColor_t;

typedef enum {
	STATE0 = BLUE | GREEN | RED,
	STATE1 = ~BLUE,
	STATE2 = ~GREEN,
	STATE3 = ~(BLUE | GREEN),
	STATE4 = ~RED,
	STATE5 = ~(BLUE | RED),
	STATE6 = ~(RED | GREEN),
	STATE7 = 0,
}eDebugState;

void ledOn(eColor_t color);
void ledOff(eColor_t color);
void ledFlip(eColor_t color);
char *ledState(eColor_t color);

void debugState(eDebugState state);

#endif /* DEBUGFUNCTION_H_ */
