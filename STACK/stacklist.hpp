/*
** Module   :STACKLIST.HPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  23/06/2004 Created
**
*/
#ifndef __STACKLIST_HPP
#define __STACKLIST_HPP
#include "idc.h"
#include "list.hpp"
//#include "..\ohci\host.hpp"

class HOST;
typedef HOST far * PHOST;

class HIGHLEVELLIST
{
public:
//    HIGHLEVELLIST();
    AddHead(PHIGHLEVELOPS pProtocolDriver);
    AddTail(PHIGHLEVELOPS pProtocolDriver);
    Remove(PHIGHLEVELOPS pProtocolDriver);
    PHIGHLEVELOPS GetHead(void);
    PHIGHLEVELOPS GetNext(void);
    PHIGHLEVELOPS GetTail(void);
    BOOL isEmpty(void);
private:
    LIST List;
};
class ADDRSPACE
{
public:
//        struct list_head host_list; /* per host list */
//        struct list_head hl_list; /* hpsb_highlevel list */

    ADDROPS far * op;

    HOSTOPS far * pHostOps;
    HIGHLEVELOPS far * pHighlevel;

    /* first address handled and first address behind, quadlet aligned */
    U64 start, end;
};
typedef ADDRSPACE far * PADDRSPACE;
class ADDRSPACELIST
{
public:
    ADDRSPACELIST();
	void AddHead(PADDRSPACE pAddrSpace);
	void AddTail(PADDRSPACE pAddrSpace);
	void AddAfter(PADDRSPACE pAddrSpace,PADDRSPACE pAfter);
    BOOL Remove(PADDRSPACE pAddrSpace);
    PADDRSPACE GetHead(void);
    PADDRSPACE GetNext(void);
    PADDRSPACE GetTail(void);
    BOOL isEmpty(void);
    void Dump(void);
private:
    LIST List;
};
class HOSTLIST
{
public:
//    HOSTLIST();
    AddHead(PHOSTOPS pHost);
    AddTail(PHOSTOPS pHost);
    Remove (PHOSTOPS pHost);
    PHOSTOPS GetHead(void);
    PHOSTOPS GetNext(void);
    PHOSTOPS GetTail(void);
    BOOL isEmpty(void);
//    void CallForEachEntry( void (*pfnFunc) (PHOST));

private:
    LIST List;
};

#endif  /*__STACKLIST_HPP*/

