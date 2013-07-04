/*
** Module   :ENTRYMGR.CPP
** Abstract :Nodemanager entry points routines
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  14/07/2004 Created
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

#include "idc.h"
#include "globalmgr.hpp"

void far __loadds sNodeMgrAddHost(HOSTOPS far * host)
{
    pGlobalNodeMgr->AddHost(host);
}
void far __loadds sNodeMgrRemoveHost(HOSTOPS far * host)
{
    pGlobalNodeMgr->RemoveHost(host);
}
void far __loadds sNodeMgrHostReset(HOSTOPS far * host)
{
    pGlobalNodeMgr->HostReset(host);
}
HOSTINFO far* far  __loadds sNodeMgrGetHostInfo(HOSTOPS far *pHost)
{
    return pGlobalNodeMgr->HostInfoList.GetHostInfo(pHost);
}
void far __loadds sNodeMgrDestroyHostInfo(HOSTOPS far *pHost)
{
//    pGlobalNodeMgr->HostInfoList.Remove(pHost);
}

