/*
** Module   :SEMS.HPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Fri  09/07/2004 Created
**
*/
#ifndef __SEMS_HPP
#define __SEMS_HPP


class COMPLETION
{
public:
    void Init(void);
    void Waitfor(void);
    void Complete(void);
private:
    BOOL finish;
};
class SEM
{
public:
//    enum STATE{request,release};
    void Init(int count=0);
    void Up(void);
    void Down(void);
    void DownWait(ULONG time);
//    void Request(void);
    //void Release(void);
private:
    int counter;// if <0, wait until zero or more
};
#endif  /*__SEMS_HPP*/

