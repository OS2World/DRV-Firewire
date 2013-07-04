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
#include "ohci.hpp"
#endif

//stack
//extern PFIRESTACK pGlobalStack;
//host
//extern STACKOPS GlobalStack;
//extern PHOST pGlobalHost;
// ohci card
extern POHCIDRIVER pOhciDriver;
extern "C" struct InfoSegGDT far * fpGINFOSEG;
extern BOOL gInInterrupt;
extern struct HOSTOPS far * fpGlobalHostOps;
extern ULONG DMARTasklet, DMATTasklet;
#endif  /*__GLOBAL_HPP*/

