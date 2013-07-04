/*
** Module   :NODEMGR.HPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  30/06/2004 Created
**
*/
#ifndef __NODEMGR_HPP
#define __NODEMGR_HPP
#include "csr1212.h"
#include "firetype.h"
#include "..\stack\host.hpp"
#include "sems.hpp"
#include "listmgr.hpp"

#define ignore_drivers FALSE
//MODULE_PARM_DESC(ignore_drivers, "Disable automatic probing for drivers.");


struct NODEMGR_CSR_INFO
{
public:
    HOSTOPS far * pHostOps;
    NODEID nodeid;
    unsigned int generation;
};
/* '1' '3' '9' '4' in ASCII */
#define IEEE1394_BUSID_MAGIC    (0x34393331)

/* This is the start of a Node entry structure. It should be a stable API
 * for which to gather info from the Node Manager about devices attached
 * to the bus.  */
struct BUS_OPTIONS
{
    UCHAR  irmc;       /* Iso Resource Manager Capable */
    UCHAR  cmc;        /* Cycle Master Capable */
    UCHAR  isc;        /* Iso Capable */
    UCHAR  bmc;        /* Bus Master Capable */
    UCHAR  pmc;        /* Power Manager Capable (PNP spec) */
    UCHAR  cyc_clk_acc;    /* Cycle clock accuracy */
    UCHAR  max_rom;    /* Maximum block read supported in the CSR */
    UCHAR  generation; /* Incremented when configrom changes */
    UCHAR  lnkspd;     /* Link speed */
    USHORT max_rec;    /* Maximum packet size node can receive */
};

struct UNIT_DIRECTORY;

struct PROTOCOL_DRIVER
{
    /* The name of the driver, e.g. SBP2 or IP1394 */
    const char *name;

    /*
     * The device id table describing the protocols and/or devices
     * supported by this driver.  This is used by the nodemgr to
     * decide if a driver could support a given node, but the
     * probe function below can implement further protocol
     * dependent or vendor dependent checking.
     */
    struct ieee1394_device_id *id_table;

    /*
     * The update function is called when the node has just
     * survived a bus reset, i.e. it is still present on the bus.
     * However, it may be necessary to reestablish the connection
     * or login into the node again, depending on the protocol. If the
     * probe fails (returns non-zero), we unbind the driver from this
     * device.
     */
    int (*fpfnUpdate)(UNIT_DIRECTORY far *ud);

    // suspend func?
    BOOL (*fpfnSuspend)(void);
    /* Our LDM structure */
//    struct device_driver driver;
};

#define UNIT_DIRECTORY_VENDOR_ID        0x01
#define UNIT_DIRECTORY_MODEL_ID         0x02
#define UNIT_DIRECTORY_SPECIFIER_ID     0x04
#define UNIT_DIRECTORY_VERSION          0x08
#define UNIT_DIRECTORY_HAS_LUN_DIRECTORY    0x10
#define UNIT_DIRECTORY_LUN_DIRECTORY        0x20

/*
 * A unit directory corresponds to a protocol supported by the
 * node. If a node supports eg. IP/1394 and AV/C, its config rom has a
 * unit directory for each of these protocols.
 */
struct UNIT_DIRECTORY
{
    struct NODE_ENTRY far *ne;  /* The node which this directory belongs to */
    OCTLET address;       /* Address of the unit directory on the node */
    UCHAR flags;       /* Indicates which entries were read */

    QUADLET vendor_id;
    struct csr1212_keyval far *vendor_name_kv;
    const char *vendor_oui;

    QUADLET model_id;
    struct csr1212_keyval far *model_name_kv;
    QUADLET specifier_id;
    QUADLET version;

    unsigned int id;

    BOOL bIgnoreDriver;

    int length;     /* Number of quadlets */
    PROTOCOL_DRIVER far *pProtocolDriver;

//    struct device device;

//    struct class_device class_dev;
    UNIT_DIRECTORY far *parent;

    struct csr1212_keyval far *ud_kv;
};


struct NODE_ENTRY
{
    U64 guid;           /* GUID of this node */
    ULONG guid_vendor_id;     /* Top 24bits of guid */
    const char *guid_vendor_oui;    /* OUI name of guid vendor id */

    HOSTOPS far * pHostOps;     /* Host this node is attached to */
    NODEID NodeId;        /* NodeID */
    struct BUS_OPTIONS busopt;  /* Bus Options */
    int needs_probe;
    unsigned int generation;    /* Synced with hpsb generation */

    /* The following is read from the config rom */
    ULONG vendor_id;
    struct csr1212_keyval far *vendor_name_kv;
    const char *vendor_oui;

    ULONG capabilities;
    TLABELPOOL far *tpool;

//    struct device device;

//    struct class_device class_dev;

    /* Means this node is not attached anymore */
    BOOL bInLimbo;

    UNIT_DIRLIST UnitDirsList;

    struct csr1212_csr far *csr;
};
class HOSTINFO
{
public:
    HOSTINFO(PHOSTOPS pHost);
    PHOSTOPS pHostOps;
    COMPLETION exited;
    SEM ResetSem;
    BOOL Kill;
    ULONG pid; //hook handle to context hook routine
    char daemon_name[32];
    NODE_ENTRYLIST NodeEntryList;
};

class NODEMGR
{
public:
    void Init(void);
    void Cleanup(void);
    void AddHost(HOSTOPS far *pHost);
    void RemoveHost(HOSTOPS far *pHost);
    void HostReset(HOSTOPS far *pHost);
    BOOL CheckIrmCapability(HOSTOPS far *host, int cycles);
    BOOL DoIrmDuties(HOSTINFO far *hi, int cycles);
    void NodeProbe(HOSTINFO far *hi, int generation);
    void NodeScan(HOSTINFO far *hi, int generation);
    void NodeScanOne(HOSTINFO far *hi, NODEID nodeid, int generation);
    //void ProbeNE(HOSTINFO *hi, NODE_ENTRY *ne, int generation);
    void ProbeNE(HOSTINFO far *hi, NODE_ENTRY far *ne, int generation);
    void UpdateNode(NODE_ENTRY far *ne, struct csr1212_csr far *csr, HOSTINFO far *hi, NODEID nodeid, unsigned int generation);
    NODE_ENTRY far *CreateNode(OCTLET guid, struct csr1212_csr far *csr, HOSTINFO far *hi, NODEID nodeid, UINT generation);
    void SuspendNE(NODE_ENTRY far *ne);
    void ResumeNE(NODE_ENTRY far *ne);
//    void RemoveNe(NODE_ENTRY far *ne);
    void RemoveNe(HOSTINFO far * hi, NODE_ENTRY far *ne);
    void RemoveUDs(NODE_ENTRY far *ne);
    void ReleaseUD(UNIT_DIRECTORY far *ud);
    void ReleaseNE(NODE_ENTRY far *ne);
    void UpdatePdrv(NODE_ENTRY far *ne);
    void ProcessRootDirectory(HOSTINFO far *hi, NODE_ENTRY far *ne);
    UNIT_DIRECTORY far *ProcessUnitDirectory(HOSTINFO far *hi, NODE_ENTRY far *ne, struct csr1212_keyval far *ud_kv, unsigned int far *id, UNIT_DIRECTORY far *parent);
    void UpdateBusOptions(NODE_ENTRY far *ne);
    void FillPacket(NODE_ENTRY far *ne, PACKET far *pkt);
    int Read(NODE_ENTRY far *ne, U64 addr,QUADLET far *buffer, int length);
    int Write(NODE_ENTRY far *ne, U64 addr, QUADLET far *buffer, int length);
    int Lock(NODE_ENTRY far *ne, U64 addr, int extcode, QUADLET far *data, QUADLET arg);

    HOSTINFOLIST HostInfoList;
    STACKOPS far * pStack;

private:

};
extern "C" void far NodemgrHostCtxRoutine(void *__hi);

#endif  /*__NODEMGR_HPP*/

