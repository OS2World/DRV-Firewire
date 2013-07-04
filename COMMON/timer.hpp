/*
** Module   :TIMER.HPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  02/06/2004 Created
**
*/
#ifndef __TIMER_HPP
#define __TIMER_HPP


class TIMER
{
public:
//    void InitTimer();
    TIMER(void);
    void DelTimerSync(void);
    void ModTimer(ULONG uldata);
    void (*function)(ULONG data);
    ULONG data;
    ULONG expire;
    BOOL active;
	BOOL done;
private:
};
extern int far pascal TimerHandler();
extern void AddTimer(TIMER far * timer);
extern TIMER far * Timerlist[10];
#endif  /*__TIMER_HPP*/

