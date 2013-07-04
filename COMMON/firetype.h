/*
** Module   :FIRETYPE.H
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Fri  21/05/2004 Created
**
*/
#ifndef __FIRETYPE_H
#define __FIRETYPE_H

typedef ULONG QUADLET;
typedef QUADLET far * PQUADLET;
typedef unsigned __int64 U64;
typedef U64 OCTLET;
typedef ULONG U32;
typedef OCTLET far * POCTLET;
typedef USHORT NODEID;
typedef OCTLET NODEADDR;
typedef USHORT ARMLENGTH;

#define BUS_MASK  0xffc0
#define BUS_SHIFT 6
#define NODE_MASK 0x003f
#define LOCAL_BUS 0xffc0
#define ALL_NODES 0x003f

#define NODEID_TO_BUS(nodeid)   ((nodeid & BUS_MASK) >> BUS_SHIFT)
#define NODEID_TO_NODE(nodeid)  (nodeid & NODE_MASK)
#define NODE_BUS_FMT        "%d-%d:%d"
#define NODE_BUS_ARGS(__host, __nodeid) \
    __host->usId, NODEID_TO_NODE(__nodeid), NODEID_TO_BUS(__nodeid)

#define NODESMAX 0x3f

#define TCODE_WRITEQ             0x0
#define TCODE_WRITEB             0x1
#define TCODE_WRITE_RESPONSE     0x2
#define TCODE_READQ              0x4
#define TCODE_READB              0x5
#define TCODE_READQ_RESPONSE     0x6
#define TCODE_READB_RESPONSE     0x7
#define TCODE_CYCLE_START        0x8
#define TCODE_LOCK_REQUEST       0x9
#define TCODE_ISO_DATA           0xa
#define TCODE_STREAM_DATA        0xa
#define TCODE_LOCK_RESPONSE      0xb

#define RCODE_COMPLETE           0x0
#define RCODE_CONFLICT_ERROR     0x4
#define RCODE_DATA_ERROR         0x5
#define RCODE_TYPE_ERROR         0x6
#define RCODE_ADDRESS_ERROR      0x7

#define EXTCODE_MASK_SWAP        0x1
#define EXTCODE_COMPARE_SWAP     0x2
#define EXTCODE_FETCH_ADD        0x3
#define EXTCODE_LITTLE_ADD       0x4
#define EXTCODE_BOUNDED_ADD      0x5
#define EXTCODE_WRAP_ADD         0x6

#define ACK_COMPLETE             0x1
#define ACK_PENDING              0x2
#define ACK_BUSY_X               0x4
#define ACK_BUSY_A               0x5
#define ACK_BUSY_B               0x6
#define ACK_TARDY                0xb
#define ACK_CONFLICT_ERROR       0xc
#define ACK_DATA_ERROR           0xd
#define ACK_TYPE_ERROR           0xe
#define ACK_ADDRESS_ERROR        0xf

/* Non-standard "ACK codes" for internal use */
#define ACKX_NONE                (-1)
#define ACKX_SEND_ERROR          (-2)
#define ACKX_ABORTED             (-3)
#define ACKX_TIMEOUT             (-4)


#define IEEE1394_SPEED_100		0x00
#define IEEE1394_SPEED_200		0x01
#define IEEE1394_SPEED_400		0x02
#define IEEE1394_SPEED_800		0x03
#define IEEE1394_SPEED_1600		0x04
#define IEEE1394_SPEED_3200		0x05
/* The current highest tested speed supported by the subsystem */
#define IEEE1394_SPEED_MAX		IEEE1394_SPEED_800

/* Maps speed values above to a string representation */
extern const char *hpsb_speedto_str[];


#define SELFID_PWRCL_NO_POWER    0x0
#define SELFID_PWRCL_PROVIDE_15W 0x1
#define SELFID_PWRCL_PROVIDE_30W 0x2
#define SELFID_PWRCL_PROVIDE_45W 0x3
#define SELFID_PWRCL_USE_1W      0x4
#define SELFID_PWRCL_USE_3W      0x5
#define SELFID_PWRCL_USE_6W      0x6
#define SELFID_PWRCL_USE_10W     0x7

#define SELFID_PORT_CHILD        0x3
#define SELFID_PORT_PARENT       0x2
#define SELFID_PORT_NCONN        0x1
#define SELFID_PORT_NONE         0x0

#pragma pack(1)
typedef struct
{
    UCHAR phy_id:6;
    UCHAR packet_identifier:2; /* always binary 10 */
    /* byte */
    UCHAR gap_count:6;
    UCHAR link_active:1;
    UCHAR extended:1; /* if true is struct ext_selfid */
    /* byte */
    UCHAR power_class:3;
    UCHAR contender:1;
    UCHAR phy_delay:2;
    UCHAR speed:2;
    /* byte */
    UCHAR more_packets:1;
    UCHAR initiated_reset:1;
    UCHAR port2:2;
    UCHAR port1:2;
    UCHAR port0:2;
} SELFID;
typedef SELFID far * PSELFID;
typedef struct
{
    UCHAR phy_id:6;
    UCHAR packet_identifier:2; /* always binary 10 */
    /* byte */
    UCHAR porta:2;
    UCHAR reserved:2;
    UCHAR seq_nr:3;
    UCHAR extended:1; /* if false is struct selfid */
    /* byte */
    UCHAR porte:2;
    UCHAR portd:2;
    UCHAR portc:2;
    UCHAR portb:2;
    /* byte */
    UCHAR more_packets:1;
    UCHAR reserved2:1;
    UCHAR porth:2;
    UCHAR portg:2;
    UCHAR portf:2;
} EXTSELFID;
typedef EXTSELFID far * PEXTSELFID;

enum DEVCTL_CMD
{
    /* Host is requested to reset its bus and cancel all outstanding async
     * requests.  If arg == 1, it shall also attempt to become root on the
     * bus.  Return void. */
    RESET_BUS,

    /* Arg is void, return value is the hardware cycle counter value. */
    GET_CYCLE_COUNTER,

    /* Set the hardware cycle counter to the value in arg, return void.
     * FIXME - setting is probably not required. */
    SET_CYCLE_COUNTER,

    /* Configure hardware for new bus ID in arg, return void. */
    SET_BUS_ID,

    /* If arg true, start sending cycle start packets, stop if arg == 0.
     * Return void. */
    ACT_CYCLE_MASTER,

    /* Cancel all outstanding async requests without resetting the bus.
     * Return void. */
    CANCEL_REQUESTS,

    /* Start or stop receiving isochronous channel in arg.  Return void.
     * This acts as an optimization hint, hosts are not required not to
     * listen on unrequested channels. */
    ISO_LISTEN_CHANNEL,
    ISO_UNLISTEN_CHANNEL
};

enum ISOCTL_CMD
{
    /* rawiso API - see iso.h for the meanings of these commands
       (they correspond exactly to the hpsb_iso_* API functions)
     * INIT = allocate resources
     * START = begin transmission/reception
     * STOP = halt transmission/reception
     * QUEUE/RELEASE = produce/consume packets
     * SHUTDOWN = deallocate resources
     */

    XMIT_INIT,
    XMIT_START,
    XMIT_STOP,
    XMIT_QUEUE,
    XMIT_SHUTDOWN,

    RECV_INIT,
    RECV_LISTEN_CHANNEL,   /* multi-channel only */
    RECV_UNLISTEN_CHANNEL, /* multi-channel only */
    RECV_SET_CHANNEL_MASK, /* multi-channel only; arg is a *u64 */
    RECV_START,
    RECV_STOP,
    RECV_RELEASE,
    RECV_SHUTDOWN,
    RECV_FLUSH
};

enum RESET_TYPES
{
    /* 166 microsecond reset -- only type of reset available on
       non-1394a capable IEEE 1394 controllers */
    LONG_RESET,

    /* Short (arbitrated) reset -- only available on 1394a capable
       IEEE 1394 capable controllers */
    SHORT_RESET,

    /* Variants, that set force_root before issueing the bus reset */
    LONG_RESET_FORCE_ROOT, SHORT_RESET_FORCE_ROOT,

    /* Variants, that clear force_root before issueing the bus reset */
    LONG_RESET_NO_FORCE_ROOT, SHORT_RESET_NO_FORCE_ROOT
};

void saveall(void);
#pragma aux saveall = \
	"push eax" \
    "push ebx" \
	"push ecx" \
	"push edx" \
	"push ebp" \
	"push esi" \
	"push edi" \
	"push es"  \
	"push fs"  \
	"push gs"  \
   parm nomemory \
   modify nomemory exact [];
void loadall(void);
#pragma aux loadall = \
    "pop gs" \
    "pop fs" \
    "pop es" \
    "pop edi" \
    "pop esi" \
    "pop ebp" \
    "pop edx" \
    "pop ecx"  \
    "pop ebx"  \
    "pop eax"  \
   parm nomemory \
   modify nomemory exact [];

#endif  /*__FIRETYPE_H*/

