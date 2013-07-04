/*
** Module   :IDC.H
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Mon  21/06/2004 Created
**
*/
#ifndef __IDC_H
#define __IDC_H
#ifdef __cplusplus
extern "C" {
#endif

//#include "..\stack\host.hpp"
//#include "..\stack\stack.hpp"
#include "firetype.h"
//stack entry points defs
class HOST;
class NODEMGR;
class OHCIDRIVER;
struct HOSTOPS;
struct HOSTINFO;
struct HIGHLEVELOPS;
struct ADDROPS;
typedef struct HOSTOPS far * PHOSTOPS;
struct STACKOPS
{
    void far (*fpfnRegisterHighlevel)(HIGHLEVELOPS far * pHighlevel); //hpsb_register_highlevel(struct hpsb_highlevel *hl);
//    void far (*fpfnRegisterAddrSpace)(void far * pAddrSpace);
    BOOL far (*fpfnRegisterAddrSpace)(HIGHLEVELOPS far *pHighlevel, HOSTOPS far *pHostOps,
                            ADDROPS far *ops, U64 start, U64 end);

//int hpsb_unregister_addrspace(struct hpsb_highlevel *hl, struct hpsb_host *host,
//                              u64 start);
    U64 far (*fpfnAllocateAndRegisterAddrSpace)(HIGHLEVELOPS far *hl,
                     HOSTOPS far *host,
                     ADDROPS far *ops,
                     U64 size, U64 alignment,
                     U64 start, U64 end);
    int far (*fpfnUnRegisterAddrSpace)(HIGHLEVELOPS far *hl, HOSTOPS far *host,
                              U64 start);

    void far (*fpfnAddHost)(PHOSTOPS pHost);
    void far (*fpfnUnRegisterHighlevel)(HIGHLEVELOPS far * pHighlevel);
    void far (*fpfnRemoveHost)(PHOSTOPS pHost);
    void far (*fpfnHostReset)(PHOSTOPS pHost);
/* these functions are called to handle transactions. They are called, when
   a packet arrives. The flags argument contains the second word of the first header
   quadlet of the incoming packet (containing transaction label, retry code,
   transaction code and priority). These functions either return a response code
   or a negative number. In the first case a response will be generated; in the
   later case, no response will be sent and the driver, that handled the request
   will send the response itself.
*/
    // called by lower level when event occur
    int far (*fpfnRead)(PHOSTOPS pHost, int nodeid, void far *data, U64 addr, USHORT length, USHORT flags);
    int far (*fpfnWrite)(PHOSTOPS pHost, int nodeid, int destid, void far *data, U64 addr, USHORT length, USHORT flags);
    int far (*fpfnLock)(PHOSTOPS pHost, int nodeid, QUADLET far *store, U64 addr, QUADLET data, QUADLET arg, int ext_tcode, USHORT flags);
    int far (*fpfnLock64)(PHOSTOPS pHost, int nodeid, OCTLET far *store, U64 addr, OCTLET data, OCTLET arg, int ext_tcode, USHORT flags);
    void far (*fpfnIsoReceive)(PHOSTOPS pHost, void far * data, int length);
    void far (*fpfnFcpRequest)(PHOSTOPS pHost, int nodeid, int direction, void far * data, int length);
};
// highlevel entry points defs
struct HIGHLEVELOPS
{
//    struct module *owner;
    char name[32];
    NODEMGR far * pHighlevel;

    /* Any of the following pointers can legally be NULL, except for
     * iso_receive which can only be NULL when you don't request
     * channels. */

    /* New host initialized.  Will also be called during
     * hpsb_register_highlevel for all hosts already installed. */
    void far (*fpfnAddHost) (HOSTOPS far * pHostOps  );

    /* Host about to be removed.  Will also be called during
     * hpsb_unregister_highlevel once for each host. */
    void far (*fpfnRemoveHost) (HOSTOPS far * pHostOps  );

    /* Host experienced bus reset with possible configuration changes.
     * Note that this one may occur during interrupt/bottom half handling.
     * You can not expect to be able to do stock hpsb_reads. */
    void far (*fpfnHostReset) (HOSTOPS far * pHostOps  );

    /* An isochronous packet was received.  Channel contains the channel
     * number for your convenience, it is also contained in the included
     * packet header (first quadlet, CRCs are missing).  You may get called
     * for channel/host combinations you did not request. */
    void far (*fpfnIsoReceive) (HOSTOPS far * pHostOps  , int channel,
                         QUADLET far *data, int length);

    /* A write request was received on either the FCP_COMMAND (direction =
     * 0) or the FCP_RESPONSE (direction = 1) register.  The cts arg
     * contains the cts field (first byte of data). */
    void far (*fpfnFcpRequest) (HOSTOPS far *host, int nodeid, int direction,
                         int cts, UCHAR far *data, int length);

    //pointers to lists funcs
    HOSTINFO far *  far (*fpfnGetHostInfo)(PHOSTOPS pHost);
    void far (*fpfnDestroyHostInfo)(HOSTOPS far * pHostOps);

};
typedef struct HIGHLEVELOPS far * PHIGHLEVELOPS;
struct ADDROPS
{
    /*
     * Null function pointers will make the respective operation complete
     * with RCODE_TYPE_ERROR.  Makes for easy to implement read-only
     * registers (just leave everything but read NULL).
     *
     * All functions shall return appropriate IEEE 1394 rcodes.
     */

    /* These functions have to implement block reads for themselves. */
    /* These functions either return a response code
       or a negative number. In the first case a response will be generated; in the
       later case, no response will be sent and the driver, that handled the request
       will send the response itself
    */
    int far (*fpfnRead) (HOSTOPS far * pHostOps  , int nodeid, QUADLET far *buffer,
                 U64 addr, int length, USHORT flags);
    int far (*fpfnWrite) (HOSTOPS far * pHostOps  , int nodeid, int destid,
          QUADLET far * data, U64 addr, int length, USHORT flags);

    /* Lock transactions: write results of ext_tcode operation into
     * *store. */
    int far (*fpfnLock) (HOSTOPS far * pHostOps  , int nodeid, QUADLET far * store,
                 U64 addr, QUADLET data, QUADLET arg, int ext_tcode, USHORT flags);
    int far (*fpfnLock64) (HOSTOPS far * pHostOps  , int nodeid, POCTLET store,
                   U64 addr, OCTLET data, OCTLET arg, int ext_tcode, USHORT flags);
};
struct DRIVEROPS
{
    char name[16];
    OHCIDRIVER far * pDriver;
    /* The hardware driver may optionally support a function that is used
     * to set the hardware ConfigROM if the hardware supports handling
     * reads to the ConfigROM on its own. */
    void far (*fpfnSetHwConfigRom)(QUADLET far * pConfigRom);
    /* This function shall implement packet transmission based on
     * packet->type.  It shall CRC both parts of the packet (unless
     * packet->type == raw) and do byte-swapping as necessary or instruct
     * the hardware to do so.  It can return immediately after the packet
     * was queued for sending.  After sending, hpsb_sent_packet() has to be
     * called.  Return 0 on success, negative errno on failure.
     * NOTE: The function must be callable in interrupt context.
     */
    int far (*fpfnTransmitPacket)(void far * pPacket);
    /* This function requests miscellanous services from the driver, see
     * above for command codes and expected actions.  Return -1 for unknown
     * command, though that should never happen.
     */
    ULONG far (*fpfnDevCtl)(enum DEVCTL_CMD Command, ULONG arg);
     /* ISO transmission/reception functions. Return 0 on success, -1
      * (or -EXXX errno code) on failure. If the low-level driver does not
      * support the new ISO API, set isoctl to NULL.
      */
    int far (*fpfnIsoCtl)(void);
//    struct hpsb_iso *iso, enum ISOCTL_CMD Command, ULONG arg);
    /* This function is mainly to redirect local CSR reads/locks to the iso
     * management registers (bus manager id, bandwidth available, channels
     * available) to the hardware registers in OHCI.  reg is 0,1,2,3 for bus
     * mgr, bwdth avail, ch avail hi, ch avail lo respectively (the same ids
     * as OHCI uses).  data and compare are the new data and expected data
     * respectively, return value is the old value.
     */
    QUADLET far (*fpfnHwCsrReg)(int reg, QUADLET data, QUADLET compare);
};

//host entry points defs
struct HOSTOPS
{
    char name[16];
//    void far * pHost;
    HOST far * pHost;
    BOOL far (*fpfnUpdateConfigRomImage)(PHOSTOPS pHostOps);
    void far (*fpfnAddHost)(PHOSTOPS pHostOps,DRIVEROPS far * pHwDriver);
    void far (*fpfnBusReset)(PHOSTOPS pHostOps);
    void far (*fpfnSelfIdComplete)(PHOSTOPS pHostOps,USHORT usPhyId, BOOL bIsRoot);
    void far (*fpfnSelfIdReceived)(PHOSTOPS pHostOps,QUADLET sid);
    void far (*fpfnPacketSent)(PHOSTOPS pHostOps,void far * packet, char ackcode);
    void far (*fpfnPacketReceived)(PHOSTOPS pHostOps,QUADLET far * data, int size, BOOL bwrite_acked);
    int  far (*fpfnRead)(PHOSTOPS pHostOps,NODEID node, unsigned int generation,U64 addr, PQUADLET buffer, int length);
    int  far (*fpfnWrite)(PHOSTOPS pHostOps,NODEID node, unsigned int generation, U64 addr, PQUADLET buffer, int length);
    int  far (*fpfnLock)(PHOSTOPS pHostOps,NODEID node, unsigned int generation, U64 addr, int extcode, PQUADLET data, QUADLET arg);
    int  far (*fpfnGetGeneration)(PHOSTOPS pHostOps);
    BOOL far (*fpfnResetBus)(PHOSTOPS pHostOps,RESET_TYPES type);
    APIRET far (*fpfnSendPhyConfig)(PHOSTOPS pHostOps,LONG lrootid, LONG lgapcnt);
};
typedef void (__far __loadds __cdecl *STACKIDC) (unsigned char type , void far * pData);
typedef struct {
   unsigned short ausReserved[3];      // 3 reserved words
   STACKIDC pfn;                         // far pointer to IDC entry
   unsigned short DS;                  // data segment of IDC
} STACK_ATTACH_DD;
extern STACKIDC StackIDC;
//IDC valid commands
#define IDC_GET_STACK   1
#define IDC_CREATE_HOST 2
#define IDC_ATTACH_NODEMGR 3
//#define IDC_GET_HOST    3
#ifdef __cplusplus
}
#endif

#endif  /*__IDC_H*/

