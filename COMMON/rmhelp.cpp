/*
** Module   :RMHELP.CPP
** Abstract :resource manager related funcs.
**
** Copyright (C) Alexandr Cherkaev
** Original copyright Timur Tabi
**
** Log: Fri  30/04/2004	Created
**
*/


#include <include.h>

#define INCL_NOPMAPI
#include <os2.h>

#include "rmhelp.hpp"  // brings in rmcalls.h

/*----------------------------------------------*/
/* GLOBAL VARS FOR RM                           */
/*                                              */
/* RM.LIB needs these declared                  */
/*----------------------------------------------*/
extern "C" PFN             RM_Help               = 0L;  /*VPNP*/
extern "C" PFN             RM_Help0              = 0L;  /*VPNP*/
extern "C" PFN             RM_Help3              = 0L;  /*VPNP*/
extern "C" ULONG           RMFlags               = 0L;  /*VPNP*/

DRIVER::DRIVER(void)
{
   fRegistered=FALSE;
}

DRIVER::~DRIVER(void)
{
   remove();
}

int DRIVER::add(DRIVERSTRUCT *pds)
{
   return fRegistered = RMCreateDriver(pds,&hdriver) == RMRC_SUCCESS;
}

void DRIVER::remove(void)
{
   if (fRegistered) {
      RMDestroyDriver(hdriver);
      fRegistered = FALSE;
   }
}

ADAPTER::ADAPTER(DRIVER *pdriver)
{
   hadapter = 0;
   this->pdriver = pdriver;
}

ADAPTER::~ADAPTER(void)
{
   remove();
}

int ADAPTER::add(ADAPTERSTRUCT *pas)
{
   fRegistered = RMCreateAdapter(pdriver->hdriver,
                       &hadapter, pas, NULL, NULL) == RMRC_SUCCESS;
   return fRegistered;
}

void ADAPTER::remove(void)
{
   RMDestroyAdapter(pdriver->hdriver, hadapter);
}

DEVICE::DEVICE(ADAPTER *padapter)
{
   fRegistered=FALSE;
   hdevice=0;
   this->padapter = padapter;
}

DEVICE::~DEVICE(void)
{
   remove();
}

int DEVICE::add(DEVICESTRUCT *pds)
{
   fRegistered = RMCreateDevice(padapter->pdriver->hdriver,
                                &hdevice, pds, padapter->hadapter, NULL) == RMRC_SUCCESS;
   return fRegistered;
}

void DEVICE::remove(void)
{
   if (fRegistered) {
      RMDestroyDevice(padapter->pdriver->hdriver, hdevice);
      fRegistered=FALSE;
   }
}


// RESOURCE *phead=NULL, *ptail=NULL;

RESOURCE::RESOURCE(ULONG ulType)
{
   padapter=NULL;
   hresource=0;
   resource.ResourceType = ulType;
   resource.Reserved = 0;
}

RESOURCE::~RESOURCE(void)
{
   remove();
}

int RESOURCE::add(ADAPTER *padapter)
{
   this->padapter = padapter;
   if (RMAllocResource(padapter->pdriver->hdriver,
                       &hresource,
                       &resource) != RMRC_SUCCESS)
      return FALSE;

   if (RMModifyResources(padapter->pdriver->hdriver,
                         padapter->hadapter,
                         RM_MODIFY_ADD,
                         hresource) != RMRC_SUCCESS) {
      RMDeallocResource(padapter->pdriver->hdriver, hresource);
      return FALSE;
   }

   fRegistered=TRUE;
   return TRUE;
}

void RESOURCE::remove(void)
{
   if (fRegistered) {
      RMDeallocResource(padapter->pdriver->hdriver, hresource);
      fRegistered=FALSE;
   }
}

PORT_RESOURCE::PORT_RESOURCE(USHORT usBase, USHORT usNumPorts, USHORT usFlags, USHORT usAddrLines)
   : RESOURCE(RS_TYPE_IO)
{
   resource.IOResource.BaseIOPort     = usBase;
   resource.IOResource.NumIOPorts     = usNumPorts;
   resource.IOResource.IOFlags        = usFlags;
   resource.IOResource.IOAddressLines = usAddrLines;
}

IRQ_RESOURCE::IRQ_RESOURCE(USHORT usIRQ, USHORT usPCIIRQ, USHORT usFlags)
   : RESOURCE(RS_TYPE_IRQ)
{
   resource.IRQResource.IRQLevel       = usIRQ;
   resource.IRQResource.PCIIrqPin      = usPCIIRQ;
   resource.IRQResource.IRQFlags       = usFlags;
   resource.IRQResource.Reserved       = 0;
   resource.IRQResource.pfnIntHandler  = NULL;
}
MEM_RESOURCE::MEM_RESOURCE(ULONG ulBase, ULONG ulSize, USHORT usFlags)
	: RESOURCE(RS_TYPE_MEM)
{
	resource.MEMResource.MemBase     	= ulBase;
    resource.MEMResource.MemSize    	= ulSize;
    resource.MEMResource.MemFlags 	    = usFlags;
    resource.MEMResource.ReservedAlign	= 0;
}
DMA_RESOURCE::DMA_RESOURCE(USHORT usChannel, USHORT usFlags)
    : RESOURCE(RS_TYPE_DMA)
{
    resource.DMAResource.DMAChannel     = usChannel;
    resource.DMAResource.DMAFlags       = usFlags;
}
TMR_RESOURCE::TMR_RESOURCE(USHORT usChannel, USHORT usFlags)
    : RESOURCE(RS_TYPE_TIMER)
{
    resource.TMRResource.TMRChannel     = usChannel;
    resource.TMRResource.TMRFlags       = usFlags;
}

