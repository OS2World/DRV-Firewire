/*
** Module   :PACKET.HPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Tue  01/06/2004 Created
**
*/
#ifndef __PACKET_HPP
#define __PACKET_HPP

#include "memory.hpp"

#define ACK_COMPLETE             0x1
#define ACK_PENDING              0x2
#define ACK_BUSY_X               0x4
#define ACK_BUSY_A               0x5
#define ACK_BUSY_B               0x6
#define ACK_TARDY                0xb
#define ACK_CONFLICT_ERROR       0xc
#define ACK_DATA_ERROR           0xd
#define ACK_TYPE_ERROR           0xe
#define ACK_ADDRESS_ERROR        0xf

/* Non-standard "ACK codes" for internal use */
#define ACKX_NONE                (-1)
#define ACKX_SEND_ERROR          (-2)
#define ACKX_ABORTED             (-3)
#define ACKX_TIMEOUT             (-4)

class HOST;
typedef HOST far * PHOST;

class PACKET
{
public:
    PACKET(USHORT usSize);
    ~PACKET();
    void SetCompleteTask(void far (*routine)(void far *), void far *data);
    int Send(void);
    void QueueComplete();
    int SendAndWait(void);
    void SendNoCare(void);
    void FillAsyncReadquadResp(int rcode, QUADLET data);
    void FillAsyncReadblockResp(int rcode, int length);
    void FillAsyncWriteResp(int rcode);
    void FillAsyncLockResp(int rcode, int extcode, int length);
    void FillAsyncReadquad(U64 addr);
    void FillAsyncReadblock(U64 addr, USHORT length);
    void FillAsyncWritequad(U64 addr, QUADLET data);
    void FillAsyncWriteblock(U64 addr, int length);
    void FillAsyncLock(U64 addr, int extcode, int length);
    void FillIsoPacket(int length, int channel, int tag, int sync);
    void FillPhyPacket(QUADLET data);
    void FillAsyncStreamPacket(int length, int channel, int tag, int sync);
    int  Success(void);
    int GetTLabel(void);
    void FreeTLabel(void);
    /* This struct is basically read-only for hosts with the exception of
     * the data buffer contents and xnext - see below. */

    /* This can be used for host driver internal linking.
     *
     * NOTE: This must be left in init state when the driver is done
     * with it (e.g. by using list_del_init()), since the core does
     * some sanity checks to make sure the packet is not on a
     * driver_list when free'ing it. */
//    struct list_head driver_list;

    NODEID NodeId;

    /* Async and Iso types should be clear, raw means send-as-is, do not
     * CRC!  Byte swapping shall still be done in this case. */
    enum { hpsb_async, hpsb_iso, hpsb_raw } type;

    /* Okay, this is core internal and a no care for hosts.
     * queued   = queued for sending
     * pending  = sent, waiting for response
     * complete = processing completed, successful or not
     */
    enum {hpsb_unused, hpsb_queued, hpsb_pending, hpsb_complete} state;

    /* These are core internal. */
    signed char tlabel;
    char AckCode;
    char tcode;

    BOOL bExpectResponse;
    BOOL bNoWaiter;

    /* Speed to transmit with: 0 = 100Mbps, 1 = 200Mbps, 2 = 400Mbps */
    UCHAR SpeedCode;

    /*
     * *header and *data are guaranteed to be 32-bit DMAable and may be
     * overwritten to allow in-place byte swapping.  Neither of these is
     * CRCed (the sizes also don't include CRC), but contain space for at
     * least one additional quadlet to allow in-place CRCing.  The memory is
     * also guaranteed to be DMA mappable.
     */
    PQUADLET pHeader;
    PQUADLET pData;
    USHORT usHeaderSize;
    USHORT usDataSize;

    PHOST pHost;
    UINT Generation;

    int RefCnt;

    /* Function (and possible data to pass to it) to call when this
     * packet is completed.  */
    void far (*pfnCompleteRoutine)(void far *);
    void far *pCompleteData;

    /* XXX This is just a hack at the moment */
//    struct sk_buff *skb;

    /* Store jiffies for implementing bus timeouts. */
    unsigned long sendtime;

    QUADLET far * EmbeddedHeader;

    // dmaable buffer data
    PMEMORY pPacketBuf;

};
typedef PACKET *PPACKET;
void PacketDump(const char * text, QUADLET far * data, int size);
void pascal far PacketContextRoutine(void);
#endif  /*__PACKET_HPP*/

