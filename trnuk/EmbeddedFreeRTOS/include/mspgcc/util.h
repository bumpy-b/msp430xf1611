#ifndef UTIL_H
#define UTIL_H

/**
 * Print a hexdump of the gived memory region. Implemented with printf()
 * as output function.
 *
 * @param buffer        [in] this data is dumped
 * @param length        [in] this number of bytes are processed
 * @param address       [in] the first address printed in the hexdump
 */
void hexdump(const void *buffer, unsigned int length, unsigned int address);

/**
 * Simple delay loop. It uses 3 CPU cycles per count plus a few cyles for
 * function call and return.
 * 
 * Calculation:
 *   total_cycles = 6 + count*3
 * 
 * This does not account for any code that the compiler generates to call this
 * function and clean up afterwards!
 * 
 * @param count         [in] number of loops, in effect the delay length
 */
void delay(unsigned short count);


/**
 * An array containing all the hexadecimal digits "0123456789ABCDEF".
 * This can be used to convert a nibble to a hex number.
 */
extern const unsigned char HEX_DIGITS[16];

/**
 * Encode a binary buffer of given size to a string of hexdigits (null terminated).
 *
 * @param dst           the resulting hex string
 * @param maxsize       available number of bytes in the output string
 * @param src           source binary buffer
 * @param size          number of binary bytes that should be encoded
 * @return              Number of hex digits in the encoded result. The return 
 *                      value smaller maxsize on success, equal or larger if the
 *                      result was truncated.
 */
unsigned int hex_encode(char *dst, unsigned int maxsize, const void *src, unsigned int size);

/**
 * Decode a string of hexdigits into a binary buffer.
 *
 * @param dst           the resulting binary
 * @param maxsize       available number of bytes in the output string
 * @param src           source string of hex digits
 * @param size          number of characters that should be decoded. this is an
 *                      even number as each hex encoded byte consists of two characters.
 * @return              Number of binary bytes that got decoded. The return value
 *                      is equal to size on success, smaller if the dst buffer
 *                      was too small.
 */
unsigned int hex_decode(void *dst, unsigned int maxsize, const char *src, unsigned int size);

/**
 * Decode a string of hexdigits into a binary buffer.
 *
 * @param srcdst        source string of hex digits and the target for the resulting binary
 * @param size          number of characters that should be decoded. this is an
 *                      even number as each hex encoded byte consists of two characters
 * @return              Number of binary bytes that got decoded. The return value
 *                      is equal to size on success, smaller if the dst buffer
 *                      was too small.
 */
unsigned int hex_decode_inline(void *srcdst, unsigned int size);

/**
 * Convert a hex digit (ASCII character) to a number
 *
 * @param x             a character
 * @return              0...15, 0 for illegal characters
 */
unsigned char hex_fromdigit(unsigned char x);

/** An array containing all hex digits as characters (uppercase). */
extern const unsigned char HEX_DIGITS[16];


/**
 * Decode an escaped, null terminated string to a binary buffer.
 *
 * \\n \\r \\t \\a \\b are output for the respecutive control characters, \\xNN
 * for control characters. unlike C's escape rules \\xNN uses exactly two digits
 * the next character does not belong to the number even if it is a hex digit.
 *
 * dst == src is allowed to decode the string inline. This is possible as
 * the string with escapes is of the same length or longer than the resulting
 * binary buffer data.
 *
 * @param dst           the resulting binary
 * @param maxsize       available number of bytes in the output buffer dst
 * @param src           source string with escapes
 * @return              Number of characters in the dst buffer
 *                      The return value is equal to size on success, smaller
 *                      if the dst buffer was too small.
 */
unsigned int escape_decode(void *dst, unsigned int maxsize, const char *src);

/**
 * Encode an binary buffer to a escaped string.
 *
 * \\n \\r \\t \\a \\b are output for the respecitve control characters, \\xNN
 * for control characters. unlike C's escape rules \\xNN generates two digits
 * the next character does not belong to the number even if it is a hex digit.
 *
 * @param dst           the resulting null terminated string
 * @param maxsize       available number of bytes in the output string dst
 * @param src           source (binary) string
 * @param size          number of characters that should be encoded.
 * @return              Number of characters in the dst buffer
 *                      The return value is equal to size on success, smaller
 *                      if the dst buffer was too small.
 */
unsigned int escape_encode(char *dst, unsigned int maxsize, const void *src, unsigned int size);



/**
 * Simple line reader. Reads with getchar(), prcoesses with 
 * ::lineeditor_simple_process_key
 *
 * @note
 *      This function blocks until a line is read, or getchar() returns an
 *      error/EOF.
 * 
 * @param line          [in] buffer to read the line to
 * @param maxlen        [in] size of the buffer
 */
void simple_readline(char *line, unsigned int maxlen);


/**
 * The line editor function stores its state in such a structure.
 * Initialize it with a pointer to a buffer for the line and its length.
 */
typedef struct {
    char *line;                 ///< buffer for the editor
    unsigned char line_size;    ///< size of the buffer
    struct {
        int echo:1;             ///< enable echo
    } flags;
    unsigned char position;     ///< internal use
} LINEEDITOR_STATE;

/**
 * Clear line.
 */
#define LINEEDITOR_RESET(state) (state)->position = 0; (state)->line[0] = '\0';

/**
 * Process the key in the line editor.
 * Currenly ony very simple editiong is supported:
 * - Backspace deletes the last character of the line
 *
 * The line is at any time terminated with a null character.
 * 
 * This function generates an echo using putchar().
 *
 * @return      true when the line is complete, false otherwise.
 */
unsigned char lineeditor_simple_process_key(LINEEDITOR_STATE *state, char key);


/**
 * Simple 16 bit bitwise xor checksum.
 *
 * The advantage of this checksum is, that the checksum itself can be embedded
 * anywhere within the data to be checked. Running checksum_xor over the entire
 * data then returns 0 for correct data or any other number for a data
 * integrity problem.
 *
 * @param       address         [in] Pointer to the memory to be checked.
 * @param       length          [in] Size of the checked mememory in bytes. 
 *                              Has to be an even number.
 * @return      checksum
 */
unsigned short checksum_xor(const void *address, unsigned int length);

#endif //UTIL_H
