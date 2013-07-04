/*
** Module   :DMA.CPP
** Abstract :
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

#include "dma.hpp"
#include "malloc.h"
#include "ddprintf.h"
#include "ohciregs.h"
#include "ohci.hpp"
#include <string.h>
#include <include.h>
// dma receive context
/* Generate the dma receive prgs and start the context */
DMA_RCV_CONTEXT::DMA_RCV_CONTEXT(POHCIDRIVER ohci,
          CONTEXT_TYPE type, int ctx, int num_desc,
          int buf_size, int split_buf_size, int context_base)
{
    int i;

    pDriver = ohci;
    DMA_RCV_CONTEXT::context_type = type;
    DMA_RCV_CONTEXT::ctx = ctx;

    DMA_RCV_CONTEXT::num_desc = num_desc;
    DMA_RCV_CONTEXT::buf_size = buf_size;
    DMA_RCV_CONTEXT::split_buf_size = split_buf_size;

    DMA_RCV_CONTEXT::ctrlSet = 0;
    DMA_RCV_CONTEXT::ctrlClear = 0;
    DMA_RCV_CONTEXT::cmdPtr = 0;

    buf_cpu =(ULONG far **) malloc(DMA_RCV_CONTEXT::num_desc * sizeof(QUADLET far *));
    buf_bus =(ULONG *)malloc(DMA_RCV_CONTEXT::num_desc * sizeof(ULONG));
//    aBufMem =(PMEMORY*) malloc(DMA_RCV_CONTEXT::num_desc * sizeof(PMEMORY));
//    aPrgMem =(PMEMORY*)malloc(DMA_RCV_CONTEXT::num_desc * sizeof(PMEMORY));

    if (buf_cpu == NULL || buf_bus == NULL)
        //||aBufMem==NULL||aPrgMem==NULL)
    {
        ddprintf("ERROR: Failed to allocate dma buffer\n");
    //        free_dma_rcv_ctx(d);
        return;
    }
    memset(buf_cpu, 0, DMA_RCV_CONTEXT::num_desc * sizeof(QUADLET far *));
    memset(buf_bus, 0, DMA_RCV_CONTEXT::num_desc * sizeof(ULONG));
//    for (i=0;i<DMA_RCV_CONTEXT::num_desc * sizeof(QUADLET far *);i++) *(buf_cpu+i)=0;
//    for (i=0;i<DMA_RCV_CONTEXT::num_desc * sizeof(ULONG);i++) *(buf_bus+i)=0;

    prg_cpu=(DMA_CMD far **)malloc(DMA_RCV_CONTEXT::num_desc * sizeof(DMA_CMD far *));
    prg_bus=(ULONG *)malloc(DMA_RCV_CONTEXT::num_desc * sizeof(ULONG));

    if (prg_cpu == NULL || prg_bus == NULL)
    {
        ddprintf("ERROR: Failed to allocate dma prg\n");
//        free_dma_rcv_ctx(d);
        return;
    }
    memset(prg_cpu, 0, DMA_RCV_CONTEXT::num_desc * sizeof(DMA_CMD far *));
//    for (i=0;i<DMA_RCV_CONTEXT::num_desc * sizeof(DMA_CMD far *);i++) *(prg_cpu+i)=0;
    memset(prg_bus, 0, DMA_RCV_CONTEXT::num_desc * sizeof(ULONG));
//    for (i=0;i<DMA_RCV_CONTEXT::num_desc * sizeof(ULONG);i++) *(prg_bus+i)=0;


    spb = (ULONG far *)malloc(split_buf_size);

    if (spb == NULL)
    {
        ddprintf("ERROR: Failed to allocate split buffer\n");
//        free_dma_rcv_ctx(d);
        return;
    }

//    d->prg_pool = pci_pool_create("ohci1394 rcv prg", ohci->dev,
//                sizeof(struct dma_cmd), 4, 0);
//    OHCI_DMA_ALLOC("dma_rcv prg pool");
    aBufMem=new MEMORY(buf_size*DMA_RCV_CONTEXT::num_desc);
    if (aBufMem->fpVirtAddr==NULL)
    {
        ddprintf("ERROR: failed to allocate dma buffer, aBufMem->fpVirtAddr==NULL\n");
        return;
    }
    aPrgMem=new MEMORY(sizeof(DMA_CMD)*DMA_RCV_CONTEXT::num_desc);
    if (aPrgMem->fpVirtAddr==NULL)
    {
        ddprintf("ERROR: failed to allocate dma program , aPrgMem->fpVirtAddr==NULL\n");
        return;
    }

    for (i=0; i<DMA_RCV_CONTEXT::num_desc; i++)
    {
//        aBufMem[i]=new MEMORY(buf_size);
//        if (aBufMem[i]==NULL)
//        {
//            ddprintf("ERROR: failed to allocate dma buffer, aBufMem[%d]==NULL\n",i);
//            return;
//        }
        buf_cpu[i]=(ULONG far *)(aBufMem->fpVirtAddr+buf_size*i);
        buf_bus[i]=aBufMem->ulPhysAddr+buf_size*i;
        if (buf_cpu[i]!=NULL)
        {
            _fmemset(buf_cpu[i], 0, DMA_RCV_CONTEXT::buf_size);
//            for (j=0;j<DMA_RCV_CONTEXT::buf_size;j++) *(buf_cpu+j)=0;
        }
        else
        {
            ddprintf("ERROR:Failed to allocate dma buffer,buf_cpu[%d]==NULL\n",i);
            return;
        }
        ddprintf("DEBUG: consistent dma_rcv buf[%d]\n", i);

        prg_cpu[i]=(DMA_CMD far *)(aPrgMem->fpVirtAddr+sizeof(DMA_CMD)*i);
        prg_bus[i]=aPrgMem->ulPhysAddr+sizeof(DMA_CMD)*i;
        if (prg_cpu[i]!=NULL)
        {
            _fmemset(prg_cpu[i], 0, sizeof(DMA_CMD));
//          for (j=0;j<sizeof(DMA_CMD);j++) *(prg_cpu[i]+j)=0;

        }
        else
        {
            ddprintf("ERROR: Failed to allocate dma program, prg_cpu[%d]==NULL\n",i);
            return;
        }
    }

//        spin_lock_init(&d->lock);

    if (context_type == DMA_CTX_ISO)
    {
//        ohci1394_init_iso_tasklet(&ohci->ir_legacy_tasklet,
//                      OHCI_ISO_MULTICHANNEL_RECEIVE,
//                      dma_rcv_tasklet, (unsigned long) d);
//        if (ohci1394_register_iso_tasklet(ohci,
//                          &ohci->ir_legacy_tasklet) < 0) {
//            PRINT(KERN_ERR, "No IR DMA context available");
//            free_dma_rcv_ctx(d);
//            return -EBUSY;
//        }

        /* the IR context can be assigned to any DMA context
         * by ohci1394_register_iso_tasklet */
//        d->ctx = ohci->ir_legacy_tasklet.context;
//        d->ctrlSet = OHCI1394_IsoRcvContextControlSet + 32*d->ctx;
//        d->ctrlClear = OHCI1394_IsoRcvContextControlClear + 32*d->ctx;
//        d->cmdPtr = OHCI1394_IsoRcvCommandPtr + 32*d->ctx;
//        d->ctxtMatch = OHCI1394_IsoRcvContextMatch + 32*d->ctx;
    }
    else
    {
        ctrlSet = context_base + OHCI1394_ContextControlSet;
        ctrlClear = context_base + OHCI1394_ContextControlClear;
        cmdPtr = context_base + OHCI1394_ContextCommandPtr;

//        task.Init(dma_rcv_tasklet, (unsigned long) this);
    }

    return;
}
DMA_RCV_CONTEXT::~DMA_RCV_CONTEXT(void)
{
//    int i;
//    struct ti_ohci *ohci = d->ohci;

    if (pDriver == NULL)
        return;

    ddprintf("DEBUG: Freeing dma_rcv_ctx %d\n", ctx);

    if (buf_cpu)
    {
//        for (i=0; i<num_desc; i++)
//            if (buf_cpu[i] && buf_bus[i])
//            {
//                ddprintf("free consistent dma_rcv buf[%d]\n", i);
//                delete aBufMem[i];
//            }
        free(buf_cpu);
        free(buf_bus);
        delete aBufMem;
    }
    if (prg_cpu)
    {
//        for (i=0; i<num_desc; i++)
//            if (prg_cpu[i] && prg_bus[i])
//            {
//                ddprintf("free consistent dma_rcv prg[%d]\n", i);
//                delete aPrgMem[i];
//            }
        ddprintf("DEBUG: free dma_rcv prg pool\n");
        free(prg_cpu);
        free(prg_bus);
        delete aPrgMem;
//        free(aBufMem);
    }
    if (spb!=NULL) free(spb);

    /* Mark this context as freed. */
    pDriver = NULL;
}

void DMA_RCV_CONTEXT::Initialize(BOOL bGenerateIRQ)
{
    int i;

    pDriver->StopContext(ctrlClear, NULL);

    for (i=0; i<num_desc; i++)
    {
        ULONG c;

        c = DMA_CTL_INPUT_MORE | DMA_CTL_UPDATE | DMA_CTL_BRANCH;
        if (bGenerateIRQ)
            c |= DMA_CTL_IRQ;

        prg_cpu[i]->ulControl = c|buf_size;

        /* End of descriptor list? */
        if (i + 1 < num_desc)
        {
            prg_cpu[i]->ulBranchAddress =(prg_bus[i+1] & 0xfffffff0)|0x1;
        }
        else
        {
            prg_cpu[i]->ulBranchAddress=(prg_bus[0] & 0xfffffff0);
        }

        prg_cpu[i]->ulAddress = buf_bus[i];
        prg_cpu[i]->ulStatus = buf_size;
    }
    //dump dma program
    ddprintf("DEBUG: recv dma prog generated\n");
    ddprintf("addr virt  | addr phys  | Control    | Address    | branchAddr | status\n");
    for (i=0;i<num_desc;i++)
        ddprintf("0x%0lx | 0x%0lx | 0x%0lx | 0x%0lx | 0x%0lx | 0x%0lx\n",prg_cpu[i], prg_bus[i],
            prg_cpu[i]->ulControl, prg_cpu[i]->ulAddress, prg_cpu[i]->ulBranchAddress,prg_cpu[i]->ulStatus );
    buf_ind = 0;
    buf_offset = 0;

    if (context_type == DMA_CTX_ISO)
    {
        /* Clear contextControl */
        pDriver->writeRegister(ctrlClear, 0xffffffff);

        /* Set bufferFill, isochHeader, multichannel for IR context */
        pDriver->writeRegister(ctrlSet, 0xd0000000);

        /* Set the context match register to match on all tags */
        pDriver->writeRegister(ctxtMatch, 0xf0000000);

        /* Clear the multi channel mask high and low registers */
        pDriver->writeRegister(OHCI1394_IRMultiChanMaskHiClear, 0xffffffff);
        pDriver->writeRegister(OHCI1394_IRMultiChanMaskLoClear, 0xffffffff);

        /* Set up isoRecvIntMask to generate interrupts */
        pDriver->writeRegister(OHCI1394_IsoRecvIntMaskSet, 1UL << ctx);
    }

    /* Tell the controller where the first AR program is */
    pDriver->writeRegister(cmdPtr, prg_bus[0] | 0x1);

    /* Run context */
    pDriver->writeRegister(ctrlSet, 0x00008000);

    ddprintf("DEBUG: Receive DMA ctx=%d initialized\n", ctx);

}
void DMA_RCV_CONTEXT::StopContext(void)
{
    if (ctrlClear)
    {
        pDriver->StopContext(ctrlClear, NULL);

        if (context_type == DMA_CTX_ISO)
        {
            /* disable interrupts */
            pDriver->writeRegister(OHCI1394_IsoRecvIntMaskClear, 1UL<<ctx);
//            ohci1394_unregister_iso_tasklet(d->ohci, &d->ohci->ir_legacy_tasklet);
        }
        else
        {
//            tasklet_kill(&d->task);
        }
    }
}
/*
 * Determine the length of a packet in the buffer
 * Optimization suggested by Pascal Drolet <pascal.drolet@informission.ca>
 */
static const int TCODE_SIZE[16] = {20, 0, 16, -1, 16, 20, 20, 0,
                -1, 0, -1, 0, -1, -1, 16, -1};

int DMA_RCV_CONTEXT::PacketLength(int idx, QUADLET far *buf_ptr,
             int offset, unsigned char tcode)
{
    int length = -1;

    if (context_type == DMA_CTX_ASYNC_REQ || context_type == DMA_CTX_ASYNC_RESP)
    {
        length = TCODE_SIZE[tcode];
        if (length == 0)
        {
            if (offset + 12 >= buf_size)
            {
                length = (buf_cpu[(idx+1)%num_desc][3-((buf_size-offset)>>2)]) >> 16;
            }
            else
            {
                length = (USHORT)(buf_ptr[3]>>16);
            }
            length+=20;
        }
    }
    else
        if (context_type == DMA_CTX_ISO)
        {
        /* Assumption: buffer fill mode with header/trailer */
            length = ((USHORT)(buf_ptr[0]>>16))+8;
        }

    if (length > 0 && length % 4)
        length += 4 - (length % 4);

    return length;
}

/* Put the buffer back into the dma context */
void DMA_RCV_CONTEXT::InsertDMABuffer(int idx)
{
//    struct ti_ohci *ohci = (struct ti_ohci*)(d->ohci);
    int i;
    ddprintf("DEBUG: Inserting dma buf ctx=%d idx=%d\n",ctx, idx);

    prg_cpu[idx]->ulStatus = buf_size;
    prg_cpu[idx]->ulBranchAddress &= (0xfffffff0);
    idx = (idx + num_desc - 1 ) % num_desc;
    prg_cpu[idx]->ulBranchAddress |= (0x00000001);
    ddprintf("DEBUG: rcv dma prog generated\n");
    ddprintf("DEBUG: curr idx %d \n",idx);
    ddprintf("addr virt  | addr phys  | Control    | Address    | branchAddr | status\n");
    for (i=0;i<num_desc;i++)
        ddprintf("0x%0lx | 0x%0lx | 0x%0lx | 0x%0lx | 0x%0lx | 0x%0lx\n",prg_cpu[i], prg_bus[i],
            prg_cpu[i]->ulControl, prg_cpu[i]->ulAddress, prg_cpu[i]->ulBranchAddress,prg_cpu[i]->ulStatus );

    /* wake up the dma context if necessary */
    if (!(pDriver->ulReadRegister(ctrlSet) & 0x400))
    {
        ddprintf("INFO: Waking dma ctx=%d ... processing is probably too slow\n",ctx);
    }

    /* do this always, to avoid race condition */
    pDriver->writeRegister(ctrlSet, 0x1000);
}
DMA_TRM_CONTEXT::~DMA_TRM_CONTEXT(void)
{
//    int i;
//    struct ti_ohci *ohci = d->ohci;

    if (pDriver== NULL)
        return;

    ddprintf("DEBUG: Freeing dma_trm_ctx %d\n", ctx);

    if (prg_cpu)
    {
//        for (i=0; i<num_desc; i++)
//            if (prg_cpu[i] && prg_bus[i])
//            {
//                delete aPrgMem[i];
//                ddprintf("free pool dma_trm prg[%d]\n", i);
//            }
        delete aPrgMem;
        ddprintf("DEBUG: free dma_trm prg pool\n");
        free(prg_cpu);
        free(prg_bus);

    }

    /* Mark this context as freed. */
    pDriver = NULL;
}
DMA_TRM_CONTEXT::DMA_TRM_CONTEXT(POHCIDRIVER pDriver,
          CONTEXT_TYPE type, int ctx, int num_desc,
          int context_base)
{
    int i;

    DMA_TRM_CONTEXT::pDriver = pDriver;
    DMA_TRM_CONTEXT::type = type;
    DMA_TRM_CONTEXT::ctx = ctx;
    DMA_TRM_CONTEXT::num_desc = num_desc;
    DMA_TRM_CONTEXT::ctrlSet = 0;
    DMA_TRM_CONTEXT::ctrlClear = 0;
    DMA_TRM_CONTEXT::cmdPtr = 0;

    prg_cpu=(AT_DMA_PRG far * *)malloc(DMA_TRM_CONTEXT::num_desc * sizeof(AT_DMA_PRG far *));
    prg_bus=(ULONG *)malloc(DMA_TRM_CONTEXT::num_desc * sizeof(ULONG));
    aPrgMem=new MEMORY(DMA_TRM_CONTEXT::num_desc * sizeof(AT_DMA_PRG));
    if (aPrgMem->fpVirtAddr==NULL)
    {
        ddprintf("ERROR: failed to allocate dma program , aPrgMem->fpVirtAddr==NULL\n");
        return;
    }

    if (prg_cpu == NULL || prg_bus == NULL||aPrgMem == NULL)
    {
        ddprintf("ERROR: Failed to allocate dma prg\n");
//        free_dma_trm_ctx(d);
        return;
    }
    memset(prg_cpu, 0, DMA_TRM_CONTEXT::num_desc * sizeof(AT_DMA_PRG far *));
    memset(prg_bus, 0, DMA_TRM_CONTEXT::num_desc * sizeof(ULONG));
//    for (i=0;i< num_desc * sizeof(ULONG);i++) *(prg_bus+i)=0;

    ddprintf("DEBUG: alloc dma_trm prg pool\n");

    for (i=0; i<DMA_TRM_CONTEXT::num_desc; i++)
    {

//        aPrgMem[i]=new MEMORY(sizeof(AT_DMA_PRG));
        prg_cpu[i]=(AT_DMA_PRG far *)(aPrgMem->fpVirtAddr+i*sizeof(AT_DMA_PRG));
        prg_bus[i]=aPrgMem->ulPhysAddr+i*sizeof(AT_DMA_PRG);
        if (prg_cpu[i]!=NULL)
        {
            _fmemset(prg_cpu[i], 0, sizeof(AT_DMA_PRG));
        }
        else
        {
            ddprintf("ERROR: Failed to allocate dma program, prg_cpu[%d]==NULL\n",i);
            return;
        }
    }

//        spin_lock_init(&d->lock);

    /* initialize tasklet */
    if (type == DMA_CTX_ISO)
    {
//        ohci1394_init_iso_tasklet(&ohci->it_legacy_tasklet, OHCI_ISO_TRANSMIT,
//                      dma_trm_tasklet, (unsigned long) d);
//        if (ohci1394_register_iso_tasklet(ohci,
//                          &ohci->it_legacy_tasklet) < 0) {
//            PRINT(KERN_ERR, "No IT DMA context available");
//            free_dma_trm_ctx(d);
//            return -EBUSY;
//        }

        /* IT can be assigned to any context by register_iso_tasklet */
//        d->ctx = ohci->it_legacy_tasklet.context;
//        d->ctrlSet = OHCI1394_IsoXmitContextControlSet + 16 * d->ctx;
//        d->ctrlClear = OHCI1394_IsoXmitContextControlClear + 16 * d->ctx;
//        d->cmdPtr = OHCI1394_IsoXmitCommandPtr + 16 * d->ctx;
    }
    else
    {
        ctrlSet = context_base + OHCI1394_ContextControlSet;
        ctrlClear = context_base + OHCI1394_ContextControlClear;
        cmdPtr = context_base + OHCI1394_ContextCommandPtr;
//        Task.Init(dma_trm_tasklet, (unsigned long)this);
    }

    return;
}

/* Initialize the dma transmit context */
void DMA_TRM_CONTEXT::Initialize(void)
{

    /* Stop the context */
    pDriver->StopContext(ctrlClear, NULL);

    prg_ind = 0;
    sent_ind = 0;
    free_prgs = num_desc;
    branchAddrPtr = NULL;
//    INIT_LIST_HEAD(&d->fifo_list);
//    INIT_LIST_HEAD(&d->pending_list);

    if (type == DMA_CTX_ISO)
    {
        /* enable interrupts */
        pDriver->writeRegister(OHCI1394_IsoXmitIntMaskSet, 1UL << ctx);
    }

    ddprintf("DEBUG: Transmit DMA ctx=%d initialized \n", ctx);
}

void DMA_TRM_CONTEXT::Reset(void)
{
//    unsigned long flags;
//    LIST_HEAD(packet_list);
//    struct ti_ohci *ohci = d->ohci;
//    struct hpsb_packet *packet, *ptmp;
      PACKET far * packet;
    //PLISTENTRY listElem;

    pDriver->StopContext(ctrlClear, NULL);

    /* Lock the context, reset it and release it. Move the packets
     * that were pending in the context to packet_list and free
     * them after releasing the lock. */

    pushf();
    cli();
    PacketList.AddList(&fifo_list);
    PacketList.AddList(&pending_list);
//    list_splice(&d->fifo_list, &packet_list);
//    list_splice(&d->pending_list, &packet_list);
//    INIT_LIST_HEAD(&d->fifo_list);
//    INIT_LIST_HEAD(&d->pending_list);
    fifo_list.DeleteAll();
    pending_list.DeleteAll();

    branchAddrPtr = NULL;
    sent_ind = prg_ind;
    free_prgs = num_desc;

    popf();

    if (PacketList.isEmpty())
        return;

    ddprintf("DEBUG: AT dma reset ctx=%d, aborting transmission\n", ctx);

    /* Now process subsystem callbacks for the packets from this
     * context. */
    packet=PacketList.GetHead();
    while (packet!=NULL)
    {
        PacketList.DeleteHead();
        pDriver->pHostOps->fpfnPacketSent(pDriver->pHostOps,packet, ACKX_ABORTED);
        packet=PacketList.GetHead();
    }

}
/* Tasklet that processes dma receive buffers */

void DMA_RCV_CONTEXT::DMATasklet(void)
{
    unsigned int split_left, idx, offset, rescount;
    unsigned char tcode;
    int length, bytes_left;
    BOOL ack;
//    unsigned long flags;

    QUADLET far *buf_ptr;
    char far *split_ptr;
    char msg[256];

//    spin_lock_irqsave(&d->lock, flags);

    idx = buf_ind;
    offset=buf_offset;
    buf_ptr=buf_cpu[idx] + offset/4;

    rescount = ((USHORT)prg_cpu[idx]->ulStatus) & 0xffff;
    bytes_left = buf_size - rescount - offset;

    while (bytes_left > 0)
    {
        tcode = ((buf_ptr[0]) >> 4) & 0xf;

        /* packet_length() will return < 4 for an error */
        length = PacketLength(idx, buf_ptr, offset, tcode);

        if (length < 4)
        { /* something is wrong */
            ddprintf("ERROR: Unexpected tcode 0x%x(0x%0lx) in AR ctx=%d, length=%d\n",
                tcode, buf_ptr[0],
                ctx, length);
            pDriver->StopContext(ctrlClear, msg);
//            spin_unlock_irqrestore(&d->lock, flags);
            return;
        }

        /* The first case is where we have a packet that crosses
         * over more than one descriptor. The next case is where
         * it's all in the first descriptor.  */
        if ((offset + length) > buf_size)
        {
            ddprintf("INFO: Split packet rcv'd\n");
            if (length > split_buf_size)
            {
                pDriver->StopContext(ctrlClear,"Split packet size exceeded\n");
                buf_ind = idx;
                buf_offset = offset;
//                spin_unlock_irqrestore(&d->lock, flags);
                return;
            }

            if ((prg_cpu[(idx+1)%num_desc]->ulStatus)
                == buf_size)
            {
                /* Other part of packet not written yet.
                 * this should never happen I think
                 * anyway we'll get it on the next call.  */
                ddprintf("INFO: Got only half a packet!\n");
                buf_ind = idx;
                buf_offset = offset;
//                spin_unlock_irqrestore(&d->lock, flags);
                return;
            }

            split_left = length;
            split_ptr = (char far *)spb;
            _fmemcpy(split_ptr,buf_ptr,buf_size-offset);
//          for (int i=0;i<buf_size-offset;i++) *(split_ptr+i)=*(buf_ptr+i);
            split_left -= buf_size-offset;
            split_ptr += buf_size-offset;
            InsertDMABuffer(idx);
            idx = (idx+1) % num_desc;
            buf_ptr = buf_cpu[idx];
            offset=0;

            while (split_left >= buf_size)
            {
                _fmemcpy(split_ptr,buf_ptr,buf_size);
                split_ptr += buf_size;
                split_left -= buf_size;
                InsertDMABuffer(idx);
                idx = (idx+1) % num_desc;
                buf_ptr = buf_cpu[idx];
            }

            if (split_left > 0) {
                _fmemcpy(split_ptr, buf_ptr, split_left);
                offset = split_left;
                buf_ptr += offset/4;
            }
        }
        else
        {
            ddprintf("DEBUG: Single packet rcv'd\n");
            _fmemcpy(spb, buf_ptr, length);
            offset += length;
            buf_ptr += length/4;
            if (offset==buf_size)
            {
                InsertDMABuffer(idx);
                idx = (idx+1) % num_desc;
                buf_ptr = buf_cpu[idx];
                offset=0;
            }
        }

        /* We get one phy packet to the async descriptor for each
         * bus reset. We always ignore it.  */
        if (tcode != OHCI1394_TCODE_PHY)
        {
//            if (!ohci->no_swap_incoming)
//                packet_swab(d->spb, tcode);
            ddprintf("DEBUG: Packet received from node"
                " %d ack=0x%x spd=%d tcode=0x%x"
                " length=%d ctx=%d tlabel=%d\n",
                (USHORT)((spb[1]>>16)&0x3f),
                (USHORT)((spb[length/4-1]>>16)&0x1f),
                (USHORT)((spb[length/4-1]>>21)&0x3),
                (USHORT)tcode, length, (USHORT)ctx,
                (USHORT)((spb[length/4-1]>>10)&0x3f));

            ack = (((spb[length/4-1]>>16)&0x1f)== 0x11) ? TRUE : FALSE;

            pDriver->pHostOps->fpfnPacketReceived(pDriver->pHostOps,spb, length-4, ack);
        }
        else
            ddprintf("DEBUG: Got phy packet ctx=%d ... discarded\n",ctx);

        rescount=((USHORT)prg_cpu[idx]->ulStatus) & 0xffff;

        bytes_left = buf_size - rescount - offset;

    }

    buf_ind = idx;
    buf_offset = offset;

//    spin_unlock_irqrestore(&d->lock, flags);
}
/* Bottom half that processes sent packets */
void DMA_TRM_CONTEXT::DMATasklet(void)
{
    PACKET far * packet;
    int i;
    //, ptmp;
    //    struct hpsb_packet *packet, *ptmp;
//    unsigned long flags;
    USHORT usStatus, usAck;
    int datasize;

//    spin_lock_irqsave(&d->lock, flags);
    pushf();
    cli();

//    list_for_each_entry_safe(packet, ptmp, &d->fifo_list, driver_list)
    ddprintf("DEBUG: trm dma prog generated\n");
    ddprintf("DEBUG: curr sent ind %d \n",sent_ind);
    ddprintf("    addr virt  | addr phys  | Control    | Address    | branchAddr | status\n");
//    for (i=0;i<sent_ind+1;i++)
//    {
        ddprintf("beg: 0x%0lx| 0x%0lx | 0x%0lx | 0x%0lx | 0x%0lx | 0x%0lx\n",prg_cpu[sent_ind], prg_bus[sent_ind],
            prg_cpu[sent_ind]->dmacmdBegin.ulControl, prg_cpu[sent_ind]->dmacmdBegin.ulAddress, prg_cpu[sent_ind]->dmacmdBegin.ulBranchAddress,prg_cpu[sent_ind]->dmacmdBegin.ulStatus );
        ddprintf("data: 0x%0lx 0x%0lx 0x%0lx 0x%0lx\n",prg_cpu[sent_ind]->data[0],prg_cpu[sent_ind]->data[1],prg_cpu[sent_ind]->data[2],prg_cpu[sent_ind]->data[3]);
        ddprintf("end: 0x%0lx| 0x%0lx | 0x%0lx | 0x%0lx | 0x%0lx | 0x%0lx\n",prg_cpu[sent_ind], prg_bus[sent_ind],
            prg_cpu[sent_ind]->dmacmdEnd.ulControl, prg_cpu[sent_ind]->dmacmdEnd.ulAddress, prg_cpu[sent_ind]->dmacmdEnd.ulBranchAddress,prg_cpu[sent_ind]->dmacmdEnd.ulStatus );
//     }


    while (!fifo_list.isEmpty())
    {
        packet=fifo_list.GetHead();
        datasize = packet->usDataSize;
        if (datasize && packet->type != PACKET::hpsb_raw)
            usStatus=(USHORT)(prg_cpu[sent_ind]->dmacmdEnd.ulStatus >> 16);
        else
            usStatus=(USHORT)(prg_cpu[sent_ind]->dmacmdBegin.ulStatus >> 16);

        if (usStatus == 0)
            /* this packet hasn't been sent yet*/
            break;

        if (datasize)
            if (((prg_cpu[sent_ind]->data[0]>>4)&0xf) == 0xa)
                ddprintf("DEBUG: Stream packet sent to channel %d tcode=0x%x "
                       "ack=0x%x spd=%d dataLength=%d ctx=%d\n",
                       (USHORT)(((prg_cpu[sent_ind]->data[0])>>8)&0x3f),
                       (USHORT)(((prg_cpu[sent_ind]->data[0])>>4)&0xf),
                       usStatus&0x1f, (usStatus>>5)&0x3,
                       (USHORT)((prg_cpu[sent_ind]->data[1])>>16),
                       ctx);
            else
                ddprintf("DEBUG: Packet sent to node %d tcode=0x%x tLabel="
                       "0x%x ack=0x%x spd=%d dataLength=%d ctx=%d\n",
                       ((prg_cpu[sent_ind]->data[1])>>16)&0x3f,
                       ((prg_cpu[sent_ind]->data[0])>>4)&0xf,
                       ((prg_cpu[sent_ind]->data[0])>>10)&0x3f,
                       usStatus&0x1f, (usStatus>>5)&0x3,
                       (prg_cpu[sent_ind]->data[3])>>16,
                       ctx);
        else
            ddprintf("DEBUG: Packet sent to node %d tcode=0x%x tLabel="
                   "0x%x ack=0x%x spd=%d data=0x%lx ctx=%d\n",
                                (USHORT)((prg_cpu[sent_ind]->data[1]>>16)&0x3f),
                                (USHORT)(((prg_cpu[sent_ind]->data[0])>>4)&0xf),
                                (USHORT)(((prg_cpu[sent_ind]->data[0])>>10)&0x3f),
                                usStatus&0x1f, usStatus>>5&0x3,
                                prg_cpu[sent_ind]->data[3],
                                (USHORT)ctx);

        if (usStatus & 0x10)
        {
            usAck = usStatus & 0xf;
        }
        else
        {
            switch (usStatus & 0x1f)
            {
                case EVT_NO_STATUS: /* that should never happen */
                case EVT_RESERVED_A: /* that should never happen */
                case EVT_LONG_PACKET: /* that should never happen */
                    ddprintf("WARNING: Received OHCI evt_* error 0x%x\n", usStatus & 0x1f);
                    usAck = ACKX_SEND_ERROR;
                    break;
                case EVT_MISSING_ACK:
                    usAck = ACKX_TIMEOUT;
                    break;
                case EVT_UNDERRUN:
                    usAck = ACKX_SEND_ERROR;
                    break;
                case EVT_OVERRUN: /* that should never happen */
                    ddprintf("WARNING: Received OHCI evt_* error 0x%x\n", usStatus & 0x1f);
                    usAck = ACKX_SEND_ERROR;
                    break;
                case EVT_DESCRIPTOR_READ:
                case EVT_DATA_READ:
                case EVT_DATA_WRITE:
                    usAck = ACKX_SEND_ERROR;
                    break;
                case EVT_BUS_RESET: /* that should never happen */
                    ddprintf("WARNING: Received OHCI evt_* error 0x%x\n", usStatus & 0x1f);
                    usAck = ACKX_SEND_ERROR;
                    break;
                case EVT_TIMEOUT:
                    usAck = ACKX_TIMEOUT;
                    break;
                case EVT_TCODE_ERR:
                    usAck = ACKX_SEND_ERROR;
                    break;
                case EVT_RESERVED_B: /* that should never happen */
                case EVT_RESERVED_C: /* that should never happen */
                    ddprintf("WARNING: Received OHCI evt_* error 0x%x\n", usStatus & 0x1f);
                    usAck = ACKX_SEND_ERROR;
                    break;
                case EVT_UNKNOWN:
                case EVT_FLUSHED:
                    usAck = ACKX_SEND_ERROR;
                    break;
                default:
                    ddprintf("ERROR: Unhandled OHCI evt_* error 0x%x\n", usStatus & 0x1f);
                    usAck = ACKX_SEND_ERROR;
//                    BUG();
            }
        }

//        list_del_init(&packet->driver_list);
        pDriver->pHostOps->fpfnPacketSent(pDriver->pHostOps,packet, usAck);

        if (datasize)
        {
//            pci_unmap_single(ohci->dev,
//                     cpu_to_le32(d->prg_cpu[d->sent_ind]->end.address),
//                     datasize, PCI_DMA_TODEVICE);
            ddprintf("DEBUG: free single Xmit data packet\n");
        }

        sent_ind = (sent_ind+1)%num_desc;
        free_prgs++;
        fifo_list.DeleteHead();
    }

//    dma_trm_flush(ohci, d);

//    spin_unlock_irqrestore(&d->lock, flags);
    popf();
}

