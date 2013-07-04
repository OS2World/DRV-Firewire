/*
** Module   :ENTRYNODE.HPP
** Abstract :Nodemanager entry points routines
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  14/07/2004 Created
**
*/
#ifndef __ENTRYNODE_HPP
#define __ENTRYNODE_HPP


void far sNodeMgrAddHost(PHOSTOPS pHostOps);
void far sNodeMgrRemoveHost(PHOSTOPS pHostOps);
void far sNodeMgrHostReset(PHOSTOPS pHostOps);
HOSTINFO far * far sNodeMgrGetHostInfo(PHOSTOPS pHostOps);
void far sNodeMgrDestroyHostInfo(HOSTOPS far * pHostOps);

#endif  /*__ENTRYNODE_HPP*/

