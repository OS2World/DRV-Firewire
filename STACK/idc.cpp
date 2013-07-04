extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define  INCL_NOPMAPI
#define  INCL_DOSDEVICES
#define  INCL_DOSDEVIOCTL
#define  INCL_DOSERRORS
#define  INCL_NOXLATE_DOS16
#include <os2.h>
}
#include "idc.h"
#include "ddprintf.h"
#include <include.h>
#include "host.hpp"
#include "global.hpp"
#include "entry.hpp"

extern "C" void __far __loadds __cdecl StackIDCEntry(unsigned char type , void far * pData)
{
    ddprintf("DEBUG: IDC entry called\n");
    ddprintf("DEBUG: pData %lx\n",pData);
//    int3();
    switch (type)
    {
        case IDC_GET_STACK:
            *(STACKOPS far * far *)pData=&GlobalStackOps;
            ddprintf("stack_bus_ops %lx\n",pData);
            return;
        case IDC_ATTACH_NODEMGR:
            fpGCsrBusOps=(csr1212_bus_ops far *) pData;
            ddprintf("fpgcsr_bus_ops %lx\n",fpGCsrBusOps);
            return;
        case IDC_CREATE_HOST:
            fpGHost=new HOST(0);
            fpGHostOps=new HOSTOPS;
            if (fpGHost==NULL) ddprintf("ERROR: Host create error\n");
            if (fpGHostOps==NULL) ddprintf("ERROR: HostOps create error\n");
            fpGHost->pStack=&GlobalStackOps;
            *(HOSTOPS far * far *)pData=fpGHostOps;
            fpGHostOps->pHost=fpGHost;
            fpGHostOps->fpfnUpdateConfigRomImage=sHostUpdateConfigRomImage;
            fpGHostOps->fpfnAddHost=sHostAddHost;
            fpGHostOps->fpfnBusReset=sHostBusReset;
            fpGHostOps->fpfnSelfIdComplete=sHostSelfIdComplete;
            fpGHostOps->fpfnSelfIdReceived=sHostSelfIdReceived;
            fpGHostOps->fpfnPacketSent=sHostPacketSent;
            fpGHostOps->fpfnPacketReceived=sHostPacketReceived;
            fpGHostOps->fpfnRead=sHostRead;
            fpGHostOps->fpfnWrite=sHostWrite;
            fpGHostOps->fpfnLock=sHostLock;
            fpGHostOps->fpfnGetGeneration=sHostGetGeneration;
            fpGHostOps->fpfnResetBus=sHostResetBus;
            fpGHostOps->fpfnSendPhyConfig=sHostSendPhyConfig;
//            ((HOST far *)pTempHostOps->pHost)->pStack=pGlobalStack;
            return;
    }
}


