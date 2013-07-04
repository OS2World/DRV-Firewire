/*
** Module   :MEMORY.HPP
** Abstract :memory buffer class
** allocate DMAble unmovable memory buffer
**
** Copyright (C) Alexandr Cherkaev
**
** Log: Sun  30/05/2004	Created
**
*/
#ifndef __MEMORY_HPP
#define __MEMORY_HPP

class MEMORY
{
public:
	MEMORY(USHORT usSize);
	~MEMORY(void);
    UCHAR far * fpVirtAddr;
	ULONG ulPhysAddr;
private:
	ULONG ulLinAddr;
	USHORT usBSize;
};
typedef MEMORY* PMEMORY;


#endif  /*__MEMORY_HPP*/

