/*
 * msp430def.h
 *
 *  Created on: 11/02/2011
 *      Author: Lior
 */

#ifndef MSP430DEF_H_
#define MSP430DEF_H_

typedef  uint8_t    u8_t;
typedef uint16_t   u16_t;
typedef uint32_t   u32_t;
typedef  int32_t   s32_t;

typedef int spl_t;
void    splx_(spl_t);
spl_t   splhigh_(void);

#define splhigh() splhigh_()
#define splx(sr) __asm__ __volatile__("bis %0, r2" : : "r" (sr))

#define splhigh() splhigh_()


#endif /* MSP430DEF_H_ */
