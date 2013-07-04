/* $Id: malloc.h,v 1.1 2004/06/21 16:29:57 doctor64 Exp $ */

/* MALLOC.H

   MODIFICATION HISTORY
   DATE       PROGRAMMER   COMMENT
   01-Jul-95  Timur Tabi   Creation
   01-Apr-96  Timur Tabi   Fixed realloc and _msize prototypes
*/

#ifndef MALLOC_INCLUDED
#define MALLOC_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

// Standard malloc.h functions

void __near *malloc(unsigned);
void free(void __near *);
void __near *realloc(void __near *, unsigned);
unsigned _msize(void __near *);
int validate(void __near *);
int quick_validate(void __near *);

// Some extensions
unsigned _memfree(void);            // returns available space

// Specialized routines
void HeapInit(void);        // initializes the heap manager
void dumpheap(void);


#ifdef __cplusplus
}
#endif

#endif
