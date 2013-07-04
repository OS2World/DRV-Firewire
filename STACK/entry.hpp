/*
** Module   :ENTRY.H
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  23/06/2004 Created
**
*/
#ifndef __ENTRY_H
#define __ENTRY_H
#include "firetype.h"
//stack entry points
void far sRegisterHighlevel(HIGHLEVELOPS far * pHighlevel);
BOOL far sRegisterAddrSpace(HIGHLEVELOPS far *pHighlevel, HOSTOPS far *pHostOps,
                            ADDROPS far *ops, U64 start, U64 end);
U64 far sAllocateAndRegisterAddrSpace(HIGHLEVELOPS far *hl,
                     HOSTOPS far *host,
                     ADDROPS far *ops,
                     U64 size, U64 alignment,
                     U64 start, U64 end);

void far sAddHost(PHOSTOPS pHost);
void far sUnRegisterHighlevel(HIGHLEVELOPS far *pHighlevel);
int far sUnRegisterAddrSpace(HIGHLEVELOPS far *hl, HOSTOPS far *host, U64 start);
void far sRemoveHost(PHOSTOPS pHost);
void far sHostReset(PHOSTOPS pHost);
/* these functions are called to handle transactions. They are called, when
   a packet arrives. The flags argument contains the second word of the first header
   quadlet of the incoming packet (containing transaction label, retry code,
   transaction code and priority). These functions either return a response code
   or a negative number. In the first case a response will be generated; in the
   later case, no response will be sent and the driver, that handled the request
   will send the response itself.
*/
    // called by lower level when event occur
int far sRead(PHOSTOPS pHost, int nodeid, void far *data, U64 addr, USHORT length, USHORT flags);
int far sWrite(PHOSTOPS pHost, int nodeid, int destid, void far *data, U64 addr, USHORT length, USHORT flags);
int far sLock(PHOSTOPS pHost, int nodeid, QUADLET far *store, U64 addr, QUADLET data, QUADLET arg, int ext_tcode, USHORT flags);
int far sLock64(PHOSTOPS pHost, int nodeid, OCTLET far *store, U64 addr, OCTLET data, OCTLET arg, int ext_tcode, USHORT flags);
void far sIsoReceive(PHOSTOPS pHost, void far * data, int length);
void far sFcpRequest(PHOSTOPS pHost, int nodeid, int direction, void far * data, int length);

//host entry points
BOOL far sHostUpdateConfigRomImage(PHOSTOPS pHostOps);
void far sHostAddHost(PHOSTOPS pHostOps, DRIVEROPS far * pHwDriver);
void far sHostBusReset(PHOSTOPS pHostOps);
void far sHostSelfIdComplete(PHOSTOPS pHostOps,USHORT usPhyId, BOOL bIsRoot);
void far sHostSelfIdReceived(PHOSTOPS pHostOps,QUADLET sid);
void far sHostPacketSent(PHOSTOPS pHostOps,void far * packet, char ackcode);
void far sHostPacketReceived(PHOSTOPS pHostOps,QUADLET far * data, int size, BOOL bwrite_acked);
int  far sHostRead(PHOSTOPS pHostOps,NODEID node, unsigned int generation,U64 addr, PQUADLET buffer, int length);
int  far sHostWrite(PHOSTOPS pHostOps,NODEID node, unsigned int generation, U64 addr, PQUADLET buffer, int length);
int  far sHostLock(PHOSTOPS pHostOps,NODEID node, unsigned int generation, U64 addr, int extcode, PQUADLET data, QUADLET arg);
int  far sHostGetGeneration(PHOSTOPS pHostOps);
BOOL far sHostResetBus(PHOSTOPS pHostOps,RESET_TYPES type);
APIRET far sHostSendPhyConfig(PHOSTOPS pHostOps,LONG lrootid, LONG lgapcnt);

#endif  /*__ENTRY_H*/

