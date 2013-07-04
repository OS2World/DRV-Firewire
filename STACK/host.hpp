/*
** Module   :HOST.HPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  19/05/2004 Created
**
*/
#ifndef __HOST_HPP
#define __HOST_HPP

#include "firetype.h"
#include "csr1212.h"
#include "timer.hpp"
#include "list.hpp"
#include "tlabelpool.hpp"
//#include "ohci.hpp"
//#include "..\stack\stack.hpp"
#include "stacklist.hpp"
#include "packet.hpp"
#include "packetlist.hpp"
#include "..\nodemgr\csr.hpp"
#include "idc.h"

class OHCIDRIVER;
typedef OHCIDRIVER* POHCIDRIVER;
class FIRESTACK;
typedef FIRESTACK* PFIRESTACK;
class PACKET;
typedef PACKET * PPACKET;

class HOST
{
public:
    HOST(UCHAR HostNumber);
//    void AddHost(POHCIDRIVER physdriver);
    void AddHost(DRIVEROPS far * physdriver);
    void RemoveHost(void);
    int  UpdateConfigRomImage();
    void SelfIdComplete(USHORT usPhyId, BOOL bIsRoot);
    BOOL ResetBus(RESET_TYPES type);
    BOOL BusReset(void);
    BOOL bCheckSelfIds(void);
    void AbortRequests(void);
    void BuildSpeedMap(USHORT NodesCount);
    void SelfIdReceived(QUADLET sid);
    void PacketSent(PPACKET packet, char ackcode);
    void PacketReceived(QUADLET far * data, size_t size, BOOL bwrite_acked);
    APIRET SendPhyConfig(LONG lrootid, LONG lgapcnt);
    void HandlePacketResponse(int tcode, QUADLET far * data, int size);
    void AddExtraConfigRoms(void);
    void RemoveExtraConfigRoms(void);
    PPACKET CreateReplyPacket(PQUADLET data, int dsize);
    int GetGeneration(void);
    void HandleIncomingPacket(int tcode,QUADLET far * data, int size, BOOL bwrite_acked);
    PPACKET MakeReadPacket(NODEID node, U64 addr, int length);
    PPACKET MakeWritePacket(NODEID node,U64 addr, PQUADLET buffer, int length);
    PPACKET MakeStreamPacket(UCHAR far *buffer, int length, int channel, int tag, int sync);
    PPACKET MakeLockPacket(NODEID node,U64 addr, int extcode, PQUADLET data,QUADLET arg);
    PPACKET MakeLock64Packet(NODEID node,U64 addr, int extcode, POCTLET data,OCTLET arg);
    PPACKET MakePhyPacket(QUADLET data);
    PPACKET MakeIsoPacket(int length, int channel,int tag, int sync);
    int Read(NODEID node, unsigned int generation,U64 addr, PQUADLET buffer, int length);
    int Write(NODEID node, unsigned int generation, U64 addr, PQUADLET buffer, int length);
    int Lock(NODEID node, unsigned int generation, U64 addr, int extcode, PQUADLET data, QUADLET arg);
    int Lock64(NODEID node, unsigned int generation,U64 addr, int extcode, POCTLET data, OCTLET arg);
    int SendGasp(int channel, unsigned int generation,PQUADLET buffer, int length, ULONG specifier_id, unsigned int version);
    int DefaultConfigEntry(void);

    LIST HostList;
    int Generation;

    PACKETLIST PendingPacket;
    TIMER Timeout;
    ULONG ulTimeoutInterval;

    UCHAR IsoListenCount[64];

    USHORT usNodeCount; // number of identified nodes on bus
    USHORT usSelfIdCount; //number of SelfID received
    USHORT usNodesActive; //number of actually active nodes

    NODEID NodeId; // node id of this host
    NODEID IRMId; // node id of this bus isocronous resource manager
    NODEID BusMgrId; //node id of this bus bus manager

    /* this nodes state */
    BOOL bInBusReset;
    BOOL bIsShutdown;

   /* this nodes' duties on the bus */
    BOOL bIsRoot;
    BOOL bIsCycmst;
    BOOL bIsIrm;
    BOOL bIsBusmgr;

    USHORT usResetRetries;
    QUADLET far * pTopologyMap;
    UCHAR far * pSpeedMap;
    CSR far * pCSR; //csr_control

    /* Per node tlabel pool allocation */
    TLABELPOOL tpool[64];

//    POHCIDRIVER pDriver;
    DRIVEROPS far * pHardDriver;

    USHORT usId;

    BOOL bUpdateConfigRom;
    TIMER DelayedReset;

    USHORT usConfigRoms;
    struct STACKOPS far * pStack;
//ADDRSPACELIST AddrSpace;

private:
    //for BuildSpeedMap
    UCHAR speedcap[NODESMAX];
    UCHAR cldcnt[NODESMAX];

};
typedef HOST far * PHOST;
/*extern HOST_DRIVER dummyDriver;*/
extern "C" void AbortTimedouts(ULONG __opaque);
extern "C" void DelayedResetBus(ULONG __opaque);

#endif  /*__HOST_HPP*/

