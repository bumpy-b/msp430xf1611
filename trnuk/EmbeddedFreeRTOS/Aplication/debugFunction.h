#ifndef DEBUGFUNCTION_H_
#define DEBUGFUNCTION_H_

/* LEDS config */

#define LED_OUT P5OUT

#define BIT_BLUE (1 << 6)
#define BIT_GREEN (1 << 5)
#define BIT_RED (1 << 4)


#define GREEN (1 << 5)
#define BLUE (1 << 6)
#define RED (1 << 4)

void ledOn(uint8_t colore);
void ledOff(uint8_t colore);
void ledFlip(uint8_t colore);

#endif /* DEBUGFUNCTION_H_ */
