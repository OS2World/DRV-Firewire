/*
** Module   :SEMS.CPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Mon  12/07/2004 Created
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

#include "sems.hpp"
#include <devhelp.h>
#include <include.h>
void COMPLETION::Init(void)
{
    finish=FALSE;
}
void COMPLETION::Waitfor(void)
{
    pushf();
    cli();
    while (!finish)
    {
        DevHelp_ProcBlock((ULONG)(void far *)&finish,8,WAIT_IS_INTERRUPTABLE);
        cli();
    }
    popf();
}
void COMPLETION::Complete(void)
{
    USHORT awakecount;
    pushf();
    cli();
    if (!finish)
    {
        finish=TRUE;
        DevHelp_ProcRun((ULONG)(void far *)&finish,&awakecount);
    }
    popf();
}
void SEM::Init(int count)
{
    counter=count;
}
void SEM::Up(void)
{
    USHORT awakecount;
    pushf();
    cli();
    counter++;
    if (counter>=0)
        DevHelp_ProcRun((ULONG)(void far *)&counter,&awakecount);
    popf();
}
void SEM::Down(void)
{
	ULONG rc;
    pushf();
    cli();
    counter--;
    while (counter<0)
    {
        rc=DevHelp_ProcBlock((ULONG)(void far *)&counter,8,WAIT_IS_INTERRUPTABLE);
        cli();
    }
    popf();
}
void SEM::DownWait(ULONG time)
{
	ULONG rc;
    pushf();
    cli();
    counter--;
    while (counter<0)
    {
        rc=DevHelp_ProcBlock((ULONG)(void far *)&counter,time,WAIT_IS_INTERRUPTABLE);
        cli();
    }
    popf();
}



