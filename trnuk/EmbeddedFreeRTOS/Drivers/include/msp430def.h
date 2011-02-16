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

#define asmv(arg) __asm__ __volatile__(arg)

typedef int spl_t;
void    splx_(spl_t);
/*
 * Mask all interrupts that can be masked.
 */
spl_t splhigh_(void)
{
  /* Clear the GIE (General Interrupt Enable) flag. */
  int sr;
  asmv("mov r2, %0" : "=r" (sr));
  asmv("bic %0, r2" : : "i" (GIE));
  return sr & GIE;		/* Ignore other sr bits. */
}

#define splhigh() splhigh_()
#define splx(sr) __asm__ __volatile__("bis %0, r2" : : "r" (sr))

#define splhigh() splhigh_()


#endif /* MSP430DEF_H_ */
