/*
** Module   :SBP2.H
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Thu  05/08/2004	Created
**
*/
#ifndef __SBP2_H
#define __SBP2_H

#include "firetype.h"

#define SBP2_DEVICE_NAME		"sbp2"

#define SBP2_MAX_SECTORS		255	/* Max sectors supported */
/*
 * SBP2 specific structures and defines
 */
#define ORB_DIRECTION_WRITE_TO_MEDIA    0x0
#define ORB_DIRECTION_READ_FROM_MEDIA   0x1
#define ORB_DIRECTION_NO_DATA_TRANSFER  0x2

#define ORB_SET_NULL_PTR(value)				((((ULONG)value) & 0x1) << 31)
#define ORB_SET_NOTIFY(value)           	((((ULONG)value) & 0x1) << 31)
#define ORB_SET_RQ_FMT(value)           	((((ULONG)value) & 0x3) << 29)	/* unused ? */
#define ORB_SET_NODE_ID(value)				((((ULONG)value) & 0xffff) << 16)
#define ORB_SET_DATA_SIZE(value)            (((ULONG)value) & 0xffff)
#define ORB_SET_PAGE_SIZE(value)            ((((ULONG)value) & 0x7) << 16)
#define ORB_SET_PAGE_TABLE_PRESENT(value)   ((((ULONG)value) & 0x1) << 19)
#define ORB_SET_MAX_PAYLOAD(value)          ((((ULONG)value) & 0xf) << 20)
#define ORB_SET_SPEED(value)                ((((ULONG)value) & 0x7) << 24)
#define ORB_SET_DIRECTION(value)            ((((ULONG)value) & 0x1) << 27)

//command ORB definition
struct SBP2_COMMAND_ORB
{
	volatile ULONG ulNextORBhi;
	volatile ULONG ulNextORBlo;
	ULONG ulDataDescriptorHi;
	ULONG ulDataDescriptorLo;
	ULONG ulMisc;
	UCHAR ucCDB[12];
};

#define LOGIN_REQUEST			0x0
#define QUERY_LOGINS_REQUEST   	0x1
#define RECONNECT_REQUEST		0x3
#define SET_PASSWORD_REQUEST   	0x4
#define LOGOUT_REQUEST			0x7
#define ABORT_TASK_REQUEST		0xb
#define ABORT_TASK_SET			0xc
#define LOGICAL_UNIT_RESET		0xe
#define TARGET_RESET_REQUEST   	0xf

#define ORB_SET_LUN(value)                      (((ULONG)value) & 0xffff)
#define ORB_SET_FUNCTION(value)                 ((((ULONG)value) & 0xf) << 16)
#define ORB_SET_RECONNECT(value)                ((((ULONG)value) & 0xf) << 20)
#define ORB_SET_EXCLUSIVE(value)                ((((ULONG)value) & 0x1) << 28)
#define ORB_SET_LOGIN_RESP_LENGTH(value)        (((ULONG)value) & 0xffff)
#define ORB_SET_PASSWD_LENGTH(value)            ((((ULONG)value) & 0xffff) << 16)

//login ORB def
struct SBP2_LOGIN_ORB
{
	ULONG ulPasswordHi;
	ULONG ulPasswordLo;
	ULONG ulLoginResponseHi;
	ULONG ulLoginResponseLo;
	ULONG ulLunMisc;
	ULONG ulPasswdRespLengths;
	ULONG ulStatusFIFOHi;
	ULONG ulStatusFIFOLo;
};

#define RESPONSE_GET_LOGIN_ID(value)            (value & 0xffff)
#define RESPONSE_GET_LENGTH(value)              ((value >> 16) & 0xffff)
#define RESPONSE_GET_RECONNECT_HOLD(value)      (value & 0xffff)

//login response
struct SBP2_LOGIN_RESPONSE
{
	ULONG ulLengthLoginID;
	ULONG ulCommandBlockAgentHi;
	ULONG ulCommandBlockAgentLo;
	ULONG ulReconnectHold;
};

#define ORB_SET_LOGIN_ID(value)                 (value & 0xffff)

#define ORB_SET_QUERY_LOGINS_RESP_LENGTH(value) (value & 0xffff)

struct SBP2_QUERY_LOGINS_ORB
{
	ULONG ulReserved1;
	ULONG ulReserved2;
	ULONG ulQueryResponseHi;
	ULONG ulQueryResponseLo;
	ULONG ulLunMisc;
	ULONG ulReservedRespLength;
	ULONG ulStatusFIFOHi;
	ULONG ulStatusFIFOLo;
};


#define RESPONSE_GET_MAX_LOGINS(value)          (value & 0xffff)
#define RESPONSE_GET_ACTIVE_LOGINS(value)       ((RESPONSE_GET_LENGTH(value) - 4) / 12)

struct SBP2_QUERY_LOGINS_RESPONSE
{
	ULONG ulLengthMaxLogins;
	ULONG ulMiscIDs;
	ULONG ulInitiatorMiscHi;
	ULONG ulInitiatorMiscLo;
};

struct SBP2_RECONNECT_ORB
{
	ULONG ulReserved1;
	ULONG ulReserved2;
    ULONG ulReserved3;
    ULONG ulReserved4;
	ULONG ulLoginIDMisc;
	ULONG ulReserved5;
	ULONG ulStatusFIFOHi;
	ULONG ulStatusFIFOLo;
};

struct SBP2_LOGOUT_ORB
{
	ULONG ulReserved1;
	ULONG ulReserved2;
    ULONG ulReserved3;
    ULONG ulReserved4;
	ULONG ulLoginIDMisc;
	ULONG ulReserved5;
	ULONG ulStatusFIFOHi;
	ULONG ulStatusFIFOLo;
};

#define PAGE_TABLE_SET_SEGMENT_BASE_HI(value)   (value & 0xffff)
#define PAGE_TABLE_SET_SEGMENT_LENGTH(value)    ((((ULONG)value) & 0xffff) << 16)

struct SBP2_UNRESTRICTED_PAGE_TABLE
{
	ULONG ulLengthSegmentBaseHi;
	ULONG ulSegmentBaseLo;
};

#define RESP_STATUS_REQUEST_COMPLETE		0x0
#define RESP_STATUS_TRANSPORT_FAILURE		0x1
#define RESP_STATUS_ILLEGAL_REQUEST			0x2
#define RESP_STATUS_VENDOR_DEPENDENT		0x3

#define SBP2_STATUS_NO_ADDITIONAL_INFO		0x0
#define SBP2_STATUS_REQ_TYPE_NOT_SUPPORTED	0x1
#define SBP2_STATUS_SPEED_NOT_SUPPORTED		0x2
#define SBP2_STATUS_PAGE_SIZE_NOT_SUPPORTED	0x3
#define SBP2_STATUS_ACCESS_DENIED			0x4
#define SBP2_STATUS_LU_NOT_SUPPORTED		0x5
#define SBP2_STATUS_MAX_PAYLOAD_TOO_SMALL	0x6
#define SBP2_STATUS_RESERVED				0x7
#define SBP2_STATUS_RESOURCES_UNAVAILABLE	0x8
#define SBP2_STATUS_FUNCTION_REJECTED		0x9
#define SBP2_STATUS_LOGIN_ID_NOT_RECOGNIZED	0xa
#define SBP2_STATUS_DUMMY_ORB_COMPLETED		0xb
#define SBP2_STATUS_REQUEST_ABORTED			0xc
#define SBP2_STATUS_UNSPECIFIED_ERROR		0xff

#define SFMT_CURRENT_ERROR					0x0
#define SFMT_DEFERRED_ERROR		  		  	0x1
#define SFMT_VENDOR_DEPENDENT_STATUS		0x3

#define SBP2_SCSI_STATUS_GOOD					0x0
#define SBP2_SCSI_STATUS_CHECK_CONDITION		0x2
#define SBP2_SCSI_STATUS_CONDITION_MET			0x4
#define SBP2_SCSI_STATUS_BUSY					0x8
#define SBP2_SCSI_STATUS_RESERVATION_CONFLICT	0x18
#define SBP2_SCSI_STATUS_COMMAND_TERMINATED		0x22

#define SBP2_SCSI_STATUS_SELECTION_TIMEOUT		0xff

#define STATUS_GET_ORB_OFFSET_HI(value)         (value & 0xffff)
#define STATUS_GET_SBP_STATUS(value)            ((value >> 16) & 0xff)
#define STATUS_GET_LENGTH(value)                ((value >> 24) & 0x7)
#define STATUS_GET_DEAD_BIT(value)              ((value >> 27) & 0x1)
#define STATUS_GET_RESP(value)                  ((value >> 28) & 0x3)
#define STATUS_GET_SRC(value)                   ((value >> 30) & 0x3)

struct SBP2_STATUS_BLOCK
{
	ULONG ulORBOffsetHiMisc;
	ULONG ulORBOffsetLo;
    UCHAR ucCommandSetDependent[24];
};

/*
 * Miscellaneous SBP2 related config rom defines
 */

/* The status fifo address definition below is used as a base for each
 * node, which a chunk seperately assigned to each unit directory in the
 * node.  For example, 0xfffe00000000ULL is used for the first sbp2 device
 * detected on node 0, 0xfffe00000020ULL for the next sbp2 device on node
 * 0, and so on.
 *
 * Note: We could use a single status fifo address for all sbp2 devices,
 * and figure out which sbp2 device the status belongs to by looking at
 * the source node id of the status write... but, using separate addresses
 * for each sbp2 unit directory allows for better code and the ability to
 * support multiple luns within a single 1394 node.
 *
 * Also note that we choose the address range below as it is a region
 * specified for write posting, where the ohci controller will
 * automatically send an ack_complete when the status is written by the
 * sbp2 device... saving a split transaction.   =)
 */
#define SBP2_STATUS_FIFO_ADDRESS				0xfffe00000000ULL
#define SBP2_STATUS_FIFO_ADDRESS_HI             0xfffe
#define SBP2_STATUS_FIFO_ADDRESS_LO             0x0

#define SBP2_STATUS_FIFO_ENTRY_TO_OFFSET(entry)			((entry) << 5)
#define SBP2_STATUS_FIFO_OFFSET_TO_ENTRY(offset)		((offset) >> 5)

#define SBP2_UNIT_DIRECTORY_OFFSET_KEY			0xd1
#define SBP2_CSR_OFFSET_KEY				   		0x54
#define SBP2_UNIT_SPEC_ID_KEY					0x12
#define SBP2_UNIT_SW_VERSION_KEY				0x13
#define SBP2_COMMAND_SET_SPEC_ID_KEY			0x38
#define SBP2_COMMAND_SET_KEY					0x39
#define SBP2_UNIT_CHARACTERISTICS_KEY			0x3a
#define SBP2_DEVICE_TYPE_AND_LUN_KEY			0x14
#define SBP2_FIRMWARE_REVISION_KEY				0x3c

#define SBP2_DEVICE_TYPE(q)					(((q) >> 16) & 0x1f)
#define SBP2_DEVICE_LUN(q)					((q) & 0xffff)

#define SBP2_AGENT_STATE_OFFSET					0x00ULL
#define SBP2_AGENT_RESET_OFFSET					0x04ULL
#define SBP2_ORB_POINTER_OFFSET					0x08ULL
#define SBP2_DOORBELL_OFFSET					0x10ULL
#define SBP2_UNSOLICITED_STATUS_ENABLE_OFFSET	0x14ULL
#define SBP2_UNSOLICITED_STATUS_VALUE			0xf

#define SBP2_BUSY_TIMEOUT_ADDRESS				0xfffff0000210ULL
#define SBP2_BUSY_TIMEOUT_VALUE					0xf

#define SBP2_AGENT_RESET_DATA					0xf

/*
 * Unit spec id and sw version entry for SBP-2 devices
 */

#define SBP2_UNIT_SPEC_ID_ENTRY					0x0000609e
#define SBP2_SW_VERSION_ENTRY					0x00010483

/* This should be safe */
#define SBP2_MAX_CMDS       8

/* This is the two dma types we use for cmd_dma below */
enum cmd_dma_types {
    CMD_DMA_NONE,
    CMD_DMA_PAGE,
    CMD_DMA_SINGLE
};

/*
 * Encapsulates all the info necessary for an outstanding command.
 */
 #define SG_ALL 0xff
#pragms pack(4);
struct SBP2_COMMAND_INFO {

//    struct list_head list;
    struct SBP2_COMMAND_ORB command_orb;
    // ____cacheline_aligned;
    ULONG command_orb_dma;
    // ____cacheline_aligned;
    Scsi_Cmnd *Current_SCpnt;
    void (*Current_done)(Scsi_Cmnd *);

    /* Also need s/g structure for each sbp2 command */
    struct SBP2_UNRESTRICTED_PAGE_TABLE scatter_gather_element[SG_ALL];
    // ____cacheline_aligned;
    ULONG sge_dma;
    // ____cacheline_aligned;
    void far *sge_buffer;
    ULONG cmd_dma;
    enum cmd_dma_types dma_type;
    unsigned long dma_size;
    int dma_dir;

};
#pragma pack();

/*
 * Other misc defines
 */
#define SBP2_128KB_BROKEN_FIRMWARE				0xa0b800

#define SBP2_DEVICE_TYPE_LUN_UNINITIALIZED			0xffffffff

struct SBP2SCSI_HOST_INFO;



/* A list of flags for detected oddities and brokeness. */
#define SBP2_BREAKAGE_128K_MAX_TRANSFER		0x1
#define SBP2_BREAKAGE_INQUIRY_HACK		0x2

/*
 * Information needed on a per scsi id basis (one for each sbp2 device)
 */
struct SCSI_ID_INSTANCE_DATA {
    /*
     * Various sbp2 specific structures
     */
    struct SBP2_COMMAND_ORB far *last_orb;
    ULONG last_orb_dma;
    struct SBP2_LOGIN_ORB far *login_orb;
    ULONG login_orb_dma;
    struct SBP2_LOGIN_RESPONSE far *login_response;
	ULONG login_response_dma;
    struct SBP2_QUERY_LOGINS_ORB far *query_logins_orb;
    ULONG query_logins_orb_dma;
    struct SBP2_QUERY_LOGINS_RESPONSE far *query_logins_response;
    ULONG query_logins_response_dma;
    struct SBP2_RECONNECT_ORB far *reconnect_orb;
    ULONG reconnect_orb_dma;
    struct SBP2_LOGOUT_ORB far *logout_orb;
    ULONG logout_orb_dma;
    struct SBP2_STATUS_BLOCK status_block;

    /*
     * Stuff we need to know about the sbp2 device itself
     */
    U64 sbp2_management_agent_addr;
    U64 sbp2_command_block_agent_addr;
    U32 speed_code;
    U32 max_payload_size;

    /*
     * Values pulled from the device's unit directory
     */
    U32 sbp2_command_set_spec_id;
    U32 sbp2_command_set;
    U32 sbp2_unit_characteristics;
    U32 sbp2_device_type_and_lun;
    U32 sbp2_firmware_revision;

    /*
     * Variable used for logins, reconnects, logouts, query logins
     */
     //atomic
    int sbp2_login_complete;

    /*
     * Pool of command orbs, so we can have more than overlapped command per id
     */
//    spinlock_t sbp2_command_orb_lock;
    LIST sbp2_command_orb_inuse;
    LIST sbp2_command_orb_completed;

    struct list_head scsi_list;

    /* Node entry, as retrieved from NodeMgr entries */
    NODE_ENTRY far *ne;
    UNIT_DIRECTORY far *ud;

    /* A backlink to our host_info */
    struct SBP2SCSI_HOST_INFO far *hi;

    /* SCSI related pointers */
//    struct scsi_device *sdev;
//    struct Scsi_Host *scsi_host;

    /* Device specific workarounds/brokeness */
    U32 workarounds;
};

/* Sbp2 host data structure (one per IEEE1394 host) */
struct SBP2SCSI_HOST_INFO {
    HOSTOPS far * pHostOps;     /* IEEE1394 host */
  	SCSIIDLIST ScsiIdList;
//    struct list_head scsi_ids;  /* List of scsi ids on this host */
};


#endif  /*__SBP2_H*/

