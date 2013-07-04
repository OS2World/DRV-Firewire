/*
** Module   :LIST.CPP
** Abstract : generic list functions
**
** Copyright (C) Alex Cherkaev
**
** Log: Thu  20/05/2004 Created
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
#include "list.hpp"
#include "ddprintf.h"
//#include <basedef.h>

LISTENTRY::LISTENTRY()
{
    pNextEntry=NULL;
    pPrevEntry=NULL;
    pContent=NULL;
}
LISTENTRY::LISTENTRY(PVOID pdata)
{
    pNextEntry=NULL;
    pPrevEntry=NULL;
    pContent=pdata;
}

LIST::LIST()
{
    pListHead=NULL;
    pListTail=NULL;
    pListCurrent=NULL;
}
PLISTENTRY LIST::GetHead()
{
    pListCurrent=pListHead;
    return pListCurrent;
}
PLISTENTRY LIST::GetCurrent()
{
    return pListCurrent;
}
PLISTENTRY LIST::GetNext()
{
    if (pListCurrent==NULL) return NULL;
    if (pListCurrent->pNextEntry==NULL)
        return NULL;
    pListCurrent=pListCurrent->pNextEntry;
    return pListCurrent;
}
PLISTENTRY LIST::GetPrev()
{
    if (pListCurrent==NULL) return NULL;
    if (pListCurrent->pPrevEntry==NULL)
        return NULL;
    pListCurrent=pListCurrent->pPrevEntry;
    return pListCurrent;
}
PLISTENTRY LIST::GetTail()
{
    pListCurrent=pListTail;
    return pListCurrent;
}
void LIST::AddHead(PLISTENTRY pListEntry)
{
    BOOL empty=isEmpty();
    pListEntry->pNextEntry=pListHead;
    if (!empty)
        pListHead->pPrevEntry=pListEntry;
    else pListTail=pListEntry;
    pListHead=pListEntry;
}
void LIST::AddTail(PLISTENTRY pListEntry)
{
    BOOL empty=isEmpty();
    pListEntry->pPrevEntry=pListTail;
    if (!empty)
        pListTail->pNextEntry=pListEntry;
    else pListHead=pListEntry;
    pListTail=pListEntry;
}
void LIST::Add(PLISTENTRY pListEntry)
{
	if (pListCurrent==NULL) return;
	pListEntry->pPrevEntry=pListCurrent;
	pListEntry->pNextEntry=pListCurrent->pNextEntry;
	pListCurrent->pNextEntry=pListEntry;
	if (pListEntry->pNextEntry!=NULL)
		pListEntry->pNextEntry->pPrevEntry=pListEntry;
}
void LIST::DeleteAll(void)
{
	while (!isEmpty())
    {
		DeleteHead();
    }
	return;
}
void LIST::DeleteHead(void)
{
	PLISTENTRY tempElem;
	if (isEmpty()) return;
	tempElem=GetHead();
	pListHead=GetNext();
	if (tempElem==pListTail) //last packet
	{
		pListHead=NULL;
		pListTail=NULL;
	}

//    delete tempElem;
}
void LIST::Delete(PLISTENTRY pDelEntry)
{
    if (isEmpty()) return;
    if (pDelEntry==NULL) return;
    if (pDelEntry->pPrevEntry!=NULL)
    {
        if (pDelEntry->pNextEntry!=NULL)
        {
            pDelEntry->pNextEntry->pPrevEntry=pDelEntry->pPrevEntry;
            pDelEntry->pPrevEntry->pNextEntry=pDelEntry->pNextEntry;
        }
        else //last in list
        {
            pDelEntry->pPrevEntry->pNextEntry=NULL;
        }
    }
    else //first in list
    {
        if (pDelEntry->pNextEntry!=NULL)
        {
            pDelEntry->pNextEntry->pPrevEntry=NULL;
        }
        //else empty list
    }
}
//void DeleteTail(void);
BOOL LIST::isEmpty(void)
{
	if (pListHead==NULL) return TRUE;
		else return FALSE;
}
void LIST::AddList(PLIST addedList)
{
	PLISTENTRY tempelem;
    if (addedList->isEmpty()) return;
	AddTail(addedList->GetHead());
	while (TRUE)
	{
		tempelem=addedList->GetNext();
		if (tempelem==NULL) return;
		AddTail(tempelem);
	}
}
void LIST::Dump(void)
{
	PLISTENTRY temp;
	temp=pListHead;
    ddprintf("first list elem 0x%x ",temp);
	while (temp!=NULL)
	{
		ddprintf( "prev 0x%x, next 0x%x, content 0x%lx\n",temp->pPrevEntry, temp->pNextEntry, temp->pContent);
		temp=temp->pNextEntry;
		ddprintf("list elem 0x%x ",temp);
	}
    ddprintf("\n");
}


