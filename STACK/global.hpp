/*
** Module   :GLOBAL.HPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  19/05/2004 Created
**
*/

#ifndef __GLOBAL_HPP
#define __GLOBAL_HPP

#ifdef __cplusplus
#include "stack.hpp"
#include "sems.hpp"
#endif

//stack
extern PFIRESTACK pGlobalStack;
extern STACKOPS GlobalStackOps;
extern ULONG PacketHookHandle;
//host
//extern STACKOPS GlobalStack;
//extern PHOST pGlobalHost;
// ohci card
//extern POHCIDRIVER pOhciDriver;
extern "C" struct InfoSegGDT far * fpGINFOSEG;
extern PACKETLIST GlobalPacketQueue;
//extern SEM PacketThreadSig;
//extern BOOL gInInterrupt;
extern NODEMGR * fpGlobalNodeMgr;
extern HIGHLEVELOPS GlobalNodeMgrOps;

extern HOST far * fpGHost;
extern HOSTOPS far * fpGHostOps;
extern csr1212_bus_ops far * fpGCsrBusOps;

#endif  /*__GLOBAL_HPP*/

