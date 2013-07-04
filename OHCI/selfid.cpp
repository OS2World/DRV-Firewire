/*
** Module   :SELFID.CPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Fri  28/05/2004 Created
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

#include "selfid.hpp"
#include "ddprintf.h"

SELFIDBUF::SELFIDBUF(USHORT usSize):MEMORY(usSize)
{
    errors=0;
    ddprintf("DEBUG: SelfId Buf Phys %lx Virt %lx \n",ulPhysAddr, fpVirtAddr);
}
