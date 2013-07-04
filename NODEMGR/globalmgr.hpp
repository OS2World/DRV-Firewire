/*
** Module   :GLOBALMGR.HPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  14/07/2004 Created
**
*/
#ifndef __GLOBALMGR_HPP
#define __GLOBALMGR_HPP


#include "nodemgr.hpp"

extern "C" struct InfoSegGDT far * fpGINFOSEG;
extern NODEMGR * pGlobalNodeMgr;
extern HIGHLEVELOPS GlobalNodeMgrOps;
//BOOL gInInterrupt;
//extern SEM gNodemgrSerialize;
//extern BOOL bInNodeMgr;
extern BOOL bInNodeMgr;
extern int ResetCount;

extern struct STACKOPS far * fpGlobalStackOps;
#endif  /*__GLOBALMGR_HPP*/

