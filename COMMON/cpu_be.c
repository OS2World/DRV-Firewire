/*
** Module   :CPU_BE.c
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  02/06/2004 Created
**
*/
//extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define  INCL_NOPMAPI
#define  INCL_DOSDEVICES
#define  INCL_DOSDEVIOCTL
#define  INCL_DOSERRORS
#define  INCL_NOXLATE_DOS16
#include <os2.h>
//}

#include "cpu_be.h"
#include "ddprintf.h"
unsigned long cpu_to_be32(unsigned long data)
{
    static unsigned char b0,b1,b2,b3;
    static unsigned long data1;
//    ddprintf("in %0lx",data);
    b0=(unsigned char)(data&0xff);
    b1=(unsigned char)((data&0xff00)>>8);
    b2=(unsigned char)((data&0xff0000)>>16);
    b3=(unsigned char)((data&0xff000000)>>24);
//    ddprintf(" b0 %x b1 %x b2 %x b3 %x",b0, b1, b2, b3);
    data1=((unsigned long)b0<<24)|((unsigned long)b1<<16)|((unsigned long)b2<<8)|((unsigned long)b3);
//    ddprintf(" - out %0lx\n",data1);
    return data1;
}
unsigned long be32_to_cpu(unsigned long data)
{
    static unsigned char b0,b1,b2,b3;
    static unsigned long data1;

//    ddprintf("in %0lx",data);

    b0=(unsigned char)(data&0xff);
    b1=(unsigned char)((data&0xff00)>>8);
    b2=(unsigned char)((data&0xff0000)>>16);
    b3=(unsigned char)((data&0xff000000)>>24);
//    ddprintf(" b0 %x b1 %x b2 %x b3 %x",b0, b1, b2, b3);
    data1=((unsigned long)b0<<24)|((unsigned long)b1<<16)|((unsigned long)b2<<8)|((unsigned long)b3);
//    ddprintf(" - out %0lx\n",data1);
    return data1;
}
unsigned short cpu_to_be16(unsigned short data)
{
    static unsigned char b0,b1;
    b0=(unsigned char)(data&0xff);
    b1=(unsigned char)((data&0xff00)>>8);

    return (((unsigned short)b0)<<8)|(b1);
}
unsigned short be16_to_cpu(unsigned short data)
{
    static unsigned char b0,b1;
    b0=(unsigned char)(data&0xff);
    b1=(unsigned char)((data&0xff00)>>8);

    return (((unsigned short)b0)<<8)|(b1);
}
unsigned __int64 be64_to_cpu(unsigned __int64 data)
{
    unsigned __int64 __x = data;
    return ((unsigned __int64)( \
            (unsigned __int64)(((__x) & 0x00000000000000ffULL) << 56) | \
            (unsigned __int64)(((__x) & 0x000000000000ff00ULL) << 40) | \
            (unsigned __int64)(((__x) & 0x0000000000ff0000ULL) << 24) | \
            (unsigned __int64)(((__x) & 0x00000000ff000000ULL) <<  8) | \
            (unsigned __int64)(((__x) & 0x000000ff00000000ULL) >>  8) | \
            (unsigned __int64)(((__x) & 0x0000ff0000000000ULL) >> 24) | \
            (unsigned __int64)(((__x) & 0x00ff000000000000ULL) >> 40) | \
            (unsigned __int64)(((__x) & 0xff00000000000000ULL) >> 56) ));
}
unsigned __int64 cpu_to_be64(unsigned __int64 data)
{
    unsigned __int64 __x = data;
    return ((unsigned __int64)( \
            (unsigned __int64)(((__x) & 0x00000000000000ffULL) << 56) | \
            (unsigned __int64)(((__x) & 0x000000000000ff00ULL) << 40) | \
            (unsigned __int64)(((__x) & 0x0000000000ff0000ULL) << 24) | \
            (unsigned __int64)(((__x) & 0x00000000ff000000ULL) <<  8) | \
            (unsigned __int64)(((__x) & 0x000000ff00000000ULL) >>  8) | \
            (unsigned __int64)(((__x) & 0x0000ff0000000000ULL) >> 24) | \
            (unsigned __int64)(((__x) & 0x00ff000000000000ULL) >> 40) | \
            (unsigned __int64)(((__x) & 0xff00000000000000ULL) >> 56) ));
}
unsigned long hweight32(unsigned long w)
{
	unsigned long res = (w & 0x55555555) + ((w >> 1) & 0x55555555);
    res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
    res = (res & 0x0F0F0F0F) + ((res >> 4) & 0x0F0F0F0F);
    res = (res & 0x00FF00FF) + ((res >> 8) & 0x00FF00FF);
    return (res & 0x0000FFFF) + ((res >> 16) & 0x0000FFFF);
}
