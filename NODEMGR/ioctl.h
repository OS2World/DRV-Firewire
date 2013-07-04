/* $Id: ioctl.h,v 1.3 2004/08/03 13:10:43 doctor64 Exp $ */

/* IOCTL.H - Definitions for IOCtl interface to GENPDD.SYS
*/

#ifndef IOCTL_INCLUDED
#define IOCTL_INCLUDED

// Parameters defined as IN are to be filled in by the caller before
// making the call.  They will not be modified by the callee

#ifndef IN
#define IN
#endif

// Parameters defined as OUT will be filled in by the callee during the
// call

#ifndef OUT
#define OUT
#endif

// Parameters defined as INOUT are to be initialized by the caller before
// making the call, and they will be modified by the callee during the
// call

#ifndef INOUT
#define INOUT
#endif

#pragma pack(1)

struct BUS_OPT
{
    UCHAR  ucIrmc;       /* Iso Resource Manager Capable */
    UCHAR  ucCmc;        /* Cycle Master Capable */
    UCHAR  ucIsc;        /* Iso Capable */
    UCHAR  ucBmc;        /* Bus Master Capable */
    UCHAR  ucPmc;        /* Power Manager Capable (PNP spec) */
    UCHAR  ucCycClkAcc;    /* Cycle clock accuracy */
    UCHAR  ucMaxRom;    /* Maximum block read supported in the CSR */
    UCHAR  ucGeneration; /* Incremented when configrom changes */
    UCHAR  ucLinkSpeed;     /* Link speed */
    USHORT usMaxRec;    /* Maximum packet size node can receive */
};

#define NODEMGR_IOCTL_CATEGORY            0x80
#define NODEMGR_IOCTL_BASE                0x40

#define NODEMGR_IOCTL_GET_VERSION       (NODEMGR_IOCTL_BASE)
#define NODEMGR_IOCTL_GET_HOSTNUM       (NODEMGR_IOCTL_BASE + 1)
#define NODEMGR_IOCTL_GET_HOST          (NODEMGR_IOCTL_BASE + 2)
#define NODEMGR_IOCTL_GET_NODENUM       (NODEMGR_IOCTL_BASE + 3)
#define NODEMGR_IOCTL_GET_NODE          (NODEMGR_IOCTL_BASE + 4)
#define NODEMGR_IOCTL_GET_UNITDIRNUM    (NODEMGR_IOCTL_BASE + 5)
#define NODEMGR_IOCTL_GET_UNITDIR       (NODEMGR_IOCTL_BASE + 6)

#define NODEMGR_NOERROR                 0

/* NODEMGR_IOCTL_GET_VERSION
   Parameter: Pointer to GENPDD_GET_VERSION structure
*/

#define NODEMGR_BUILD_ALPHA       10
#define NODEMGR_BUILD_BETA        80
#define NODEMGR_BUILD_GAMMA       99
#define NODEMGR_BUILD_RELEASE     100

typedef struct _NODEMGR_GET_VERSION
{
   OUT USHORT usMajorVersion;
   OUT USHORT usMinorVersion;
   OUT USHORT usBuildLevel;
   OUT USHORT usYear;
   OUT UCHAR  bMonth, bDay;
} NODEMGR_GET_VERSION;

/* NODEMGR_IOCTL_GET_HOSTNUM
   Parameter: Pointer to GENPDD_GET_HOSTNUM structure
*/

typedef struct _NODEMGR_GET_HOSTNUM
{
   OUT UCHAR  ucHostNum;
} NODEMGR_GET_HOSTNUM;

/* NODEMGR_IOCTL_GET_HOST
   Parameter: Pointer to GENPDD_GET_HOST structure
*/

typedef struct _NODEMGR_GET_HOST
{
   IN UCHAR   ucHostNum;
   OUT USHORT usNodeCount; // number of identified nodes on bus
   OUT USHORT usSelfIdCount; //number of SelfID received
   OUT USHORT usNodesActive; //number of actually active nodes

   OUT USHORT usNodeId; // node id of this host
   OUT USHORT usIRMId; // node id of this bus isocronous resource manager
   OUT USHORT usBusMgrId; //node id of this bus bus manager

    /* this nodes state */
   OUT USHORT bInBusReset;
   OUT USHORT bIsShutdown;

   /* this nodes' duties on the bus */
   OUT USHORT bIsRoot;
   OUT USHORT bIsCycmst;
   OUT USHORT bIsIrm;
   OUT USHORT bIsBusmgr;

} NODEMGR_GET_HOST;

/* NODEMGR_IOCTL_GET_NODENUM
   Parameter: Pointer to GENPDD_GET_NODENUM structure
*/

typedef struct _NODEMGR_GET_NODENUM
{
   IN UCHAR  ucHostNum;

   OUT UCHAR ucNodeNum;
} NODEMGR_GET_NODENUM;

/* NODEMGR_IOCTL_GET_NODE
   Parameter: Pointer to GENPDD_GET_NODE structure
*/

typedef struct _NODEMGR_GET_NODE
{
   IN UCHAR   ucHostNum;
   IN UCHAR   ucNodeNum;

   OUT ULONG  ulGUIDHi; // hi part of guid
   OUT ULONG  ulGUIDLo; // lo part of guid
   OUT ULONG  ulGuidVendorId;     /* Top 24bits of guid */
   OUT USHORT usNodeId;
   OUT struct BUS_OPT BusOptions;
   OUT USHORT bInLimbo;
   OUT USHORT bNeedProbe;
   OUT USHORT usGeneration;
   OUT ULONG  ulVendorId;
   OUT char   strVendorName[60];
   OUT ULONG  ulCapabilities;
   OUT UCHAR  ucUnitDirNum;

} NODEMGR_GET_NODE;

/* NODEMGR_IOCTL_GET_NODENUM
   Parameter: Pointer to GENPDD_GET_NODENUM structure
*/

typedef struct _NODEMGR_GET_UNITDIRNUM
{
   IN UCHAR  ucHostNum;
   IN UCHAR  ucNodeNum;

   OUT UCHAR ucUnitDirNum;
} NODEMGR_GET_UNITDIRNUM;

/* NODEMGR_IOCTL_GET_UNITDIR
   Parameter: Pointer to GENPDD_GET_UNITDIR structure
*/

typedef struct _NODEMGR_GET_UNITDIR
{
   IN UCHAR   ucHostNum;
   IN UCHAR   ucNodeNum;
   IN UCHAR   ucUnitDirNum;

   OUT ULONG  ulAddrHi; //address hi
   OUT ULONG  ulAddrLo; //address lo
   OUT UCHAR  ucFlags;
   OUT ULONG  ulVendorId;
   OUT char   strVendorName[60];
   OUT ULONG  ulModelId;
   OUT char   strModelName[60];
   OUT ULONG  ulSpecifierId;
   OUT ULONG  ulVersion;
   OUT USHORT usId;
} NODEMGR_GET_UNITDIR;

#pragma pack()

#endif
