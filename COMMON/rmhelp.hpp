/*
** Module   :RMHELP.HPP
** Abstract :resource manager related funcs.
**
** Copyright (C) Alexandr Cherkaev
** Original copyright Timur Tabi
**
** Log: Fri  30/04/2004	Created
**
*/
#ifndef __RMHELP_HPP
#define __RMHELP_HPP


#ifdef __cplusplus
extern "C" {
#endif

#include <rmcalls.h>

#ifdef __cplusplus
}
#endif

class DRIVER {
   int fRegistered;        // True if this driver is registered with RM
public:
   HDRIVER hdriver;
   DRIVER(void);
   ~DRIVER(void);
   int add(DRIVERSTRUCT *);
   void remove(void);
};

class ADAPTER {
   int fRegistered;        // True if this adapter is registered with RM
public:
   HADAPTER hadapter;
   DRIVER *pdriver;
   ADAPTER(DRIVER *);
   ~ADAPTER(void);
   int add(ADAPTERSTRUCT *);
   void remove(void);
};

class DEVICE {
   int fRegistered;        // True if this device is registered with RM
public:
   HDEVICE hdevice;
   ADAPTER *padapter;
   DEVICE(ADAPTER *);
   ~DEVICE(void);
   int add(DEVICESTRUCT *);
   void remove(void);
};

class RESOURCE {
   int fRegistered;        // True if this resource is registered with RM
   HRESOURCE hresource;
protected:
   ADAPTER *padapter;
   RESOURCESTRUCT resource;
   RESOURCE(ULONG ulType);
   ~RESOURCE(void);
public:
   int add(ADAPTER *);
   void remove(void);
};

class PORT_RESOURCE : public RESOURCE {
public:
   PORT_RESOURCE(USHORT usBase, USHORT usNumPorts, USHORT usFlags, USHORT usAddrLines);
};

class IRQ_RESOURCE : public RESOURCE {
public:
   IRQ_RESOURCE(USHORT usIRQ, USHORT usPCIIRQ, USHORT usFlags);
};
class MEM_RESOURCE : public RESOURCE {
public:
   MEM_RESOURCE(ULONG ulBase, ULONG ulSize, USHORT usFlags);
};
class DMA_RESOURCE : public RESOURCE {
public:
   DMA_RESOURCE(USHORT usChannel, USHORT usFlags);
};
class TMR_RESOURCE : public RESOURCE {
public:
	TMR_RESOURCE(USHORT usChannel, USHORT usFlags);
};


#endif  /*__RMHELP_HPP*/

