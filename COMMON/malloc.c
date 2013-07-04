/* $Id: malloc.c,v 1.1 2004/06/21 16:29:57 doctor64 Exp $ */

/* MALLOC.C - simple heap for PDD's

   MODIFICATION HISTORY
   VERSION    PROGRAMMER   COMMENT
   13-Apr-98  Timur Tabi   Creation
*/

#include <string.h>

#include <include.h>
#define INCL_NOPMAPI
#define INCL_TYPES
#define  INCL_NOXLATE_DOS16
#include <os2.h>

#include "end.h"

#include "ddprintf.h"

// If SIGNATURE is defined, then each MEMBLOCK will include a signature
// This is useful for debugging.  It verifies that a free() request points to
//  a valid block.  The signature is a 32-bit value that should be somewhat
//  unique and identifiable
// #define  SIGNATURE      0x12345678

typedef struct _MEMBLOCK {
   unsigned uSize;
   struct _MEMBLOCK *pmbNext;
#ifdef SIGNATURE
   unsigned long ulSignature;
#endif
   char achBlock[1];
} MEMBLOCK, __near *PMEMBLOCK;

#define HDR_SIZE ( sizeof(MEMBLOCK) - 1 )

PMEMBLOCK pmbUsed=NULL, pmbFree=(PMEMBLOCK) abHeap;
extern unsigned uMemFree;

#pragma data_seg ("_initdata","endds");

void dumpheap(void)
{
   int i;
   PMEMBLOCK pmb;
   unsigned u=0;

   pmb=pmbUsed;
   ddprintf("GENPDD: Heap Dump - Used blocks\n");
   for (i=0; i<10; i++) {
      ddprintf("  pmb=%p, length=%ui\n",(void __far *) pmb, pmb->uSize);
      u+=pmb->uSize;
      pmb=pmb->pmbNext;
      if (!pmb) break;
   }
   ddprintf("  Total used = %ui\n",u);

   u=0;
   pmb=pmbFree;
   ddprintf("GENPDD: Heap Dump - Free blocks\n");
   for (i=0; i<10; i++) {
      ddprintf("  pmb=%p, length=%ui\n",(void __far *) pmb, pmb->uSize);
      u+=pmb->uSize;
      pmb=pmb->pmbNext;
      if (!pmb) break;
   }
   ddprintf("  Total free = %ui\n",u);
}

/* NOTE: if the size of the block to allocate is equal to the size of the
   free block, we should remove the free block from the list, rather than
   just set its size to zero
*/
PMEMBLOCK make_new_free(PMEMBLOCK pmbOldFree, unsigned uSize) {
   PMEMBLOCK pmbNewFree;

   pmbNewFree = (PMEMBLOCK) ((char __near *) pmbOldFree + uSize + HDR_SIZE);
   pmbNewFree->uSize = pmbOldFree->uSize - uSize - HDR_SIZE;
   pmbNewFree->pmbNext = pmbOldFree->pmbNext;

   return pmbNewFree;
}

unsigned _msize(void __near *pvBlock)
{
   PMEMBLOCK pmb;

   if (!pvBlock)
      return 0;

   pmb = (PMEMBLOCK) ((char __near *) pvBlock - HDR_SIZE);

   return pmb->uSize;
}

/* malloc()
This function searches the list of free blocks until it finds one big enough
to hold the amount of memory requested, which is passed in uSize.  In order
for a free block to be "big enough", it not only has to hold the requested
memory, but it must also be able to hold another free block.  This is because
the remainder of the free block that is not converted to a used block
must be converted into another free block.  In other words:

    _______________________free block_______________________________________
   |  free    |                                                             |
   |  block   |       space available                                       |
   |  header  |          (pmbFree->uSize bytes long)                        |
   |(HDR_SIZE |                                                             |
   |__bytes)__|_____________________________________________________________|

   <--- h ---> <------------------------- l -------------------------------->

Must become:

    _______________________used block_______________________________________
   |  used    |                                     |  free    |            |
   |  block   |  space allocated                    |  block   | space      |
   |  header  |     (pmbUsed->uSize bytes long)     |  header  | available  |
   |(HDR_SIZE |                                     |(HDR_SIZE |            |
   |__bytes)__|_____________________________________|__bytes)__|____________|

   <--- h ---> <-------------- n ------------------><--- h ---> <--- m ----->

To keep the programming simple, m must be at least 4 bytes long.  This
assures that everything is aligned (because HDR_SIZE must be a multiple of
4 bytes), but it is inefficient.  A later version should correct this.

This means that in order for a free block to be chosen, this equation must
hold: l >= n + h + 4.  The local variable uFitSize is equal to n + h + 4.
*/

void __near *malloc(unsigned uSize)
{
   PMEMBLOCK pmb,pmbPrev;
   unsigned uFitSize;

   if (!uSize)
      return 0;

   uSize = (uSize + 3) & -4;
   uFitSize = uSize + HDR_SIZE + 4;

   if (pmbFree->uSize >= uFitSize) {         // is the first free block big enough?
      pmb=pmbFree;                           // this is where the new block goes
      pmbFree=make_new_free(pmbFree,uSize);  // update the free list

      pmb->pmbNext = pmbUsed;
      pmb->uSize = uSize;
#ifdef SIGNATURE
      pmb->ulSignature = SIGNATURE;
#endif
      pmbUsed = pmb;

      uMemFree -= uSize;
      return (void __near *) pmb->achBlock;
   }

   pmbPrev=pmbFree;

   for (pmb=pmbFree->pmbNext; pmb; pmbPrev=pmb, pmb=pmb->pmbNext)
      if (pmb->uSize >= uFitSize) {
         pmbPrev->pmbNext=make_new_free(pmb,uSize);

         pmb->pmbNext = pmbUsed;
         pmb->uSize = uSize;
#ifdef SIGNATURE
         pmb->ulSignature = SIGNATURE;
#endif
         pmbUsed = pmb;

         uMemFree -= uSize;
         return (void __near *) pmb->achBlock;
      }

   return 0;
}

/* void compact(void)
This function compacts the free blocks together.  This function is a
companion to free(), and thus the algorithm is tailored to how free()
works.  Change the algorithm in free(), and you'll have to change this
function too.

When free() frees a block, it sets the head of the free list, pmbFree, to
point to it.  Then the newly freed block points to whatever the old pmbFree
pointed to.  In other words, each new free block is added to the head of
the free list.

If compact() is always called after a block is freed, then it can be
guaranteed that the free list is always compacted (i.e. you won't find
two adjacent free blocks anywhere in the heap) _before_ each call to free().
Therefore, the newly freed block can be located in only one of the
following positions:
1. Not adjacent to any other free blocks (compacting not necessary)
2. Physically before another free block
3. Physically after another free block
4. Between two free blocks (i.e. the block occupies the entire space
   between two free blocks)

Since the newly freed block is the first in the free list, compact()
starts with the second block in the list (i.e. pmbFree->pmbNext).
Each free block is then compared with the newly freed block for
adjacency.  If a given block is located before the new block, then it
can't possibly be also located after, and vice versa.  Hence, the
"else if" statement in the middle of the loop.

Also, the newly freed block can only be adjacent to at most two
other blocks.  Therefore, the operation of combining two adjacent
free blocks can only happen at most twice.  Hence, the boolean variable
fFreedOne.  After two blocks are combined, if fFreedOne is TRUE, the
routine exits.  fFreedOne is initially false.  The 1st time two blocks are
combined, it is set to TRUE, and the routine continues.  The 2nd time
this happens, fFreedOne is already TRUE, so the routine knows that it
is done.

Helper macro after() takes a PMEMBLOCK (call it pmb) as a parameter,
and calculates where an adjacent free block would exist if it were
physically located after pmb.

Helper function remove() removes an element from the free list.
*/

#define after(pmb) ((PMEMBLOCK) ((char __near *) pmb + pmb->uSize + HDR_SIZE))

void remove(PMEMBLOCK pmb)
{
   PMEMBLOCK pmbPrev;

   if (pmb == pmbFree) {
      pmbFree = pmbFree->pmbNext;
      return;
   }

   for (pmbPrev=pmbFree; pmbPrev; pmbPrev=pmbPrev->pmbNext)
      if (pmbPrev->pmbNext == pmb) {
         pmbPrev->pmbNext = pmb->pmbNext;
         return;
      }
}

void compact(void)
{
   PMEMBLOCK pmb;
   int fFreedOne = FALSE;

   for (pmb=pmbFree->pmbNext; pmb; pmb=pmb->pmbNext) {
      if (pmb == pmb->pmbNext) {
         ddprintf("GENPDD: heap loop, %p points to itself\n", (void __far *) pmb);
         int3();
      }

      if (after(pmb)  == pmbFree) {
// ddprintf("GENPDD: compact found pointer %p (size=%ui) before pmbFree %p\n", (void __far *) pmb, pmb->uSize, (void __far *) pmbFree);
         pmb->uSize += HDR_SIZE + pmbFree->uSize;
         remove(pmbFree);
         if (fFreedOne) return;
         fFreedOne = TRUE;
      } else if (after(pmbFree) == pmb) {
// ddprintf("GENPDD: compact found pointer %p (size=%ui) after pmbFree %p\n", (void __far *) pmb, pmb->uSize, (void __far *) pmbFree);
         pmbFree->uSize += HDR_SIZE + pmb->uSize;
         remove(pmb);
         if (fFreedOne) return;
         fFreedOne = TRUE;
      }
   }
}

void free(void __near *pvBlock)
{
   PMEMBLOCK pmb,pmbPrev,pmbBlock;
   int fSentinel;

   if (!pvBlock) return;     // support freeing of NULL

   pmbBlock=(PMEMBLOCK) ((char __near *) pvBlock - HDR_SIZE);
#ifdef SIGNATURE
   if (pmbBlock->ulSignature != SIGNATURE) {
      ddprintf("GENPDD: free signature failed pmb=%p\n", pmbBlock);
      int3();
   }
#endif
   uMemFree += pmbBlock->uSize;

   if (pmbBlock == pmbUsed) {       // are we freeing the first block?
      pmbUsed = pmbUsed->pmbNext;   // the 2nd block is now the 1st
      pmbBlock->pmbNext = pmbFree;  // this block is now free, so it points to 1st free block
#ifdef SIGNATURE
      pmbBlock->ulSignature = 0;
#endif
      pmbFree = pmbBlock;           // this is now the 1st free block
      compact();
      return;
   }

   pmbPrev=pmbUsed;
   fSentinel = FALSE;
   for (pmb=pmbUsed->pmbNext; pmb; pmbPrev=pmb, pmb=pmb->pmbNext)
      if (pmb == pmbBlock) {
         if (fSentinel) {
            ddprintf("GENPDD: free sentinel triggered, pmb=%p\n", (void __far *) pmb);
            int3();
         }
         pmbPrev->pmbNext=pmb->pmbNext;   // delete this block from the chain
         pmbBlock->pmbNext=pmbFree;
#ifdef SIGNATURE
         pmbBlock->ulSignature = 0;
#endif
         pmbFree=pmbBlock;
         compact();
         fSentinel = TRUE;
      }
}

unsigned _memfree(void)
{
   return uMemFree;
}

void __near *realloc(void __near *pvBlock, unsigned usLength)
{
   void __near *pv;

   if (!pvBlock)                 // if ptr is null, alloc block
      return malloc(usLength);

   if (!usLength) {              // if length is 0, free block
      free(pvBlock);
      return 0;
   }

   pv=malloc(usLength);          // attempt to allocate the new block
   if (!pv)                      // can't do it?
      return 0;                  // just fail.  Version 2 will try harder

   memcpy(pv, pvBlock, min(_msize(pvBlock), usLength));
   free(pvBlock);
   return pv;
}

int validate(void __near *p)
{
   PMEMBLOCK pmb = pmbUsed;

   while (pmb)
   {
      if (p == pmb->achBlock)
      {
#ifdef SIGNATURE
         if (pmb->ulSignature != SIGNATURE)
            return FALSE;
#endif
         return TRUE;
      }
      pmb = pmb->pmbNext;
   }

   return FALSE;
}

int quick_validate(void __near *p)
{
   if ((p < abHeap) || (p > &end_of_heap))
      return FALSE;

#ifdef SIGNATURE
   PMEMBLOCK pmb = (PMEMBLOCK) (((char __near *) p) - HDR_SIZE);
   if (pmb->ulSignature != SIGNATURE)
      return FALSE;
#endif

   return TRUE;
}



#pragma code_seg ("_inittext");

void HeapInit(void)
{
   pmbFree->uSize = ((unsigned) &end_of_heap - (unsigned) abHeap) - HDR_SIZE;
   pmbFree->uSize &= 0xFFFC;  // round to nearest 4 bytes

#ifdef SIGNATURE
   pmbFree->ulSignature = SIGNATURE;
#endif

   pmbFree->pmbNext = 0;
   uMemFree = pmbFree->uSize;
}

