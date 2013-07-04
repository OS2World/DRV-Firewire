/*
** Module   :ENTRY.CPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  23/06/2004 Created
**
*/
extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define INCL_TYPES
#define  INCL_DOSDEVICES
#define  INCL_DOSDEVIOCTL
#define  INCL_NOXLATE_DOS16
#include <os2.h>
}

#include "idc.h"
#include "global.hpp"
void far __loadds sRegisterHighlevel(HIGHLEVELOPS far * pHighlevel)
{
    pGlobalStack->RegisterHighlevel(pHighlevel);
}
//void far __loadds sRegisterAddrSpace(void far * pAddrSpace)
BOOL far __loadds sRegisterAddrSpace(HIGHLEVELOPS far *pHighlevel, HOSTOPS far *pHostOps,
                            ADDROPS far *ops, U64 start, U64 end)
{
    return pGlobalStack->RegisterAddrSpace(pHighlevel, pHostOps, ops, start, end);
}
U64 far __loadds sAllocateAndRegisterAddrSpace(HIGHLEVELOPS far *hl,
                     HOSTOPS far *host,
                     ADDROPS far *ops,
                     U64 size, U64 alignment,
                     U64 start, U64 end)
{
    return pGlobalStack->AllocateAndRegisterAddrSpace(hl, host, ops, size, alignment, start, end);
}


void far __loadds sAddHost(PHOSTOPS pHost)
{
    pGlobalStack->AddHost(pHost);
}
void far __loadds sUnRegisterHighlevel(HIGHLEVELOPS far *pHighlevel)
{
    pGlobalStack->UnRegisterHighlevel(pHighlevel);
}
int far __loadds sUnRegisterAddrSpace(HIGHLEVELOPS far *hl, HOSTOPS far *host, U64 start)
{
//    pGlobalStack->UnRegisterAddrSpace((ADDRSPACE far *)pAddrSpace);
return 0;
}
void far __loadds sRemoveHost(PHOSTOPS pHost)
{
    pGlobalStack->RemoveHost(pHost);
}
void far __loadds sHostReset(PHOSTOPS pHost)
{
    pGlobalStack->HostReset(pHost);
}
/* these functions are called to handle transactions. They are called, when
   a packet arrives. The flags argument contains the second word of the first header
   quadlet of the incoming packet (containing transaction label, retry code,
   transaction code and priority). These functions either return a response code
   or a negative number. In the first case a response will be generated; in the
   later case, no response will be sent and the driver, that handled the request
   will send the response itself.
*/
    // called by lower level when event occur
int far __loadds sRead(PHOSTOPS pHost, int nodeid, void far *data, U64 addr, USHORT length, USHORT flags)
{
    return pGlobalStack->Read(pHost, nodeid, data, addr, length, flags);
}
int far __loadds sWrite(PHOSTOPS pHost, int nodeid, int destid, void far *data, U64 addr, USHORT length, USHORT flags)
{
    return pGlobalStack->Write(pHost, nodeid, destid, data,  addr, length, flags);
}
int far __loadds sLock(PHOSTOPS pHost, int nodeid, QUADLET far *store, U64 addr, QUADLET data, QUADLET arg, int ext_tcode, USHORT flags)
{
    return pGlobalStack->Lock(pHost, nodeid, store, addr, data, arg, ext_tcode, flags);
}
int far __loadds sLock64(PHOSTOPS pHost, int nodeid, OCTLET far *store, U64 addr, OCTLET data, OCTLET arg, int ext_tcode, USHORT flags)
{
    return pGlobalStack->Lock64(pHost, nodeid, store, addr, data, arg, ext_tcode, flags);
}
void far __loadds sIsoReceive(PHOSTOPS pHost, void far * data, int length)
{
    pGlobalStack->IsoReceive(pHost, data, length);
}
void far __loadds sFcpRequest(PHOSTOPS pHost, int nodeid, int direction, void far * data, int length)
{
    pGlobalStack->FcpRequest(pHost, nodeid, direction, data, length);
}
BOOL far __loadds sHostUpdateConfigRomImage(PHOSTOPS pHostOps)
{
    return ((HOST *)pHostOps->pHost)->UpdateConfigRomImage();
}
void far __loadds sHostAddHost(PHOSTOPS pHostOps, DRIVEROPS far * pHwDriver)
{
    ((HOST *)pHostOps->pHost)->AddHost(pHwDriver);
}
void far __loadds sHostBusReset(PHOSTOPS pHostOps)
{
    ((HOST *)pHostOps->pHost)->BusReset();
}
void far __loadds sHostSelfIdComplete(PHOSTOPS pHostOps,USHORT usPhyId, BOOL bIsRoot)
{
    ((HOST *)pHostOps->pHost)->SelfIdComplete(usPhyId, bIsRoot);
}
void far __loadds sHostSelfIdReceived(PHOSTOPS pHostOps,QUADLET sid)
{
    ((HOST *)pHostOps->pHost)->SelfIdReceived(sid);
}
void far __loadds sHostPacketSent(PHOSTOPS pHostOps,void far * packet, char ackcode)
{
    ((HOST *)pHostOps->pHost)->PacketSent((PACKET *)packet, ackcode);
}
void far __loadds sHostPacketReceived(PHOSTOPS pHostOps,QUADLET far * data, int size, BOOL bwrite_acked)
{
    ((HOST *)pHostOps->pHost)->PacketReceived(data, size, bwrite_acked);
}
int  far __loadds sHostRead(PHOSTOPS pHostOps,NODEID node, unsigned int generation,U64 addr, PQUADLET buffer, int length)
{
    return ((HOST *)pHostOps->pHost)->Read(node, generation, addr, buffer, length);
}
int  far __loadds sHostWrite(PHOSTOPS pHostOps,NODEID node, unsigned int generation, U64 addr, PQUADLET buffer, int length)
{
    return ((HOST *)pHostOps->pHost)->Write(node, generation, addr, buffer, length);
}
int  far __loadds sHostLock(PHOSTOPS pHostOps,NODEID node, unsigned int generation, U64 addr, int extcode, PQUADLET data, QUADLET arg)
{
    return ((HOST *)pHostOps->pHost)->Lock(node, generation, addr, extcode, data, arg);
}
int  far __loadds sHostGetGeneration(PHOSTOPS pHostOps)
{
    return ((HOST *)pHostOps->pHost)->GetGeneration();
}

BOOL far __loadds sHostResetBus(PHOSTOPS pHostOps,RESET_TYPES type)
{
    return ((HOST *)pHostOps->pHost)->ResetBus(type);
}

APIRET far __loadds sHostSendPhyConfig(PHOSTOPS pHostOps,LONG lrootid, LONG lgapcnt)
{
    return ((HOST *)pHostOps->pHost)->SendPhyConfig(lrootid, lgapcnt);
}

