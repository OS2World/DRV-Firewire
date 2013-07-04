/*
** Module   :ISR.H
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Fri  14/05/2004 Created
**
*/
#ifndef __ISR_H
#define __ISR_H

#ifdef __cplusplus
extern "C" {
#endif

int pascal far IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif  /*__ISR_H*/

