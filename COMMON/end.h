/*
** Module   :END.H
** Abstract :end of dataseg
**
** Copyright (C) Alexandr Cherkaev
** original code by Timur Tabi
**
** Log: Fri  30/04/2004	Created
**
*/
#ifndef __END_H
#define __END_H



#ifdef __cplusplus
extern "C" {
#endif

extern int end_of_data;
extern int end_of_initdata;
extern char abHeap[];
extern char end_of_heap;

void end_of_text(void);

#ifdef __cplusplus
}
#endif


#endif  /*__END_H*/

