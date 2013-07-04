/*
** Module   :OHCI.CPP
** Abstract :ohci controller class. handles all physical card activities
**
** Copyright (C) Alexandr Cherkaev
**
** Log: Wed  05/05/2004 Created
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

#include "ddprintf.h"

#include <devhelp.h>
#include "ohciregs.h"

#include "delay.h"
#include "ohci.hpp"
#include "..\stack\host.hpp"
//#include "stack.hpp"
#include "..\stack\packet.hpp"
#include "params.hpp"
#include "isr.h"
#include "global.hpp"
#include <devhelp.h>
#include <include.h>
void far __loadds sSetHwConfigRom(QUADLET far * pConfigRom)
{
    pOhciDriver->SetHwConfigRom(pConfigRom);
}
int far __loadds sTransmitPacket(void far * pPacket)
{
    return pOhciDriver->TransmitPacket((PACKET far *) pPacket);
}
ULONG far __loadds sDevCtl(enum DEVCTL_CMD Command, ULONG arg)
{
    return pOhciDriver->DevCtl(Command, arg);
}
int far __loadds sIsoCtl(void)
{
    return 0;
}
//    struct hpsb_iso *iso, enum ISOCTL_CMD Command, ULONG arg);
QUADLET far __loadds sHwCsrReg(int reg, QUADLET data, QUADLET compare)
{
    return pOhciDriver->HwCsrReg(reg, data, compare);
}

OHCIDRIVER::OHCIDRIVER(void)
{
//    bIsSetHwConfigRom=FALSE;
}
BOOL OHCIDRIVER::Probe(void)
{
   APIRET rc;
   //here we can start search on PCI bus for device
   // pci bus check
    pPCICard=new PCIDEV;
    if (pPCICard==NULL) ddprintf("ERROR: pPCICard is NULL\n");
    if (pPCICard->ucMajorVer < PCI_BIOS_VERSION)
    {   //pci version too old
        ddprintf("ERROR: Too old pci bios version\n");
        ddprintf("ERROR: Driver failed to install\n");
        // free objects
        delete pPCICard;
        // exiting
        return FALSE;
    }
    ddprintf("PCIBUS: mech %d, majorV %d, minorV %d, bus %d \n",pPCICard->ucHardwMechanism,pPCICard->ucMajorVer,pPCICard->ucMinorVer,pPCICard->ucLastBus);
    //find device. only one currently
    rc=pPCICard->findDeviceClass(0x000c0010,0);
    if (rc!=0)
    {
        ddprintf("PCIBUS: find device error %xh \n",rc);
        if (rc==0x86) ddprintf("ERROR: OHCI firewire card not found\n");
        // free objects
        delete pPCICard;
        // exiting
        return FALSE;
    }
    ddprintf("PCIBUS: device busno %d devfunc %d\n",pPCICard->ucBus,pPCICard->ucDevFunc);
    ddprintf("PCICARD: vendorid: %x deviceid %x\n",pPCICard->usGetVendorID(),pPCICard->usGetDeviceID());
    //get registers physical address
    ulRegistersPhys=pPCICard->ulGetBaseAddr();
    //
    ddprintf("DEBUG: ohci registers physaddr %lx\n",ulRegistersPhys);
    //allocate GDT selector for card registers mapping
    rc=DevHelp_AllocGDTSelector(&selRegisters,1);
    ddprintf("DEBUG: AllocGDTSelector() rc %x\n",rc);
    if (rc)
    {
      //error
      delete pPCICard;
        return FALSE;
    }
   //fill selector, map phys addr
    rc=DevHelp_PhysToGDTSelector(ulRegistersPhys,OHCI1394_REGISTER_SIZE,selRegisters);
    ddprintf("DEBUG: PhysToGDTSelector() rc %x\n",rc);
    if (rc)
    {
      //error
        DevHelp_FreeGDTSelector(selRegisters);
        delete pPCICard;
        return FALSE;
    }
   //make pointer to registers area
    fpulRegisters=(ULONG far *)MAKEP(selRegisters,0);
    ddprintf("DEBUG: card mmaped registers: selector %lx, pointer %lx\n",selRegisters, fpulRegisters);
    // enable phys dma by default, no if specified in parameters
    if (ParamsPhysDma==1) bPhysDma=TRUE; else bPhysDma=FALSE;

    // linux ohci1394_pci_probe() start
    // hpsb_alloc_host, alloc host and csr, TODO later
    //host = hpsb_alloc_host(&ohci1394_driver, sizeof(struct ti_ohci), &dev->dev);
   //    pHost=new HOST;
    // hardware swapping not needed
    pPCICard->writeRegDWord(OHCI1394_PCI_HCI_Control,0);
    // CSR config ROM allocation
    pCSRConfigROM=new CSRCONFIGROM(OHCI_CONFIG_ROM_LEN);
    if (pCSRConfigROM==NULL) ddprintf("ERROR: pCSRConfigROM is NULL\n");
    // self-id dma buff alloc
    pSelfIdBuf=new SELFIDBUF(OHCI1394_SI_DMA_BUF_SIZE);
    if (pSelfIdBuf==NULL) ddprintf("ERROR: pSelfIdBuf is NULL\n");
    // no selfid errors on start
    pSelfIdBuf->errors=0;
    // AR DMA request context alloc
    pARReqContext=new DMA_RCV_CONTEXT(this,
                        DMA_CTX_ASYNC_REQ, 0, AR_REQ_NUM_DESC,
                        AR_REQ_BUF_SIZE, AR_REQ_SPLIT_BUF_SIZE,
                        OHCI1394_AsReqRcvContextBase);
    if (pARReqContext==NULL) ddprintf("ERROR: pARReqContext is NULL\n");
    // AR DMA response context alloc
    pARRespContext=new DMA_RCV_CONTEXT(this,
                        DMA_CTX_ASYNC_RESP, 0, AR_RESP_NUM_DESC,
                        AR_RESP_BUF_SIZE, AR_RESP_SPLIT_BUF_SIZE,
                        OHCI1394_AsRspRcvContextBase);
    if (pARRespContext==NULL) ddprintf("ERROR: pARRespContext is NULL\n");
    // AT DMA request context alloc
    pATReqContext=new DMA_TRM_CONTEXT(this,
                        DMA_CTX_ASYNC_REQ, 0, AT_REQ_NUM_DESC,
                        OHCI1394_AsReqTrContextBase);
    if (pATReqContext==NULL) ddprintf("ERROR: pATReqContext is NULL\n");
    // AT DMA response context alloc
    pATRespContext=new DMA_TRM_CONTEXT(this,
                        DMA_CTX_ASYNC_RESP, 1, AT_RESP_NUM_DESC,
                        OHCI1394_AsRspTrContextBase);
    if (pATRespContext==NULL) ddprintf("ERROR: pATRespContext is NULL\n");
    SoftReset();
    /* Now enable LPS, which we need in order to start accessing
    * most of the registers.  In fact, on some cards (ALI M5251),
    * accessing registers in the SClk domain without LPS enabled
    * will lock up the machine.  Wait 50msec to make sure we have
    * full link enabled.  */
    writeRegister(OHCI1394_HCControlSet, OHCI1394_HCControl_LPS);
    // disable and clear interrupt
    writeRegister(OHCI1394_IntEventClear, 0xffffffff);
    writeRegister(OHCI1394_IntMaskClear, 0xffffffff);

    // wait 50 ms
    // mdelay(50);
    DevIODelay(50000);

    //isohronous context init, TODO later

    //setup irq handler, only shared i think
    DevHelp_SetIRQ((NPFN)&IRQHandler,pPCICard->ucGetIntrLine(),TRUE);

    // init card itself
    Initialize();

    /* Set certain csr values */
    pHostOps->pHost->pCSR->GuidHi=ulReadRegister(OHCI1394_GUIDHi);
    pHostOps->pHost->pCSR->GuidLo=ulReadRegister(OHCI1394_GUIDLo);
    pHostOps->pHost->pCSR->CycClkAcc = 100;  /* how do we determine clk accuracy? */
    pHostOps->pHost->pCSR->MaxRec=(ulReadRegister(OHCI1394_BusOptions)>>12)&0xf;
    pHostOps->pHost->pCSR->LinkSpeed=ulReadRegister(OHCI1394_BusOptions)&0x7;
    //fill pointers struct for host
    OhciHwDriver.fpfnSetHwConfigRom=sSetHwConfigRom;
    OhciHwDriver.fpfnTransmitPacket=sTransmitPacket;
    OhciHwDriver.fpfnDevCtl=sDevCtl;
    OhciHwDriver.fpfnIsoCtl=NULL;
    OhciHwDriver.fpfnHwCsrReg=sHwCsrReg;
    OhciHwDriver.pDriver=this;
    strcpy(OhciHwDriver.name,"ohci");
    //info stack what driver ready.
    pHostOps->fpfnAddHost(pHostOps,&OhciHwDriver);
    return TRUE;
}

OHCIDRIVER::~OHCIDRIVER(void)
{
    fpulRegisters=NULL;
    DevHelp_FreeGDTSelector(selRegisters);
   delete pPCICard;
}

ULONG OHCIDRIVER::ulReadRegister(USHORT usRegister)
{
    usRegister=usRegister>>2;
    return *(fpulRegisters+usRegister);
}
APIRET OHCIDRIVER::writeRegister(USHORT usRegister, ULONG ulValue)
{
    usRegister=usRegister>>2;
    *(fpulRegisters+usRegister)=ulValue;
   return NO_ERROR;
}
void OHCIDRIVER::SoftReset(void)
{
    int i;
    writeRegister(OHCI1394_HCControlSet, OHCI1394_HCControl_softReset);
    for (i=0; i<OHCI_LOOP_COUNT; i++)
    {
        if (!(ulReadRegister(OHCI1394_HCControlSet)&OHCI1394_HCControl_softReset))
            break;
        // do some sleep?
        DevIODelay(1000);
    }
    ddprintf("VERBOSE: Soft reset finished.\n");
}
UCHAR OHCIDRIVER::ReadPhysReg(UCHAR ucAddr)
{
    int i;
    ULONG r=0;
    pushf();
    cli();
    writeRegister(OHCI1394_PhyControl,(ucAddr<<8)|0x00008000);
    for (i=0;i<OHCI_LOOP_COUNT; i++)
    {
        if (ulReadRegister(OHCI1394_PhyControl)&0x80000000)
            break;
        //do some sleep?
        DevIODelay(1000);
    }
    r=ulReadRegister(OHCI1394_PhyControl);
    if (i>=OHCI_LOOP_COUNT)
        ddprintf("WARNING: Get PHY reg read timeout, 0x%lx,0x%lx, %d\n",r,r&0x80000000,i);
    popf();
    return (UCHAR)((r&0x00ff0000)>>16);
}
void OHCIDRIVER::WritePhysReg(UCHAR ucAddr, UCHAR ucValue)
{
    int i;
    ULONG r=0;

    pushf();
    cli();
    writeRegister(OHCI1394_PhyControl,(ucAddr<<8)|ucValue|0x00004000);
    for (i=0;i<OHCI_LOOP_COUNT; i++)
    {
        r=ulReadRegister(OHCI1394_PhyControl);
        if (!(r&0x00004000))
            break;
        //do some sleep?
        DevIODelay(1000);
    }
    if (i>=OHCI_LOOP_COUNT)
        ddprintf("WARNING: Set PHY reg timeout, 0x%lx,0x%lx, %d\n",r,r&0x00004000,i);
    popf();
    return;
}
/* Or's our value into the current value */
void OHCIDRIVER::MaskPhysReg(UCHAR ucAddr, UCHAR ucValue)
{
    UCHAR old;
    old=ReadPhysReg(ucAddr);
    old|=ucValue;
    WritePhysReg(ucAddr,ucValue);
}
void OHCIDRIVER::Initialize(void)
{
    ULONG buf;
    int num_ports, i;

    // bus options defaults
    buf=ulReadRegister(OHCI1394_BusOptions);
    buf|=0xE0000000;  // Enable IRMC, CMC, ISC
    buf&=~0x00FF0000; // set cyc_clk_acc to zero
    buf&=~0x18000000; // Disable PMC and BMC
    writeRegister(OHCI1394_BusOptions,buf);

    // set bus number
    writeRegister(OHCI1394_NodeID, 0x0000ffc0);

    //enable posted write
    writeRegister(OHCI1394_HCControlSet,OHCI1394_HCControl_postedWriteEnable);

    //clear link control register
    writeRegister(OHCI1394_LinkControlClear, 0xffffffff);

    //enable cycle timer, cycle master and set IRM contender bit in our selfID packet
    writeRegister(OHCI1394_LinkControlSet, OHCI1394_LinkControl_CycleTimerEnable|OHCI1394_LinkControl_CycleMaster);
    MaskPhysReg(4,0xc0);

    //setup SelfID dma buffer
    writeRegister(OHCI1394_SelfIDBuffer, pSelfIdBuf->ulPhysAddr);

    //enable SelfID and Phys
    writeRegister(OHCI1394_LinkControlSet, OHCI1394_LinkControl_RcvSelfID|OHCI1394_LinkControl_RcvPhyPkt);

    //setup ConfigROM mapping
    writeRegister(OHCI1394_ConfigROMmap, pCSRConfigROM->ulPhysAddr);

    //get our max packet size
    usMaxPacketSize=1<<(((ulReadRegister(OHCI1394_BusOptions)>>12)&0xf)+1);

    //don't accept phy packets into AR request context
    writeRegister(OHCI1394_LinkControlClear, 0x00000400);

    //clear interrupt mask
    writeRegister(OHCI1394_IsoRecvIntMaskClear, 0xffffffff);
    writeRegister(OHCI1394_IsoRecvIntEventClear, 0xffffffff);

    //initialize AR DMA
    pARReqContext->Initialize(FALSE);
    pARRespContext->Initialize(FALSE);

    //initialize AT dma
    pATReqContext->Initialize();
    pATRespContext->Initialize();

    /*
     * Accept AT requests from all nodes. This probably
     * will have to be controlled from the subsystem
     * on a per node basis.
     */
    writeRegister(OHCI1394_AsReqFilterHiSet, 0x80000000);

    // Specify AT retries
    writeRegister(OHCI1394_ATRetries,
           OHCI1394_MAX_AT_REQ_RETRIES |
          (OHCI1394_MAX_AT_RESP_RETRIES<<4) |
          (OHCI1394_MAX_PHYS_RESP_RETRIES<<8));

    // We don't want hardware swapping
    writeRegister(OHCI1394_HCControlClear, OHCI1394_HCControl_noByteSwap);

    //Enable interrupts
    writeRegister(OHCI1394_IntMaskSet,
          OHCI1394_unrecoverableError |
          OHCI1394_masterIntEnable |
          OHCI1394_busReset |
          OHCI1394_selfIDComplete |
          OHCI1394_RSPkt |
          OHCI1394_RQPkt |
          OHCI1394_respTxComplete |
          OHCI1394_reqTxComplete |
          OHCI1394_isochRx |
          OHCI1394_isochTx |
          OHCI1394_cycleInconsistent);

    // Enable link
    writeRegister(OHCI1394_HCControlSet, OHCI1394_HCControl_linkEnable);
    UCHAR t1,v1,v2;
    ULONG t2;
    t1=pPCICard->ucGetIntrLine();
    t2=pPCICard->ulGetBaseAddr();
    buf=ulReadRegister(OHCI1394_Version);
    v1=(UCHAR)((((buf) >> 16) & 0xf) + (((buf) >> 20) & 0xf) * 10);
    v2=(UCHAR)((((buf) >> 4) & 0xf) + ((buf) & 0xf) * 10);
    ddprintf("OHCI1394 %d.%d (PCI):IRQ=[%d] base addr=[%0lx-%0lx] Max Packet=[%d]\n",v1,v2,t1,t2,t2+OHCI1394_REGISTER_SIZE-1,usMaxPacketSize);

    /* Check all of our ports to make sure that if anything is
     * connected, we enable that port. */
    num_ports=ReadPhysReg(2)&0xf;
    for (i=0;i<num_ports;i++)
    {
        UCHAR status;

        WritePhysReg(7,(UCHAR)i);
        status=ReadPhysReg(8);
        if (status&0x20)
            WritePhysReg(8,status&~1);
    }

    // Serial EEPROM Sanity check.
    if ((usMaxPacketSize<512)||(usMaxPacketSize>4096))
    {
        /* Serial EEPROM contents are suspect, set a sane max packet
         * size and print the raw contents for bug reports if verbose
         * debug is enabled. */
        ddprintf("WARNING: Serial EEPROM has suspicious values, attempting to setting usMaxPacketSize to 512 bytes \n");
        writeRegister(OHCI1394_BusOptions,(ulReadRegister(OHCI1394_BusOptions)&0xf007)|0x8002);
        usMaxPacketSize=512;
        ddprintf("DEBUG: EEPROM Present: %d \n",(ulReadRegister(OHCI1394_Version)>>24)&0x1);
        writeRegister(OHCI1394_GUID_ROM, 0x80000000);
        for (i=0;((i<1000)&&(ulReadRegister(OHCI1394_GUID_ROM)&0x80000000));i++)
        {
   //            udelay(10);
         DevIODelay(10);
        // do some sleep?
        }
        for (i=0;i<0x20;i++)
        {
            writeRegister(OHCI1394_GUID_ROM, 0x02000000);
            ddprintf("    EEPROM %02x: %02x\n",i,(ulReadRegister(OHCI1394_GUID_ROM)>>16)&0xff);
        }
    }
}
BOOL OHCIDRIVER::IrqHandler(void)
{

    ULONG event, node_id;
    int phyid = -1, isroot = 0;
    //int3();

    /* Read and clear the interrupt event register.  Don't clear
     * the busReset event, though. This is done when we get the
     * selfIDComplete interrupt. */
    //    spin_lock_irqsave(&ohci->event_lock, flags);
    pushf();
    cli();
    event=ulReadRegister(OHCI1394_IntEventClear);
    writeRegister(OHCI1394_IntEventClear,event&~OHCI1394_busReset);
    popf();
    //    spin_unlock_irqrestore(&ohci->event_lock, flags);

    if (!event)
        return FALSE;

    /* If event is ~(u32)0 cardbus card was ejected.  In this case
     * we just return, and clean up in the ohci1394_pci_remove
     * function. */
//    if (event == ~(ULONG) 0) {
//        DBGMSG("Device removed.");
//        return IRQ_NONE;
//    }

    ddprintf("DEBUG: IntEvent: %lx\n", event);

    if (event&OHCI1394_unrecoverableError)
    {
        int ctx;
        ddprintf("ERROR: Unrecoverable error!\n");

        if (ulReadRegister(OHCI1394_AsReqTrContextControlSet)&0x800)
            ddprintf("DEBUG: Async Req Tx Context died: ctrl[%lx] cmdptr[%lx]\n",
                ulReadRegister(OHCI1394_AsReqTrContextControlSet),
                ulReadRegister(OHCI1394_AsReqTrCommandPtr));

        if (ulReadRegister(OHCI1394_AsRspTrContextControlSet) & 0x800)
            ddprintf("DEBUG: Async Rsp Tx Context died: ctrl[%lx] cmdptr[%lx]\n",
                ulReadRegister(OHCI1394_AsRspTrContextControlSet),
                ulReadRegister(OHCI1394_AsRspTrCommandPtr));

        if (ulReadRegister(OHCI1394_AsReqRcvContextControlSet) & 0x800)
            ddprintf("DEBUG: Async Req Rcv Context died: ctrl[%lx] cmdptr[%lx]\n",
                ulReadRegister(OHCI1394_AsReqRcvContextControlSet),
                ulReadRegister(OHCI1394_AsReqRcvCommandPtr));

        if (ulReadRegister(OHCI1394_AsRspRcvContextControlSet) & 0x800)
            ddprintf("DEBUG: Async Rsp Rcv Context died: ctrl[%lx] cmdptr[%lx]\n",
                ulReadRegister(OHCI1394_AsRspRcvContextControlSet),
                ulReadRegister(OHCI1394_AsRspRcvCommandPtr));

        for (ctx=0; ctx<this->ucNbIsoXmitCtx; ctx++)
        {
            if (ulReadRegister(OHCI1394_IsoXmitContextControlSet + (16 * ctx)) & 0x800)
                ddprintf("DEBUG: Iso Xmit %d Context died: ctrl[%lx] cmdptr[%lx]\n", ctx,
                    ulReadRegister(OHCI1394_IsoXmitContextControlSet + (16 * ctx)),
                    ulReadRegister(OHCI1394_IsoXmitCommandPtr + (16 * ctx)));
        }

        for (ctx = 0; ctx < this->ucNbIsoRcvCtx; ctx++)
        {
            if (ulReadRegister(OHCI1394_IsoRcvContextControlSet + (32 * ctx)) & 0x800)
                ddprintf("DEBUG: Iso Recv %d Context died: ctrl[%lx] cmdptr[%lx] match[%lx]\n", ctx,
                    ulReadRegister(OHCI1394_IsoRcvContextControlSet + (32 * ctx)),
                    ulReadRegister(OHCI1394_IsoRcvCommandPtr + (32 * ctx)),
                    ulReadRegister(OHCI1394_IsoRcvContextMatch + (32 * ctx)));
        }

        event&=~OHCI1394_unrecoverableError;
    }

    if (event&OHCI1394_cycleInconsistent)
    {
        /* We subscribe to the cycleInconsistent event only to
         * clear the corresponding event bit... otherwise,
         * isochronous cycleMatch DMA won't work. */
        ddprintf("DEBUG: OHCI1394_cycleInconsistent event\n");
        event &= ~OHCI1394_cycleInconsistent;
    }

    if (event&OHCI1394_busReset) {
        /* The busReset event bit can't be cleared during the
         * selfID phase, so we disable busReset interrupts, to
         * avoid burying the cpu in interrupt requests. */
        pushf();
        cli();
        writeRegister(OHCI1394_IntMaskClear, OHCI1394_busReset);

        if (bCheckBusReset)
        {
            int loop_count = 0;

         //            udelay(10);
           DevIODelay(10);

            while (ulReadRegister(OHCI1394_IntEventSet)&OHCI1394_busReset)
            {
                writeRegister(OHCI1394_IntEventClear,OHCI1394_busReset);
                popf();
            //                udelay(10);
                DevIODelay(10);
                pushf();
                cli();

                /* The loop counter check is to prevent the driver
                 * from remaining in this state forever. For the
                 * initial bus reset, the loop continues for ever
                 * and the system hangs, until some device is plugged-in
                 * or out manually into a port! The forced reset seems
                 * to solve this problem. This mainly effects nForce2. */
                if (loop_count > 10000)
                {
                    DevCtl(RESET_BUS, LONG_RESET);
                    ddprintf("WARNING: Detected bus-reset loop. Forced a bus reset!\n");
                    loop_count = 0;
                }
                loop_count++;
            }
        }
        popf();
        if (!((HOST far*)(pHostOps->pHost))->bInBusReset)
        {
            ddprintf("VERBOSE: irq_handler: Bus reset requested\n");

            /* Subsystem call */
            pHostOps->fpfnBusReset(pHostOps);
        }
        event&=~OHCI1394_busReset;
    }

    if (event&OHCI1394_reqTxComplete)
    {
        ddprintf("DEBUG: Got reqTxComplete interrupt status=0x%0lx\n",ulReadRegister(pATReqContext->ctrlSet));
        if (ulReadRegister(pATReqContext->ctrlSet) & 0x800)
            StopContext(pATReqContext->ctrlClear,"reqTxComplete");
        else
            pATReqContext->DMATasklet();
            //tasklet_schedule(&d->task);
//            ddprintf("DMAT hook armed \n");
//            DevHelp_ArmCtxHook((ULONG)pATReqContext, DMATTasklet);
        event&=~OHCI1394_reqTxComplete;
    }
    if (event&OHCI1394_respTxComplete)
    {
        ddprintf("DEBUG: Got respTxComplete interrupt status=0x%0lx\n",ulReadRegister(pATRespContext->ctrlSet));
        if (ulReadRegister(pATRespContext->ctrlSet) & 0x800)
            StopContext(pATRespContext->ctrlClear,"respTxComplete");
        else
//            pATRespContext->DMATasklet();
            ddprintf("DMAT hook armed \n");
            DevHelp_ArmCtxHook((ULONG)pATRespContext, DMATTasklet);
//            tasklet_schedule(&d->task);
        event&=~OHCI1394_respTxComplete;
    }
    if (event&OHCI1394_RQPkt)
    {
        ddprintf("DEBUG: Got RQPkt interrupt status=0x%0lx\n",ulReadRegister(pARReqContext->ctrlSet));
        if (ulReadRegister(pARReqContext->ctrlSet) & 0x800)
            StopContext(pARReqContext->ctrlClear,"RQPkt");
        else
//            pARReqContext->DMATasklet();
            ddprintf("DMAR hook armed \n");
            DevHelp_ArmCtxHook((ULONG)pARReqContext, DMARTasklet);
//            tasklet_schedule(&d->task);
        event&=~OHCI1394_RQPkt;
    }
    if (event&OHCI1394_RSPkt)
    {
        ddprintf("DEBUG: Got RSPkt interrupt status=0x%0lx\n",ulReadRegister(pARRespContext->ctrlSet));
        if (ulReadRegister(pARRespContext->ctrlSet) & 0x800)
            StopContext(pARRespContext->ctrlClear,"RSPkt");
        else
//            pARRespContext->DMATasklet();
            ddprintf("DMAR hook armed \n");
            DevHelp_ArmCtxHook((ULONG)pARRespContext, DMARTasklet);
//            tasklet_schedule(&d->task);
        event&=~OHCI1394_RSPkt;
    }
    if (event&OHCI1394_isochRx)
    {
        ULONG ulRXEvent;

        ulRXEvent=ulReadRegister(OHCI1394_IsoRecvIntEventSet);
        writeRegister(OHCI1394_IsoRecvIntEventClear,ulRXEvent);
//        ohci_schedule_iso_tasklets(ohci, rx_event, 0);
        event&=~OHCI1394_isochRx;
    }
    if (event&OHCI1394_isochTx)
    {
        ULONG ulTXEvent;

        ulTXEvent=ulReadRegister(OHCI1394_IsoXmitIntEventSet);
        writeRegister(OHCI1394_IsoXmitIntEventClear, ulTXEvent);
//        ohci_schedule_iso_tasklets(ohci, 0, tx_event);
        event&=~OHCI1394_isochTx;
    }
    if (event&OHCI1394_selfIDComplete)
    {
        if (((HOST far*)(pHostOps->pHost))->bInBusReset)
        {
            node_id=ulReadRegister(OHCI1394_NodeID);

            if (!(node_id & 0x80000000))
            {
                ddprintf("WARNING: SelfID received, but NodeID invalid (probably new bus reset occurred): %lx\n",
                      node_id);
                goto selfid_not_valid;
            }

            phyid=(int)node_id&0x0000003f;
            isroot = (node_id & 0x40000000) != 0;

            ddprintf("DEBUG: SelfID interrupt received (phyid %d, is root? %d)\n", phyid,isroot );
            HandleSelfId(phyid);

            /* Clear the bus reset event and re-enable the
             * busReset interrupt.  */
            pushf();
            cli();
            writeRegister(OHCI1394_IntEventClear, OHCI1394_busReset);
            writeRegister(OHCI1394_IntMaskSet, OHCI1394_busReset);
            popf();

            /* Accept Physical requests from all nodes. */
            writeRegister(OHCI1394_AsReqFilterHiSet, 0xffffffff);
            writeRegister(OHCI1394_AsReqFilterLoSet, 0xffffffff);

            /* Turn on phys dma reception.
             *
             * TODO: Enable some sort of filtering management.
             */
            if (bPhysDma)
            {
                writeRegister(OHCI1394_PhyReqFilterHiSet, 0xffffffff);
                writeRegister(OHCI1394_PhyReqFilterLoSet, 0xffffffff);
                writeRegister(OHCI1394_PhyUpperBound, 0xffff0000);
            }
            else
            {
                writeRegister(OHCI1394_PhyReqFilterHiSet, 0x00000000);
                writeRegister(OHCI1394_PhyReqFilterLoSet, 0x00000000);
            }

            ddprintf("DEBUG: PhyReqFilter=%lx%lx\n",
                   ulReadRegister(OHCI1394_PhyReqFilterHiSet),
                   ulReadRegister(OHCI1394_PhyReqFilterLoSet));

            pHostOps->fpfnSelfIdComplete(pHostOps,phyid, isroot);
        } else
            ddprintf("WARNING: SelfID received outside of bus reset sequence\n");

selfid_not_valid:
        event&=~OHCI1394_selfIDComplete;
    }

    /* Make sure we handle everything, just in case we accidentally
     * enabled an interrupt that we didn't write a handler for.  */
    if (event)
        ddprintf("ERROR: Unhandled interrupt(s) 0x%lx\n",event);

    return TRUE;
}
void OHCIDRIVER::HandleSelfId(UINT phyid)
{
    QUADLET far * q = (QUADLET far *)pSelfIdBuf->fpVirtAddr;
    QUADLET self_id_count=ulReadRegister(OHCI1394_SelfIDCount);
    UINT size;
    QUADLET q0, q1;

    /* Check status of self-id reception */
    q0 = q[0];

    if ((self_id_count & 0x80000000) ||
        ((self_id_count & 0x00FF0000) != (q0 & 0x00FF0000)))
    {
        ddprintf("WARNING: Error in reception of SelfID packets [0x%08x/0x%08x] (count: %d)\n",self_id_count, q0, pSelfIdBuf->errors);

        /* Tip by James Goodwin <jamesg@Filanet.com>:
         * We had an error, generate another bus reset in response.  */
        if (pSelfIdBuf->errors<OHCI1394_MAX_SELF_ID_ERRORS)
        {
            MaskPhysReg(1, 0x40);
            pSelfIdBuf->errors++;
        }
        else
        {
            ddprintf("ERROR: Too many errors on SelfID error reception, giving up!\n");
        }
        return;
    }

    /* SelfID Ok, reset error counter. */
    pSelfIdBuf->errors = 0;

    size = (UINT)((self_id_count & 0x00001FFC) >> 2) - 1;
    q++;

    while (size > 0)
    {
        q0 = q[0];
        q1 = q[1];

        if (q0 == ~q1) {
            ddprintf("DEBUG: SelfID packet 0x%lx received\n", q0);
            pHostOps->fpfnSelfIdReceived(pHostOps,cpu_to_be32(q0));
            if (((q0 & 0x3f000000) >> 24) == phyid)
                ddprintf("DEBUG: SelfID for this node is 0x%lx\n", q0);
        }
        else
        {
            ddprintf("DEBUG: SelfID is inconsistent [0x%lx/0x%lx]\n", q0, q1);
        }
        q += 2;
        size -= 2;
    }
    ddprintf("DEBUG: SelfID complete\n");

    return;
}
ULONG OHCIDRIVER::DevCtl(DEVCTL_CMD cmd, ULONG arg)
{
   ULONG retval = 0;
   UCHAR phy_reg;

   switch (cmd)
   {
      case RESET_BUS:
         switch (arg)
         {
            case SHORT_RESET:
               phy_reg=ReadPhysReg(5);
               phy_reg|=0x40;
               WritePhysReg(5, phy_reg); /* set ISBR */
               break;
            case LONG_RESET:
               phy_reg=ReadPhysReg( 1);
               phy_reg|=0x40;
               WritePhysReg(1, phy_reg); /* set IBR */
               break;
            case SHORT_RESET_NO_FORCE_ROOT:
               phy_reg=ReadPhysReg(1);
               if (phy_reg&0x80)
               {
                  phy_reg&=~0x80;
                  WritePhysReg(1, phy_reg); /* clear RHB */
               }

               phy_reg = ReadPhysReg(5);
               phy_reg |= 0x40;
               WritePhysReg(5, phy_reg); /* set ISBR */
               break;
            case LONG_RESET_NO_FORCE_ROOT:
               phy_reg=ReadPhysReg(1);
               phy_reg&=~0x80;
               phy_reg|=0x40;
               WritePhysReg(1, phy_reg); /* clear RHB, set IBR */
               break;
            case SHORT_RESET_FORCE_ROOT:
               phy_reg=ReadPhysReg(1);
               if (!(phy_reg&0x80))
               {
                  phy_reg|=0x80;
                  WritePhysReg(1,phy_reg); /* set RHB */
               }

               phy_reg=ReadPhysReg(5);
               phy_reg|=0x40;
               WritePhysReg(5, phy_reg); /* set ISBR */
               break;
            case LONG_RESET_FORCE_ROOT:
               phy_reg=ReadPhysReg(1);
               phy_reg|=0xc0;
               WritePhysReg(1, phy_reg); /* set RHB and IBR */
               break;
            default:
               retval=-1;
         }
         break;

      case GET_CYCLE_COUNTER:
         retval=ulReadRegister(OHCI1394_IsochronousCycleTimer);
         break;

      case SET_CYCLE_COUNTER:
         writeRegister(OHCI1394_IsochronousCycleTimer, arg);
         break;

      case SET_BUS_ID:
         ddprintf("ERROR: devctl() command SET_BUS_ID err\n");
         break;

      case ACT_CYCLE_MASTER:
         if (arg)
         {
            /* check if we are root and other nodes are present */
            ULONG nodeId;
            nodeId=ulReadRegister(OHCI1394_NodeID);
            if ((nodeId&(1UL<<30))&&(nodeId&0x3f))
            {
            /*
             * enable cycleTimer, cycleMaster
             */
               ddprintf("DEBUG: Cycle master enabled\n");
               writeRegister(OHCI1394_LinkControlSet,
                 OHCI1394_LinkControl_CycleTimerEnable |
                 OHCI1394_LinkControl_CycleMaster);
            }
         }
         else
         {
         /* disable cycleTimer, cycleMaster, cycleSource */
            writeRegister(OHCI1394_LinkControlClear,
              OHCI1394_LinkControl_CycleTimerEnable |
              OHCI1394_LinkControl_CycleMaster |
              OHCI1394_LinkControl_CycleSource);
         }
         break;

      case CANCEL_REQUESTS:
         ddprintf("DEBUG: Cancel request received\n");
         pATReqContext->Reset();
         pATRespContext->Reset();
         break;
/*
      case ISO_LISTEN_CHANNEL:
         {
         OCTLET mask;

         if (arg<0 || arg>63)
         {
            ddprintf("ERROR: devctl(): IS0 listen channel %d is out of range",arg);
            return -1;
         }
*/
      /* activate the legacy IR context */
/*    if (ohci->ir_legacy_context.ohci == NULL) {
         if (alloc_dma_rcv_ctx(ohci, &ohci->ir_legacy_context,
                     DMA_CTX_ISO, 0, IR_NUM_DESC,
                     IR_BUF_SIZE, IR_SPLIT_BUF_SIZE,
                     OHCI1394_IsoRcvContextBase) < 0) {
            PRINT(KERN_ERR, "%s: failed to allocate an IR context",
                  __FUNCTION__);
            return -ENOMEM;
         }
         ohci->ir_legacy_channels = 0;
         initialize_dma_rcv_ctx(&ohci->ir_legacy_context, 1);

         DBGMSG("ISO receive legacy context activated");
      }

      mask = (u64)0x1<<arg;

                spin_lock_irqsave(&ohci->IR_channel_lock, flags);

      if (ohci->ISO_channel_usage & mask) {
         PRINT(KERN_ERR,
               "%s: IS0 listen channel %d is already used",
               __FUNCTION__, arg);
         spin_unlock_irqrestore(&ohci->IR_channel_lock, flags);
         return -EFAULT;
      }

      ohci->ISO_channel_usage |= mask;
      ohci->ir_legacy_channels |= mask;

      if (arg>31)
         reg_write(ohci, OHCI1394_IRMultiChanMaskHiSet,
              1<<(arg-32));
      else
         reg_write(ohci, OHCI1394_IRMultiChanMaskLoSet,
              1<<arg);

                spin_unlock_irqrestore(&ohci->IR_channel_lock, flags);
                DBGMSG("Listening enabled on channel %d", arg);
                break;
        }*/
/* case ISO_UNLISTEN_CHANNEL:
        {
      u64 mask;

      if (arg<0 || arg>63) {
         PRINT(KERN_ERR,
               "%s: IS0 unlisten channel %d is out of range",
               __FUNCTION__, arg);
         return -EFAULT;
      }

      mask = (u64)0x1<<arg;

                spin_lock_irqsave(&ohci->IR_channel_lock, flags);

      if (!(ohci->ISO_channel_usage & mask)) {
         PRINT(KERN_ERR,
               "%s: IS0 unlisten channel %d is not used",
               __FUNCTION__, arg);
         spin_unlock_irqrestore(&ohci->IR_channel_lock, flags);
         return -EFAULT;
      }

      ohci->ISO_channel_usage &= ~mask;
      ohci->ir_legacy_channels &= ~mask;

      if (arg>31)
         reg_write(ohci, OHCI1394_IRMultiChanMaskHiClear,
              1<<(arg-32));
      else
         reg_write(ohci, OHCI1394_IRMultiChanMaskLoClear,
              1<<arg);

                spin_unlock_irqrestore(&ohci->IR_channel_lock, flags);
                DBGMSG("Listening disabled on channel %d", arg);

      if (ohci->ir_legacy_channels == 0) {
         stop_dma_rcv_ctx(&ohci->ir_legacy_context);
         free_dma_rcv_ctx(&ohci->ir_legacy_context);
         DBGMSG("ISO receive legacy context deactivated");
      }
                break;
        }
        */
      default:
         ddprintf("WARNING: ohci_devctl cmd %d not implemented yet\n",cmd);
         break;
   }
   return retval;
}
void OHCIDRIVER::SetHwConfigRom(PQUADLET pConfigRom)
{
    writeRegister(OHCI1394_ConfigROMhdr, be32_to_cpu(pConfigRom[0]));
    writeRegister(OHCI1394_BusOptions, be32_to_cpu(pConfigRom[2]));

    _fmemcpy(pCSRConfigROM->fpVirtAddr, pConfigRom, OHCI_CONFIG_ROM_LEN);
}
/* Transmission of an async or iso packet */
int OHCIDRIVER::TransmitPacket(PACKET far * pPacket)
{
    DMA_TRM_CONTEXT * d;

    if (pPacket->usDataSize >usMaxPacketSize)
    {
        ddprintf("ERROR: Transmit packet size %Zd is too big\n",pPacket->usDataSize);
        return -1;
        //EOVERFLOW;
    }

    /* Decide whether we have an iso, a request, or a response packet */
    if (pPacket->type==PACKET::hpsb_raw)
        d = pATReqContext;
    else if ((pPacket->tcode == TCODE_ISO_DATA) && (pPacket->type == PACKET::hpsb_iso))
        {
        /* The legacy IT DMA context is initialized on first
         * use.  However, the alloc cannot be run from
         * interrupt context, so we bail out if that is the
         * case. I don't see anyone sending ISO packets from
         * interrupt context anyway... */

//            if (ohci->it_legacy_context.ohci == NULL)
//            {
//                if (in_interrupt())
//                    {
//                        PRINT(KERN_ERR,
//                      "legacy IT context cannot be initialized during interrupt");
//                return -EINVAL;
//            }

//            if (alloc_dma_trm_ctx(ohci, &ohci->it_legacy_context, DMA_CTX_ISO, 0, IT_NUM_DESC, OHCI1394_IsoXmitContextBase) < 0) {
//                PRINT(KERN_ERR,
//                      "error initializing legacy IT context");
//                return -ENOMEM;
//            }

//            initialize_dma_trm_ctx(&ohci->it_legacy_context);
//        }

//        d = &ohci->it_legacy_context;
        }
        else if ((pPacket->tcode & 0x02) && (pPacket->tcode != TCODE_ISO_DATA))
                d=pATRespContext;
             else
                d=pATReqContext;
        pushf();
        cli();
        d->pending_list.AddTail(pPacket);
        //        list_add_tail(&packet->driver_list, &d->pending_list);
        DMATrmFlush(d);

        popf();

    return 0;

}
QUADLET OHCIDRIVER::HwCsrReg( int reg, QUADLET data, QUADLET compare)
{
    int i;

    writeRegister(OHCI1394_CSRData, data);
    writeRegister(OHCI1394_CSRCompareData, compare);
    writeRegister(OHCI1394_CSRControl, reg & 0x3);

    for (i = 0; i < OHCI_LOOP_COUNT; i++)
    {
        if (ulReadRegister(OHCI1394_CSRControl) & 0x80000000)
            break;

        DevIODelay(1000);
    }

    return ulReadRegister(OHCI1394_CSRData);

}
BOOL OHCIDRIVER::StopContext(int reg, char far *msg)
{
    int i=0;

    /* stop the channel program if it's still running */
    writeRegister(reg, 0x8000);

    /* Wait until it effectively stops */
    while (ulReadRegister(reg) & 0x400)
    {
        i++;
        if (i>5000)
        {
            ddprintf("ERROR: Runaway loop while stopping context: %s...\n",(char far *) (msg ? msg : ""));
            return TRUE;
        }

    //        mb();
    //        udelay(10);
        DevIODelay(10);
    }
    if (msg) ddprintf("%s: dma prg stopped\n", (char far *)msg);
    return FALSE;
}
/*
 * This function fills the FIFO with the (eventual) pending packets
 * and runs or wakes up the DMA prg if necessary.
 *
 * The function MUST be called with the d->lock held.
 */
void OHCIDRIVER::DMATrmFlush(DMA_TRM_CONTEXT *d)
{
    PACKET far * packet;
    int idx = d->prg_ind;
    int z = 0;

    /* insert the packets into the dma fifo */
    //    list_for_each_entry_safe(packet, ptmp, &d->pending_list, driver_list)
    packet=d->pending_list.GetHead();
    while (packet!=NULL)
    {
        if (!d->free_prgs)
            break;

        /* For the first packet only */
        if (!z)
            z = (packet->usDataSize) ? 3 : 2;

        /* Insert the packet */
        //        list_del_init(&packet->driver_list);
        d->pending_list.DeleteHead();
        InsertPacket(d, packet);
        packet=d->pending_list.GetHead();
    }

    /* Nothing must have been done, either no free_prgs or no packets */
    if (z == 0)
        return;

    /* Is the context running ? (should be unless it is
       the first packet to be sent in this context) */
    if (!(ulReadRegister(d->ctrlSet) & 0x8000))
    {
        ULONG nodeId = ulReadRegister(OHCI1394_NodeID);

        ddprintf("DEBUG: Starting transmit DMA ctx=%d\n",d->ctx);
        writeRegister(d->cmdPtr, d->prg_bus[idx] | z);

        /* Check that the node id is valid, and not 63 */
        if (!(nodeId & 0x80000000) || (nodeId & 0x3f) == 63)
            ddprintf("ERROR: Running dma failed because Node ID is not valid\n");
        else
            writeRegister(d->ctrlSet, 0x8000);
    }
    else
    {
        /* Wake up the dma context if necessary */
        if (!(ulReadRegister(d->ctrlSet) & 0x400))
            ddprintf("DEBUG: Waking transmit DMA ctx=%d\n",d->ctx);

        /* do this always, to avoid race condition */
        writeRegister(d->ctrlSet, 0x1000);
    }

    return;
}

/*
 * Insert a packet in the DMA fifo and generate the DMA prg
 * FIXME: rewrite the program in order to accept packets crossing
 *        page boundaries.
 *        check also that a single dma descriptor doesn't cross a
 *        page boundary.
 */
void OHCIDRIVER::InsertPacket(DMA_TRM_CONTEXT *d, PACKET far * packet)
{
    ULONG cycleTimer;
    int idx = d->prg_ind;

    ddprintf("DEBUG: Inserting packet for node " NODE_BUS_FMT ", tlabel=%d, tcode=0x%x, speed=%d\n",
           NODE_BUS_ARGS(pHostOps->pHost, packet->NodeId), packet->tlabel,
           packet->tcode, packet->SpeedCode);

    d->prg_cpu[idx]->dmacmdBegin.ulAddress = 0;
    d->prg_cpu[idx]->dmacmdBegin.ulBranchAddress = 0;

    if (d->type == DMA_CTX_ASYNC_RESP)
    {
        /*
         * For response packets, we need to put a timeout value in
         * the 16 lower bits of the status... let's try 1 sec timeout
         */
        cycleTimer = ulReadRegister(OHCI1394_IsochronousCycleTimer);
        d->prg_cpu[idx]->dmacmdBegin.ulStatus =(((((cycleTimer>>25)&0x7)+1)&0x7)<<13) |
            ((cycleTimer&0x01fff000)>>12);

        ddprintf("DEBUG: cycleTimer: %lx timeStamp: %lx\n",
               cycleTimer, d->prg_cpu[idx]->dmacmdBegin.ulStatus);
    } else
        d->prg_cpu[idx]->dmacmdBegin.ulStatus = 0;

    if ( (packet->type == PACKET::hpsb_async) || (packet->type == PACKET::hpsb_raw) )
    {
        if (packet->type == PACKET::hpsb_raw)
        {
            d->prg_cpu[idx]->data[0] = (OHCI1394_TCODE_PHY<<4);
            d->prg_cpu[idx]->data[1] = (packet->pHeader[0]);
            d->prg_cpu[idx]->data[2] = (packet->pHeader[1]);
        }
        else
        {
            d->prg_cpu[idx]->data[0] = ((ULONG)packet->SpeedCode)<<16 |
                                (packet->pHeader[0] & 0xFFFF);

            if (packet->tcode == TCODE_ISO_DATA)
            {
                /* Sending an async stream packet */
                d->prg_cpu[idx]->data[1] = packet->pHeader[0] & 0xFFFF0000;
            }
            else
            {
                /* Sending a normal async request or response */
                d->prg_cpu[idx]->data[1] =
                    (packet->pHeader[1] & 0xFFFF) |
                    (packet->pHeader[0] & 0xFFFF0000);
                d->prg_cpu[idx]->data[2] = packet->pHeader[2];
                d->prg_cpu[idx]->data[3] = packet->pHeader[3];
            }
        }

        if (packet->usDataSize) /* block transmit */
        {
            if (packet->tcode == TCODE_STREAM_DATA)
            {
                d->prg_cpu[idx]->dmacmdBegin.ulControl =
                    (DMA_CTL_OUTPUT_MORE | DMA_CTL_IMMEDIATE | 0x8);
            }
            else
            {
                d->prg_cpu[idx]->dmacmdBegin.ulControl =(DMA_CTL_OUTPUT_MORE |
                            DMA_CTL_IMMEDIATE | 0x10);
            }
            d->prg_cpu[idx]->dmacmdEnd.ulControl =(DMA_CTL_OUTPUT_LAST |
                        DMA_CTL_IRQ |
                        DMA_CTL_BRANCH |
                        packet->usDataSize);
            /*
             * Check that the packet data buffer
             * does not cross a page boundary.
             *
             * XXX Fix this some day. eth1394 seems to trigger
             * it, but ignoring it doesn't seem to cause a
             * problem.
                         */
#if 0
                        if (cross_bound((unsigned long)packet->data,
                                        packet->data_size)>0) {
                                /* FIXME: do something about it */
                                PRINT(KERN_ERR,
                                      "%s: packet data addr: %p size %Zd bytes "
                                      "cross page boundary", (char far *)__FUNCTION__,
                                      packet->data, packet->data_size);
                        }
#endif
            DevHelp_VirtToPhys(packet->pData,&d->prg_cpu[idx]->dmacmdEnd.ulAddress);
    //            pci_map_single(ohci->dev, packet->data,
    //                                                 packet->data_size,
    //                                               PCI_DMA_TODEVICE));
            ddprintf("DEBUG: single, block transmit packet\n");

            d->prg_cpu[idx]->dmacmdEnd.ulBranchAddress = 0;
            d->prg_cpu[idx]->dmacmdEnd.ulStatus = 0;
            if (d->branchAddrPtr)
                *(d->branchAddrPtr)=d->prg_bus[idx]|0x3;
                d->branchAddrPtr = &(d->prg_cpu[idx]->dmacmdEnd.ulBranchAddress);
        }
        else
        { /* quadlet transmit */
            if (packet->type == PACKET::hpsb_raw)
                d->prg_cpu[idx]->dmacmdBegin.ulControl =(DMA_CTL_OUTPUT_LAST |
                            DMA_CTL_IMMEDIATE |
                            DMA_CTL_IRQ |
                            DMA_CTL_BRANCH |
                            (packet->usHeaderSize + 4));
            else
                d->prg_cpu[idx]->dmacmdBegin.ulControl =
                    (DMA_CTL_OUTPUT_LAST |
                            DMA_CTL_IMMEDIATE |
                            DMA_CTL_IRQ |
                            DMA_CTL_BRANCH |
                            packet->usHeaderSize);

            if (d->branchAddrPtr)
                *(d->branchAddrPtr) =(d->prg_bus[idx] | 0x2);
                d->branchAddrPtr=&(d->prg_cpu[idx]->dmacmdBegin.ulBranchAddress);
         }
    }
    else
    {
    /* iso packet */
        d->prg_cpu[idx]->data[0] = ((ULONG)packet->SpeedCode)<<16 |
                        (packet->pHeader[0] & 0xFFFF);
        d->prg_cpu[idx]->data[1] = packet->pHeader[0] & 0xFFFF0000;

        d->prg_cpu[idx]->dmacmdBegin.ulControl=(DMA_CTL_OUTPUT_MORE |
                    DMA_CTL_IMMEDIATE | 0x8);
        d->prg_cpu[idx]->dmacmdEnd.ulControl=(DMA_CTL_OUTPUT_LAST |
                    DMA_CTL_UPDATE |
                    DMA_CTL_IRQ |
                    DMA_CTL_BRANCH |
                    packet->usDataSize);
        DevHelp_VirtToPhys(packet->pData,&d->prg_cpu[idx]->dmacmdEnd.ulAddress);
    //        d->prg_cpu[idx]->dmacmdEnd.ulAddress = cpu_to_le32(
    //                pci_map_single(ohci->dev, packet->data,
    //                packet->data_size, PCI_DMA_TODEVICE));
        ddprintf("DEBUG: single, iso transmit packet\n");

        d->prg_cpu[idx]->dmacmdEnd.ulBranchAddress = 0;
        d->prg_cpu[idx]->dmacmdEnd.ulStatus = 0;
        ddprintf("DEBUG: Iso xmit context info: header[%0lx %0lx]\n"
                       "                       begin=%0lx %0lx %0lx %0lx\n"
                       "                             %0lx %0lx %0lx %0lx\n"
                       "                       end  =%0lx %0lx %0lx %0lx\n",
                       d->prg_cpu[idx]->data[0], d->prg_cpu[idx]->data[1],
                       d->prg_cpu[idx]->dmacmdBegin.ulControl,
                       d->prg_cpu[idx]->dmacmdBegin.ulAddress,
                       d->prg_cpu[idx]->dmacmdBegin.ulBranchAddress,
                       d->prg_cpu[idx]->dmacmdBegin.ulStatus,
                       d->prg_cpu[idx]->data[0],
                       d->prg_cpu[idx]->data[1],
                       d->prg_cpu[idx]->data[2],
                       d->prg_cpu[idx]->data[3],
                       d->prg_cpu[idx]->dmacmdEnd.ulControl,
                       d->prg_cpu[idx]->dmacmdEnd.ulAddress,
                       d->prg_cpu[idx]->dmacmdEnd.ulBranchAddress,
                       d->prg_cpu[idx]->dmacmdEnd.ulStatus);
        if (d->branchAddrPtr)
            *(d->branchAddrPtr)=(d->prg_bus[idx] | 0x3);
        d->branchAddrPtr = &(d->prg_cpu[idx]->dmacmdEnd.ulBranchAddress);
    }
    d->free_prgs--;

    /* queue the packet in the appropriate context queue */
    //    list_add_tail(&packet->driver_list, &d->fifo_list);
    d->fifo_list.AddTail(packet);
    d->prg_ind = (d->prg_ind + 1) % d->num_desc;
}
/* Count the number of available iso contexts */
int OHCIDRIVER::GetNbIsoCtx(USHORT reg)
{
    int i,ctx=0;
    ULONG tmp;

    writeRegister(reg, 0xffffffff);
    tmp=ulReadRegister(reg);

    ddprintf("DEBUG: Iso contexts reg: %lx implemented: %lx\n", reg, tmp);

    /* Count the number of contexts */
    for (i=0; i<32; i++)
    {
            if (tmp & 1) ctx++;
        tmp >>= 1;
    }
    return ctx;
}

void far DMARTaskletRoutine(void near * data)
#pragma aux DMARTaskletRoutine parm [ax] modify [];
{
	saveall();
    DMA_RCV_CONTEXT * context=(DMA_RCV_CONTEXT *)data;
    context->DMATasklet();
    ddprintf("DEBUG: DMA R hook finished\n");
    loadall();
}
void far DMATTaskletRoutine(void * data)
#pragma aux DMATTaskletRoutine parm [ax];
{
	saveall();
    DMA_TRM_CONTEXT * context=(DMA_TRM_CONTEXT *)data;
    context->DMATasklet();
    ddprintf("DEBUG: DMA T hook finished\n");
    loadall();
}
