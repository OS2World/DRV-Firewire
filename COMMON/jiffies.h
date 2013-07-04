/*
** Module   :JIFFIES.H
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Thu  03/06/2004 Created
**
*/
#ifndef __JIFFIES_H
#define __JIFFIES_H
#ifdef __cplusplus         //###
extern "C" {               //###
#endif                     //###

#ifndef HZ
#define HZ 100
#endif
#include <infoseg.h>
extern struct InfoSegGDT far * fpGINFOSEG;

unsigned long getJiffies(void);

#ifdef __cplusplus         //###
}               //###
#endif                     //###


#endif  /*__JIFFIES_H*/

