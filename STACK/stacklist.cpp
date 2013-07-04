
extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define  INCL_NOPMAPI
#define  INCL_DOSDEVICES
#define  INCL_DOSDEVIOCTL
#define  INCL_DOSERRORS
#define  INCL_NOXLATE_DOS16
#include <os2.h>
}
#include "stacklist.hpp"
#include "stack.hpp"
#include "host.hpp"
#include "ddprintf.h"
#include "global.hpp"


ADDROPS dummy_ops =
{
	NULL,
    NULL,
    NULL,
    NULL
};
ADDRSPACE dummy_zero_addr,dummy_max_addr;
/* =
{
    dummy_ops, //ops
	NULL,	//host
	NULL,	//highlevel
	0, 		//startaddr
	0		//end addr
};
ADDRSPACE dummy_max_addr =
{
    dummy_ops, //ops
    NULL,   //host
    NULL,   //highlevel
    ((U64) 1) << 48,      //startaddr
    ((U64) 1) << 48       //end addr
};*/
HIGHLEVELLIST::AddHead(PHIGHLEVELOPS pProtocolDriver)
{
    PLISTENTRY pNewEntry;
    pNewEntry=new LISTENTRY;
    if (pNewEntry==NULL) ddprintf("ERROR: pNewEntry is NULL\n");
    pNewEntry->pContent=pProtocolDriver;
    List.AddHead(pNewEntry);
}
HIGHLEVELLIST::AddTail(PHIGHLEVELOPS pProtocolDriver)
{
    PLISTENTRY pNewEntry;
    pNewEntry=new LISTENTRY;
    if (pNewEntry==NULL) ddprintf("ERROR: pNewEntry is NULL\n");
    pNewEntry->pContent=pProtocolDriver;
    List.AddTail(pNewEntry);
}
PHIGHLEVELOPS HIGHLEVELLIST::GetHead(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetHead();
    if (pEntry==NULL) return NULL;
    return (PHIGHLEVELOPS)pEntry->pContent;
}
PHIGHLEVELOPS HIGHLEVELLIST::GetNext(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetNext();
    if (pEntry==NULL) return NULL;
    return (PHIGHLEVELOPS)pEntry->pContent;
}
PHIGHLEVELOPS HIGHLEVELLIST::GetTail(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetTail();
    return (PHIGHLEVELOPS)pEntry->pContent;
}

HIGHLEVELLIST::Remove(PHIGHLEVELOPS pProtocolDriver)
{
}
ADDRSPACELIST::ADDRSPACELIST(void)
{
    PLISTENTRY pNewEntry;
    pNewEntry=new LISTENTRY;
    dummy_max_addr.op=&dummy_ops;
    dummy_zero_addr.op=&dummy_ops;
    dummy_max_addr.pHostOps=NULL;
    dummy_zero_addr.pHostOps=NULL;
    dummy_max_addr.pHighlevel=NULL;
    dummy_zero_addr.pHighlevel=NULL;
    dummy_max_addr.start=((U64) 1) << 48;
    dummy_zero_addr.start=0;
    dummy_max_addr.end=((U64) 1) << 48;
    dummy_zero_addr.end=0;

    pNewEntry->pContent=&dummy_max_addr;
	List.AddHead(pNewEntry);
    pNewEntry=new LISTENTRY;
    pNewEntry->pContent=&dummy_zero_addr;

	List.AddHead(pNewEntry);
}
void ADDRSPACELIST::AddHead(PADDRSPACE pAddrSpace)
{
    PLISTENTRY pNewEntry;
    pNewEntry=new LISTENTRY;
    if (pNewEntry==NULL) ddprintf("ERROR: pNewEntry is NULL\n");
    pNewEntry->pContent=pAddrSpace;
    List.AddHead(pNewEntry);
}
void ADDRSPACELIST::AddTail(PADDRSPACE pAddrSpace)
{
    PLISTENTRY pNewEntry;
    pNewEntry=new LISTENTRY;
    if (pNewEntry==NULL) ddprintf("ERROR: pNewEntry is NULL\n");
    pNewEntry->pContent=pAddrSpace;
    List.AddTail(pNewEntry);
}
void ADDRSPACELIST::AddAfter(PADDRSPACE pAddrSpace,PADDRSPACE pAfter)
{
    PLISTENTRY pNewEntry,pAfterEntry;
    pNewEntry=new LISTENTRY;
    if (pNewEntry==NULL) ddprintf("ERROR: pNewEntry is NULL\n");
    pNewEntry->pContent=pAddrSpace;
    pAfterEntry=List.GetHead();
    while (pAfterEntry!=NULL)
    {
        if (pAfterEntry->pContent==pAfter)
        {
            List.Add(pNewEntry);
            break;
        }
        pAfterEntry=List.GetNext();
    }
}
BOOL ADDRSPACELIST::Remove(PADDRSPACE pAddrSpace)
{
    PLISTENTRY pTempEntry;
    ADDRSPACE far * pTempAS;
    pTempEntry=List.GetHead();
    while (pTempEntry!=NULL)
    {
        pTempAS=(ADDRSPACE far *)pTempEntry->pContent;
        if (pTempAS==NULL) {ddprintf("ERROR:damaged element in addrspacelist\n"); return FALSE;}
        if ((pTempAS->start==pAddrSpace->start)&&(pTempAS->end==pAddrSpace->end)
            &&(pTempAS->pHostOps==pAddrSpace->pHostOps))
        {
            List.Delete(pTempEntry);
            delete pTempEntry;
            return TRUE;
        }
        pTempEntry=List.GetNext();
    }
    return FALSE;
}
PADDRSPACE ADDRSPACELIST::GetHead(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetHead();
    if (pEntry==NULL) return NULL;
    return (PADDRSPACE)pEntry->pContent;
}
PADDRSPACE ADDRSPACELIST::GetNext(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetNext();
    if (pEntry==NULL) return NULL;
    return (PADDRSPACE)pEntry->pContent;
}

PADDRSPACE ADDRSPACELIST::GetTail(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetTail();
    if (pEntry==NULL) return NULL;
    return (PADDRSPACE)pEntry->pContent;
}

void ADDRSPACELIST::Dump(void)
{
	List.Dump();
}
//HOSTLIST::HOSTLIST(void)
//{
//    pList=new LIST;
//    if (pList==NULL) ddprintf("ERROR: pList is NULL\n");
//}
HOSTLIST::AddHead(PHOSTOPS pHost)
{
    PLISTENTRY pNewEntry;
    pNewEntry=new LISTENTRY;
    if (pNewEntry==NULL) ddprintf("ERROR: pNewEntry is NULL\n");
    pNewEntry->pContent=pHost;
    List.AddHead(pNewEntry);
}
HOSTLIST::AddTail(PHOSTOPS pHost)
{
    PLISTENTRY pNewEntry;
    pNewEntry=new LISTENTRY;
    if (pNewEntry==NULL) ddprintf("ERROR: pNewEntry is NULL\n");
    pNewEntry->pContent=pHost;
    List.AddTail(pNewEntry);
}
HOSTLIST::Remove(PHOSTOPS pHost)
{
}
PHOSTOPS HOSTLIST::GetHead(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetHead();
    if (pEntry==NULL) return NULL;
    return (PHOSTOPS)pEntry->pContent;
}
PHOSTOPS HOSTLIST::GetNext(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetNext();
    if (pEntry==NULL) return NULL;
    return (PHOSTOPS)pEntry->pContent;
}
PHOSTOPS HOSTLIST::GetTail(void)
{
    PLISTENTRY pEntry;
    pEntry=List.GetTail();
    if (pEntry==NULL) return NULL;
    return (PHOSTOPS)pEntry->pContent;
}

