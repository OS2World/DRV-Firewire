/*
** Module   :GLOBALMGR.CPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  14/07/2004 Created
**
*/
#ifndef __GLOBALMGR_CPP
#define __GLOBALMGR_CPP


extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define INCL_TYPES
#define  INCL_DOSDEVICES
#define  INCL_DOSDEVIOCTL
#define  INCL_NOXLATE_DOS16
#include <os2.h>
}

//#include "ohci.hpp"
//#include "host.hpp"
//#include "stack.hpp"
#include <infoseg.h>
#include "globalmgr.hpp"

struct InfoSegGDT far * fpGINFOSEG;
STACKIDC StackIDC;
NODEMGR * pGlobalNodeMgr;
HIGHLEVELOPS GlobalNodeMgrOps;
//BOOL gInInterrupt;
//SEM gNodemgrSerialize;
//BOOL bInNodeMgr;
BOOL bInNodeMgr;
int ResetCount;

struct STACKOPS far * fpGlobalStackOps;

#endif  /*__GLOBALMGR_CPP*/

