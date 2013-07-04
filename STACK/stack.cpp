/*
** Module   :STACK.CPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Thu  20/05/2004 Created
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

#include "stack.hpp"
#include "host.hpp"
#include "ddprintf.h"
#include "global.hpp"
#include "fireerror.h"
//#include "nodemgr.hpp"
#include <include.h>
//#define min(a,b)  (((a) < (b)) ? (a) : (b))

FIRESTACK::FIRESTACK(void)
{
}
void FIRESTACK::RegisterHighlevel(PHIGHLEVELOPS pHighlevel)
{
    PHOSTOPS pCurrHost;
    HighlevelList.AddTail(pHighlevel);
    if (pHighlevel->fpfnAddHost)
    {
        pCurrHost=HostList.GetHead();
        while (pCurrHost!=NULL)
        {
            pHighlevel->fpfnAddHost(pCurrHost);
            pCurrHost=HostList.GetNext();
        }
    }
}
void FIRESTACK::HostReset(PHOSTOPS phost)
{
    PHIGHLEVELOPS pCurrHighlevel;
//        struct hpsb_highlevel *hl;

//    read_lock(&hl_irqs_lock);
    pCurrHighlevel=HighlevelList.GetHead();
    while (pCurrHighlevel!=NULL)
    {
        if (pCurrHighlevel->fpfnHostReset)
            pCurrHighlevel->fpfnHostReset(phost);
        pCurrHighlevel=HighlevelList.GetNext();
    }
    //    list_for_each_entry(hl, &hl_irqs, irq_list) {
    //                if (hl->host_reset)
    //                        hl->host_reset(host);
    //        }
//    read_unlock(&hl_irqs_lock);
}
//HIGHLEVELLIST::HIGHLEVELLIST(void)
//{
//    pList=new LIST;
//    if (pList==NULL) ddprintf("ERROR: pList is NULL\n");
//}
void FIRESTACK::AddHost(PHOSTOPS pHost)
{
    //        struct hpsb_highlevel *hl;
    PHIGHLEVELOPS hl;
    HostList.AddTail(pHost);

    hl=HighlevelList.GetHead();
    while (hl!=NULL)
    {
        if (hl->fpfnAddHost)
            hl->fpfnAddHost(pHost);
        if (pHost->fpfnUpdateConfigRomImage)
            if (pHost->fpfnUpdateConfigRomImage(pHost))
                ddprintf("ERROR: Failed to generate Configuration ROM image for host %s-%d\n", (char far *)hl->name, pHost->pHost->usId);
        hl=HighlevelList.GetNext();
    }
    //        list_for_each_entry(hl, &hl_drivers, hl_list) {
    //        if (hl->add_host)
    //            hl->add_host(host);
    //        }
//    up_read(&hl_drivers_sem);
//    if (host->update_config_rom) {
//        if (hpsb_update_config_rom_image(host) < 0)
//            HPSB_ERR("Failed to generate Configuration ROM image for "
//                 "host %s-%d", hl->name, host->id);
//    }
}
void FIRESTACK::RemoveHost(PHOSTOPS pHost)
{
    PHIGHLEVELOPS hl;
    PADDRSPACE as;

    /* First, let the highlevel driver unreg */
    hl=HighlevelList.GetHead();
    while (hl!=NULL)
    {
        if (hl->fpfnRemoveHost)
            hl->fpfnRemoveHost(pHost);
//        DestroyHostInfo(hl,pHost);
        hl=HighlevelList.GetNext();
    }
    as=AddrSpaceList.GetHead();
    while (as!=NULL)
    {
        if (as->pHostOps==pHost)
            AddrSpaceList.Remove(as);
        as=AddrSpaceList.GetNext();
    }
    HostList.Remove(pHost);
}
int FIRESTACK::Read(HOSTOPS far * pHost, int nodeid, void far * data, U64 addr, USHORT length, USHORT flags)
{
    PADDRSPACE as;
    //        struct hpsb_address_serve *as;
    unsigned int partlength;
    int rcode = RCODE_ADDRESS_ERROR;

//        read_lock(&addr_space_lock);

    //    list_for_each_entry(as, &host->addr_space, host_list) {
    as=AddrSpaceList.GetHead();
    while (!as==NULL)
    {
        if (as->start > addr)
            break;

        if (as->end > addr)
        {
            partlength = min(as->end - addr, (U64) length);

            if (as->op->fpfnRead)
            {
                rcode = as->op->fpfnRead(pHost, nodeid, (PQUADLET)data,
                                        addr, partlength, flags);
            }
            else
            {
                rcode = RCODE_TYPE_ERROR;
            }

            data=(void far *)((ULONG)data+partlength);
            //MAKEP(SELECTOROF(data),OFFSETOF(data)+partlength);
            length -= partlength;
            addr+= partlength;

            if ((rcode != RCODE_COMPLETE) || !length)
            {
                break;
            }
        }
        as=AddrSpaceList.GetNext();
    }

//        read_unlock(&addr_space_lock);

    if (length && (rcode == RCODE_COMPLETE))
    {
        rcode = RCODE_ADDRESS_ERROR;
    }

    return rcode;
}
int FIRESTACK::Write(HOSTOPS far * pHost, int nodeid, int destid, void far *data, U64 addr, USHORT length, USHORT flags)
{
    PADDRSPACE as;
    //        struct hpsb_address_serve *as;
    unsigned int partlength;
    int rcode = RCODE_ADDRESS_ERROR;

//        read_lock(&addr_space_lock);
//
    //    list_for_each_entry(as, &host->addr_space, host_list) {
    as=AddrSpaceList.GetHead();
    while (!as==NULL)
    {
        if (as->start > addr)
            break;

        if (as->end > addr)
        {
            partlength = min(as->end - addr, (U64) length);

            if (as->op->fpfnWrite)
            {
                rcode = as->op->fpfnWrite(pHost, nodeid, destid,
                                      (PQUADLET)data, addr, partlength, flags);
            }
            else
            {
                rcode = RCODE_TYPE_ERROR;
            }

            //            data += partlength;
            //MAKEP(SELECTOROF(data),OFFSETOF(data)+partlength);
            data=(void far *)((ULONG)data+partlength);
            length -= partlength;
            addr += partlength;

            if ((rcode != RCODE_COMPLETE) || !length)
            {
                 break;
            }
        }
       as=AddrSpaceList.GetNext();
    }

//        read_unlock(&addr_space_lock);

    if (length && (rcode == RCODE_COMPLETE))
    {
        rcode = RCODE_ADDRESS_ERROR;
    }

    return rcode;
}
int FIRESTACK::Lock(PHOSTOPS pHost, int nodeid, PQUADLET store, U64 addr, QUADLET data, QUADLET arg, int ext_tcode, USHORT flags)
{
    PADDRSPACE as;
//        struct hpsb_address_serve *as;
    int rcode = RCODE_ADDRESS_ERROR;

//        read_lock(&addr_space_lock);

    //    list_for_each_entry(as, &host->addr_space, host_list) {
    as=AddrSpaceList.GetHead();
    while (!as==NULL)
    {
        if (as->start > addr)
            break;

        if (as->end > addr)
        {
            if (as->op->fpfnLock)
            {
                rcode = as->op->fpfnLock(pHost, nodeid, store, addr,
                                        data, arg, ext_tcode, flags);
            }
            else
            {
                rcode = RCODE_TYPE_ERROR;
            }

            break;
        }
        as=AddrSpaceList.GetNext();
    }

//        read_unlock(&addr_space_lock);

    return rcode;
}
int FIRESTACK::Lock64(PHOSTOPS pHost, int nodeid, POCTLET store, U64 addr, OCTLET data, OCTLET arg, int ext_tcode, USHORT flags)
{
    PADDRSPACE as;
//        struct hpsb_address_serve *as;
    int rcode = RCODE_ADDRESS_ERROR;

//        read_lock(&addr_space_lock);

    //    list_for_each_entry(as, &host->addr_space, host_list) {
    as=AddrSpaceList.GetHead();
    while (!as==NULL)
    {
        if (as->start > addr)
            break;

        if (as->end > addr)
        {
            if (as->op->fpfnLock64)
            {
                rcode = as->op->fpfnLock64(pHost, nodeid, store,
                                        addr, data, arg,
                                        ext_tcode, flags);
            }
            else
            {
                rcode = RCODE_TYPE_ERROR;
            }

            break;
        }
        as=AddrSpaceList.GetNext();
    }

//        read_unlock(&addr_space_lock);

    return rcode;
}
void FIRESTACK::IsoReceive(PHOSTOPS pHost, PVOID data, int length)
{
    PHIGHLEVELOPS hl;
    //    struct hpsb_highlevel *hl;
    int channel = (UCHAR)((((PQUADLET)data)[0] >> 8) & 0x3f);

//    read_lock(&hl_irqs_lock);
    //    list_for_each_entry(hl, &hl_irqs, irq_list) {
    hl=HighlevelList.GetHead();
    while (!hl==NULL)
    {

        if (hl->fpfnIsoReceive)
            hl->fpfnIsoReceive(pHost, channel, (PQUADLET)data, length);
        hl=HighlevelList.GetNext();
    }
//    read_unlock(&hl_irqs_lock);
}
void FIRESTACK::FcpRequest(PHOSTOPS pHost, int nodeid, int direction, PVOID data, int length)
{
    PHIGHLEVELOPS hl;
    //        struct hpsb_highlevel *hl;
    ULONG cts = ((PQUADLET)data)[0] >> 4;

//        read_lock(&hl_irqs_lock);
    //    list_for_each_entry(hl, &hl_irqs, irq_list) {
    hl=HighlevelList.GetHead();
    while (!hl==NULL)
    {

        if (hl->fpfnFcpRequest)
            hl->fpfnFcpRequest(pHost, nodeid, direction, (int)cts, (UCHAR far *)data,
                            length);
        hl=HighlevelList.GetNext();
    }

//        read_unlock(&hl_irqs_lock);
}
/*BOOL FIRESTACK::RegisterAddrSpace(PADDRSPACE pAddrSpace )
{
//        struct hpsb_address_serve *as;
//    struct list_head *lh;
    BOOL retval = FALSE;
//        unsigned long flags;

    if (((pAddrSpace->start|pAddrSpace->end) & 3)||(pAddrSpace->start >= pAddrSpace->end) || (pAddrSpace->end > 0x1000000000000ULL))
    {
        ddprintf("ERROR: RegisterAddrSpace called with invalid addresses\n");
        return FALSE;
    }

    pushf();
    cli();
//    write_lock_irqsave(&addr_space_lock, flags);
    AddrSpaceList.AddTail(pAddrSpace);
    retval=TRUE;

//    list_for_each(lh, &host->addr_space) {
//        struct hpsb_address_serve *as_this =
//            list_entry(lh, struct hpsb_address_serve, host_list);
//        struct hpsb_address_serve *as_next =
//            list_entry(lh->next, struct hpsb_address_serve, host_list);

//        if (as_this->end > as->start)
//            break;

//        if (as_next->start >= as->end) {
//            list_add(&as->host_list, lh);
//            list_add_tail(&as->hl_list, &hl->addr_list);
//            retval = 1;
//            break;
//        }
//    }

    popf();
//    write_unlock_irqrestore(&addr_space_lock, flags);

//    if (retval == 0)
//        kfree(as);

    return retval;
} */
void FIRESTACK::UnRegisterHighlevel(PHIGHLEVELOPS pHighlevel)
{
    PHOSTOPS pHost;
    PADDRSPACE as;

    pHost=HostList.GetHead();
    while (pHost!=NULL)
    {
        if (pHighlevel->fpfnRemoveHost)
            pHighlevel->fpfnRemoveHost(pHost);

//        DestroyHostInfo(pHighlevel,pHost);
        pHost=HostList.GetNext();
    }
    as=AddrSpaceList.GetHead();
    while (as!=NULL)
    {
        if (as->pHighlevel==pHighlevel)
            AddrSpaceList.Remove(as);
        as=AddrSpaceList.GetNext();
    }
    HighlevelList.Remove(pHighlevel);

}
BOOL FIRESTACK::UnRegisterAddrSpace(PADDRSPACE pAddrSpace)
{
    int retval = 0;
//        struct hpsb_address_serve *as;
//        struct list_head *lh, *next;
//        unsigned long flags;

//        write_lock_irqsave(&addr_space_lock, flags);
    pushf();
    cli();
    if (AddrSpaceList.Remove(pAddrSpace)) retval=TRUE;
//    list_for_each_safe (lh, next, &hl->addr_list) {
//                as = list_entry(lh, struct hpsb_address_serve, hl_list);
//                if (as->start == start && as->host == host) {
//            __delete_addr(as);
//                        retval = 1;
//                        break;
//                }
//        }

//        write_unlock_irqrestore(&addr_space_lock, flags);
    popf();
    return retval;
}
int FIRESTACK::ListenChannel( PHOSTOPS host, unsigned int channel)
{
    if (channel > 63)
    {
        ddprintf("ERROR: ListenChannel called with invalid channel\n");
        return -FIRE_ERROR_INVALID;
    }

    if (host->pHost->IsoListenCount[channel]++ == 0)
    {
        return host->pHost->pHardDriver->fpfnDevCtl(ISO_LISTEN_CHANNEL, (char)channel);
    }

    return 0;
}
void FIRESTACK::UnlistenChannel(PHOSTOPS host, unsigned int channel)
{
        if (channel > 63)
        {
            ddprintf("ERROR: UnlistenChannel called with invalid channel\n");
            return;
        }

        if (--host->pHost->IsoListenCount[channel] == 0)
        {
            host->pHost->pHardDriver->fpfnDevCtl(ISO_UNLISTEN_CHANNEL, channel);
        }
}
/*PHOSTINFO FIRESTACK::GetHostInfo(PHIGHLEVELOPS hl,PHOSTOPS host)
{
    PHOSTINFO hi = NULL;

    if (!hl || !host)
        return NULL;

//    read_lock(&hl->host_info_lock);
//    list_for_each_entry(hi, &hl->host_info_list, list) {
    //hi=(hl->pHostInfoList)HostInfoList.GetHead();
    //while (hi!=NULL)
    //{

        //if (hi->host == host)
        //{
//            read_unlock(&hl->host_info_lock);
            //return hi;
        //}
        //hi=hl->HostInfoList.GetNext();
    //}
    hl->fpfnGetHostInfo(host);
//    read_unlock(&hl->host_info_lock);

    return NULL;
}
void FIRESTACK::DestroyHostInfo(PHIGHLEVELOPS hl,PHOSTOPS pHostOps)
{
    hl->fpfnDestroyHostInfo(pHostOps);
}
*/
U64 FIRESTACK::AllocateAndRegisterAddrSpace(HIGHLEVELOPS far *hl,
                     HOSTOPS far *host,
                     ADDROPS far *ops,
                     U64 size, U64 alignment,
                     U64 start, U64 end)
{
    ADDRSPACE far *as, far *a1, far *a2;
//    struct list_head *entry;
    U64 retval = ~0ULL;
//    unsigned long flags;
    U64 align_mask = ~(alignment - 1);

    if ((alignment & 3) || (alignment > 0x800000000000ULL) ||
        ((hweight32(alignment >> 32) +
          hweight32(alignment & 0xffffffff) != 1)))
        {
            ddprintf("ERROR: %s called with invalid alignment: 0x%lx%lx",
             (char far *)__FUNCTION__, (ULONG)(alignment&0xffffffff00000000ULL),(ULONG)(alignment&0xffffffff));
            return retval;
        }

    if (start == ~0ULL && end == ~0ULL)
    {
        start = CSR1212_ALL_SPACE_BASE + 0xffff00000000ULL;  /* ohci1394.c limit */
        end = CSR1212_ALL_SPACE_END;
    }

    if (((start|end) & ~align_mask) || (start >= end) || (end > 0x1000000000000ULL))
    {
        ddprintf("ERROR: %s called with invalid addresses (start = %lx%lx    end = %lx%lx)",
             (char far *)__FUNCTION__, (ULONG)(start&0xffffffff00000000ULL),(ULONG)(start&0xffffffff), (ULONG)(end&0xffffffff00000000ULL),(ULONG)(end&0xffffffff));
        return retval;
    }

    as = (ADDRSPACE far *) new ADDRSPACE;
    if (as == NULL)
    {
        return retval;
    }

//    INIT_LIST_HEAD(&as->host_list);
//    INIT_LIST_HEAD(&as->hl_list);
    as->op = ops;
    as->pHostOps = host;

  //  write_lock_irqsave(&addr_space_lock, flags);
    pushf();
    cli();
    //something weird ;)
//    list_for_each(entry, &host->addr_space) {
    U64 a1sa, a1ea;
    U64 a2sa, a2ea;
    a1=AddrSpaceList.GetHead();
    a2=AddrSpaceList.GetNext();
//        a1 = list_entry(entry, struct hpsb_address_serve, host_list);
//        a2 = list_entry(entry->next, struct hpsb_address_serve, host_list);
    while (a1!=NULL)
    {
        a1sa = a1->start & align_mask;
        a1ea = (a1->end + alignment -1) & align_mask;
        a2sa = a2->start & align_mask;
        a2ea = (a2->end + alignment -1) & align_mask;

        if ((a2sa - a1ea >= size) && (a2sa - start >= size) && (end - a1ea >= size))
        {
            as->start = max(start, a1ea);
            as->end = as->start + size;
//        list_add(&as->host_list, entry);
//        list_add_tail(&as->hl_list, &hl->addr_list);
            AddrSpaceList.AddAfter(as,a1);
            retval = as->start;
            break;
        }
        a1=a2;
        a2=AddrSpaceList.GetNext();
    }

//    write_unlock_irqrestore(&addr_space_lock, flags);
    popf();

    if (retval == ~0ULL)
    {
//        kfree(as);
        delete (ADDROPS *)as;
    }

    return retval;
}

BOOL FIRESTACK::RegisterAddrSpace(HIGHLEVELOPS far *hl, HOSTOPS far *host,
                            ADDROPS far *ops, U64 start, U64 end)
{
    ADDRSPACE far *as, far * as_this, far * as_next;

//    struct list_head *lh;
    BOOL retval = FALSE;
//        unsigned long flags;

    if (((start|end) & 3) || (start >= end) || (end > 0x1000000000000ULL))
    {
        ddprintf("ERROR %s called with invalid addresses", (char far *)__FUNCTION__);
        return FALSE;
    }

    as = (ADDRSPACE far *) new ADDRSPACE;
    if (as == NULL)
    {
        return FALSE;
    }

//        INIT_LIST_HEAD(&as->host_list);
//        INIT_LIST_HEAD(&as->hl_list);
    as->op = ops;
    as->start = start;
    as->end = end;
    as->pHostOps = host;

//   write_lock_irqsave(&addr_space_lock, flags);
    pushf();
    cli();

//    list_for_each(lh, &host->addr_space) {
//        struct hpsb_address_serve *as_this =
//            list_entry(lh, struct hpsb_address_serve, host_list);
//        struct hpsb_address_serve *as_next =
//            list_entry(lh->next, struct hpsb_address_serve, host_list);
//	AddrSpaceList.Dump();
    as_this=AddrSpaceList.GetHead();
    as_next=AddrSpaceList.GetNext();
    while (as_this!=NULL)
    {

        if (as_this->end > as->start)
            break;

        if (as_next->start >= as->end)
        {
            AddrSpaceList.AddAfter(as,as_this);
//         list_add(&as->host_list, lh);
//            list_add_tail(&as->hl_list, &hl->addr_list);
			AddrSpaceList.Dump();
            retval = TRUE;
            break;
        }
        as_this=as_next;
        as_next=AddrSpaceList.GetNext();
    }
//    write_unlock_irqrestore(&addr_space_lock, flags);
    popf();

    if (retval == FALSE)
        delete (ADDROPS *)as;

    return retval;
}

