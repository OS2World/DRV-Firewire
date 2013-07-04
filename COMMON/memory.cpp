/*
** Module   :MEMORY.CPP
** Abstract : memory class functions
**
** Copyright (C) Alexandr Cherkaev
**
** Log: Sun  30/05/2004	Created
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
#include "ddprintf.h"

#include <devhelp.h>

#include "memory.hpp"
#define VMDHA_SELMAP            0x0080  // undoc
MEMORY::MEMORY(USHORT usSize)
{
	ULONG ulPhysPtr, ulLinPhys;
	APIRET rc;
    fpVirtAddr=NULL;
	ulPhysAddr=0;
    ulPhysPtr=(ULONG)(ULONG __far *)&ulPhysAddr;
    // align to page
    usSize=usSize+4095;
    usSize=usSize&0xf000;
//    usSize=usSize-usSize%16;
    rc=DevHelp_VirtToLin(SELECTOROF(ulPhysPtr),OFFSETOF(ulPhysPtr),&ulLinPhys);
    if (rc!=0)
    {
        ddprintf("ERROR: VirtToLin failed.\n");
        return;
    }
    rc=DevHelp_VMAlloc(VMDHA_FIXED|VMDHA_CONTIG|VMDHA_USEHIGHMEM|VMDHA_SELMAP,usSize,ulLinPhys,&ulLinAddr,(PPVOID)&fpVirtAddr);
    if (rc!=0)
    {
        ddprintf("ERROR: VMAlloc(VMDHA_USEHIGHMEM) fail. try low\n");
        rc=DevHelp_VMAlloc(VMDHA_FIXED|VMDHA_CONTIG|VMDHA_SELMAP,usSize,ulLinPhys,&ulLinAddr,(PPVOID)&fpVirtAddr);
        if (rc!=0)
        {
            ddprintf("ERROR: VMAlloc() fail. bail out\n");
			return;
        }
    }
    int ofs=(int)(16-ulPhysAddr&0x0fUL);

    fpVirtAddr=fpVirtAddr+ofs;
    ulPhysAddr=ulPhysAddr+ofs;
    usBSize=usSize;
    ddprintf("DEBUG: Memory block size %d real size %d allocated, addr: phys: %lx lin: %lx virt: %lx \n",usSize-16, usSize, ulPhysAddr,ulLinAddr,fpVirtAddr);
}
MEMORY::~MEMORY()
{
    APIRET rc;
	rc=DevHelp_VMFree(ulLinAddr);
	if (rc!=0)
    {
		ddprintf("ERROR: VMFree failed.\n");
		return;
    }
    ddprintf("DEBUG: Memory block size %d real size %d freed, addr: phys: %lx lin: %lx virt: %lx \n",usBSize-16, usBSize, ulPhysAddr,ulLinAddr,fpVirtAddr);
}
