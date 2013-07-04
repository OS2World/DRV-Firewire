/*
** Module   :STRATEGY.CPP
** Abstract :
**
** Copyright (C) Alexandr Cherkaev
** original code by Timur Tabi
**
** Log: Fri  30/04/2004	Created
**
*/


#define INCL_NOPMAPI
#define INCL_DOSINFOSEG
#include <os2.h>
#include <audio.h>

#include <include.h>
#include <devhelp.h>
#include "ddprintf.h"
#include "strategy.h"
#include "header.h"
#include "end.h"

void OHCI_StrategyInitComplete(PREQPACKET prp)
{
}

#define MAX_OPENS   12

typedef struct _OPEN {
   USHORT usSysFileNum;
   PDEV_HEADER pdh;
} OPEN, *POPEN;

OPEN opens[MAX_OPENS];

OPEN *allocate_open(USHORT usSysFileNum)
{
   unsigned i;

   for (i=0; i<MAX_OPENS; i++)
      if (opens[i].usSysFileNum == 0) {
         opens[i].usSysFileNum = usSysFileNum;
         return &opens[i];
      }

   return NULL;
}

void deallocate_open(USHORT usSysFileNum)
{
   unsigned i;

   for (i=0; i<MAX_OPENS; i++)
      if (opens[i].usSysFileNum == usSysFileNum) {
         opens[i].usSysFileNum = 0;
         break;
      }
}

void OHCI_StrategyOpen(PREQPACKET prp)
{
   OPEN *popen = allocate_open(prp->s.open_close.usSysFileNum);
   if (!popen)
      prp->usStatus |= RPERR | RPGENFAIL;
}

void OHCI_StrategyClose(PREQPACKET prp)
{
   deallocate_open(prp->s.open_close.usSysFileNum);
}

void far OHCI_StrategyInit(PREQPACKET prp);
void OHCI_StrategyIoctl(PREQPACKET prp);

extern "C" void __far OHCI_StrategyHandler(PREQPACKET prp);
#pragma aux OHCI_StrategyHandler parm [es bx];

extern "C" void __far OHCI_StrategyHandler(PREQPACKET prp)
{
    prp->usStatus = RPDONE;
    ddprintf("OHCI: Request code %x\n",prp->bCommand);
#ifdef DEBUG
    int3();
#endif
    switch (prp->bCommand) {
      case 0x1b: 	//basedev init
         OHCI_StrategyInit(prp);
         break;
      case 0xD:
         OHCI_StrategyOpen(prp);
         break;
      case 0xE:
         OHCI_StrategyClose(prp);
         break;
      case 0x10:
         OHCI_StrategyIoctl(prp);
         break;
      case 0x1F:
         OHCI_StrategyInitComplete(prp);
         break;
      default:
         prp->usStatus = RPDONE | RPERR | RPGENFAIL;
   }
}

