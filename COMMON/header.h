/*
** Module   :HEADER.H
** Abstract :device driver header
**
** Copyright (C) Alexandr Cherkaev
** original code by Timur Tabi
**
** Log: Fri  30/04/2004	Created
**
*/
#ifndef __HEADER_H
#define __HEADER_H



#pragma pack(1);

#define DA_CHAR         0x8000   // Character PDD
#define DA_IDCSET       0x4000   // IDC entry point set
#define DA_BLOCK        0x2000   // Block device driver
#define DA_SHARED       0x1000   // Shared device
#define DA_NEEDOPEN     0x800    // Open/Close required

#define DA_OS2DRVR      0x0080   // Standard OS/2 driver
#define DA_IOCTL2       0x0100   // Supports IOCTL2
#define DA_USESCAP      0x0180   // Uses Capabilities bits

#define DA_CLOCK        8        // Clock device
#define DA_NULL         4        // NULL device
#define DA_STDOUT       2        // STDOUT device
#define DA_STDIN        1        // STDIN device

#define DC_INITCPLT     0x10     // Supports Init Complete
#define DC_ADD          8        // ADD driver
#define DC_PARALLEL     4        // Supports parallel ports
#define DC_32BIT        2        // Supports 32-bit addressing
#define DC_IOCTL2       1        // Supports DosDevIOCtl2 and Shutdown (1C)

typedef void (__near *PFNENTRY) (void);

typedef struct _DEV_HEADER {
   struct _DEV_HEADER __far *pNextDD;
   unsigned short usAttribs;
   PFNENTRY pfnStrategy;
   PFNENTRY pfnIDC;
   char abName[8];
   unsigned long ulReserved[2];
   unsigned long ulCaps;
} DEV_HEADER, __far *PDEV_HEADER;

#pragma pack();

// pseudo-variable that points to device header
#define phdr ((DEV_HEADER *) 0)

// pseudo-variable that points to end of headers
#define phdrEnd ( (PDEV_HEADER) (ULONG) (-1) )



#endif  /*__HEADER_H*/

