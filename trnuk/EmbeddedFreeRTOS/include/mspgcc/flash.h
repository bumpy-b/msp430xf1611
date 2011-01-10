#ifndef MSPGCC_FLASH_H
#define MSPGCC_FLASH_H

/**
 * Erase a single flash segment.
 *
 * This function modifies FCTL1 and FCTL3.
 *
 * @note
 *      SegemntA on F2xx can not be modfied until unlocked with
 *      flash_lock_segmentA(0)
 *
 * @note
 *      FCTL2 has to be set up by the user. It is not altered by this
 *      function. It contains the clock settings.
 *
 * @param address       [in] any address within the segment to erase
 */
void flash_erase_segment(void *address);

/**
 * Lock or unlock SegemntA on F2xx devices.
 *
 * This function modifies FCTL3.
 *
 * @note
 *      It is good practise to lock the segment again after it has beed
 *      modified.
 *
 * @param lock          [in] nonzero to lock, zero to unlock the segment from
 *                      flash write or erase operations
 */
void flash_lock_segmentA(int lock);

/**
 * Copy a memory block to the flash. Like memcpy but with Flash controller
 * enabled.
 *
 * This function modifies FCTL1 and FCTL3.
 *
 * @note
 *      FCTL2 has to be set up by the user. It is not altered by this
 *      function. It contains the clock settings.
 *
 * @note
 *      SegemntA on F2xx can not be modfied until unlocked with
 *      flash_lock_segmentA(0)
 *
 * Examples:
 *    int intvar = 1234;
 *    flash_write((void *)0x1000, &intvar, sizeof(int));
 *
 *    struct { ... } somestructure;
 *    flash_write((void *)0x1000, &somestructure, sizeof(somestructure));
 *
 * @param dst           [in] the memory is written here
 * @param src           [in] the memory that is read
 * @param size          [in] number of bytes to copy
 */
void flash_write(void *dst, const void *src, unsigned int size);

/**
 * Copy a memory block to the flash. Like memcpy but with Flash controller
 * enabled. This function works with 16 Bit moves, the source and destiantion
 * pointers must be even!
 *
 * This function modifies FCTL1 and FCTL3.
 *
 * @note
 *      FCTL2 has to be set up by the user. It is not altered by this
 *      function. It contains the clock settings.
 *
 * @note
 *      SegemntA on F2xx can not be modfied until unlocked with
 *      flash_lock_segmentA(0)
 *
 * Examples:
 *    int intvar = 1234;
 *    flash_write_word((void *)0x1000, &intvar, sizeof(int));
 *
 *    struct { ... } somestructure;
 *    flash_write_word((void *)0x1000, &somestructure, sizeof(somestructure));
 *
 * @param dst           [in] the memory is written here (even addresses only!)
 * @param src           [in] the memory that is read (even addresses only!)
 * @param size          [in] number of bytes to copy (even sizes only!)
 */
void flash_write_word(void *dst, const void *src, unsigned int size);

#endif //MSPGCC_FLASH_H
