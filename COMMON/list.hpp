/*
** Module   :LIST.HPP
** Abstract : generic linked list
**
** Copyright (C) Alex Cherkaev
**
** Log: Thu  20/05/2004 Created
**
*/

#ifndef __LIST_HPP
#define __LIST_HPP

class LISTENTRY
{
public:
    LISTENTRY();
    LISTENTRY(PVOID pdata);
    PVOID pContent; // pointer to list element data
    LISTENTRY* pNextEntry;
    LISTENTRY* pPrevEntry;
};
typedef LISTENTRY* PLISTENTRY;
class LIST;
typedef LIST * PLIST;

class LIST //generic linked list class
{
public:
    LIST();
    PLISTENTRY GetHead(void);
    PLISTENTRY GetCurrent(void);
    PLISTENTRY GetNext(void);
    PLISTENTRY GetPrev(void);
    PLISTENTRY GetTail(void);
    void AddHead(PLISTENTRY pListEntry);
    void AddTail(PLISTENTRY pListEntry);
    void Add(PLISTENTRY pListEntry);
    void DeleteAll(void);
    void DeleteHead(void);
    void Delete(PLISTENTRY pDelEntry);
    void DeleteTail(void);
    BOOL isEmpty(void);
    void AddList(PLIST addedList);
    void Dump(void);

private:
    PLISTENTRY pListHead; // list head
    PLISTENTRY pListTail; // list tail
    PLISTENTRY pListCurrent;
};


#endif  /*__LIST_HPP*/

