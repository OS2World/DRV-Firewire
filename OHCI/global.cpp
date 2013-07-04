/*
** Module   :GLOBAL.CPP
** Abstract :global data
**
** Copyright (C) Alex Cherkaev
**
** Log: Tue  01/06/2004 Created
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

#include "ohci.hpp"
#include <infoseg.h>
#include "global.hpp"
#include "idc.h"
//stack
//PFIRESTACK pGlobalStack;
//STACKOPS GlobalStack;
//host
//PHOST pGlobalHost;
// ohci card
POHCIDRIVER pOhciDriver;
//infoseg
struct InfoSegGDT far * fpGINFOSEG;
STACKIDC StackIDC;
struct HOSTOPS far * fpGlobalHostOps;
BOOL gInInterrupt;
ULONG DMARTasklet, DMATTasklet;
