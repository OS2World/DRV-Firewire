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

void NodeMgr_StrategyInitComplete(PREQPACKET prp)
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

void NodeMgr_StrategyOpen(PREQPACKET prp)
{
   OPEN *popen = allocate_open(prp->s.open_close.usSysFileNum);
   if (!popen)
      prp->usStatus |= RPERR | RPGENFAIL;
}

void NodeMgr_StrategyClose(PREQPACKET prp)
{
   deallocate_open(prp->s.open_close.usSysFileNum);
}

void far NodeMgr_StrategyInit(PREQPACKET prp);
void NodeMgr_StrategyIoctl(PREQPACKET prp);

extern "C" void __far NodeMgr_StrategyHandler(PREQPACKET prp);
#pragma aux NodeMgr_StrategyHandler parm [es bx];

extern "C" void __far NodeMgr_StrategyHandler(PREQPACKET prp)
{
    prp->usStatus = RPDONE;
    ddprintf("NodeMgr: Request code %x\n",prp->bCommand);
#ifdef DEBUG
    int3();
#endif
    switch (prp->bCommand) {
      case 0x1b: 	//basedev init
         NodeMgr_StrategyInit(prp);
         break;
      case 0xD:
         NodeMgr_StrategyOpen(prp);
         break;
      case 0xE:
         NodeMgr_StrategyClose(prp);
         break;
      case 0x10:
         NodeMgr_StrategyIoctl(prp);
         break;
      case 0x1F:
         NodeMgr_StrategyInitComplete(prp);
         break;
      default:
         prp->usStatus = RPDONE | RPERR | RPGENFAIL;
   }
}

