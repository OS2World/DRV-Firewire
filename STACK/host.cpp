/*
** Module   :HOST.CPP
** $Id: host.cpp,v 1.14 2004/07/29 16:53:45 doctor64 Exp $
** Abstract : host related functions
**
** Copyright (C) Alex Cherkaev
**
** Log: Fri  21/05/2004 Created
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

//#include "ohci.hpp"
#include "host.hpp"
//#include "stack.hpp"
#include "packet.hpp"
#include "ddprintf.h"
#include "csr1212.h"
#include "jiffies.h"
#include "fireerror.h"
#include "global.hpp"
#include <include.h>
void AbortTimedouts(ULONG __opaque)
{
	ddprintf("DEBUG: AbortTimedouts \n");
    HOST *host = (HOST*)__opaque;
    PPACKET packet;
    unsigned long expire;

    pushf();
    cli();
    expire = host->pCSR->expire;
    popf();

    /* Hold the lock around this, since we aren't dequeuing all
     * packets, just ones we need. */
    pushf();
    cli();

    //    while (!skb_queue_empty(&host->pending_packet_queue)) {
    while (!host->PendingPacket.isEmpty())
    {
    //        skb = skb_peek(&host->pending_packet_queue);
    //        packet = (struct hpsb_packet *)skb->data;
        packet=(PPACKET)host->PendingPacket.GetHead();

        if (getJiffies()-(packet->sendtime + expire)>0)
        {
            host->PendingPacket.DeleteHead();
    //            __skb_unlink(skb, skb->list);
            packet->state = PACKET::hpsb_complete;
            packet->AckCode = ACKX_TIMEOUT;
            packet->QueueComplete();
        }
        else
        {
            /* Since packets are added to the tail, the oldest
             * ones are first, always. When we get to one that
             * isn't timed out, the rest aren't either. */
            break;
        }
    }

    if (!host->PendingPacket.isEmpty())
        host->Timeout.ModTimer(getJiffies() + host->ulTimeoutInterval);
    //        mod_timer(&host->timeout, jiffies + host->timeout_interval);

    popf();
}

void DelayedResetBus(ULONG __opaque)
{
	ddprintf("DEBUG: DelayedResetBus \n");
    HOST * pWorkHost;
    pWorkHost=(HOST far *)__opaque;
    int lgeneration=pWorkHost->pCSR->generation+1;

    /* The generation field rolls over to 2 rather than 0 per IEEE
     * 1394a-2000. */
    if (lgeneration>0xf || lgeneration<2)
        lgeneration=2;

    CSR_SET_BUS_INFO_GENERATION(pWorkHost->pCSR->rom, lgeneration);
    if (csr1212_generate_csr_image(pWorkHost->pCSR->rom)!=CSR1212_SUCCESS)
    {
        /* CSR image creation failed, reset generation field and do not
         * issue a bus reset. */
        CSR_SET_BUS_INFO_GENERATION(pWorkHost->pCSR->rom, pWorkHost->pCSR->generation);
        return;
    }

    pWorkHost->pCSR->generation=(UCHAR)lgeneration;

    pWorkHost->bUpdateConfigRom=FALSE;
    if (pWorkHost->pHardDriver->fpfnSetHwConfigRom)
        pWorkHost->pHardDriver->fpfnSetHwConfigRom(pWorkHost->pCSR->rom->bus_info_data);

    pWorkHost->pCSR->genTimestamp[pWorkHost->pCSR->generation]=getJiffies();
    pWorkHost->ResetBus(SHORT_RESET);
}


/**
 * hpsb_alloc_host - allocate a new host controller.
 * @drv: the driver that will manage the host controller
 * @extra: number of extra bytes to allocate for the driver
 *
 * Allocate a &hpsb_host and initialize the general subsystem specific
 * fields.  If the driver needs to store per host data, as drivers
 * usually do, the amount of memory required can be specified by the
 * @extra parameter.  Once allocated, the driver should initialize the
 * driver specific parts, enable the controller and make it available
 * to the general subsystem using hpsb_add_host().
 *
 * Return Value: a pointer to the &hpsb_host if succesful, %NULL if
 * no memory was available.
 */
HOST::HOST(UCHAR HostNumber)
{
    pushf();
    cli();
    Generation=0;
    popf();
    ulTimeoutInterval=0;
    int i;
    for (i=0;i<64;i++) (IsoListenCount[i]=0);

    usNodeCount=0; // number of identified nodes on bus
    usSelfIdCount=0; //number of SelfID received
    usNodesActive=0; //number of actually active nodes

    NodeId=0; // node id of this host
    IRMId=0; // node id of this bus isocronous resource manager
    BusMgrId=0; //node id of this bus bus manager

    /* this nodes state */
    bInBusReset=FALSE;
    bIsShutdown=FALSE;

    /* this nodes' duties on the bus */
    bIsRoot=FALSE;
    bIsCycmst=FALSE;
    bIsIrm=FALSE;
    bIsBusmgr=FALSE;

    usResetRetries=0;
    pTopologyMap=NULL;
    pSpeedMap=NULL;


    usId=0;

    bUpdateConfigRom=FALSE;

    usConfigRoms=0;
//    pHardDriver=NULL;
//    pStack=&GlobalStack;

    int hostnum = 0;
    pCSR=new CSR;
    if (pCSR==NULL) ddprintf("ERROR: pCSR is NULL\n");
    pCSR->rom = csr1212_create_csr(fpGCsrBusOps, CSR_BUS_INFO_SIZE, this);
    DelayedReset.function=DelayedResetBus;
    DelayedReset.data=(ULONG)(HOST far *)this;
    AddTimer(&DelayedReset);
    for (i = 2; i < 16; i++)
        pCSR->genTimestamp[i]=getJiffies()-60*HZ;
    pushf();
    cli();
    Generation=0;
    popf();
    Timeout.data = (ULONG)(HOST far *)this;
    Timeout.function = AbortTimedouts;
    AddTimer(&Timeout);
    ulTimeoutInterval = 10000; // HZ/20; // 50ms by default
    pTopologyMap=pCSR->pTopologyMap+3;
    pSpeedMap=(UCHAR *)(pCSR->pSpeedMap+2);
    //    while (nodemgr_for_each_host(&hostnum, alloc_hostnum_cb))
    //        hostnum++;

    usId=HostNumber;
    for (i=0;i<64;i++)
    {
        tpool[i].pool=0;
        tpool[i].count=63;
        tpool[i].next=0;
        tpool[i].allocations=0;
    }


    // linux device manager registration
    //    memcpy(&h->device, &nodemgr_dev_template_host, sizeof(h->device));
    //    h->device.parent = dev;
    //    snprintf(h->device.bus_id, BUS_ID_SIZE, "fw-host%d", h->id);

    //    h->class_dev.dev = &h->device;
    //    h->class_dev.class = &hpsb_host_class;
    //    snprintf(h->class_dev.class_id, BUS_ID_SIZE, "fw-host%d", h->id);

    //    device_register(&h->device);
    //    class_device_register(&h->class_dev);
    //    get_device(&h->device);

    //    up(&host_num_alloc);

    //    return h;
}
void HOST::AddHost(DRIVEROPS far * physdriver)
{
    pHardDriver=physdriver;
    if (DefaultConfigEntry())
        return;
    AddExtraConfigRoms();

//    pStack->fpfnAddHost((void far *)this);
    //temp workaround for static host structs
    pStack->fpfnAddHost(fpGHostOps);
}

void HOST::RemoveHost(void)
{
    bIsShutdown=TRUE;
    pHardDriver=NULL;

    pStack->fpfnRemoveHost(fpGHostOps);

    RemoveExtraConfigRoms();

}
/* Updates the configuration rom image of a host.  rom_version must be the
 * current version, otherwise it will fail with return value -1. If this
 * host does not support config-rom-update, it will return -EINVAL.
 * Return value 0 indicates success.
 */

int HOST::UpdateConfigRomImage()
{
    ULONG ulreset_time;
    int next_gen = pCSR->generation+1;

    if (!bUpdateConfigRom)
        return -1;

    if (next_gen > 0xf)
        next_gen = 2;

    /* Stop the delayed interrupt, we're about to change the config rom and
     * it would be a waste to do a bus reset twice. */
    DelayedReset.DelTimerSync();

    /* IEEE 1394a-2000 prohibits using the same generation number
     * twice in a 60 second period. */
    if (getJiffies()-pCSR->genTimestamp[next_gen]<60*HZ)
      /* Wait 60 seconds from the last time this generation number was
        * used. */
        ulreset_time=(60*HZ)+pCSR->genTimestamp[next_gen];
    else
        /* Wait 1 second in case some other code wants to change the
         * Config ROM in the near future. */
        ulreset_time=getJiffies()+HZ;

    /* This will add the timer as well as modify it */
    DelayedReset.ModTimer(ulreset_time);

    return 0;
}
void HOST::SelfIdComplete(USHORT usPhyId, BOOL bIsRoot)
{
    if (!bInBusReset)
        ddprintf("WARNING: SelfID completion called outside of bus reset!\n");

    NodeId=LOCAL_BUS|usPhyId;
    HOST::bIsRoot=bIsRoot;

    if (!bCheckSelfIds())
    {
        if (usResetRetries++<20)
        {
            /* selfid stage did not complete without error */
            ddprintf("WARNING: Error in SelfID stage, resetting\n");
            bInBusReset=FALSE;
            /* this should work from ohci1394 now... */
            ResetBus(LONG_RESET);
            return;
        }
        else
        {
            ddprintf("WARNING: Stopping out-of-control reset loop\n");
            ddprintf("WARNING: topology map and speed map will not be valid\n");
            usResetRetries=0;
        }
    }
    else
    {
        usResetRetries=0;
        BuildSpeedMap(usNodeCount);
    }
    ddprintf("DEBUG: SelfIdComplete() called with successful SelfID stage irm_id: 0x%x node_id: 0x%x\n",IRMId,NodeId);

    /* IRMId is kept up to date by bCheckSelfIds() */
    if (IRMId==NodeId)
    {
        bIsIrm=TRUE;
    }
    else
    {
        bIsBusmgr=FALSE;
        bIsIrm=FALSE;
    }

    if (bIsRoot)
    {
        pHardDriver->fpfnDevCtl(ACT_CYCLE_MASTER, 1);
        bIsCycmst=TRUE;
    }
    pushf();
    cli();
    Generation++;
    popf();
    bInBusReset=FALSE;
    pStack->fpfnHostReset(fpGHostOps);
}
BOOL HOST::ResetBus(RESET_TYPES type)
{
    if (!bInBusReset)
    {
        pHardDriver->fpfnDevCtl(RESET_BUS, type);
        return FALSE;
    }
    else
        return TRUE;
}
BOOL HOST::BusReset(void)
{
    if (bInBusReset)
    {
        ddprintf("ERROR: BusReset() called while bus reset already in progress\n");
        return TRUE;
    }

    AbortRequests();
    bInBusReset=TRUE;
    IRMId=-1;
    bIsIrm=FALSE;
    BusMgrId=-1;
    bIsBusmgr=FALSE;
    bIsCycmst=FALSE;
    usNodeCount=0;
    usSelfIdCount=0;

    return FALSE;
}
/*
 * Verify num_of_selfids SelfIDs and return number of nodes.  Return zero in
 * case verification failed.
 */
BOOL HOST::bCheckSelfIds(void)
{
    int nodeid = -1;
    int rest_of_selfids = usSelfIdCount;
    PSELFID sid=(PSELFID)pTopologyMap;
    PEXTSELFID esid;
    int esid_seq = 23;

    usNodesActive=0;

    while (rest_of_selfids--)
    {
        if (!sid->extended)
        {
            nodeid++;
            esid_seq = 0;

            if (sid->phy_id != nodeid)
            {
                ddprintf("ERROR: SelfIDs failed monotony check with %d\n", sid->phy_id);
                return FALSE;
             }

            if (sid->link_active)
            {
                usNodesActive++;
                if (sid->contender)
                    IRMId=LOCAL_BUS|sid->phy_id;
            }
        }
        else
        {
            esid=(PEXTSELFID)sid;

            if ((esid->phy_id!=nodeid)||(esid->seq_nr!=esid_seq))
            {
                ddprintf("ERROR: SelfIDs failed monotony check with %d/%d\n",esid->phy_id,esid->seq_nr);
                return FALSE;
            }
            esid_seq++;
        }
        sid++;
    }
    esid = (PEXTSELFID)(sid - 1);
    while (esid->extended)
    {
        if ((esid->porta==0x2)||(esid->portb==0x2)||(esid->portc==0x2)||(esid->portd==0x2)
          ||(esid->porte==0x2)||(esid->portf==0x2)||(esid->portg==0x2)||(esid->porth==0x2))
        {
            ddprintf("ERROR: SelfIDs failed root check on extended SelfID\n");
            return FALSE;
        }
        esid--;
    }

    sid = (PSELFID)esid;
    if ((sid->port0==0x2)||(sid->port1==0x2)||(sid->port2==0x2))
    {
        ddprintf("ERROR: SelfIDs failed root check\n");
        return FALSE;
    }

    usNodeCount=nodeid+1;
    return TRUE;
}
void HOST::AbortRequests(void)
{
    PPACKET packet;

    pHardDriver->fpfnDevCtl(CANCEL_REQUESTS, 0);

    //    while ((skb = skb_dequeue(&host->pending_packet_queue)) != NULL) {
    while (!PendingPacket.isEmpty())
    {
        packet=(PPACKET)PendingPacket.GetHead();
        packet->state = PACKET::hpsb_complete;
        packet->AckCode = ACKX_ABORTED;
        packet->QueueComplete();
        PendingPacket.DeleteHead();
   }
}
void HOST::BuildSpeedMap(USHORT nodecount)
{
    UCHAR far * map=pSpeedMap;
    PSELFID sid;
    PEXTSELFID esid;
    int i, j, n;

    for (i=0;i<(nodecount*64);i+=64)
    {
        for (j = 0;j<nodecount;j++)
        {
            map[i+j]=IEEE1394_SPEED_MAX;
        }
    }

    for (i=0;i<nodecount;i++)
    {
        cldcnt[i]=0;
    }

    /* find direct children count and speed */
    for (sid=(PSELFID)&pTopologyMap[usSelfIdCount-1],n=nodecount-1;(PVOID)sid>=(PVOID)pTopologyMap; sid--)
    {
        if (sid->extended)
        {
            esid=(PEXTSELFID)sid;

            if (esid->porta == 0x3) cldcnt[n]++;
            if (esid->portb == 0x3) cldcnt[n]++;
            if (esid->portc == 0x3) cldcnt[n]++;
            if (esid->portd == 0x3) cldcnt[n]++;
            if (esid->porte == 0x3) cldcnt[n]++;
            if (esid->portf == 0x3) cldcnt[n]++;
            if (esid->portg == 0x3) cldcnt[n]++;
            if (esid->porth == 0x3) cldcnt[n]++;
        }
        else
        {
            if (sid->port0 == 0x3) cldcnt[n]++;
            if (sid->port1 == 0x3) cldcnt[n]++;
            if (sid->port2 == 0x3) cldcnt[n]++;

            speedcap[n]=sid->speed;
            n--;
        }
    }

    /* set self mapping */
    for (i=0; i<nodecount; i++)
    {
        map[64*i+i]=speedcap[i];
    }

    /* fix up direct children count to total children count;
     * also fix up speedcaps for sibling and parent communication */
    for (i=1; i<nodecount; i++)
    {
        for (j = cldcnt[i], n = i - 1; j > 0; j--)
        {
            cldcnt[i]+=cldcnt[n];
            speedcap[n]=min(speedcap[n], speedcap[i]);
            n-=cldcnt[n]+1;
        }
    }

    for (n=0; n<nodecount; n++)
    {
        for (i=n-cldcnt[n]; i<=n; i++)
        {
            for (j=0; j<(n-cldcnt[n]); j++)
            {
                map[j*64+i]=map[i*64+j]= min(map[i*64+j], speedcap[n]);
            }
            for (j=n+1; j<nodecount; j++)
            {
                map[j*64+i]=map[i*64+j]=min(map[i*64+j], speedcap[n]);
            }
        }
    }
}
void HOST::SelfIdReceived(QUADLET sid)
{
    if (bInBusReset)
    {
        ddprintf("DEBUG: Including SelfID 0x%lx\n", sid);
        pTopologyMap[usSelfIdCount++]=sid;
    }
    else
    {
        ddprintf("ERROR: Spurious SelfID packet (0x%lx) received from bus %d\n",sid,NODEID_TO_BUS(NodeId));
    }
}
void HOST::PacketSent(PPACKET packet, char ackcode)
{
    packet->AckCode=ackcode;

    if (packet->bNoWaiter)
    {
        /* must not have a tlabel allocated */
        delete packet;
        return;
    }

    if (ackcode!=ACK_PENDING||!packet->bExpectResponse)
    {
        pushf();
        cli();
        packet->RefCnt--;
        popf();
//        skb_unlink(packet->skb);

        packet->state = PACKET::hpsb_complete;
        packet->QueueComplete();
        return;
    }

    if (packet->state == PACKET::hpsb_complete)
    {
        delete packet;
        return;
    }

    pushf();
    cli();
    packet->RefCnt--;
    popf();
    packet->state = PACKET::hpsb_pending;
    packet->sendtime = getJiffies();

    Timeout.ModTimer(getJiffies()+ ulTimeoutInterval);
}

/**
 * HOST::SendPhyConfig - transmit a PHY configuration packet on the bus
 * @rootid: root whose force_root bit should get set (-1 = don't set force_root)
 * @gapcnt: gap count value to set (-1 = don't set gap count)
 *
 * This function sends a PHY config packet on the bus through the specified host.
 *
 * Return value: 0 for success or error number otherwise.
 */
APIRET HOST::SendPhyConfig(LONG lrootid, LONG lgapcnt)
{
    PPACKET pTempPacket;
    APIRET retval=0;

    if (lrootid>=ALL_NODES||lrootid<-1||lgapcnt>0x3f||lgapcnt<-1||
       (lrootid ==-1&&lgapcnt ==-1))
    {
        ddprintf("ERROR: Invalid Parameter: rootid = %d gapcnt = %d",lrootid,lgapcnt);
        //invlaid parametr
        return -1;
    }

    pTempPacket=new PACKET(0);
    if (pTempPacket==NULL) ddprintf("ERROR: pTempPacket is NULL\n");

    pTempPacket->pHost=this;
    pTempPacket->usHeaderSize=8;
    pTempPacket->usDataSize=0;
    pTempPacket->bExpectResponse=FALSE;
    pTempPacket->bNoWaiter=FALSE;
    pTempPacket->type=PACKET::hpsb_raw;
    pTempPacket->pHeader[0]=0;
    if (lrootid!=-1)
        pTempPacket->pHeader[0]|=lrootid<<24|1UL<<23;
    if (lgapcnt!=-1)
        pTempPacket->pHeader[0]|=lgapcnt<<16|1UL<<22;

    pTempPacket->pHeader[1]=~pTempPacket->pHeader[0];

    pTempPacket->Generation=GetGeneration();

    retval=pTempPacket->SendAndWait();
    delete pTempPacket;

    return retval;
}
/*int dummyTransmitPacket(PPACKET pPacket)
{
    return 0;
}
int dummyDevCtl(enum DEVCTL_CMD c, int arg)
{
        return -1;
}

//int dummyISOCtl(struct hpsb_iso *iso, enum isoctl_cmd command, unsigned long arg)
//{
//    return -1;
//}
HOST_DRIVER dummyDriver =
{
    NULL, //pfnSetHwConfigRom
    dummyTransmitPacket, //pfnTransmitPacket
    dummyDevCtl, //pfnDevCtl
//  NULL, //pfnIsoCtl
    NULL  //pfnHwCsrReg
//    .isoctl =          dummy_isoctl
};*/
void HOST::HandlePacketResponse(int tcode, QUADLET far * data, int size)
{
//        struct hpsb_packet *packet = NULL;
//    struct sk_buff *skb;
    int tcode_match = 0;
    int tlabel;
    PPACKET packet;

    tlabel=(data[0]>>10)&0x3f;

    pushf();
    cli();

    //    skb_queue_walk(&host->pending_packet_queue, skb) {
    while (!PendingPacket.isEmpty())
    {
        packet=(PPACKET)PendingPacket.GetHead();
        if ((packet->tlabel==tlabel)&&(packet->NodeId==(data[1]>>16)))
            break;
        packet = NULL;
    }

    if (packet==NULL)
    {
        ddprintf("ERROR: unsolicited response packet received - no tlabel match\n");
        PacketDump("contents:", data, 16);
        popf();
        return;
    }

    switch (packet->tcode)
    {
        case TCODE_WRITEQ:
        case TCODE_WRITEB:
            if (tcode != TCODE_WRITE_RESPONSE)
                break;
            tcode_match = 1;
            _fmemcpy(packet->pHeader, data, 12);
            break;
        case TCODE_READQ:
            if (tcode != TCODE_READQ_RESPONSE)
                break;
            tcode_match = 1;
            _fmemcpy(packet->pHeader, data, 16);
            break;
        case TCODE_READB:
            if (tcode != TCODE_READB_RESPONSE)
                break;
            tcode_match = 1;
//        BUG_ON(packet->skb->len - sizeof(*packet) < size - 16);
            _fmemcpy(packet->pHeader, data, 16);
            _fmemcpy(packet->pData, data + 4, size - 16);
            break;
        case TCODE_LOCK_REQUEST:
            if (tcode != TCODE_LOCK_RESPONSE)
             break;
         tcode_match = 1;
            size = min((size - 16), (size_t)8);
//            BUG_ON(packet->skb->len - sizeof(*packet) < size);
            _fmemcpy(packet->pHeader, data, 16);
            _fmemcpy(packet->pData, data + 4, size);
            break;
    }

    if (!tcode_match)
    {
        ddprintf("ERROR: unsolicited response packet received - tcode mismatch\n");
        PacketDump("contents:", data, 16);
        popf();
        return;
    }

//    __skb_unlink(skb, skb->list);
    popf();

    if (packet->state == PACKET::hpsb_queued)
    {
        packet->sendtime = getJiffies();
        packet->AckCode = ACK_PENDING;
    }

    packet->state = PACKET::hpsb_complete;
    packet->QueueComplete();
}
void HOST::AddExtraConfigRoms(void)
{
}
void HOST::RemoveExtraConfigRoms(void)
{
}
void HOST::PacketReceived(QUADLET far * data, size_t size, BOOL bwrite_acked)
{
    int tcode;

    if (bInBusReset)
    {
        ddprintf("WARNING: received packet during reset; ignoring\n");
        return;
    }

    PacketDump("received packet:", data, size);

    tcode=(char)(data[0] >> 4) & 0xf;

    switch (tcode)
    {
        case TCODE_WRITE_RESPONSE:
        case TCODE_READQ_RESPONSE:
        case TCODE_READB_RESPONSE:
        case TCODE_LOCK_RESPONSE:
            HandlePacketResponse(tcode, data, size);
            break;
        case TCODE_WRITEQ:
        case TCODE_WRITEB:
        case TCODE_READQ:
        case TCODE_READB:
        case TCODE_LOCK_REQUEST:
            HandleIncomingPacket(tcode, data, size, bwrite_acked);
            break;
        case TCODE_ISO_DATA:
            pStack->fpfnIsoReceive(fpGHostOps, data, size);
            break;

        case TCODE_CYCLE_START:
            /* simply ignore this packet if it is passed on */
            break;

        default:
            ddprintf("ERROR: received packet with bogus transaction code %d\n",tcode);
            break;
    }
}
PPACKET HOST::CreateReplyPacket(PQUADLET data, int dsize)
{
    PPACKET p;

    p=new PACKET(dsize);
    if (p==NULL)
    {
        ddprintf("ERROR: creating packet failed \n");
        return NULL;
    }
    p->type=PACKET::hpsb_async;
    p->state=PACKET::hpsb_unused;
    p->pHost=this;
    p->NodeId=(NODEID)(data[1]>>16);
    p->tlabel=(data[0]>>10)&0x3f;
    p->bNoWaiter=1;

    p->Generation=GetGeneration();

    if (dsize % 4)
        p->pData[dsize/4]=0;

    return p;
}
int HOST::GetGeneration(void)
{
    int temp;
    pushf();
    cli();
    temp=Generation;
    popf();
    return temp;
}

void HOST::HandleIncomingPacket(int tcode,QUADLET far * data, int size, BOOL bwrite_acked)
{
    PPACKET packet;
    int length, rcode, extcode;
    QUADLET buffer;
    NODEID source =(NODEID)( data[1] >> 16);
    NODEID dest = (NODEID)(data[0] >> 16);
    USHORT flags = (USHORT) data[0];
    U64 addr;

    /* big FIXME - no error checking is done for an out of bounds length */

    switch (tcode)
    {
        case TCODE_WRITEQ:
            addr = (((U64)(data[1] & 0xffff)) << 32) | data[2];
            rcode = pStack->fpfnWrite(fpGHostOps, source, dest, data+3, addr, 4, flags);

            if (!bwrite_acked&&(NODEID_TO_NODE(data[0]>>16)!=NODE_MASK)&&(rcode >= 0))
            {
                /* not a broadcast write, reply */
                packet = CreateReplyPacket(data, 0);
                if (packet == NULL) break;
                packet->FillAsyncWriteResp(rcode);
                packet->SendNoCare();
            }
            break;

        case TCODE_WRITEB:
            addr = (((U64)(data[1] & 0xffff)) << 32) | data[2];
            rcode = pStack->fpfnWrite(fpGHostOps, source, dest, data+4,addr, (USHORT)(data[3]>>16), flags);

            if (!bwrite_acked&&(NODEID_TO_NODE(data[0]>>16)!=NODE_MASK)&&(rcode>=0))
            {
                /* not a broadcast write, reply */
                packet = CreateReplyPacket(data, 0);
                if (packet == NULL) break;
                packet->FillAsyncWriteResp(rcode);
                packet->SendNoCare();
            }
            break;

        case TCODE_READQ:
            addr = (((U64)(data[1] & 0xffff)) << 32) | data[2];
            rcode = pStack->fpfnRead(fpGHostOps, source, &buffer, addr, 4, flags);

            if (rcode >= 0)
            {
                packet = CreateReplyPacket(data, 0);
                if (packet == NULL) break;
                packet->FillAsyncReadquadResp(rcode, buffer);
                packet->SendNoCare();
            }
            break;

        case TCODE_READB:
            length = (USHORT)(data[3] >> 16);
            packet = CreateReplyPacket(data, length);
            if (packet == NULL) break;

            addr = (((U64)(data[1] & 0xffff)) << 32) | data[2];
            rcode = pStack->fpfnRead(fpGHostOps, source, packet->pData, addr, length, flags);

            if (rcode >= 0)
            {
                packet->FillAsyncReadblockResp(rcode, length);
                packet->SendNoCare();
            }
            else
            {
                delete packet;
            }
            break;

        case TCODE_LOCK_REQUEST:
            length = (USHORT)(data[3] >> 16);
            extcode = (USHORT)(data[3] & 0xffff);
            addr = (((U64)(data[1] & 0xffff)) << 32) | data[2];

            packet = CreateReplyPacket(data, 8);
            if (packet == NULL) break;

            if ((extcode == 0) || (extcode >= 7))
            {
                /* let switch default handle error */
                length = 0;
            }
            switch (length)
            {
                case 4:
                    rcode = pStack->fpfnLock(fpGHostOps, source, packet->pData, addr,data[4], 0, extcode,flags);
                    packet->FillAsyncLockResp(rcode, extcode, 4);
                    break;
                case 8:
                    if ((extcode!=EXTCODE_FETCH_ADD)&&(extcode != EXTCODE_LITTLE_ADD))
                    {
                        rcode = pStack->fpfnLock(fpGHostOps, source,packet->pData, addr,
                                                       data[5], data[4],
                                                       extcode, flags);
                        packet->FillAsyncLockResp(rcode, extcode, 4);
                    }
                    else
                    {
                        rcode = pStack->fpfnLock64(fpGHostOps, source,(POCTLET)packet->pData, addr,
                                             *(POCTLET)(data + 4), 0ULL,
                                             extcode, flags);
                        packet->FillAsyncLockResp(rcode, extcode, 8);
                    }
                    break;
                case 16:
                    rcode = pStack->fpfnLock64(fpGHostOps, source,
                                     (POCTLET)packet->pData, addr,
                                     *(POCTLET)(data + 6),
                                     *(POCTLET)(data + 4),
                                     extcode, flags);
                    packet->FillAsyncLockResp(rcode, extcode, 8);
                    break;
                default:
                    rcode = RCODE_TYPE_ERROR;
                    packet->FillAsyncLockResp(rcode, extcode, 0);
            }

            if (rcode >= 0)
            {
                packet->SendNoCare();
            }
            else
            {
                delete packet;
            }
            break;
    }

}
PPACKET HOST::MakeReadPacket(NODEID node, U64 addr, int length)
{
    PPACKET packet;

    if (length == 0)
        return NULL;

    packet = new PACKET(length);
    if (!packet)
        return NULL;

    packet->pHost=this;
    packet->NodeId=node;

    if (packet->GetTLabel())
    {
        delete packet;
        return NULL;
    }

    if (length == 4)
        packet->FillAsyncReadquad(addr);
    else
        packet->FillAsyncReadblock(addr, length);

    return packet;
}

PPACKET HOST::MakeWritePacket(NODEID node,U64 addr, PQUADLET buffer, int length)
{
    PPACKET packet;

    if (length == 0)
        return NULL;

    packet= new PACKET(length);
    if (!packet)
        return NULL;

    if (length % 4) { /* zero padding bytes */
        packet->pData[length >> 2] = 0;
    }
    packet->pHost=this;
    packet->NodeId=node;

    if (packet->GetTLabel())
    {
        delete packet;
        return NULL;
    }

    if (length == 4)
        packet->FillAsyncWritequad(addr, buffer ? *buffer : 0);
    else
    {
        packet->FillAsyncWriteblock(addr, length);
        if (buffer)
            _fmemcpy(packet->pData, buffer, length);
    }

    return packet;
}
PPACKET HOST::MakeStreamPacket(UCHAR far *buffer, int length, int channel, int tag, int sync)
{
    PPACKET packet;

    if (length == 0)
        return NULL;

    packet = new PACKET(length);
    if (!packet)
        return NULL;

    if (length % 4) { /* zero padding bytes */
        packet->pData[length >> 2] = 0;
    }
    packet->pHost=this;

    if (packet->GetTLabel())
    {
        delete packet;
        return NULL;
    }

    packet->FillAsyncStreamPacket(length, channel, tag, sync);
    if (buffer)
        _fmemcpy(packet->pData, buffer, length);

    return packet;
}
PPACKET HOST::MakeLockPacket(NODEID node,U64 addr, int extcode, PQUADLET data,QUADLET arg)
{
    PPACKET p;
    ULONG length;

    p= new PACKET(8);
    if (!p) return NULL;

    p->pHost=this;
    p->NodeId=node;
    if (p->GetTLabel())
    {
        delete p;
        return NULL;
    }

    switch (extcode)
    {
        case EXTCODE_FETCH_ADD:
        case EXTCODE_LITTLE_ADD:
            length = 4;
            if (data)
                p->pData[0] = *data;
            break;
        default:
            length = 8;
            if (data)
            {
                p->pData[0]=arg;
                p->pData[1]=*data;
            }
            break;
    }
    p->FillAsyncLock(addr, extcode, length);

    return p;
}

PPACKET HOST::MakeLock64Packet(NODEID node,U64 addr, int extcode, POCTLET data,OCTLET arg)
{
    PPACKET p;
    ULONG length;

    p = new PACKET(16);
    if (!p) return NULL;

    p->pHost = this;
    p->NodeId = node;
    if (p->GetTLabel())
    {
        delete p;
        return NULL;
    }

    switch (extcode)
    {
        case EXTCODE_FETCH_ADD:
        case EXTCODE_LITTLE_ADD:
            length = 8;
            if (data)
            {
                p->pData[0] = (QUADLET)(*data >> 32);
                p->pData[1] = (QUADLET)(*data & 0xffffffff);
            }
            break;
        default:
            length = 16;
            if (data)
            {
                p->pData[0] = (QUADLET)(arg >> 32);
                p->pData[1] = (QUADLET)(arg & 0xffffffff);
                p->pData[2] = (QUADLET)(*data >> 32);
                p->pData[3] = (QUADLET)(*data & 0xffffffff);
            }
            break;
    }
    p->FillAsyncLock(addr, extcode, length);

    return p;
}

PPACKET HOST::MakePhyPacket(QUADLET data)
{
    PPACKET p;

    p = new PACKET(0);
    if (!p) return NULL;

    p->pHost = this;
    p->FillPhyPacket(data);

    return p;
}
PPACKET HOST::MakeIsoPacket(int length, int channel,int tag, int sync)
{
    PPACKET p;

    p = new PACKET(length);
    if (!p) return NULL;

    p->pHost=this;
    p->FillIsoPacket(length, channel, tag, sync);

    p->Generation = GetGeneration();

    return p;
}
int HOST::Read(NODEID node, unsigned int generation,U64 addr, PQUADLET buffer, int length)
{
    PPACKET packet;
    int retval = 0;

    if (length == 0) return -FIRE_ERROR_INVALID;

    //    BUG_ON(in_interrupt()); // We can't be called in an interrupt, yet
//    if (gInInterrupt) ddprintf("BUG: HOST::Read cant be called from interrupt\n");

    packet = MakeReadPacket(node, addr, length);

    if (!packet) return -FIRE_ERROR_NOMEMORY;

    packet->Generation = generation;
    retval = packet->SendAndWait();
    if (retval < 0)
        goto read_fail;

    retval = packet->Success();

    if (retval == 0)
    {
        if (length == 4)
            *buffer = packet->pHeader[3];
        else
            _fmemcpy(buffer, packet->pData, length);
    }

read_fail:
    packet->FreeTLabel();
    delete packet;

    return retval;
}

int HOST::Write(NODEID node, unsigned int generation, U64 addr, PQUADLET buffer, int length)
{
    PPACKET packet;
    int retval;

    if (length == 0) return -FIRE_ERROR_INVALID;

    //    BUG_ON(in_interrupt()); // We can't be called in an interrupt, yet
//    if (gInInterrupt) ddprintf("BUG: HOST::Write cant be called from interrupt\n");

    packet = MakeWritePacket(node, addr, buffer, length);

    if (!packet) return -FIRE_ERROR_NOMEMORY;

    packet->Generation = generation;
    retval = packet->SendAndWait();
    if (retval < 0)
        goto write_fail;

    retval = packet->Success();

write_fail:
    packet->FreeTLabel();
    delete packet;

    return retval;
}

int HOST::Lock(NODEID node, unsigned int generation, U64 addr, int extcode, PQUADLET data, QUADLET arg)
{
    PPACKET packet;
    int retval = 0;

    //    BUG_ON(in_interrupt()); // We can't be called in an interrupt, yet
//    if (gInInterrupt) ddprintf("BUG: HOST::Lock cant be called from interrupt\n");

    packet = MakeLockPacket(node, addr, extcode, data, arg);
    if (!packet) return -FIRE_ERROR_NOMEMORY;

    packet->Generation = generation;
    retval = packet->SendAndWait();
    if (retval < 0)
        goto lock_fail;

    retval = packet->Success();

    if (retval == 0) *data = packet->pData[0];

lock_fail:
    packet->FreeTLabel();
    delete packet;

    return retval;
}

int HOST::Lock64(NODEID node, unsigned int generation,U64 addr, int extcode, POCTLET data, OCTLET arg)
{
    PPACKET packet;
    int retval = 0;

//    if (gInInterrupt) ddprintf("BUG: HOST::Lock cant be called from interrupt\n");

    packet = MakeLock64Packet(node, addr, extcode, data, arg);
    if (!packet) return -FIRE_ERROR_NOMEMORY;

    packet->Generation = generation;
    retval = packet->SendAndWait();
    if (retval < 0)
        goto lock64_fail;

    retval = packet->Success();

    if (retval == 0)
        *data = (U64)packet->pData[1] << 32 | packet->pData[0];

lock64_fail:
    packet->FreeTLabel();
    delete packet;

    return retval;
}
int HOST::SendGasp(int channel, unsigned int generation,PQUADLET buffer, int length, ULONG specifier_id, unsigned int version)
{
    PPACKET packet;
    int retval = 0;
    USHORT specifier_id_hi = (USHORT)((specifier_id & 0x00ffff00) >> 8);
    UCHAR specifier_id_lo = (UCHAR)(specifier_id & 0xff);

    ddprintf("DEBUG: Send GASP: channel = %d, length = %d\n", channel, length);

    length += 8;

    packet = MakeStreamPacket(NULL, length, channel, 3, 0);
    if (!packet) return -FIRE_ERROR_NOMEMORY;

    packet->pData[0] = cpu_to_be32((NodeId << 16) | specifier_id_hi);
    packet->pData[1] = cpu_to_be32((((ULONG)specifier_id_lo) << 24) | (version & 0x00ffffff));

    _fmemcpy(&(packet->pData[2]), buffer, length - 8);

    packet->Generation = generation;

    packet->bNoWaiter = TRUE;

    retval = packet->Send();
    if (retval < 0)
        delete packet;

    return retval;
}
int HOST::DefaultConfigEntry(void)
{
    struct csr1212_keyval far *root;
    struct csr1212_keyval far *vend_id = NULL;
    struct csr1212_keyval far *text = NULL;
    static char csr_name[64];
    int ret;

//    sprintf(csr_name, "Linux - %s", host->driver->name);
    strcpy(csr_name,"OS2 - ");
    _fstrcat(csr_name,pHardDriver->name);
    root = pCSR->rom->root_kv;

    vend_id = csr1212_new_immediate(CSR1212_KV_ID_VENDOR, pCSR->GuidHi >> 8);
    text = csr1212_new_string_descriptor_leaf(csr_name);

    if (!vend_id || !text) {
        if (vend_id)
            csr1212_release_keyval(vend_id);
        if (text)
            csr1212_release_keyval(text);
        csr1212_destroy_csr(pCSR->rom);
        return -FIRE_ERROR_NOMEMORY;
    }

    ret = csr1212_associate_keyval(vend_id, text);
    csr1212_release_keyval(text);
    ret |= csr1212_attach_keyval_to_directory(root, vend_id);
    if (ret != CSR1212_SUCCESS) {
        csr1212_release_keyval(vend_id);
        csr1212_destroy_csr(pCSR->rom);
        return -FIRE_ERROR_NOMEMORY;
    }

    bUpdateConfigRom = TRUE;

    return 0;
}
