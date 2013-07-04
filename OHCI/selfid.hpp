/*
** Module   :SELFID.HPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Fri  14/05/2004 Created
**
*/
#ifndef __SELFID_HPP
#define __SELFID_HPP
#include "memory.hpp"

class SELFIDBUF:public MEMORY
{
public:
    SELFIDBUF(USHORT usSize);
    UCHAR errors;
};

#endif  /*__SELFID_HPP*/

