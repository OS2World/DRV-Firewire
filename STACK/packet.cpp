/*
** Module   :PACKET.CPP
** Abstract :packet related functions
**
** Copyright (C) Alex Cherkaev
**
** Log: Tue  01/06/2004 Created
**
*/
extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define  INCL_NOPMAPI
#define  INCL_DOSDEVICES
#define  INCL_DOSDEVIOCTL
#define  INCL_DOSERRORS
#define  INCL_NOXLATE_DOS16
#include <os2.h>
}
#include "firetype.h"
#include "fireerror.h"
#include "host.hpp"
#include "packet.hpp"
#include "global.hpp"
#include "ddprintf.h"
#include <devhelp.h>
#include <include.h>

void PacketDump(const char * text, QUADLET far * data, int size)
{
    int i;

    size /= 4;
    size = (size > 4 ? 4 : size);

    ddprintf("ieee1394: %s", text);
    for (i = 0; i < size; i++)
        ddprintf(" %lx", data[i]);
    ddprintf("\n");
}

/**
 * SetCompleteTask - set the task that runs when a packet
 * completes. You cannot call this more than once on a single packet
 * before it is sent.
 *
 * @routine: function to call
 * @data: data (if any) to pass to the above function
 */
void PACKET::SetCompleteTask(void far (*routine)(void far *), void far *data)
{
    if (pfnCompleteRoutine!=NULL)
        ddprintf("WARNING: PACKET::SetCompleteTask() pfnCompleteRoutine not NULL! \n");
    pfnCompleteRoutine=routine;
    pCompleteData = data;
    return;
}
/**
 * hpsb_alloc_packet - allocate new packet structure
 * @data_size: size of the data block to be allocated
 *
 * This function allocates, initializes and returns a new &struct hpsb_packet.
 * It can be used in interrupt context.  A header block is always included, its
 * size is big enough to contain all possible 1394 headers.  The data block is
 * only allocated when @data_size is not zero.
 *
 * For packets for which responses will be received the @data_size has to be big
 * enough to contain the response's data block since no further allocation
 * occurs at response matching time.
 *
 * The packet's generation value will be set to the current generation number
 * for ease of use.  Remember to overwrite it with your own recorded generation
 * number if you can not be sure that your code will not race with a bus reset.
 *
 * Return value: A pointer to a &struct hpsb_packet or NULL on allocation
 * failure.
 */
PACKET::PACKET(USHORT usData)
{
    //struct sk_buff *skb;
    int i;
    NodeId=0;
    type=hpsb_async;
    tlabel=0;
    AckCode=0;
    tcode=0;
    bExpectResponse=FALSE;;
    bNoWaiter=FALSE;
    SpeedCode=0;
    usHeaderSize=0;
    usDataSize=0;
    pHost=NULL;
    Generation=0;
    RefCnt=0;
    pfnCompleteRoutine=NULL;
    pCompleteData=NULL;
    sendtime=0;

    usData=((usData+3)&~3);

    pPacketBuf=new MEMORY(usData+sizeof(PACKET)+6*sizeof(QUADLET));
    if (pPacketBuf==NULL) ddprintf("ERROR: pPacketBuf is NULL\n");

    for (i=0;i<usData+sizeof(PACKET)+6*sizeof(QUADLET);i++) pPacketBuf->fpVirtAddr[i]=0;
    EmbeddedHeader=(ULONG far *)(pPacketBuf->fpVirtAddr+sizeof(PACKET));
    pHeader=EmbeddedHeader;
    state=hpsb_unused;
    Generation=-1;
//    pDriverList=new DRIVERLIST;
    pushf();
    cli();
    RefCnt=1;
    popf();

    if (usData)
    {
        pData=(PQUADLET)(pPacketBuf->fpVirtAddr+sizeof(PACKET)+6*sizeof(QUADLET));
        usDataSize=usData;
    }

}
/**
 * hpsb_free_packet - free packet and data associated with it
 *
 * This function will free packet->data and finally the packet itself.
 */
PACKET::~PACKET()
{
    pushf();
    cli();
    RefCnt--;
    if (RefCnt==0)
    {
        popf();
//        if (!DriverList.isEmpty())
//        {
//            ddprintf("ERROR: ~PACKET() bug: DriverList not free!\n");
//        }
//        delete pDriverList;
        delete pPacketBuf;
    }
    else
    {
        popf();
    }
}
/**
 * hpsb_send_packet - transmit a packet on the bus
 * @packet: packet to send
 *
 * The packet is sent through the host specified in the packet->host field.
 * Before sending, the packet's transmit speed is automatically determined
 * using the local speed map when it is an async, non-broadcast packet.
 *
 * Possibilities for failure are that host is either not initialized, in bus
 * reset, the packet's generation number doesn't match the current generation
 * number or the host reports a transmit error.
 *
 * Return value: 0 on success, negative errno on failure.
 */
int PACKET::Send(void)
{

    if (pHost->bIsShutdown)
        return -1;
    if (pHost->bInBusReset||(Generation!=pHost->Generation))
        return -2;

    state = hpsb_queued;

    /* This just seems silly to me */
    if (bNoWaiter&&bExpectResponse)
        ddprintf("WARNING: PACKET::Send()  No Waiter and Expect Responce in some time\n");

    if (!bNoWaiter||bExpectResponse)
    {
        pushf();
        cli();
        RefCnt++;
        popf();
        ((HOST *)pHost)->PendingPacket.AddTail(this);
    }

    if (NodeId==pHost->NodeId)
    {
    /* it is a local request, so handle it locally */

       PMEMORY pdata;
      int size=usDataSize+usHeaderSize;
       pdata=new MEMORY(size);
       if (pdata==NULL) ddprintf("ERROR: pdata is NULL\n");
        if (pdata->fpVirtAddr==NULL)
        {
            ddprintf("ERROR: unable to allocate memory for concatenating header and data\n");
        }
//      int i;
        _fmemcpy(pdata->fpVirtAddr, pHeader, usHeaderSize);
        if (usDataSize)
                _fmemcpy((pdata->fpVirtAddr) + usHeaderSize, pData, usDataSize);
        PacketDump("send packet local:",pHeader,usHeaderSize);
        ((HOST *)pHost)->PacketSent(this, this->bExpectResponse ? ACK_PENDING : ACK_COMPLETE);
        ((HOST *)pHost)->PacketReceived((QUADLET far *)pdata->fpVirtAddr, size, FALSE);
        delete pdata;
        return 0;
    }

    if (type==hpsb_async&&NodeId!=ALL_NODES)
    {
        SpeedCode=pHost->pSpeedMap[NODEID_TO_NODE(pHost->NodeId)*64+NODEID_TO_NODE(NodeId)];
    }

    switch (SpeedCode)
    {
        case 2:
            PacketDump("send packet 400:", pHeader,usHeaderSize);
            break;
        case 1:
            PacketDump("send packet 200:", pHeader,usHeaderSize);
            break;
        default:
            PacketDump("send packet 100:", pHeader,usHeaderSize);
    }

    return pHost->pHardDriver->fpfnTransmitPacket(this);
}
void far completePacket(void far * data)
{
    ((COMPLETION *)data)->Complete();
}
int PACKET::SendAndWait(void)
{
    COMPLETION * done;
    int retval;

    done=new COMPLETION;
    done->Init();
    SetCompleteTask(completePacket, done);
    retval = Send();
    if (retval==0)
        done->Waitfor();
    delete done;
    return retval;
}
void PACKET::SendNoCare(void)
{
    if (Send()<0)
    {
        delete this;
    }
}
void PACKET::QueueComplete()
{
    if (pfnCompleteRoutine != NULL)
        {
            pfnCompleteRoutine(pCompleteData);
            pfnCompleteRoutine=NULL;
            pCompleteData=NULL;
//            GlobalPacketQueue.AddTail(this);
//            PacketThreadSig.Up();
//		    DevHelp_ArmCtxHook(0,PacketHookHandle);
            // arm context hook
//            DevHelp_ArmCtxHook(0,PacketHookHandle);
            //skb_queue_tail(&hpsbpkt_queue, packet->skb);
            /* Signal the kernel thread to handle this */
            //up(&khpsbpkt_sig);
        }
    return;
}
void far pascal PacketContextRoutine(void)
{
//static int hpsbpkt_thread(void *__hi)
//{
//    struct sk_buff *skb;
//    struct hpsb_packet *packet;
    void far (*complete_routine)(void far *);
    void far *complete_data;
    PACKET far * packet;
    ddprintf("DEBUG: Packet thread hook started\n");

//    daemonize("khpsbpkt");

//    while (TRUE)
//    {
//    	PacketThreadSig.DownWait(-1);
//        while ((skb = skb_dequeue(&hpsbpkt_queue)) != NULL) {
        packet=GlobalPacketQueue.GetHead();
        while (packet!=NULL)
        {
            complete_routine = packet->pfnCompleteRoutine;
            complete_data = packet->pCompleteData;
            packet->pfnCompleteRoutine=NULL;
            packet->pCompleteData=NULL;
			if (complete_routine==NULL) ddprintf("ERROR: complete_routine is NULL! \n");
            complete_routine(complete_data);
            GlobalPacketQueue.DeleteHead();
	        packet=GlobalPacketQueue.GetHead();
        }
//    }
//    complete_and_exit(&khpsbpkt_complete, 0);
}


#define PREP_ASYNC_HEAD_RCODE(tc) \
    tcode = tc; \
    pHeader[0] = ((ULONG)NodeId << 16) | ((ULONG)tlabel << 10) \
        | (1 << 8) | (tc << 4); \
    pHeader[1]=((ULONG)pHost->NodeId << 16) | ((ULONG)rcode << 12); \
    pHeader[2]=0

void PACKET::FillAsyncReadquadResp(int rcode, QUADLET data)
{
    PREP_ASYNC_HEAD_RCODE(TCODE_READQ_RESPONSE);

    pHeader[3]=data;
    usHeaderSize=16;
    usDataSize=0;
}

void PACKET::FillAsyncReadblockResp(int rcode, int length)
{
    if (rcode!=RCODE_COMPLETE)
        length=0;

    PREP_ASYNC_HEAD_RCODE(TCODE_READB_RESPONSE);
    pHeader[3]=length << 16;
    usHeaderSize=16;
    usDataSize=length+(length%4?4-(length%4):0);
}

void PACKET::FillAsyncWriteResp(int rcode)
{
    PREP_ASYNC_HEAD_RCODE(TCODE_WRITE_RESPONSE);
    pHeader[2] = 0;
    usHeaderSize = 12;
    usDataSize = 0;
}

void PACKET::FillAsyncLockResp(int rcode, int extcode, int length)
{
    if (rcode != RCODE_COMPLETE)
        length = 0;

    PREP_ASYNC_HEAD_RCODE(TCODE_LOCK_RESPONSE);
    pHeader[3]=(length << 16) | extcode;
    usHeaderSize=16;
    usDataSize=length;
}
#define PREP_ASYNC_HEAD_ADDRESS(tc) \
    tcode=tc; \
    pHeader[0]=((ULONG)NodeId << 16) | ((ULONG)tlabel << 10) \
            | (1 << 8) | (tc << 4); \
    pHeader[1]=((ULONG)pHost->NodeId << 16) | (ULONG)(addr >> 32); \
    pHeader[2]=(ULONG)addr & 0xffffffff
void PACKET::FillAsyncReadquad(U64 addr)
{
    PREP_ASYNC_HEAD_ADDRESS(TCODE_READQ);

    usHeaderSize=12;
    usDataSize=0;
    bExpectResponse=TRUE;
}
void PACKET::FillAsyncReadblock(U64 addr, USHORT length)
{
    PREP_ASYNC_HEAD_ADDRESS(TCODE_READB);
    pHeader[3]=(ULONG)length << 16;
    usHeaderSize=16;
    usDataSize=0;
    bExpectResponse=TRUE;
}
void PACKET::FillAsyncWritequad(U64 addr, QUADLET data)
{
    PREP_ASYNC_HEAD_ADDRESS(TCODE_WRITEQ);
    pHeader[3]=data;
    usHeaderSize=16;
    usDataSize=0;
    bExpectResponse=TRUE;
}
void PACKET::FillAsyncWriteblock(U64 addr, int length)
{
    PREP_ASYNC_HEAD_ADDRESS(TCODE_WRITEB);
    pHeader[3]=(ULONG)length << 16;
    usHeaderSize=16;
    bExpectResponse=TRUE;
    usDataSize=length+(length % 4 ? 4 - (length % 4) : 0);
}
void PACKET::FillAsyncLock(U64 addr, int extcode, int length)
{
    PREP_ASYNC_HEAD_ADDRESS(TCODE_LOCK_REQUEST);
    pHeader[3] = ((ULONG)length << 16) | extcode;
    usHeaderSize=16;
    usDataSize=length;
    bExpectResponse = 1;
}
void PACKET::FillIsoPacket(int length, int channel, int tag, int sync)
{
    pHeader[0]=((ULONG)length << 16) | (tag << 14) | (channel << 8)
            | (TCODE_ISO_DATA << 4) | sync;

    usHeaderSize=4;
    usDataSize=length;
    type=hpsb_iso;
    tcode=TCODE_ISO_DATA;
}
void PACKET::FillPhyPacket(QUADLET data)
{
    pHeader[0]=data;
    pHeader[1]=~data;
    usHeaderSize=8;
    usDataSize=0;
    bExpectResponse=0;
    type=hpsb_raw;             /* No CRC added */
    SpeedCode=IEEE1394_SPEED_100; /* Force speed to be 100Mbps */
}
void PACKET::FillAsyncStreamPacket(int length, int channel, int tag, int sync)
{
    pHeader[0]=((ULONG)length << 16) | ((ULONG)tag << 14) | (channel << 8)
              | (TCODE_STREAM_DATA << 4) | sync;

    usHeaderSize=4;
    usDataSize=length;
    type=hpsb_async;
    tcode=TCODE_ISO_DATA;
}
int PACKET::Success(void)
{
    switch (AckCode)
    {
        case ACK_PENDING:
            switch ((pHeader[1] >> 12) & 0xf)
            {
                case RCODE_COMPLETE:
                    return FIRE_NO_ERROR;
                case RCODE_CONFLICT_ERROR:
                    return FIRE_ERROR_AGAIN;
                case RCODE_DATA_ERROR:
                    return FIRE_ERROR_REMOTEIO;
                case RCODE_TYPE_ERROR:
                    return FIRE_ERROR_ACCESS;
                case RCODE_ADDRESS_ERROR:
                    return FIRE_ERROR_INVALID;
                default:
                    ddprintf("ERROR: received reserved rcode %d from node %d\n",
                             (pHeader[1] >> 12) & 0xf, NodeId);
                    return FIRE_ERROR_AGAIN;
            }
//            HPSB_PANIC("reached unreachable code 1 in %s", __FUNCTION__);

        case ACK_BUSY_X:
        case ACK_BUSY_A:
        case ACK_BUSY_B:
            return FIRE_ERROR_BUSY;

        case ACK_TYPE_ERROR:
            return FIRE_ERROR_ACCESS;

        case ACK_COMPLETE:
            if (tcode == TCODE_WRITEQ||tcode == TCODE_WRITEB)
            {
                return 0;
            }
            else
            {
                ddprintf("ERROR: impossible ack_complete from node %d (tcode %d)\n", NodeId, tcode);
                return FIRE_ERROR_AGAIN;
            }
        case ACK_DATA_ERROR:
            if (tcode==TCODE_WRITEB||tcode==TCODE_LOCK_REQUEST)
            {
                return FIRE_ERROR_AGAIN;
            }
            else
            {
                ddprintf("ERROR: impossible ack_data_error from node %d (tcode %d)\n",NodeId,tcode);
                return FIRE_ERROR_AGAIN;
            }

        case ACK_ADDRESS_ERROR:
            return FIRE_ERROR_INVALID;

        case ACK_TARDY:
        case ACK_CONFLICT_ERROR:
        case ACKX_NONE:
        case ACKX_SEND_ERROR:
        case ACKX_ABORTED:
        case ACKX_TIMEOUT:
            /* error while sending */
            return FIRE_ERROR_AGAIN;

        default:
            ddprintf("ERROR: got invalid ack %d from node %d (tcode %d)\n",
                         AckCode, NodeId, tcode);
            return FIRE_ERROR_AGAIN;
        }
}
/**
 * hpsb_get_tlabel - allocate a transaction label
 * @packet: the packet who's tlabel/tpool we set
 *
 * Every asynchronous transaction on the 1394 bus needs a transaction
 * label to match the response to the request.  This label has to be
 * different from any other transaction label in an outstanding request to
 * the same node to make matching possible without ambiguity.
 *
 * There are 64 different tlabels, so an allocated tlabel has to be freed
 * with hpsb_free_tlabel() after the transaction is complete (unless it's
 * reused again for the same target node).
 *
 * Return value: Zero on success, otherwise non-zero. A non-zero return
 * generally means there are no available tlabels. If this is called out
 * of interrupt or atomic context, then it will sleep until can return a
 * tlabel.
 */
int PACKET::GetTLabel(void)
{
    char i;
    //    unsigned long flags;
    //    struct hpsb_tlabel_pool *tp;
    TLABELPOOL far  *tp;

    tp = &pHost->tpool[NodeId & NODE_MASK];

    //      if (irqs_disabled() || in_atomic()) {
    if (!_IF())
    {
    //        ddprintf("BUG - called from cli()\n");
        if (tp->count==0)
            return 1;
//        if (down_trylock(&tp->count))
//            return 1;
    }
    else
    {
        tp->count--;
    //        down(&tp->count);
    }

    //    spin_lock_irqsave(&tp->lock, flags);
    pushf();
    cli();

//    tlabel = find_next_zero_bit(tp->pool, 64, tp->next);
    for (i=tp->next; i<64;i++)
        if ((tp->pool&(1ULL<<i))==0)
            break;
    tlabel=i;

    if (tlabel > 63)
    {
//        tlabel = find_first_zero_bit(pool, 64);
        for (i=0;i<64;i++)
            if ((tp->pool&(1ULL<<i))==0)
                break;
            tlabel=i;
    }
    tp->next = (tlabel + 1) % 64;
    /* Should _never_ happen */
    if ((tp->pool&(1ULL<<tlabel))==1)
        ddprintf("BUG: TLABEL already used!\n");
    tp->pool=tp->pool|(1ULL<<tlabel);
    //    BUG_ON(test_and_set_bit(packet->tlabel, tp->pool));

    tp->allocations++;
    popf();
//    spin_unlock_irqrestore(&tp->lock, flags);

    return 0;
}
/**
 * hpsb_free_tlabel - free an allocated transaction label
 * @packet: packet whos tlabel/tpool needs to be cleared
 *
 * Frees the transaction label allocated with hpsb_get_tlabel().  The
 * tlabel has to be freed after the transaction is complete (i.e. response
 * was received for a split transaction or packet was sent for a unified
 * transaction).
 *
 * A tlabel must not be freed twice.
 */

void PACKET::FreeTLabel(void)
{
//        unsigned long flags;
//    struct hpsb_tlabel_pool *tp;
    TLABELPOOL far  * tp;

    tp = &pHost->tpool[NodeId & NODE_MASK];

//    BUG_ON(packet->tlabel > 63 || packet->tlabel < 0);
    if (tlabel>63||tlabel<0)
        ddprintf("BUG: tlabel in packet out of range\n");

//        spin_lock_irqsave(&tp->lock, flags);
    pushf();
    cli();
    if ((tp->pool&(1ULL<<tlabel))==0)
        ddprintf("BUG: transaction label already clear\n");
    tp->pool=tp->pool&(~(1ULL<<tlabel));
//    BUG_ON(!test_and_clear_bit(packet->tlabel, tp->pool));
//        spin_unlock_irqrestore(&tp->lock, flags);
    popf();

//    up(&tp->count);
    tp->count++;
}


