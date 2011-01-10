#ifndef MSPGCC_FLL_H
#define MSPGCC_FLL_H

/**
 * Formula to calculate the multiplier, for fll_adjust(), based on SMCLK
 * and ACLK.
 *
 * @param smclk the desired SMCLK speed (after divider)
 * @param aclk  the actual ACLK speed (after divider)
 */
#define FLL_MULTIPLIER(smclk, aclk)   ((smclk)/(aclk))

/**
 * Adjust SMCLK using ACLK, doing the software FLL in F1xx devices.
 *
 * This function modifies TACTL, CCR2, CCTL2, BCSCTL1 and DCOCTL.
 * The Timer_A module has to be re-setup by the user after this function
 * was run.
 *
 * @note
 *      BCSCTL2 has to be set up by the user. It is not altered by this
 *      function. It contains the ACLK divider settings.
 *
 * @note
 *      Timer_A is stopped after this function is run.
 *
 * @note
 *      This function has to timeout, if the crystal fails or the frequency
 *      is impossible to set, this function does not return!
 *
 * Example, setting the CPU to 3 MHz based on a watch crystal (32.768 kHz):
 *
 *   BCSCTL1 = XT2OFF|RSEL2|DIVA_DIV4;          // select 8192 Hz from XT1
 *   BCSCTL2 = 0;
 *   delay(0xffff);                             // give osillator some time to settle
 *   fll_adjust(FLL_MULTIPLIER(3000000, 8192)); // adjust frequency
 *
 * @param multiplier    [in] MCLK = multiplier * ACLK
 */
void fll_adjust(unsigned short multiplier);


#endif //MSPGCC_FLL_H
