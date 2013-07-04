/*
** Module   :CONFIGROM.HPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Fri  14/05/2004 Created
**
*/

#ifndef __CONFIGROM_HPP
#define __CONFIGROM_HPP
#include "memory.hpp"

class CSRCONFIGROM:public MEMORY
{
public:
    CSRCONFIGROM(USHORT usSize);
    USHORT usLength;
};

#endif  /*__CONFIGROM_HPP*/

