/*
** Module   :PCIDEV.HPP
** Abstract :PCI device support class
**
** Copyright (C) Alexandr Cherkaev
**
** Log: Mon  03/05/2004 Created
**
*/
#ifndef __PCIDEV_HPP
#define __PCIDEV_HPP
#include "strategy.h"

/*
         OEMHLP PCI Interface Subfunctions
*/
#define  OEMHLP_QUERY_PCI_BIOS      0x00
#define  OEMHLP_FIND_PCI_DEVICE     0x01
#define  OEMHLP_FIND_PCI_CLASS      0x02
#define  OEMHLP_READ_PCI_CONFIG     0x03
#define  OEMHLP_WRITE_PCI_CONFIG    0x04

#define  PCI_BIOS_VERSION     2

struct   OEMHLPCIParam     // OEMHLP PCI Parameter packet
{                          // ===========================
   UCHAR subfunc;          // subfunction number
   union
   {
      struct
      {
         USHORT device;
         USHORT vendor;
                           // subfunction = OEMHLP_FIND_PCI_DEVICE
      } id;                //  identification
      struct
      {
         UCHAR bus;        //    bus number
         UCHAR devfunc;    //    device (upper 5 bits) and function (lower 3 bits)
         UCHAR reg;        //    configuration register
         UCHAR size;       //    size in bytes (1, 2, 4)
                           // subfunction = OEMHLP_READ_PCI_CONFIG, OEMHLP_WRITE_PCI_CONFIG
      } conf;              //  configuration space
                           // subfunction = OEMHLP_FIND_PCI_CLASS
      ULONG classcode;         //  class code (base Class, Sub-class, Interface)
   };                      //  in lower 3 bytes (0x00CCSSII)
   union
   {
      UCHAR index;         // zero-based index to find identical devices
      ULONG data;          // data to be written to configuration register
   };
};
struct   OEMHLPCIData   // OEMHLP PCI Data packet
{                       // ======================
   UCHAR rc;            // return code
   union
   {
      struct
      {
         UCHAR mech;    //   hardware mechanism
         UCHAR majorV;
         UCHAR minorV;
         UCHAR bus;     //   number of last bus
                        // subfunction = OEMHLP_QUERY_PCI_BIOS
      } info;           //  information specific to the installed PCI BIOS
      struct
      {
         UCHAR bus;     //   bus number
         UCHAR devfunc; //   device (upper 5 bits) and function (lower 3 bits)
                        // subfunction = OEMHLP_FIND_PCI_DEVICE, OEMHLP_FIND_PCI_CLASS
      } loc;            //  location of device
                        // subfunction = OEMHLP_READ_PCI_CONFIG
      ULONG data;       //  data read from configuration space
   };
};
#ifdef __cplusplus
extern "C" {
#endif


//typedef void (*OEMHLP) (REQPACKET _far *psz );
//typedef void far (__far *PFNOEMHLP) (REQPACKET _far *psz );
typedef void (__far *OEMHLP) (void);
//void (const far *const OEMHLP)(void) ;


//void __far _oemhlp(void);

//extern OEMHLP oemhlp;
typedef struct {
   unsigned short ausReserved[3];      // 3 reserved words

   OEMHLP pfn;                         // far pointer to IDC entry
   unsigned short DS;                  // data segment of IDC
} OEMHLP_ATTACH_DD;

//#pragma aux (STRATEGY) fnOEMHLP;

void __far _fnOEMHLP(void);

extern OEMHLP fnOEMHLP;
#pragma aux fnOEMHLP far parm [es bx];
#ifdef __cplusplus
}
#endif

class PCIDEV
{
public:
    PCIDEV(void);
    ~PCIDEV(void);
    APIRET findDevice(USHORT usDeviceID, USHORT usVendorID, UCHAR ucIndex);
    APIRET findDeviceClass(ULONG ulClassCode, UCHAR ucIndex);
    UCHAR  readRegByte(UCHAR ucRegister);
    USHORT readRegWord(UCHAR ucRegister);
    ULONG  readRegDWord(UCHAR ucRegister);
    APIRET writeRegByte(UCHAR ucRegister, UCHAR ucValue);
    APIRET writeRegWord(UCHAR ucRegister, USHORT usValue);
    APIRET writeRegDWord(UCHAR ucRegister, ULONG ulValue);
    USHORT usGetDeviceID(void);
    USHORT usGetVendorID(void);
    //get/set Command and Status Register
    USHORT usGetCommand(void);
    APIRET setCommand(USHORT ulCommandValue);
    USHORT usGetStatus(void);
    ULONG  usGetClassCode(void);
    UCHAR  ucGetRevisionID(void);
    UCHAR  ucGetBIST(void);
    UCHAR  ucGetHeaderType(void);
    UCHAR  ucGetLatency(void);
    APIRET setLatency(UCHAR ucLatency);
    UCHAR  ucGetCache(void);
    ULONG  ulGetBaseAddr(void);
    USHORT usGetSubsystemID(void);
    USHORT usGetSubsystemVendorID(void);
    ULONG  ulGetExpROMAddr(void);
    UCHAR  ucGetCapPtr(void);
    UCHAR  ucGetMaxLatency(void);
    UCHAR  ucGetMinGrant(void);
    UCHAR  ucGetIntrPin(void);
    UCHAR  ucGetIntrLine(void);

    // PCI BIOS info, set by queryBIOS()
    UCHAR ucHardwMechanism, ucMajorVer, ucMinorVer, ucLastBus;
    // device info, set by find fincs
    UCHAR ucBus, ucDevFunc;
private:
//    HFILE handle; //for device=
    OEMHLP_ATTACH_DD DDTable; //for basedev
    REQPACKET rpOEMHLP;
    PREQPACKET prpOEMHLP;

    OEMHLPCIParam param;
    OEMHLPCIData data;
    APIRET AccessPCIBIOS();
    APIRET queryBIOS(void);
    APIRET OpenOEMHlp(void);
    APIRET CloseOEMHlp(void);


};
//extern "C" OEMHLP pfnOEMHLPFunc;




#endif  /*__PCIDEV_HPP*/

