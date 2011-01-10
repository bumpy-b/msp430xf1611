/**
 * This function whifts up to 32 bits out. The LSB bit is shifted out first.
 * Can be used to shift data to a shift register.
 *
 * @param width         [in]    Number of bits to shift out
 * @param data          [in]    Up to 32 bits that are shifted out. Data has
 *                              to be right aligned, if width is less than 32.
 */
void spi_push(unsigned char width, unsigned long data);

/**
 * Transmit one byte. MSB is shifted out/in first.
 *
 * @param data          [in]    The byte that is sent
 * @return              the byte that is read
 */
unsigned char spi_shift(unsigned char data);

/**
 * Transmit a byte array. Using spi_shift.
 *
 * If output is NULL, then the received data is not stored.
 * If input is NULL, 0xff is sent for each byte.
 *
 * Both buffers, if not NULL, must have at leasth length bytes space.
 * It is valid, tough not very useful to set both pointers to NULL.
 *
 * @param output        [out]   Buffer for the received data or NULL
 * @param input         [in]    Buffer for the data that is sent or NULL
 * @param length        [in]    number of bytes to transmit.
 */
void spi_transfer(void *output, const void *input, int length);


// functions that have to be provided by the user
void SPI_CS_SET(void);          ///< set chip select
void SPI_CS_CLEAR(void);        ///< clear chip select
void SPI_CLK_SET(void);         ///< set clock line, may add delay before edge to slow down
void SPI_CLK_CLEAR(void);       ///< clear clock line, may add delay before edge to slow down
void SPI_DATA_SET(void);        ///< set data line (master out)
void SPI_DATA_CLEAR(void);      ///< clear data line (master out)
int SPI_DATA_READ(void);        ///< read data line (master in)
