/* $Id: ioctl.cpp,v 1.5 2004/08/05 09:31:17 doctor64 Exp $ */

/* IOCTL.CPP

   MODIFICATION HISTORY
   DATE       PROGRAMMER   COMMENT
   15-Aug-98  Timur Tabi   Creation
*/


#define INCL_NOPMAPI
#include <os2.h>
#include <string.h>

#include "strategy.h"
#include "header.h"
#include "ddprintf.h"
#include "malloc.h"
#include "ioctl.h"
#include "rmhelp.hpp"
//#include "isr2.hpp"
#include "version.h"
#include "nodemgr.hpp"
#include "globalmgr.hpp"
#include <include.h>
#include <devhelp.h>

USHORT usYear;
USHORT usMonth;
USHORT usDay;

ADAPTER *padpt;

int CheckP1(PREQPACKET prp, unsigned uLength)
{
   if (!prp->s.ioctl.pvParm)
      return FALSE;

   return prp->s.ioctl.usPLength == uLength;
}

int CheckP2(PREQPACKET prp, unsigned uLength)
{
   if (!prp->s.ioctl.pvData)
      return FALSE;

   return prp->s.ioctl.usDLength == uLength;
}

void SetError(PREQPACKET prp, BYTE error)
{
   if (error)
      prp->usStatus = RPDONE | RPERR | RPDEV | error;
}

void WriteULONG(PREQPACKET prp, unsigned uOffset, ULONG ul)
{
   char __far *pc=(char __far *) prp->s.ioctl.pvData;

   if (!pc) {
      ddprintf("NodeMgr: WriteULONG, Func=%x, Offset=%ui, pc=NULL\n", (USHORT) prp->s.ioctl.bCode, uOffset);
      return;
   }
   if ( uOffset+sizeof(ULONG) > prp->s.ioctl.usDLength ) {
      ddprintf("NodeMgr: WriteULONG, Func=%x, Offset=%ui, DLength=%ui, pc=%p\n", (USHORT) prp->s.ioctl.bCode, uOffset, prp->s.ioctl.usDLength, pc);
      return;
   }

   *((ULONG __far *) (pc+uOffset))=ul;
}

int WriteString(PREQPACKET prp, unsigned uOffset, char *psz, unsigned uLength)
{
   char __far *pc=(char __far *) prp->s.ioctl.pvData;

   if (!pc) {
      ddprintf("NodeMgr: WriteString, Func=%ui, Offset=%ui, pc=NULL\n", (USHORT) prp->s.ioctl.bCode, uOffset);
      return FALSE;
   }
   if ( uOffset+uLength > prp->s.ioctl.usDLength ) {
      ddprintf("NodeMgr: WriteString, Func=%ui, Offset=%ui, DLength=%ui, pc=%p\n", (USHORT) prp->s.ioctl.bCode, uOffset, prp->s.ioctl.usDLength, pc);
      return FALSE;
   }

   _fstrncpy(pc+uOffset,psz,uLength);
   return TRUE;
}

unsigned ReadInt(PREQPACKET prp, unsigned uOffset)
{
   char __far *pc = (char __far *) prp->s.ioctl.pvParm + uOffset;
   unsigned __far *pu = (unsigned __far *) pc;

   return *pu;
}

void __far *ReadPtr(PREQPACKET prp, unsigned uOffset)
{
   char __far *pc = (char __far *) prp->s.ioctl.pvParm + uOffset;
   void __far * __far *ppv = (void __far * __far *) pc;

   return *ppv;
}


void IoctlGetVersion(PREQPACKET prp)
{

   NODEMGR_GET_VERSION __far *p = (NODEMGR_GET_VERSION __far *) prp->s.ioctl.pvParm;

   p->usMajorVersion = VERSION_MAJOR;
   p->usMinorVersion = VERSION_MINOR;
   p->usBuildLevel = VERSION_LEVEL;

   p->usYear = usYear;
   p->bMonth = (UCHAR) usMonth;
   p->bDay = (UCHAR) usDay;
}

void IoctlGetHostNum(PREQPACKET prp)
{
    HOSTINFO far * tempHostInfo;
    NODEMGR_GET_HOSTNUM __far *p = (NODEMGR_GET_HOSTNUM __far *) prp->s.ioctl.pvParm;
    char i=0;
    tempHostInfo=pGlobalNodeMgr->HostInfoList.GetHead();
    while (tempHostInfo!=NULL)
    {
        i++;
        tempHostInfo=pGlobalNodeMgr->HostInfoList.GetNext();
    }
    p->ucHostNum=i;
}

void IoctlGetHost(PREQPACKET prp)
{
    HOSTINFO * tempHostInfo;
    NODEMGR_GET_HOST __far *p = (NODEMGR_GET_HOST __far *) prp->s.ioctl.pvParm;
    char i=-1;
    tempHostInfo=pGlobalNodeMgr->HostInfoList.GetHead();
    while (tempHostInfo!=NULL)
    {
        i++;
        if (i==p->ucHostNum)
        {
            p->usNodeCount=tempHostInfo->pHostOps->pHost->usNodeCount;
            p->usSelfIdCount=tempHostInfo->pHostOps->pHost->usSelfIdCount;
            p->usNodesActive=tempHostInfo->pHostOps->pHost->usNodesActive;
            p->usNodeId=tempHostInfo->pHostOps->pHost->NodeId;
            p->usIRMId=tempHostInfo->pHostOps->pHost->IRMId;
            p->usBusMgrId=tempHostInfo->pHostOps->pHost->BusMgrId;
            p->bInBusReset=tempHostInfo->pHostOps->pHost->bInBusReset;
            p->bIsShutdown=tempHostInfo->pHostOps->pHost->bIsShutdown;
            p->bIsRoot=tempHostInfo->pHostOps->pHost->bIsRoot;
            p->bIsCycmst=tempHostInfo->pHostOps->pHost->bIsCycmst;
            p->bIsIrm=tempHostInfo->pHostOps->pHost->bIsIrm;
            p->bIsBusmgr=tempHostInfo->pHostOps->pHost->bIsBusmgr;
            return;
        }
        tempHostInfo=pGlobalNodeMgr->HostInfoList.GetNext();
    }
}
void IoctlGetNodeNum(PREQPACKET prp)
{
    HOSTINFO * tempHostInfo;
    NODE_ENTRY * tempNodeEntry;
    NODEMGR_GET_NODENUM __far *p = (NODEMGR_GET_NODENUM __far *) prp->s.ioctl.pvParm;
    char i=-1;
    char j=0;
    tempHostInfo=pGlobalNodeMgr->HostInfoList.GetHead();
    while (tempHostInfo!=NULL)
    {
        i++;
        if (i==p->ucHostNum)
        {
            tempNodeEntry=tempHostInfo->NodeEntryList.GetHead();
            while (tempNodeEntry!=NULL)
            {
                j++;
                tempNodeEntry=tempHostInfo->NodeEntryList.GetNext();
            }
            p->ucNodeNum=j;
            return;
        }
        tempHostInfo=pGlobalNodeMgr->HostInfoList.GetNext();
    }
}
void IoctlGetNode(PREQPACKET prp)
{
    HOSTINFO * tempHostInfo;
    NODE_ENTRY  * tempNodeEntry;
    NODEMGR_GET_NODE __far *p = (NODEMGR_GET_NODE __far *) prp->s.ioctl.pvParm;
    char i=-1;
    char j=-1;
    int len;
    tempHostInfo=pGlobalNodeMgr->HostInfoList.GetHead();
    while (tempHostInfo!=NULL)
    {
        i++;
        if (i==p->ucHostNum)
        {
            tempNodeEntry=tempHostInfo->NodeEntryList.GetHead();
            while (tempNodeEntry!=NULL)
            {
                j++;
                if (j==p->ucNodeNum)
                {
                    p->ulGUIDHi=(ULONG)(tempNodeEntry->guid>>32);
                    p->ulGUIDLo=(ULONG)(tempNodeEntry->guid&0xffffffff);
                    p->ulGuidVendorId=tempNodeEntry->guid_vendor_id;
                    p->usNodeId=tempNodeEntry->NodeId;
                    p->BusOptions.ucIrmc=tempNodeEntry->busopt.irmc;
                    p->BusOptions.ucCmc=tempNodeEntry->busopt.cmc;
                    p->BusOptions.ucIsc=tempNodeEntry->busopt.isc;
                    p->BusOptions.ucBmc=tempNodeEntry->busopt.bmc;
                    p->BusOptions.ucPmc=tempNodeEntry->busopt.pmc;
                    p->BusOptions.ucCycClkAcc=tempNodeEntry->busopt.cyc_clk_acc;
                    p->BusOptions.ucMaxRom=tempNodeEntry->busopt.max_rom;
                    p->BusOptions.ucGeneration=tempNodeEntry->busopt.generation;
                    p->BusOptions.ucLinkSpeed=tempNodeEntry->busopt.lnkspd;
                    p->BusOptions.usMaxRec=tempNodeEntry->busopt.max_rec;
                    p->bNeedProbe=tempNodeEntry->needs_probe;
                    p->usGeneration=tempNodeEntry->generation;
                    p->ulVendorId=tempNodeEntry->vendor_id;
                    p->bInLimbo=tempNodeEntry->bInLimbo;
                    if (tempNodeEntry->vendor_name_kv!=NULL)
                    {
                    	len=(tempNodeEntry->vendor_name_kv->value.leaf.len-2)*sizeof(QUADLET);
	                    _fmemcpy(p->strVendorName,CSR1212_TEXTUAL_DESCRIPTOR_LEAF_DATA(tempNodeEntry->vendor_name_kv),len);
	                    p->strVendorName[len+1]=0;
	                }
	                else p->strVendorName[0]=0;
                    p->ulCapabilities=tempNodeEntry->capabilities;
//                  p->ucUnitDirNum=tempNodeEntry->;
                    return;
                }
                tempNodeEntry=tempHostInfo->NodeEntryList.GetNext();
            }
        }
        tempHostInfo=pGlobalNodeMgr->HostInfoList.GetNext();
    }

}
void IoctlGetUnitDirNum(PREQPACKET prp)
{
    HOSTINFO * tempHostInfo;
    NODE_ENTRY * tempNodeEntry;
    UNIT_DIRECTORY * tempUnitDir;
    NODEMGR_GET_UNITDIRNUM __far *p = (NODEMGR_GET_UNITDIRNUM __far *) prp->s.ioctl.pvParm;
    char i=-1;
    char j=-1;
    char k=0;
    tempHostInfo=pGlobalNodeMgr->HostInfoList.GetHead();
    while (tempHostInfo!=NULL)
    {
        i++;
        if (i==p->ucHostNum)
        {
            tempNodeEntry=tempHostInfo->NodeEntryList.GetHead();
            while (tempNodeEntry!=NULL)
            {
                j++;
                if (j==p->ucNodeNum)
                {
                    tempUnitDir=tempNodeEntry->UnitDirsList.GetHead();
                    while (tempUnitDir!=NULL)
                    {
                        k++;
                        tempUnitDir=tempNodeEntry->UnitDirsList.GetNext();
                    }
                    p->ucUnitDirNum=k;
                    return;
                }
                tempNodeEntry=tempHostInfo->NodeEntryList.GetNext();
            }
        }
        tempHostInfo=pGlobalNodeMgr->HostInfoList.GetNext();
    }
}
void IoctlGetUnitDir(PREQPACKET prp)
{
    HOSTINFO * tempHostInfo;
    NODE_ENTRY * tempNodeEntry;
    UNIT_DIRECTORY * tempUnitDir;
    NODEMGR_GET_UNITDIR __far *p = (NODEMGR_GET_UNITDIR __far *) prp->s.ioctl.pvParm;
    char i=-1;
    char j=-1;
    char k=-1;
    int len;
    tempHostInfo=pGlobalNodeMgr->HostInfoList.GetHead();
    while (tempHostInfo!=NULL)
    {
        i++;
        if (i==p->ucHostNum)
        {
            tempNodeEntry=tempHostInfo->NodeEntryList.GetHead();
            while (tempNodeEntry!=NULL)
            {
                j++;
                if (j==p->ucNodeNum)
                {
                    tempUnitDir=tempNodeEntry->UnitDirsList.GetHead();
                    while (tempUnitDir!=NULL)
                    {
                        k++;
                        if (k==p->ucUnitDirNum)
                        {
                                p->ulAddrHi=(ULONG)(tempUnitDir->address>>32);
                                p->ulAddrLo=(ULONG)(tempUnitDir->address&0xffffffff);
                                p->ucFlags=tempUnitDir->flags;
                                p->ulVendorId=tempUnitDir->vendor_id;
                                if (tempUnitDir->vendor_name_kv!=NULL)
                                {
	                                len=(tempUnitDir->vendor_name_kv->value.leaf.len-2)*sizeof(QUADLET);
    	                            _fmemcpy(p->strVendorName,CSR1212_TEXTUAL_DESCRIPTOR_LEAF_DATA(tempUnitDir->vendor_name_kv),len);
        	                        p->strVendorName[len+1]=0;
        	                    }
        	                    else p->strVendorName[0]=0;

//                              strVendorName[60];
                                p->ulModelId=tempUnitDir->model_id;
                                if (tempUnitDir->model_name_kv!=NULL)
                                {
                                	len=(tempUnitDir->model_name_kv->value.leaf.len-2)*sizeof(QUADLET);
	                                _fmemcpy(p->strModelName,CSR1212_TEXTUAL_DESCRIPTOR_LEAF_DATA(tempUnitDir->model_name_kv),len);
    	                            p->strModelName[len+1]=0;
    	                        }
    	                        else p->strModelName[0]=0;
    	                        //                              strModelName[60];
                                p->ulSpecifierId=tempUnitDir->specifier_id;
                                p->ulVersion=tempUnitDir->version;
                                p->usId=tempUnitDir->id;
                                return;
                        }
                        tempUnitDir=tempNodeEntry->UnitDirsList.GetNext();
                    }
                }
                tempNodeEntry=tempHostInfo->NodeEntryList.GetNext();
            }
        }
        tempHostInfo=pGlobalNodeMgr->HostInfoList.GetNext();
    }

}

void NodeMgr_StrategyIoctl(PREQPACKET prp)
{
    if (prp->s.ioctl.bCategory != NODEMGR_IOCTL_CATEGORY)
    {
        prp->usStatus |= RPERR | RPBADCMD;
        return;
    }

    switch (prp->s.ioctl.bCode)
    {
    //place ioctl process code here
        case NODEMGR_IOCTL_GET_VERSION:
            IoctlGetVersion(prp);
            break;
        case NODEMGR_IOCTL_GET_HOSTNUM:
            IoctlGetHostNum(prp);
            break;
        case NODEMGR_IOCTL_GET_HOST:
            IoctlGetHost(prp);
            break;
        case NODEMGR_IOCTL_GET_NODENUM:
            IoctlGetNodeNum(prp);
            break;
        case NODEMGR_IOCTL_GET_NODE:
            IoctlGetNode(prp);
            break;
        case NODEMGR_IOCTL_GET_UNITDIRNUM:
            IoctlGetUnitDirNum(prp);
            break;
        case NODEMGR_IOCTL_GET_UNITDIR:
            IoctlGetUnitDir(prp);
            break;
      default:
         prp->usStatus |= RPERR | RPBADCMD;
         return;
   }
}

