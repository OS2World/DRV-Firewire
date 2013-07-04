/*
** Module   :PCIDEV.CPP
** Abstract : PCI device support class
**
** Copyright (C) Alexandr Cherkaev
**
** Log: Mon  03/05/2004 Created
**
*/

extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define  INCL_NOPMAPI
#define  INCL_DOSDEVICES
#define  INCL_DOSDEVIOCTL
#define  INCL_NOXLATE_DOS16
#include <os2.h>
#include <devhelp.h>
}
#include "ddprintf.h"
#include "pcidev.hpp"
#include "pciregs.h"
//#include "iprintf.h"

// for device=
//#define  AccessPCIBIOS()   DosDevIOCtl((&data),(&param),OEMHLP_PCI,IOCTL_OEMHLP,(handle))
//for basedev=
//extern "C" void CallOEMHLP(PREQPACKET prp);
//#pragma aux CallOEMHLP far parm loadds [es bx];
OEMHLP pOEMHLP;
extern "C" void CallOEMHLP(OEMHLP pFunctionCall, PREQPACKET prp)
{
      _asm {
         push es
         push bx
         push ds
         push di
         push si
         mov  bx, word ptr prp[0]
         mov  es, word ptr prp[2]
         ;            mov  ds, oemhlpDS
      }

      (*pFunctionCall)();

      _asm {
         pop si
         pop di
         pop ds
         pop bx
         pop es
      }
}
//extern "C" {
// void CallOEMHLP(PREQPACKET prp);
//}
APIRET PCIDEV::AccessPCIBIOS()
{
	// Setup IOCTL Request Packet
	rpOEMHLP.s.ioctl.bCategory     = 0x00;
    rpOEMHLP.s.ioctl.bCode     = 0x0b;//PCI_FUNC;//0x0b
    rpOEMHLP.s.ioctl.pvParm= (PUCHAR)&param;
    rpOEMHLP.s.ioctl.pvData= (PUCHAR)&data;
    rpOEMHLP.bLength=sizeof(rpOEMHLP);
    rpOEMHLP.bUnit=0;
    rpOEMHLP.bCommand=0x10;      // Generic IOCtl 0x10
    rpOEMHLP.usStatus=0;

    CallOEMHLP(pOEMHLP,prpOEMHLP);
    return 0;
}
PCIDEV::PCIDEV(void)
{
    ucHardwMechanism=0;
    ucMajorVer=0;
    ucMinorVer=0;
    ucLastBus=0;
    ucBus=0;
    ucDevFunc=0;
//    handle=0;
    prpOEMHLP=&rpOEMHLP;

    OpenOEMHlp();
    queryBIOS();
}
PCIDEV::~PCIDEV(void)
{
    CloseOEMHlp();
}
APIRET PCIDEV::queryBIOS(void)
{
    param.subfunc=OEMHLP_QUERY_PCI_BIOS;
    if (AccessPCIBIOS())
    {  // cannot access PCI BIOS
        return -1;
    }
    ucHardwMechanism=data.info.mech;
    ucMajorVer=data.info.majorV;
    ucMinorVer=data.info.minorV;
    ucLastBus=data.info.bus;
    return 0;
}
APIRET PCIDEV::findDevice(USHORT usDeviceID, USHORT usVendorID, UCHAR ucIndex)
{
    param.subfunc=OEMHLP_FIND_PCI_DEVICE;
    param.id.vendor=usVendorID;
    param.id.device=usDeviceID;
    param.index=ucIndex;
    if (AccessPCIBIOS())
    {  // cannot access PCI BIOS
        return -1;
    }
    if (data.rc)
        return data.rc;

    ucBus=data.loc.bus;
    ucDevFunc=data.loc.devfunc;
    return 0;
}
APIRET PCIDEV::findDeviceClass(ULONG ulClassCode, UCHAR ucIndex)
{
    APIRET rc;
    param.subfunc=OEMHLP_FIND_PCI_CLASS;
    param.classcode=ulClassCode;
    param.index=ucIndex;
    rc=AccessPCIBIOS();
    if (rc)
    {  // cannot access PCI BIOS
//        return rc;
    }
    if (data.rc)
        return data.rc;

    ucBus=data.loc.bus;
    ucDevFunc=data.loc.devfunc;
    return 0;
}
UCHAR PCIDEV::readRegByte(UCHAR ucRegister)
{
    param.subfunc=OEMHLP_READ_PCI_CONFIG;
    param.conf.bus=ucBus;
    param.conf.devfunc=ucDevFunc;
    param.conf.reg=ucRegister;
    param.conf.size=1;
    if (AccessPCIBIOS())
    {  // cannot access PCI BIOS
        return -1;
    }
    ddprintf("DEBUG: readPCIRegByte %x = %x\n",ucRegister, data.data);
    return (UCHAR)data.data;
}
USHORT PCIDEV::readRegWord(UCHAR ucRegister)
{
    param.subfunc=OEMHLP_READ_PCI_CONFIG;
    param.conf.bus=ucBus;
    param.conf.devfunc=ucDevFunc;
    param.conf.reg=ucRegister;
    param.conf.size=2;
    if (AccessPCIBIOS())
    {  // cannot access PCI BIOS
        return -1;
    }
    ddprintf("DEBUG: readPCIRegWord %x = %x\n",ucRegister, data.data);
    return (USHORT)data.data;
}
ULONG PCIDEV::readRegDWord(UCHAR ucRegister)
{
    param.subfunc=OEMHLP_READ_PCI_CONFIG;
    param.conf.bus=ucBus;
    param.conf.devfunc=ucDevFunc;
    param.conf.reg=ucRegister;
    param.conf.size=4;
    if (AccessPCIBIOS())
    {  // cannot access PCI BIOS
        return -1;
    }
    ddprintf("DEBUG: readPCIRegDWord %x = %0lx\n",ucRegister, data.data);
    return data.data;
}
APIRET PCIDEV::writeRegByte(UCHAR ucRegister, UCHAR ucValue)
{
    param.subfunc=OEMHLP_WRITE_PCI_CONFIG;
    param.conf.bus=ucBus;
    param.conf.devfunc=ucDevFunc;
    param.conf.reg=ucRegister;
    param.conf.size=1;
    param.data=(ULONG)ucValue;
    if (AccessPCIBIOS())
    {  // cannot access PCI BIOS
        return -1;
    }
    return 0;
}
APIRET PCIDEV::writeRegWord(UCHAR ucRegister, USHORT usValue)
{
    param.subfunc=OEMHLP_WRITE_PCI_CONFIG;
    param.conf.bus=ucBus;
    param.conf.devfunc=ucDevFunc;
    param.conf.reg=ucRegister;
    param.conf.size=2;
    param.data=(ULONG)usValue;
    if (AccessPCIBIOS())
    {  // cannot access PCI BIOS
        return -1;
    }
    return 0;
}
APIRET PCIDEV::writeRegDWord(UCHAR ucRegister, ULONG ulValue)
{
    param.subfunc=OEMHLP_WRITE_PCI_CONFIG;
    param.conf.bus=ucBus;
    param.conf.devfunc=ucDevFunc;
    param.conf.reg=ucRegister;
    param.conf.size=4;
    param.data=ulValue;
    if (AccessPCIBIOS())
    {  // cannot access PCI BIOS
        return -1;
    }
    return 0;
}
USHORT PCIDEV::usGetDeviceID(void)
{
    return readRegWord(PCIR_DEVICE);
}
USHORT PCIDEV::usGetVendorID(void)
{
    return readRegWord(PCIR_VENDOR);
}
//get/set Command and Status Register
USHORT PCIDEV::usGetCommand(void)
{
    return readRegWord(PCIR_COMMAND);
}
APIRET PCIDEV::setCommand(USHORT ulCommandValue)
{
    return writeRegWord(PCIR_COMMAND,ulCommandValue);
}
USHORT PCIDEV::usGetStatus(void)
{
    return readRegWord(PCIR_STATUS);
}
ULONG PCIDEV::usGetClassCode(void)
{
    return readRegDWord(PCIR_REVID)>>8;
}
UCHAR PCIDEV::ucGetRevisionID(void)
{
    return readRegByte(PCIR_REVID);
}
UCHAR PCIDEV::ucGetBIST(void)
{
    return readRegByte(PCIR_BIST);
}
UCHAR PCIDEV::ucGetHeaderType(void)
{
    return readRegByte(PCIR_HEADERTYPE);
}
UCHAR PCIDEV::ucGetLatency(void)
{
    return readRegByte(PCIR_LATTIMER);
}
APIRET PCIDEV::setLatency(UCHAR ucLatency)
{
    return writeRegByte(PCIR_LATTIMER,ucLatency);
}
UCHAR PCIDEV::ucGetCache(void)
{
    return readRegByte(PCIR_CACHELNSZ);
}
ULONG PCIDEV::ulGetBaseAddr(void)
{
    return readRegDWord(PCIR_MAPS)&0xfffffff0;;
}
USHORT PCIDEV::usGetSubsystemID(void)
{
    return readRegWord(PCIR_SUBDEV_0);
}
USHORT PCIDEV::usGetSubsystemVendorID(void)
{
    return readRegWord(PCIR_SUBVEND_0);
}
ULONG PCIDEV::ulGetExpROMAddr(void)
{
    return readRegDWord(PCIR_BIOS);
}
UCHAR PCIDEV::ucGetCapPtr(void)
{
    return readRegByte(PCIR_CAP_PTR);
}
UCHAR PCIDEV::ucGetMaxLatency(void)
{
    return readRegByte(PCIR_MAXLAT);
}
UCHAR PCIDEV::ucGetMinGrant(void)
{
	return readRegByte(PCIR_MINGNT);
}
UCHAR PCIDEV::ucGetIntrPin(void)
{
    return readRegByte(PCIR_INTPIN);
}
UCHAR PCIDEV::ucGetIntrLine(void)
{
    return readRegByte(PCIR_INTLINE);
}

APIRET PCIDEV::OpenOEMHlp(void)
{
//    USHORT action;
    // for device=
//    return DosOpen((PUCHAR)"OEMHLP$",&handle,&action,0L,0,1,0x40,0L);
	//for basedev
	if (!DevHelp_AttachDD((NPSZ) "OEMHLP$ ", (NPBYTE) &DDTable))
	{
		if (OFFSETOF(DDTable.pfn))
		{
			pOEMHLP=(OEMHLP)DDTable.pfn;
			return 0;
		}
	}
	return 1;
}
APIRET PCIDEV::CloseOEMHlp(void)
{
	//for device=
//    return DosClose(handle);
	//for basedev
	return 0;
}

