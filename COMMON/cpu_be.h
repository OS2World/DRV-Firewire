/*
** Module   :CPU_BE.H
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  02/06/2004 Created
**
*/
#ifndef __CPU_BE_H
#define __CPU_BE_H
#ifdef __cplusplus         //###
extern "C" {               //###
#endif                     //###
unsigned short cpu_to_be16(unsigned short data);
unsigned short be16_to_cpu(unsigned short data);
unsigned long cpu_to_be32(unsigned long data);
unsigned long be32_to_cpu(unsigned long data);
unsigned __int64 cpu_to_be64(unsigned __int64 data);
unsigned __int64 be64_to_cpu(unsigned __int64 data);
unsigned long hweight32(unsigned long w);
#ifdef __cplusplus         //###
}
#endif                     //###

#endif  /*__CPU_BE_H*/

