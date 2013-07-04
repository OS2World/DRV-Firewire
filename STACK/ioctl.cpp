/* $Id: ioctl.cpp,v 1.2 2004/07/14 13:50:40 doctor64 Exp $ */

/* IOCTL.CPP

   MODIFICATION HISTORY
   DATE       PROGRAMMER   COMMENT
   15-Aug-98  Timur Tabi   Creation
*/


#define INCL_NOPMAPI
#include <os2.h>
#include <string.h>

#include "strategy.h"
#include "header.h"
#include "ddprintf.h"
#include "malloc.h"
#include "ioctl.h"
#include "rmhelp.hpp"
//#include "isr2.hpp"
#include "version.h"

#include <include.h>
#include <devhelp.h>

USHORT usYear;
USHORT usMonth;
USHORT usDay;

ADAPTER *padpt;

int CheckP1(PREQPACKET prp, unsigned uLength)
{
   if (!prp->s.ioctl.pvParm)
      return FALSE;

   return prp->s.ioctl.usPLength == uLength;
}

int CheckP2(PREQPACKET prp, unsigned uLength)
{
   if (!prp->s.ioctl.pvData)
      return FALSE;

   return prp->s.ioctl.usDLength == uLength;
}

void SetError(PREQPACKET prp, BYTE error)
{
   if (error)
      prp->usStatus = RPDONE | RPERR | RPDEV | error;
}

void WriteULONG(PREQPACKET prp, unsigned uOffset, ULONG ul)
{
   char __far *pc=(char __far *) prp->s.ioctl.pvData;

   if (!pc) {
      ddprintf("Stack: WriteULONG, Func=%x, Offset=%ui, pc=NULL\n", (USHORT) prp->s.ioctl.bCode, uOffset);
      return;
   }
   if ( uOffset+sizeof(ULONG) > prp->s.ioctl.usDLength ) {
      ddprintf("Stack: WriteULONG, Func=%x, Offset=%ui, DLength=%ui, pc=%p\n", (USHORT) prp->s.ioctl.bCode, uOffset, prp->s.ioctl.usDLength, pc);
      return;
   }

   *((ULONG __far *) (pc+uOffset))=ul;
}

int WriteString(PREQPACKET prp, unsigned uOffset, char *psz, unsigned uLength)
{
   char __far *pc=(char __far *) prp->s.ioctl.pvData;

   if (!pc) {
      ddprintf("Stack: WriteString, Func=%ui, Offset=%ui, pc=NULL\n", (USHORT) prp->s.ioctl.bCode, uOffset);
      return FALSE;
   }
   if ( uOffset+uLength > prp->s.ioctl.usDLength ) {
      ddprintf("Stack: WriteString, Func=%ui, Offset=%ui, DLength=%ui, pc=%p\n", (USHORT) prp->s.ioctl.bCode, uOffset, prp->s.ioctl.usDLength, pc);
      return FALSE;
   }

   _fstrncpy(pc+uOffset,psz,uLength);
   return TRUE;
}

unsigned ReadInt(PREQPACKET prp, unsigned uOffset)
{
   char __far *pc = (char __far *) prp->s.ioctl.pvParm + uOffset;
   unsigned __far *pu = (unsigned __far *) pc;

   return *pu;
}

void __far *ReadPtr(PREQPACKET prp, unsigned uOffset)
{
   char __far *pc = (char __far *) prp->s.ioctl.pvParm + uOffset;
   void __far * __far *ppv = (void __far * __far *) pc;

   return *ppv;
}


void IoctlGetVersion(PREQPACKET prp)
{
/*
   GENPDD_GET_VERSION __far *p = (GENPDD_GET_VERSION __far *) prp->s.ioctl.pvParm;

   p->usMajorVersion = VERSION_MAJOR;
   p->usMinorVersion = VERSION_MINOR;
   p->usBuildLevel = VERSION_LEVEL;

   p->usYear = usYear;
   p->bMonth = (UCHAR) usMonth;
   p->bDay = (UCHAR) usDay;
*/
}


void Stack_StrategyIoctl(PREQPACKET prp)
{
   if (prp->s.ioctl.bCategory != I1394_IOCTL_CATEGORY) {
      prp->usStatus |= RPERR | RPBADCMD;
      return;
   }

   switch (prp->s.ioctl.bCode)
   {
    //place ioctl process code here
//      case GENPDD_IOCTL_GET_VERSION:
//         IoctlGetVersion(prp);
//         break;
      default:
         prp->usStatus |= RPERR | RPBADCMD;
         return;
   }
}

