/*
** Module   :INIT.CPP
** Abstract :INIT routines
**
** Copyright (C) Alexandr Cherkaev
** original code by Timur Tabi
**
** Log: Fri  30/04/2004 Created
**
*/


#pragma code_seg ("_inittext");
#pragma data_seg ("_initdata","endds");

extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define INCL_TYPES
#define  INCL_DOSDEVICES
#define  INCL_DOSDEVIOCTL
#define  INCL_NOXLATE_DOS16
#include <os2.h>
}
#include "firetype.h"
#include "ddprintf.h"

#include <ctype.h>
#include <string.h>

#include <include.h>
#include <devhelp.h>

#include "strategy.h"
#include "header.h"
#include "end.h"
#include "version.h"
#include "rmhelp.hpp"
#include "malloc.h"
//#include "ohci.hpp"
//#include "host.hpp"
//#include "stack.hpp"
#include "global.hpp"
#include "timer.hpp"
#include "stack.hpp"
#include "entry.hpp"
//#include "nodemgr.hpp"
// The device header name specified on the command line
char szCL_DevName[8] = {' ',' ',' ',' ',' ',' ',' ',' '};

// True if the /V parameter was specified
//extern "C" int fVerbose;

// True if /O:INT3 was specified
int fInt3 = FALSE;

//char szMCA[] = "MicroChannel machine.\n";
//char szPrintf[] = "PRINTF.SYS found.\n";
char szBadParam[] = "Bad Parameter.\n";
char szRMDriver[] = "RM: Can't allocate driver.\n";
char szRMAdapter[] = "RM: Can't allocate adapter.\n";
char szRMDevice[] = "RM: Can't allocate device.\n";
char szRMAllocate[] = "RM: Can't allocate resource.\n";

extern USHORT usYear;
extern USHORT usMonth;
extern USHORT usDay;


USHORT sz2us(char __far *sz, int base)
{
   static char digits[] = "0123456789ABCDEF";

   USHORT us=0;
   char *pc;

// skip leading spaces
   while (*sz == ' ') sz++;

// skip leading zeros
   while (*sz == '0') sz++;

// accumulate digits - return error if unexpected character encountered
   for (;;sz++) {
      pc = (char *) memchr(digits, toupper(*sz), base);
      if (!pc)
         return us;
      us = (us * base) + (pc - digits);
   }
}

int IsWhitespace(char ch)
{
   if ( ch > '9' && ch < 'A')
      return TRUE;
   if ( ch < '0' || ch > 'Z')
      return TRUE;

   return FALSE;
}

char __far *SkipWhite(char __far *psz)
{
   while (*psz) {
      if (!IsWhitespace((char) toupper(*psz))) return psz;
      psz++;
   }
   return NULL;
}

int CopyDevicename(char __far *psz)
{
   int i,j;
   char ch;

// first, check if the filename is valid
   for (i=0; i<9; i++) {
      ch=(char) toupper(psz[i]);
      if (!ch || ch == ' ')
         break;
      if (i==8)                           // too long?
         return FALSE;
      if ( ch > '9' && ch < 'A')
         return FALSE;
      if ( (ch != '$' && ch < '0') || ch > 'Z')
         return FALSE;
   }
   if (!i) return FALSE;                        // zero-length name?

   for (j=0; j<i; j++)
      szCL_DevName[j]=(char) toupper(psz[j]);

   for (; j<8; j++)
      szCL_DevName[j]=' ';

   return TRUE;
}

#define OPTION(sz)   (!_fstrnicmp(pszOption, sz, sizeof(sz)-1))

int DoOption(char __far *pszOption)
{
   if OPTION("INT3") {
      fInt3 = TRUE;
      return TRUE;
   }
    if OPTION("COM1")
    {
        setddprintfport(1);
        return TRUE;
    }
    if OPTION("COM2")
    {
        setddprintfport(2);
        return TRUE;
    }

   return FALSE;
}

int DoParm(char cParm, int iPort, char __far *pszOption)
{
//   USHORT us;

   switch (cParm) {
      case 'N':                     // device header name
         if (iPort) return FALSE;
         if (!pszOption) return FALSE;
         if (!CopyDevicename(pszOption)) return FALSE;
         break;
      case 'O':                     // miscellaneous options
         if (iPort) return FALSE;
         if (!pszOption) return FALSE;
         return DoOption(pszOption);
      case 'V':                     // Verbose option
         if (iPort) return FALSE;
         if (pszOption) return FALSE;
//         fVerbose=TRUE;
         break;
      default:
         return FALSE;              // unknown parameter
    }

   return TRUE;
}

/* Function: ParseParm
   Input: pointer to the letter of the parameter (e.g. the 'P' in 'P1:330').
          length of this parameter, which must be at least 1
   Output: TRUE if the parameter was value
   Purpose: parses the string into three parts: the letter parameter, the port
            number, and the option string.  Calls DoParm with these values.
   Notes:
      the following describes the format of valid parameters.
         1. Parameters consist of a letter, an optional number, and an
            optional 'option'.  The format is x[n][:option], where 'x' is the
            letter, 'n' is the number, and 'option' is the option.
         2. Blanks are delimeters between parameters, therefore there are no
            blanks within a parameter.
         3. The option is preceded by a colon
      This gives us only four possibilities:
         P (length == 1)
         P1 (length == 2)
         P:option (length >= 3)
         P1:option (length >= 4)
*/
int ParseParm(char __far *pszParm, int iLength)
{
   char ch,ch1=(char) toupper(*pszParm);       // get letter

   if (iLength == 1)                // only a letter?
      return DoParm(ch1,0,NULL);

   ch = pszParm[1];                 // should be either 1-9 or :
   if (ch < '1' || (ch > '9' && ch != ':'))
      return FALSE;

   if (iLength == 2) {
      if (ch == ':')
         return FALSE;
      return DoParm(ch1,ch - '0',NULL);
   }

   if (iLength == 3) {
      if (ch != ':')
         return FALSE;
      return DoParm(ch1,0,pszParm+2);
   }

   if (ch == ':')
      return DoParm(ch1,0,pszParm+2);

   return DoParm(ch1,ch - '0',pszParm+3);
}

int GetParms(char __far *pszCmdLine)
{
   int iLength;

   while (*pszCmdLine != ' ') {              // skip over filename
      if (!*pszCmdLine) return TRUE;         // no params?  then just exit
      pszCmdLine++;
   }

   while (TRUE) {
      pszCmdLine=SkipWhite(pszCmdLine);      // move to next param
      if (!pszCmdLine) return TRUE;          // exit if no more

      for (iLength=0; pszCmdLine[iLength]; iLength++)    // calculate length
         if (pszCmdLine[iLength] == ' ') break;          //  of parameter

      if (!ParseParm(pszCmdLine,iLength))    // found parameter, so parse it
         return FALSE;

      while (*pszCmdLine != ' ') {              // skip over parameter
         if (!*pszCmdLine) return TRUE;         // no params?  then just exit
         pszCmdLine++;
      }
   }
}
// For resource management

//DRIVERSTRUCT driverstruct =
//{
//   (PSZ) "1394tst.SYS",                            /* DrvrName                */
//   (PSZ) "Firewire test driver",                   /* DrvrDescript            */
//   (PSZ) "Netlabs",                                /* VendorName              */
//   CMVERSION_MAJOR,                                /* MajorVer                */
//   CMVERSION_MINOR,                                /* MinorVer                */
//   {2004, 04, 30},                                    /* Date                    */
//   DRF_STATIC,                                     /* DrvrFlags               */
//   DRT_OS2,                                        /* DrvrType                */
//   DRS_APP_HELPER,                                 /* DrvrSubType             */
//   NULL                                            /* DrvrCallback            */
//   (PSZ) "GENPDD.SYS",                             /* DrvrName                */
//   (PSZ) "Generic PDD",                            /* DrvrDescript            */
//   (PSZ) "Theta Band Software",                    /* VendorName              */
//   CMVERSION_MAJOR,                                /* MajorVer                */
//   CMVERSION_MINOR,                                /* MinorVer                */
//   1998, 8, 15,                                    /* Date                    */
//   DRF_STATIC,                                     /* DrvrFlags               */
//   DRT_OS2,                                        /* DrvrType                */
 //  DRS_APP_HELPER,                                 /* DrvrSubType             */
//   NULL                                            /* DrvrCallback            */

//};

//ADAPTERSTRUCT adapterstruct =
//{
//  (PSZ) "FireWire Test",                     /* AdaptDescriptName; */
//  AS_NO16MB_ADDRESS_LIMIT,             /* AdaptFlags;        */
//  AS_BASE_RESERVED,                    /* BaseType;          */
//  AS_SUB_OTHER,                        /* SubType;           */
//  AS_INTF_GENERIC,                     /* InterfaceType;     */
//  AS_HOSTBUS_UNKNOWN,                  /* HostBusType;       */
//  AS_BUSWIDTH_UNKNOWN,                 /* HostBusWidth;      */
//  NULL                                 /* pAdjunctList;      */
//};

//DEVICESTRUCT devicestruct =
//{
//  (PSZ) "FIREWIRE_1 Test PDD",        /* DevDescriptName   */
//  DS_FIXED_LOGICALNAME,                /* DevFlags;         */
//  DS_TYPE_UNKNOWN,                     /* DevType;          */
//  NULL                                 /* pAdjunctList;     */
//};

char szDate[] = __DATE__;
char szMonths[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

void far Stack_StrategyInit(PREQPACKET prp)
{
//   APIRET rc;
//   ULONG ultemp;
//
    int i;
//   static PRINTF_ATTACH_DD DDTable;
   PVOID       pDOSVar;


   Device_Help = prp->s.init_in.ulDevHlp;

   prp->s.init_out.usCodeEnd = 0;
   prp->s.init_out.usDataEnd = 0;
   prp->usStatus = RPDONE | RPERR | RPGENFAIL;

   setddprintfport(1);

//   if (!DevHelp_AttachDD((NPSZ) "PRINTF$ ", (NPBYTE) &DDTable))
//      if (OFFSETOF(DDTable.pfn)) {
//         printf = (PRINTF) DDTable.pfn;
//         fVerbose = TRUE;
//         iprintf(szPrintf);
//      }

   if (!GetParms(prp->s.init_in.szArgs)) {
      ddprintf(szBadParam);
      return;
   }

//      szDate[3] = 0;
//      szDate[6] = 0;
//      char *pc = strstr(szMonths, szDate);
//      usMonth = 1 + ((USHORT) (pc - szMonths)) / 3;
//      usDay = sz2us(&szDate[4], 10);
//    usYear = sz2us(&szDate[7], 10);
//    gInInterrupt=FALSE;
    ddprintf("FireWire Stack Driver %d.0%d \nCopyright (C) Alex Cherkaev\nCompiled %s\n", VERSION_MAJOR, VERSION_MINOR,(char far *) __DATE__);
    //\n);
    if (fInt3)
    int3();
    HeapInit();
//   ddprintf("DEBUG: heapSize %d\n",_memfree());

//   MonoInit();
//      if (szCL_DevName[0] != ' ')               // Was a valid device name specified?
//      memcpy(phdr->abName,szCL_DevName,8);   // yes, copy it to the dev header

    if (DevHelp_GetDOSVar( DHGETDOSV_SYSINFOSEG, 0, &pDOSVar ) )
        return;

    fpGINFOSEG=(InfoSegGDT __far *)MAKEP( *((PSEL)pDOSVar), 0 );
    ddprintf("DEBUG: pGINFOSEG %lx \n",fpGINFOSEG);

    //timer init
    for (i=0;i<10;i++) Timerlist[i]=NULL;
//    pOhciDriver=new OHCIDRIVER;
//    if (pOhciDriver==NULL) ddprintf("ERROR: pOHCIDRIVER is NULL\n");
//    pGlobalHost=new HOST(0);
//    if (pGlobalHost==NULL) ddprintf("ERROR: pGlobalHost is NULL\n");
    pGlobalStack=new FIRESTACK;
    if (pGlobalStack==NULL) ddprintf("ERROR: pFireStack is NULL\n");
    GlobalStackOps.fpfnRegisterHighlevel=sRegisterHighlevel;
    GlobalStackOps.fpfnRegisterAddrSpace=sRegisterAddrSpace;
    GlobalStackOps.fpfnAllocateAndRegisterAddrSpace=sAllocateAndRegisterAddrSpace;
    GlobalStackOps.fpfnAddHost=sAddHost;
    GlobalStackOps.fpfnUnRegisterHighlevel=sUnRegisterHighlevel;
    GlobalStackOps.fpfnUnRegisterAddrSpace=sUnRegisterAddrSpace;
    GlobalStackOps.fpfnRemoveHost=sRemoveHost;
    GlobalStackOps.fpfnHostReset=sHostReset;
    GlobalStackOps.fpfnRead=sRead;
    GlobalStackOps.fpfnWrite=sWrite;
    GlobalStackOps.fpfnLock=sLock;
    GlobalStackOps.fpfnLock64=sLock64;
    GlobalStackOps.fpfnIsoReceive=sIsoReceive;
    GlobalStackOps.fpfnFcpRequest=sFcpRequest;


    ddprintf("DEBUG: heapSize %d\n",_memfree());
    // make links
//    pGlobalHost->pDriver=pOhciDriver;
//    pOhciDriver->pHost=pGlobalHost;
//    pGlobalHost->pStack=pGlobalStack;
    //init
//    pOhciDriver->Probe();
    DevHelp_SetTimer((NPFN)TimerHandler);
//    PacketThreadSig.Init(0);
//    DevHelp_AllocateCtxHook((NPFN)PacketContextRoutine,&PacketHookHandle);
//    DevHelp_ArmCtxHook(0,PacketHookHandle);

    //test
//    ultemp=pGlobalHost->pDriver->ulReadRegister(0x1c);
//    ddprintf("1394 bus id: %lx \n",ultemp);


//   static char driver_mem[sizeof(DRIVER)];
//    DRIVER *pdrv;
//    pdrv = new DRIVER;
//    if (!pdrv->add(&driverstruct)) {
//       ddprintf(szRMDriver);
//       delete pdrv;
//       return;
//    }
//   static char adapter_mem[sizeof(ADAPTER)];
//   ADAPTER *padpt;
//   padpt = new ADAPTER(pdrv);
// here may setup real values of controller for RM

//   if (!padpt->add(&adapterstruct)) {
//      ddprintf(szRMAdapter);
//      delete padpt;
//      delete pdrv;
//      return;
//   }


 //  static char device_mem[sizeof(DEVICE)];
/*   DEVICE *pdev = new DEVICE(padpt);
   if (!pdev->add(&devicestruct)) {
      ddprintf(szRMDevice);
      delete pdev;
      delete padpt;
      delete pdrv;
      return;
   }
*/

   prp->usStatus = RPDONE;
   prp->s.init_out.usCodeEnd = (USHORT) end_of_text;
   prp->s.init_out.usDataEnd = (USHORT) &end_of_heap;
}

