/*
** Module   :PACKETLIST.HPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Thu  24/06/2004 Created
**
*/
#ifndef __PACKETLIST_HPP
#define __PACKETLIST_HPP
#include "firetype.h"
#include "list.hpp"
#include "packet.hpp"


class PACKETLIST
{
public:
    PACKETLIST(void);
    void AddHead(PACKET far * pPacket);
    void AddTail(PACKET far * pPacket);
    void Remove(PACKET far * pPacket);
    void DeleteAll(void);
    void DeleteHead(void);
    void DeleteTail(void);
    BOOL isEmpty(void);
    PACKET far * GetHead(void);
    PACKET far * GetNext(void);
    PACKET far * GetTail(void);
    void AddList(PACKETLIST far * AddedList);
private:
    LIST List;
};

#endif  /*__PACKETLIST_HPP*/

