/*
** Module   :STACK.HPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Thu  20/05/2004 Created
**
*/

#ifndef __STACK_HPP
#define __STACK_HPP
#include "list.hpp"
#include "host.hpp"
#include "firetype.h"
//#include "..\ohci\ohci.hpp"
#include "idc.h"
#include "stacklist.hpp"
//#include "..\nodemgr\nodemgr.hpp"
//class HOST;
//typedef HOST* PHOST;
//class OHCIDRIVER;
//typedef OHCIDRIVER* POHCIDRIVER;


class FIRESTACK //main firewire stack
{
public:
    FIRESTACK(); //init stack, ieee1394_init(), init protocol_list and addrspace_list
    void RegisterHighlevel(PHIGHLEVELOPS pHighlevel); //hpsb_register_highlevel(struct hpsb_highlevel *hl);
//    BOOL RegisterAddrSpace(PADDRSPACE pAddrSpace);
    U64 AllocateAndRegisterAddrSpace(HIGHLEVELOPS far *hl, HOSTOPS far *host,
                     ADDROPS far *ops, U64 size, U64 alignment, U64 start, U64 end);

    BOOL RegisterAddrSpace(HIGHLEVELOPS far *hl, HOSTOPS far *host,
                            ADDROPS far *ops, U64 start, U64 end);

    void AddHost(PHOSTOPS pHost);
    void UnRegisterHighlevel(PHIGHLEVELOPS pHighlevel);
    BOOL UnRegisterAddrSpace(PADDRSPACE pAddrSpace);
    void RemoveHost(PHOSTOPS pHost);
    void HostReset(PHOSTOPS pHost);
/* these functions are called to handle transactions. They are called, when
   a packet arrives. The flags argument contains the second word of the first header
   quadlet of the incoming packet (containing transaction label, retry code,
   transaction code and priority). These functions either return a response code
   or a negative number. In the first case a response will be generated; in the
   later case, no response will be sent and the driver, that handled the request
   will send the response itself.
*/
    // called by lower level when event occur
    int Read(PHOSTOPS pHost, int nodeid, void far *data, U64 addr, USHORT length, USHORT flags);
    int Write(PHOSTOPS pHost, int nodeid, int destid, void far *data, U64 addr, USHORT length, USHORT flags);
    int Lock(PHOSTOPS pHost, int nodeid, PQUADLET store, U64 addr, QUADLET data, QUADLET arg, int ext_tcode, USHORT flags);
    int Lock64(PHOSTOPS pHost, int nodeid, POCTLET store, U64 addr, OCTLET data, OCTLET arg, int ext_tcode, USHORT flags);
    void IsoReceive(PHOSTOPS pHost, void far * data, int length);
    void FcpRequest(PHOSTOPS pHost, int nodeid, int direction, void far * data, int length);

    int ListenChannel(PHOSTOPS host, unsigned int channel);
    void UnlistenChannel(PHOSTOPS host, unsigned int channel);

private:
    // list of high-level drivers
    HIGHLEVELLIST HighlevelList;
    // list of registered address space handlers
    ADDRSPACELIST AddrSpaceList;
    //HIGHLEVELLIST IRQList;
    // list of registered hosts
    HOSTLIST HostList;
//    HOSTINFOLIST HostInfoList;
/*    PHOSTINFO FIRESTACK::GetHostInfo(PHIGHLEVELOPS hl,PHOSTOPS host);
    void DestroyHostInfo(PHIGHLEVELOPS hl,PHOSTOPS pHost);*/
};
typedef FIRESTACK * PFIRESTACK;


#endif  /*__STACK_HPP*/

