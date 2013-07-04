/*
** Module   :OHCI.HPP
** Abstract :ohci controller class
**
** Copyright (C) Alexandr Cherkaev
**
** Log: Wed  05/05/2004 Created
**
*/
#ifndef __OHCI_HPP
#define __OHCI_HPP

#include "pcidev.hpp"
#include "configrom.hpp"
#include "selfid.hpp"
#include "dma.hpp"
#include "firetype.h"
#include "..\stack\packet.hpp"
#include "..\stack\packetlist.hpp"
#include "idc.h"

//class HOST;
//typedef HOST* PHOST;
class FIRESTACK;
typedef FIRESTACK* PFIRESTACK;



class OHCIDRIVER
//:BASEDRIVER
{
public:
    OHCIDRIVER(void);
    ~OHCIDRIVER(void);
    ULONG ulReadRegister(USHORT usRegister);
    APIRET writeRegister(USHORT usRegister, ULONG ulValue);
    UCHAR ReadPhysReg(UCHAR ucAddr);
    void WritePhysReg(UCHAR ucAddr, UCHAR ucValue);
    void MaskPhysReg(UCHAR ucAddr, UCHAR ucValue);
    void SoftReset(void);
    BOOL Probe(void);
    void Initialize(void);
    virtual ULONG DevCtl(DEVCTL_CMD cmd, ULONG arg);
    void HandleSelfId(UINT phyid);
    BOOL StopContext(int reg, char far *msg);

    virtual void SetHwConfigRom(PQUADLET pConfigRom);
    virtual int TransmitPacket(PACKET far * pPacket);
    virtual QUADLET HwCsrReg( int reg, QUADLET data, QUADLET compare);
    void DMATrmFlush(DMA_TRM_CONTEXT *d);
    void InsertPacket(DMA_TRM_CONTEXT *d, PACKET far * packet);
    int GetNbIsoCtx(USHORT reg);
    // irq handler
    BOOL IrqHandler();
    // host
    //HOST * pHost;

    USHORT usMaxPacketSize;
    //async receive
    // AR DMA request context
    DMA_RCV_CONTEXT * pARReqContext;
    // AR DMA response context
    DMA_RCV_CONTEXT * pARRespContext;

    //async transmit
    // AT DMA request context
    DMA_TRM_CONTEXT * pATReqContext;
    // AT DMA response context
    DMA_TRM_CONTEXT * pATRespContext;

    // iso contexts
    //iso receive
    UCHAR ucNbIsoRcvCtx;
    ULONG ulIRCtxUsage;
    ULONG ulIRMultichannelUsed;
    //iso transmit
    UCHAR ucNbIsoXmitCtx;
    ULONG ulITCtxUsage;

    OCTLET u64ISOChannelUsage;

    /* Swap the selfid buffer? */
    BOOL bSelfIdSwap;
    /* Some Apple chipset seem to swap incoming headers for us */
    BOOL bNoSwapIncoming;
    /* Force extra paranoia checking on bus-reset handling */
    BOOL bCheckBusReset;
    // module config
    //MODULE_PARM_DESC(phys_dma, "Enable physical dma (default = 1).");
    BOOL bPhysDma;

    // pointer to physcard class
    PCIDEV * pPCICard; //dev
    HOSTOPS far * pHostOps;
    DRIVEROPS OhciHwDriver;

//    PACKETLIST pending_list;
//    PACKETLIST fifo_list;

private:
    CSRCONFIGROM * pCSRConfigROM;
    SELFIDBUF * pSelfIdBuf;
    // memory-mapped chip registers
    ULONG ulRegistersPhys;
    SEL selRegisters;
    ULONG far * fpulRegisters;

};
typedef OHCIDRIVER* POHCIDRIVER;

void far DMARTaskletRoutine(void * data);
void far DMATTaskletRoutine(void * data);

#endif  /*__OHCI_HPP*/

