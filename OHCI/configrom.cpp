/*
** Module   :CONFIGROM.CPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Fri  14/05/2004 Created
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

#include "configrom.hpp"
#include "ddprintf.h"

CSRCONFIGROM::CSRCONFIGROM(USHORT usSize):MEMORY(usSize)
{
	usLength=0;
    ddprintf("VERBOSE: ConfigROM Buf Phys %lx Virt %lx \n",ulPhysAddr, fpVirtAddr);
}
