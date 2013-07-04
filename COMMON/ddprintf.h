/*DDK*************************************************************************/
/*                                                                           */
/* COPYRIGHT    Copyright (C) 1992 IBM Corporation                           */
/*                                                                           */
/*    The following IBM OS/2 source code is provided to you solely for       */
/*    the purpose of assisting you in your development of OS/2 device        */
/*    drivers. You may use this code in accordance with the IBM License      */
/*    Agreement provided in the IBM Developer Connection Device Driver       */
/*    Source Kit for OS/2. This Copyright statement may not be removed.      */
/*                                                                           */
/*****************************************************************************/
/******************************************************************************
* ddprintf.h - Prototypes for functions and data in .C file of same name
*
*
* The following IBM OS/2 source code is provided to you solely for the
* the purpose of assisting you in your development of OS/2 device drivers.
* You may use this code in accordance with the IBM License Agreement
* provided in the IBM Device Driver Source Kit for OS/2.
******************************************************************************/
#ifndef DDPRINTF_INCLUDED
#define DDPRINTF_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif
    // quick little check to see if we are in ring3 or ring0
    // will return 0 if we are at ring0 and 3 if we are at ring3
   USHORT CheckForRing3(void);
   #pragma aux CheckForRing3 = \
      "push   cs"     \
      "pop    ax "    \
      "and    ax,0003H" \
      parm nomemory  \
      modify [ax];
#ifdef DEBUG
	#define ddprintf dprintf
    void cdecl ddprintf (char far *St , ...);
    void setddprintfport(char port);
#else
	#define ddprintf(...)
	void setddprintfport();
#endif
    void ddputstring (char far *St);

#ifdef __cplusplus
}
#endif

#endif
