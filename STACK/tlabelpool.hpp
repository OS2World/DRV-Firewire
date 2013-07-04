/*
** Module   :TLABELPOOL.HPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  02/06/2004 Created
**
*/
#ifndef __TLABELPOOL_HPP
#define __TLABELPOOL_HPP


typedef struct
{
    U64 pool;
    int count;
    UCHAR next;
    ULONG allocations;
} TLABELPOOL;
#endif  /*__TLABELPOOL_HPP*/

