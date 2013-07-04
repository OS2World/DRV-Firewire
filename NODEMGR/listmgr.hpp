/*
** Module   :LISTMGR.HPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  14/07/2004 Created
**
*/
#ifndef __LISTMGR_HPP
#define __LISTMGR_HPP

#include "list.hpp"
#include "firetype.h"
#include "..\stack\host.hpp"

struct UNIT_DIRECTORY;
struct NODE_ENTRY;
class HOSTINFO;
typedef UNIT_DIRECTORY far * PUNIT_DIRECTORY;
typedef NODE_ENTRY far * PNODE_ENTRY;
typedef HOSTINFO far * PHOSTINFO;

class UNIT_DIRLIST
{
public:
//    HOSTLIST();
    void AddHead(PUNIT_DIRECTORY pUnitDirectory);
    void AddTail(PUNIT_DIRECTORY pUnitDirectory);
    PUNIT_DIRECTORY GetHead(void);
    PUNIT_DIRECTORY GetNext(void);
    PUNIT_DIRECTORY GetTail(void);
    BOOL isEmpty(void);
    BOOL Remove(PUNIT_DIRECTORY pUnitDirectory);
    void DeleteHead(void);
private:
    LIST List;
};
class NODE_ENTRYLIST
{
public:
//    HOSTLIST();
    void AddHead(PNODE_ENTRY pHostInfo);
    void AddTail(PNODE_ENTRY pHostInfo);
    BOOL Remove (PNODE_ENTRY pHostInfo);
    PNODE_ENTRY GetHead(void);
    PNODE_ENTRY GetNext(void);
    PNODE_ENTRY GetTail(void);
    BOOL isEmpty(void);
    PNODE_ENTRY FindByNodeid(NODEID nodeid);
    PNODE_ENTRY FindByGuid(U64 guid);
private:
    LIST List;
};
class HOSTINFOLIST
{
public:
//    HOSTLIST();
    AddHead(PHOSTINFO pHostInfo);
    AddTail(PHOSTINFO pHostInfo);
    Remove (PHOSTINFO pHostInfo);
    PHOSTINFO GetHead(void);
    PHOSTINFO GetNext(void);
    PHOSTINFO GetTail(void);
    PHOSTINFO GetHostInfo(HOSTOPS far * pHost);
    BOOL isEmpty(void);
private:
    LIST List;
};

#endif  /*__LISTMGR_HPP*/

