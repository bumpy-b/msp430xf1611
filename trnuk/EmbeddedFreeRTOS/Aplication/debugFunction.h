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
	STATE0 = 0,
	STATE1 = RED,
	STATE2 = GREEN,
	STATE3 = RED | GREEN,
	STATE4 = BLUE,
	STATE5 = BLUE | RED,
	STATE6 = BLUE | GREEN,
	STATE7 = BLUE | GREEN | RED,
}eDebugState;

void ledOn(eColor_t color);
void ledOff(eColor_t color);
void ledFlip(eColor_t color);


#endif /* DEBUGFUNCTION_H_ */
