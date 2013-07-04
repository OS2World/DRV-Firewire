/*
** Module   :ISR.CPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Tue  18/05/2004 Created
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

#include "global.hpp"

#include <devhelp.h>
#include "isr.h"
#include <include.h>

int pascal far IRQHandler(void)
{
    gInInterrupt=TRUE;
//    int3();
    if (pOhciDriver->IrqHandler())
    {
        // we process interrupt
        cli();
        DevHelp_EOI(pOhciDriver->pPCICard->ucGetIntrLine());
        clc();
        sti();
//        int3();
        gInInterrupt=FALSE;
        return 0;
    }
    // not our interrupt
    stc();
    sti();
    gInInterrupt=FALSE;
    return 0;
}


