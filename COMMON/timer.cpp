/*
** Module   :TIMER.CPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Fri  04/06/2004 Created
**
*/
extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define  INCL_NOPMAPI
#define  INCL_DOSDEVICES
#define  INCL_DOSDEVIOCTL
#define  INCL_DOSERRORS
#define  INCL_NOXLATE_DOS16
#include <os2.h>
}

#include "timer.hpp"
#include "jiffies.h"
#include "ddprintf.h"
#include "firetype.h"
#include <devhelp.h>
#include <include.h>
TIMER far * Timerlist[10];

TIMER::TIMER(void)
{
    function=NULL;
    data=0;
    expire=0;
    active=FALSE;
    done=TRUE;
}
void TIMER::DelTimerSync(void)
{
    active=FALSE;
    done=TRUE;
}
void TIMER::ModTimer(ULONG uldata)
{
	pushf();
	cli();
    expire=uldata;
    active=TRUE;
    done=FALSE;
    popf();
}
int far pascal TimerHandler(void)
{
	saveall();
    int i;
//    int3();
    pushf();
    cli();
    for (i=0;i<10;i++)
    {
        if (Timerlist[i]!=NULL)
            if (Timerlist[i]->active)
                if (Timerlist[i]->expire<=getJiffies())
                	if (!Timerlist[i]->done)
                		if (Timerlist[i]->function!=NULL)
                        {
                            //int3();
                            ddprintf("DEBUG:Timer activated\n");
                            Timerlist[i]->done=TRUE;
                            (Timerlist[i]->function)(Timerlist[i]->data);
                        }
    }
    popf();
    loadall();
    return 0;
}
void AddTimer(TIMER far * timer)
{
    int i;
    for (i=0;i<10;i++)
    {
        if (Timerlist[i]==NULL)
        {
            Timerlist[i]=timer;
            return;
        }
    }
}
