#ifndef PACKEBUF_H_
#define PACKEBUF_H_

/**
 * \brief      The size of the packetbuf, in bytes
 */
#define PACKETBUF_SIZE 128

/**
 * \brief      The size of the packetbuf header, in bytes
 */
#define PACKETBUF_HDR_SIZE 32

/**
 * \brief      Clear and reset the packetbuf
 *
 *             This function clears the packetbuf and resets all
 *             internal state pointers (header size, header pointer,
 *             external data pointer). It is used before preparing a
 *             packet in the packetbuf.
 *
 */
void packetbuf_clear(void);

/**
 * \brief      Get a pointer to the data in the packetbuf
 * \return     Pointer to the packetbuf data
 *
 *             This function is used to get a pointer to the data in
 *             the packetbuf. The data is either stored in the packetbuf,
 *             or referenced to an external location.
 *
 *             For outbound packets, the packetbuf consists of two
 *             parts: header and data. The header is accessed with the
 *             packetbuf_hdrptr() function.
 *
 *             For incoming packets, both the packet header and the
 *             packet data is stored in the data portion of the
 *             packetbuf. Thus this function is used to get a pointer to
 *             the header for incoming packets.
 *
 */
void *packetbuf_dataptr(void);

/**
 * \brief      Get a pointer to the header in the packetbuf, for outbound packets
 * \return     Pointer to the packetbuf header
 *
 *             For outbound packets, the packetbuf consists of two
 *             parts: header and data. This function is used to get a
 *             pointer to the header in the packetbuf. The header is
 *             stored in the packetbuf.
 *
 */
void *packetbuf_hdrptr(void);

/**
 * \brief      Get the length of the header in the packetbuf, for outbound packets
 * \return     Length of the header in the packetbuf
 *
 *             For outbound packets, the packetbuf consists of two
 *             parts: header and data. This function is used to get
 *             the length of the header in the packetbuf. The header is
 *             stored in the packetbuf and accessed via the
 *             packetbuf_hdrptr() function.
 *
 */
uint8_t packetbuf_hdrlen(void);


/**
 * \brief      Get the length of the data in the packetbuf
 * \return     Length of the data in the packetbuf
 *
 *             For outbound packets, the packetbuf consists of two
 *             parts: header and data. This function is used to get
 *             the length of the data in the packetbuf. The data is
 *             stored in the packetbuf and accessed via the
 *             packetbuf_dataptr() function.
 *
 *             For incoming packets, both the packet header and the
 *             packet data is stored in the data portion of the
 *             packetbuf. This function is then used to get the total
 *             length of the packet - both header and data.
 *
 */
uint16_t packetbuf_datalen(void);

/**
 * \brief      Get the total length of the header and data in the packetbuf
 * \return     Length of data and header in the packetbuf
 *
 */
uint16_t packetbuf_totlen(void);

/**
 * \brief      Set the length of the data in the packetbuf
 * \param len  The length of the data
 *
 *             For outbound packets, the packetbuf consists of two
 *             parts: header and data. This function is used to set
 *             the length of the data in the packetbuf.
 */
void packetbuf_set_datalen(uint16_t len);

/**
 * \brief      Point the packetbuf to external data
 * \param ptr  A pointer to the external data
 * \param len  The length of the external data
 *
 *             For outbound packets, the packetbuf consists of two
 *             parts: header and data. This function is used to make
 *             the packetbuf point to external data. The function also
 *             specifies the length of the external data that the
 *             packetbuf references.
 */
void packetbuf_reference(void *ptr, uint16_t len);

/**
 * \brief      Check if the packetbuf references external data
 * \retval     Non-zero if the packetbuf references external data, zero otherwise.
 *
 *             For outbound packets, the packetbuf consists of two
 *             parts: header and data. This function is used to check
 *             if the packetbuf points to external data that has
 *             previously been referenced with packetbuf_reference().
 *
 */
int packetbuf_is_reference(void);

/**
 * \brief      Get a pointer to external data referenced by the packetbuf
 * \retval     A pointer to the external data
 *
 *             For outbound packets, the packetbuf consists of two
 *             parts: header and data. The data may point to external
 *             data that has previously been referenced with
 *             packetbuf_reference(). This function is used to get a
 *             pointer to the external data.
 *
 */
void *packetbuf_reference_ptr(void);

/**
 * \brief      Compact the packetbuf
 *
 *             This function compacts the packetbuf by copying the data
 *             portion of the packetbuf so that becomes consecutive to
 *             the header. It also copies external data that has
 *             previously been referenced with packetbuf_reference()
 *             into the packetbuf.
 *
 *             This function is called by the Rime code before a
 *             packet is to be sent by a device driver. This assures
 *             that the entire packet is consecutive in memory.
 *
 */
void packetbuf_compact(void);

/**
 * \brief      Copy from external data into the packetbuf
 * \param from A pointer to the data from which to copy
 * \param len  The size of the data to copy
 * \retval     The number of bytes that was copied into the packetbuf
 *
 *             This function copies data from a pointer into the
 *             packetbuf. If the data that is to be copied is larger
 *             than the packetbuf, only the data that fits in the
 *             packetbuf is copied. The number of bytes that could be
 *             copied into the rimbuf is returned.
 *
 */
int packetbuf_copyfrom(const void *from, uint16_t len);

/**
 * \brief      Copy the entire packetbuf to an external buffer
 * \param to   A pointer to the buffer to which the data is to be copied
 * \retval     The number of bytes that was copied to the external buffer
 *
 *             This function copies the packetbuf to an external
 *             buffer. Both the data portion and the header portion of
 *             the packetbuf is copied. If the packetbuf referenced
 *             external data (referenced with packetbuf_reference()) the
 *             external data is copied.
 *
 *             The external buffer to which the packetbuf is to be
 *             copied must be able to accomodate at least
 *             (PACKETBUF_SIZE + PACKETBUF_HDR_SIZE) bytes. The number of
 *             bytes that was copied to the external buffer is
 *             returned.
 *
 */
int packetbuf_copyto(void *to);

/**
 * \brief      Copy the header portion of the packetbuf to an external buffer
 * \param to   A pointer to the buffer to which the data is to be copied
 * \retval     The number of bytes that was copied to the external buffer
 *
 *             This function copies the header portion of the packetbuf
 *             to an external buffer.
 *
 *             The external buffer to which the packetbuf is to be
 *             copied must be able to accomodate at least
 *             PACKETBUF_HDR_SIZE bytes. The number of bytes that was
 *             copied to the external buffer is returned.
 *
 */
int packetbuf_copyto_hdr(uint8_t *to);

/**
 * \brief      Extend the header of the packetbuf, for outbound packets
 * \param size The number of bytes the header should be extended
 * \retval     Non-zero if the header could be extended, zero otherwise
 *
 *             This function is used to allocate extra space in the
 *             header portion in the packetbuf, when preparing outbound
 *             packets for transmission. If the function is unable to
 *             allocate sufficient header space, the function returns
 *             zero and does not allocate anything.
 *
 */
int packetbuf_hdralloc(int size);

/**
 * \brief      Reduce the header in the packetbuf, for incoming packets
 * \param size The number of bytes the header should be reduced
 * \retval     Non-zero if the header could be reduced, zero otherwise
 *
 *             This function is used to remove the first part of the
 *             header in the packetbuf, when processing incoming
 *             packets. If the function is unable to remove the
 *             requested amount of header space, the function returns
 *             zero and does not allocate anything.
 *
 */
int packetbuf_hdrreduce(int size);

/* Packet attributes stuff below: */

typedef uint16_t packetbuf_attr_t;

struct packetbuf_attr {
/*   uint8_t type; */
  packetbuf_attr_t val;
};
struct packetbuf_addr {
/*   uint8_t type; */
  uint8_t addr;
};

extern const char *packetbuf_attr_strings[];

#define PACKETBUF_ATTR_PACKET_TYPE_DATA 0
#define PACKETBUF_ATTR_PACKET_TYPE_ACK 1
enum {
  PACKETBUF_ATTR_NONE,
  PACKETBUF_ATTR_CHANNEL,
  PACKETBUF_ATTR_PACKET_ID,
  PACKETBUF_ATTR_PACKET_TYPE,
  PACKETBUF_ATTR_EPACKET_ID,
  PACKETBUF_ATTR_EPACKET_TYPE,
  PACKETBUF_ATTR_HOPS,
  PACKETBUF_ATTR_TTL,
  PACKETBUF_ATTR_REXMIT,
  PACKETBUF_ATTR_MAX_REXMIT,
  PACKETBUF_ATTR_NUM_REXMIT,
  PACKETBUF_ATTR_LINK_QUALITY,
  PACKETBUF_ATTR_RSSI,
  PACKETBUF_ATTR_TIMESTAMP,
  PACKETBUF_ATTR_RADIO_TXPOWER,

  PACKETBUF_ATTR_LISTEN_TIME,
  PACKETBUF_ATTR_TRANSMIT_TIME,

  PACKETBUF_ATTR_NETWORK_ID,

  PACKETBUF_ATTR_RELIABLE,
  PACKETBUF_ATTR_ERELIABLE,

  PACKETBUF_ADDR_SENDER,
  PACKETBUF_ADDR_RECEIVER,
  PACKETBUF_ADDR_ESENDER,
  PACKETBUF_ADDR_ERECEIVER,

  PACKETBUF_ATTR_MAX
};

#define PACKETBUF_NUM_ADDRS 4
#define PACKETBUF_NUM_ATTRS (PACKETBUF_ATTR_MAX - PACKETBUF_NUM_ADDRS)
#define PACKETBUF_ADDR_FIRST PACKETBUF_ADDR_SENDER

static struct packetbuf_attr packetbuf_attrs[PACKETBUF_NUM_ATTRS];
static struct packetbuf_addr packetbuf_addrs[PACKETBUF_NUM_ATTRS];

static int               packetbuf_set_attr(uint8_t type, const packetbuf_attr_t val);
static packetbuf_attr_t    packetbuf_attr(uint8_t type);
// static int               packetbuf_set_addr(uint8_t type, const uint8_t *addr);
static const uint8_t *packetbuf_addr(uint8_t type);

static inline int
packetbuf_set_attr(uint8_t type, const packetbuf_attr_t val)
{
/*   packetbuf_attrs[type].type = type; */
  packetbuf_attrs[type].val = val;
  return 1;
}
static inline packetbuf_attr_t
packetbuf_attr(uint8_t type)
{
  return packetbuf_attrs[type].val;
}

/*
static inline int
packetbuf_set_addr(uint8_t type, const uint8_t *addr)
{
/ *   packetbuf_addrs[type - PACKETBUF_ADDR_FIRST].type = type; * /
  rimeaddr_copy(&packetbuf_addrs[type - PACKETBUF_ADDR_FIRST].addr, addr);
  return 1;
}
*/

static inline const uint8_t *
packetbuf_addr(uint8_t type)
{
  return &packetbuf_addrs[type - PACKETBUF_ADDR_FIRST].addr;
}

void              packetbuf_attr_clear(void);

void              packetbuf_attr_copyto(struct packetbuf_attr *attrs,
				      struct packetbuf_addr *addrs);
void              packetbuf_attr_copyfrom(struct packetbuf_attr *attrs,
					struct packetbuf_addr *addrs);

#define PACKETBUF_ATTRIBUTES(...) { __VA_ARGS__ PACKETBUF_ATTR_LAST }
#define PACKETBUF_ATTR_LAST { PACKETBUF_ATTR_NONE, 0 }

#define PACKETBUF_ATTR_BIT  1
#define PACKETBUF_ATTR_BYTE 8
#define PACKETBUF_ADDRSIZE (sizeof(uint8_t) * PACKETBUF_ATTR_BYTE)

struct packetbuf_attrlist {
  uint8_t type;
  uint8_t len;
};

#endif /* PACKEBUF_H_ */
