/*
** Module   :HEADER.C
** Abstract : device driver header
**
** Copyright (C) Alexandr Cherkaev
** original code by Timur Tabi
**
** Log: Fri  30/04/2004	Created
**
*/


#pragma code_seg ("_inittext");
#pragma data_seg ("_header","data");

#define INCL_NOPMAPI
#include <os2.h>

#include <include.h>
#include "header.h"

void __far NodeMgr_StrategyHandler(void);
void __far __loadds __cdecl StackIDCEntry(unsigned char type , void far * pData);

#define ATTRIB          DA_CHAR | DA_USESCAP | DA_NEEDOPEN
//|
#define CAPS            DC_INITCPLT | DC_IOCTL2 | DC_32BIT
//| DC_ADD
// | DC_32BIT

DEV_HEADER header[2] =
{
   {
      (struct _DEV_HEADER __far *)-1,
      ATTRIB,
      (PFNENTRY) NodeMgr_StrategyHandler,
      (PFNENTRY) 0,
      {'F','I','R','E','M','G','R','$'},
      0,0,
      CAPS
   },
};

