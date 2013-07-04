/*
** Module   :JIFFIES.C
** Abstract : linux compat jiffies
**
** Copyright (C) Alex Cherkaev
**
** Log: Thu  03/06/2004 Created
**
*/

#define INCL_NOPMAPI
#define INCL_DOSINFOSEG
#include <os2.h>
#include "jiffies.h"
//#include "global.hpp"

unsigned long getJiffies(void)
{
 return (unsigned long)(fpGINFOSEG->SIS_MsCount/10);
}

