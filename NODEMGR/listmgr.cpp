/*
** Module   :LISTMGR.CPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  14/07/2004 Created
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

#include "listmgr.hpp"
#include "nodemgr.hpp"
#include "ddprintf.h"

HOSTINFOLIST::AddHead(PHOSTINFO pHostInfo)
{
    PLISTENTRY pNewEntry;
    pNewEntry=new LISTENTRY;
    if (pNewEntry==NULL) ddprintf("ERROR: pNewEntry is NULL\n");
    pNewEntry->pContent=pHostInfo;
    List.AddHead(pNewEntry);
}
HOSTINFOLIST::AddTail(PHOSTINFO pHostInfo)
{
    PLISTENTRY pNewEntry;
    pNewEntry=new LISTENTRY;
    if (pNewEntry==NULL) ddprintf("ERROR: pNewEntry is NULL\n");
    pNewEntry->pContent=pHostInfo;
    List.AddTail(pNewEntry);
}
HOSTINFOLIST::Remove(PHOSTINFO pHostInfo)
{
    PLISTENTRY pTempEntry;
    HOSTINFO far * pTempAS;
    pTempEntry=List.GetHead();
    while (pTempEntry!=NULL)
    {
        pTempAS=(HOSTINFO far *)pTempEntry->pContent;
        if (pTempAS==NULL) {ddprintf("ERROR: damaged element in nodeentrylist\n"); return FALSE;}
        if (pTempAS==pHostInfo)
        {
            List.Delete(pTempEntry);
            delete pTempEntry;
            return TRUE;
        }
        pTempEntry=List.GetNext();
    }
    return FALSE;
}
PHOSTINFO HOSTINFOLIST::GetHead(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetHead();
    if (pEntry==NULL) return NULL;
    return (PHOSTINFO)pEntry->pContent;
}
PHOSTINFO HOSTINFOLIST::GetNext(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetNext();
    if (pEntry==NULL) return NULL;
    return (PHOSTINFO)pEntry->pContent;
}
PHOSTINFO HOSTINFOLIST::GetTail(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetTail();
    if (pEntry==NULL) return NULL;
    return (PHOSTINFO)pEntry->pContent;
}
PHOSTINFO HOSTINFOLIST::GetHostInfo(PHOSTOPS pHostOps)
{
    PLISTENTRY pTempEntry;
    HOSTINFO far * pTempAS;
    pTempEntry=List.GetHead();
    while (pTempEntry!=NULL)
    {
        pTempAS=(HOSTINFO far *)pTempEntry->pContent;
        if (pTempAS==NULL) {ddprintf("ERROR: damaged element in hostinfolist\n"); return NULL;}
        if (pTempAS->pHostOps==pHostOps)
        {
            return pTempAS;
        }
        pTempEntry=List.GetNext();
    }
    return NULL;
}
void NODE_ENTRYLIST::AddHead(PNODE_ENTRY pNodeEntry)
{
    PLISTENTRY pNewEntry;
    pNewEntry=new LISTENTRY;
    if (pNewEntry==NULL) ddprintf("ERROR: pNewEntry is NULL\n");
    pNewEntry->pContent=pNodeEntry;
    List.AddHead(pNewEntry);
}
void NODE_ENTRYLIST::AddTail(PNODE_ENTRY pNodeEntry)
{
    PLISTENTRY pNewEntry;
    pNewEntry=new LISTENTRY;
    if (pNewEntry==NULL) ddprintf("ERROR: pNewEntry is NULL\n");
    pNewEntry->pContent=pNodeEntry;
    List.AddTail(pNewEntry);
}
BOOL NODE_ENTRYLIST::Remove(PNODE_ENTRY pNodeEntry)
{
    PLISTENTRY pTempEntry;
    NODE_ENTRY far * pTempAS;
    pTempEntry=List.GetHead();
    while (pTempEntry!=NULL)
    {
        pTempAS=(NODE_ENTRY far *)pTempEntry->pContent;
        if (pTempAS==NULL) {ddprintf("ERROR: damaged element in nodeentrylist\n"); return FALSE;}
        if (pTempAS==pNodeEntry)
        {
            List.Delete(pTempEntry);
            delete pTempEntry;
            return TRUE;
        }
        pTempEntry=List.GetNext();
    }
    return FALSE;
}
PNODE_ENTRY NODE_ENTRYLIST::GetHead(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetHead();
    if (pEntry==NULL) return NULL;
    return (PNODE_ENTRY)pEntry->pContent;
}
PNODE_ENTRY NODE_ENTRYLIST::GetNext(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetNext();
    if (pEntry==NULL) return NULL;
    return (PNODE_ENTRY)pEntry->pContent;
}

PNODE_ENTRY NODE_ENTRYLIST::GetTail(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetTail();
    if (pEntry==NULL) return NULL;
    return (PNODE_ENTRY)pEntry->pContent;
}
PNODE_ENTRY NODE_ENTRYLIST::FindByNodeid(NODEID nodeid)
{
    NODE_ENTRY far * pTempAS;
    pTempAS=GetHead();
    while (pTempAS!=NULL)
    {
        if (pTempAS->NodeId==nodeid)
        {
            return pTempAS;
        }
        pTempAS=GetNext();
    }
    return NULL;
}

PNODE_ENTRY NODE_ENTRYLIST::FindByGuid(U64 guid)
{
    NODE_ENTRY far * pTempAS;
    pTempAS=GetHead();
    while (pTempAS!=NULL)
    {
        if (pTempAS->guid==guid)
        {
            return pTempAS;
        }
        pTempAS=GetNext();
    }
    return NULL;
}
void UNIT_DIRLIST::AddHead(PUNIT_DIRECTORY pUnitDirectory)
{
    PLISTENTRY pNewEntry;
    pNewEntry=new LISTENTRY;
    if (pNewEntry==NULL) ddprintf("ERROR: pNewEntry is NULL\n");
    pNewEntry->pContent=pUnitDirectory;
    List.AddHead(pNewEntry);
}
void UNIT_DIRLIST::AddTail(PUNIT_DIRECTORY pUnitDirectory)
{
    PLISTENTRY pNewEntry;
    pNewEntry=new LISTENTRY;
    if (pNewEntry==NULL) ddprintf("ERROR: pNewEntry is NULL\n");
    pNewEntry->pContent=pUnitDirectory;
    List.AddTail(pNewEntry);
}
BOOL UNIT_DIRLIST::Remove(PUNIT_DIRECTORY pUnitDirectory)
{
    PLISTENTRY pTempEntry;
    UNIT_DIRECTORY far * pTempAS;
    pTempEntry=List.GetHead();
    while (pTempEntry!=NULL)
    {
        pTempAS=(UNIT_DIRECTORY far *)pTempEntry->pContent;
        if (pTempAS==NULL) {ddprintf("ERROR: damaged element in nodeentrylist\n"); return FALSE;}
        if (pTempAS==pUnitDirectory)
        {
            List.Delete(pTempEntry);
            delete pTempEntry;
            return TRUE;
        }
        pTempEntry=List.GetNext();
    }
    return FALSE;
}
PUNIT_DIRECTORY UNIT_DIRLIST::GetHead(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetHead();
    if (pEntry==NULL) return NULL;
    return (PUNIT_DIRECTORY)pEntry->pContent;
}
PUNIT_DIRECTORY UNIT_DIRLIST::GetNext(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetNext();
    if (pEntry==NULL) return NULL;
    return (PUNIT_DIRECTORY)pEntry->pContent;
}

PUNIT_DIRECTORY UNIT_DIRLIST::GetTail(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetTail();
    if (pEntry==NULL) return NULL;
    return (PUNIT_DIRECTORY)pEntry->pContent;
}
void UNIT_DIRLIST::DeleteHead(void)
{
	List.DeleteHead();
}

