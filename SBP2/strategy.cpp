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

void SBP2StrategyInitComplete(PREQPACKET prp)
{
}


void far SBP2StrategyInit(PREQPACKET prp);
void SBP2StrategyIoctl(PREQPACKET prp);

extern "C" void __far SBP2StrategyHandler(PREQPACKET prp);
#pragma aux SBP2StrategyHandler parm [es bx];

extern "C" void __far SBP2StrategyHandler(PREQPACKET prp)
{
    prp->usStatus = RPDONE;
    ddprintf("SBP2: Request code %x\n",prp->bCommand);
#ifdef DEBUG
    int3();
#endif
    switch (prp->bCommand) {
      case 0x1b: 	//basedev init
         SBP2StrategyInit(prp);
         break;
      case 0x1F:
         //NodeMgr_StrategyInitComplete(prp);
         break;
      default:
         prp->usStatus = RPDONE | RPERR | RPGENFAIL;
   }
}

