/* $Id: ioctl.h,v 1.1 2004/06/25 16:04:44 doctor64 Exp $ */

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

#define I1394_IOCTL_CATEGORY           0x80
#define I1394_IOCTL_BASE               0x40

#pragma pack()

#endif
