/*
** Module   :PACKETLIST.CPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Thu  24/06/2004 Created
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


#include "packetlist.hpp"
#include "ddprintf.h"

PACKETLIST::PACKETLIST(void)
{
//    pList=new LIST;
//    if (pList==NULL) ddprintf("ERROR: pList is NULL\n");
}
void PACKETLIST::AddHead(PACKET far *  pPacket)
{
    PLISTENTRY pNewEntry;
    pNewEntry=new LISTENTRY;
    if (pNewEntry==NULL) ddprintf("ERROR: pNewEntry is NULL\n");
    pNewEntry->pContent=pPacket;
    List.AddHead(pNewEntry);
}
void PACKETLIST::AddTail(PACKET far *  pPacket)
{
    PLISTENTRY pNewEntry;
    pNewEntry=new LISTENTRY;
    if (pNewEntry==NULL) ddprintf("ERROR: pNewEntry is NULL\n");
    pNewEntry->pContent=pPacket;
    List.AddTail(pNewEntry);
}
void PACKETLIST::Remove(PACKET far *  pPacket)
{
}
void PACKETLIST::DeleteHead(void)
{
    List.DeleteHead();
}
void PACKETLIST::DeleteAll(void)
{
	List.DeleteAll();
}
BOOL PACKETLIST::isEmpty(void)
{
    return List.isEmpty();
}
PACKET far *  PACKETLIST::GetHead(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetHead();
    if (pEntry==NULL) return NULL;
    return (PACKET far * )pEntry->pContent;
}
PACKET far *  PACKETLIST::GetNext(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetNext();
    if (pEntry==NULL) return NULL;
    return (PACKET far * )pEntry->pContent;
}
PACKET far *  PACKETLIST::GetTail(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetTail();
    if (pEntry==NULL) return NULL;
    return (PACKET far * )pEntry->pContent;
}
void PACKETLIST::AddList(PACKETLIST far * AddedList)
{
	List.AddList(&AddedList->List);
}

