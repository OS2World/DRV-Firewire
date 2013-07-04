/*
** Module   :SBP2.CPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  27/10/2004	Created
**
*/

/*
 * Export information about protocols/devices supported by this driver.
 */
struct ieee1394_device_id sbp2_id_table[] = {
    {
        IEEE1394_MATCH_SPECIFIER_ID |
        IEEE1394_MATCH_VERSION, //match_flags
        0, //vendor id
        0, //model id
        SBP2_UNIT_SPEC_ID_ENTRY & 0xffffff, //specifier_id
        SBP2_SW_VERSION_ENTRY & 0xffffff //version
    },
    { }
};
struct HIGHLEVELOPS sbp2_highlevel =
{
	SBP2_DEVICE_NAME, //name
	NULL, //pHighlevel
    NULL, //add_host
	NULL, //remove_host,
    sbp2_host_reset,
    NULL, //iso_receive
    NULL, //fcprequest
    NULL, // gethostinfo
    NULL  //destroyhostinfo
};

struct ADDROPS sbp2_ops =
{
	NULL,  //read
    sbp2_handle_status_write,       //write
    NULL,       //lock
    NULL        //lock64
};

#ifdef CONFIG_IEEE1394_SBP2_PHYS_DMA
struct ADDROPS sbp2_physdma_ops =
{
	sbp2_handle_physdma_read,  //read
    sbp2_handle_physdma_write,       //write
    NULL,       //lock
    NULL        //lock64
};
#endif

struct PROTOCOL_DRIVER sbp2_driver =
{
	"SBP2 Driver", //name
	sbp2_id_table, //id_table
	sbp2_update, //update
	NULL, 		//suspend
	// linux LDM not finished yet, FIXME
//	.driver		= {
//		.name		= SBP2_DEVICE_NAME,
//		.bus		= &ieee1394_bus_type,
//		.probe		= sbp2_probe,
//		.remove		= sbp2_remove,
//	},
};

/*
 * Debug levels, configured via kernel config, or enable here.
 */

/* #define CONFIG_IEEE1394_SBP2_DEBUG_ORBS */
/* #define CONFIG_IEEE1394_SBP2_DEBUG_DMA */
/* #define CONFIG_IEEE1394_SBP2_DEBUG 1 */
/* #define CONFIG_IEEE1394_SBP2_DEBUG 2 */
/* #define CONFIG_IEEE1394_SBP2_PACKET_DUMP */

#ifdef CONFIG_IEEE1394_SBP2_DEBUG_ORBS
#define SBP2_ORB_DEBUG(fmt, args...)    ddprintf("error: sbp2(%s): "fmt, __FUNCTION__, ## args)
ULONG global_outstanding_command_orbs = 0;
#define outstanding_orb_incr global_outstanding_command_orbs++
#define outstanding_orb_decr global_outstanding_command_orbs--
#else
#define SBP2_ORB_DEBUG(fmt, args...)
#define outstanding_orb_incr
#define outstanding_orb_decr
#endif

#ifdef CONFIG_IEEE1394_SBP2_DEBUG_DMA
#define SBP2_DMA_ALLOC(fmt, args...) \
    ddprintf("error: sbp2(%s)alloc(%d): "fmt, __FUNCTION__, \
         ++global_outstanding_dmas, ## args)
#define SBP2_DMA_FREE(fmt, args...) \
    ddprintf("error: sbp2(%s)free(%d): "fmt, __FUNCTION__, \
         --global_outstanding_dmas, ## args)
static u32 global_outstanding_dmas = 0;
#else
#define SBP2_DMA_ALLOC(fmt, args...)
#define SBP2_DMA_FREE(fmt, args...)
#endif

#if CONFIG_IEEE1394_SBP2_DEBUG >= 2
#define SBP2_DEBUG(fmt, args...)    ddprintf("error: sbp2: "fmt, ## args)
#define SBP2_INFO(fmt, args...)     ddprintf("error: sbp2: "fmt, ## args)
#define SBP2_NOTICE(fmt, args...)   ddprintf("error: sbp2: "fmt, ## args)
#define SBP2_WARN(fmt, args...)     ddprintf("error: sbp2: "fmt, ## args)
#elif CONFIG_IEEE1394_SBP2_DEBUG == 1
#define SBP2_DEBUG(fmt, args...)    ddprintf("debug: sbp2: "fmt, ## args)
#define SBP2_INFO(fmt, args...)     ddprintf("info: sbp2: "fmt, ## args)
#define SBP2_NOTICE(fmt, args...)   ddprintf("notice: sbp2: "fmt, ## args)
#define SBP2_WARN(fmt, args...)     ddprintf("warn: sbp2: "fmt, ## args)
#else
#define SBP2_DEBUG(fmt, args...)
#define SBP2_INFO(fmt, args...)     ddprintf("info: sbp2: "fmt, ## args)
#define SBP2_NOTICE(fmt, args...)   ddprintf("notice: sbp2: "fmt, ## args)
#define SBP2_WARN(fmt, args...)     ddprintf("warn: sbp2: "fmt, ## args)
#endif

#define SBP2_ERR(fmt, args...)      ddprintf("error: sbp2: "fmt, ## args)


/* List of device firmware's that require a forced 36 byte inquiry.  */
ULONG sbp2_broken_inquiry_list[] = {
    0x00002800, /* Stefan Richter <richtest@bauwesen.tu-cottbus.de> */
            /* DViCO Momobay CX-1 */
    0x00000200  /* Andreas Plesch <plesch@fas.harvard.edu> */
            /* QPS Fire DVDBurner */
};

#define NUM_BROKEN_INQUIRY_DEVS \
    (sizeof(sbp2_broken_inquiry_list)/sizeof(*sbp2_broken_inquiry_list))


/**************************************
 * General utility functions
 **************************************/

/*
 * Converts a buffer from be32 to cpu byte ordering. Length is in bytes.
 */
static __inline__ void sbp2util_be32_to_cpu_buffer(void *buffer, int length)
{
    u32 *temp = buffer;

    for (length = (length >> 2); length--; )
        temp[length] = be32_to_cpu(temp[length]);

    return;
}

/*
 * Converts a buffer from cpu to be32 byte ordering. Length is in bytes.
 */
static __inline__ void sbp2util_cpu_to_be32_buffer(void *buffer, int length)
{
    u32 *temp = buffer;

    for (length = (length >> 2); length--; )
        temp[length] = cpu_to_be32(temp[length]);

    return;
}

#ifdef CONFIG_IEEE1394_SBP2_PACKET_DUMP
/*
 * Debug packet dump routine. Length is in bytes.
 */
static void sbp2util_packet_dump(void far *buffer, int length, char far *dump_name, ULONG dump_phys_addr)
{
    int i;
    unsigned char far *dump = buffer;

    if (!dump || !length || !dump_name)
        return;

    if (dump_phys_addr)
        ddprintf("[%s, 0x%x]", dump_name, dump_phys_addr);
    else
        ddprintf("[%s]", dump_name);
    for (i = 0; i < length; i++) {
        if (i > 0x3f) {
            ddprintf("\n   ...");
            break;
        }
        if ((i & 0x3) == 0)
            ddprintf("  ");
        if ((i & 0xf) == 0)
            ddprintf("\n   ");
        ddprintf("%02x ", (int) dump[i]);
    }
    ddprintf("\n");

    return;
}
#else
#define sbp2util_packet_dump(w,x,y,z)
#endif

/*
 * Goofy routine that basically does a down_timeout function.
 */
static int sbp2util_down_timeout(atomic_t *done, int timeout)
{
    int i;

    for (i = timeout; (i > 0 && atomic_read(done) == 0); i-= HZ/10) {
        set_current_state(TASK_INTERRUPTIBLE);
        if (schedule_timeout(HZ/10))    /* 100ms */
            return(1);
    }
    return ((i > 0) ? 0:1);
}

/* Free's an allocated packet */
void sbp2_free_packet(struct hpsb_packet *packet)
{
    packet->FreeTLabel();
    delete packet;
}

/* This is much like hpsb_node_write(), except it ignores the response
 * subaction and returns immediately. Can be used from interrupts.
 */
int sbp2util_node_write_no_wait(struct node_entry *ne, u64 addr,
                quadlet_t *buffer, size_t length)
{
    struct hpsb_packet *packet;

    packet = hpsb_make_writepacket(ne->host, ne->nodeid,
                       addr, buffer, length);
        if (!packet)
                return -ENOMEM;

    hpsb_set_packet_complete_task(packet, (void (*)(void*))sbp2_free_packet,
                      packet);

    hpsb_node_fill_packet(ne, packet);

        if (hpsb_send_packet(packet) < 0) {
        sbp2_free_packet(packet);
        return -EIO;
    }

    return 0;
}

/*
 * This function is called to create a pool of command orbs used for
 * command processing. It is called when a new sbp2 device is detected.
 */
static int sbp2util_create_command_orb_pool(struct scsi_id_instance_data *scsi_id)
{
    struct sbp2scsi_host_info *hi = scsi_id->hi;
    int i;
    unsigned long flags, orbs;
    struct sbp2_command_info *command;

    orbs = serialize_io ? 2 : SBP2_MAX_CMDS;

    spin_lock_irqsave(&scsi_id->sbp2_command_orb_lock, flags);
    for (i = 0; i < orbs; i++) {
        command = (struct sbp2_command_info *)
            kmalloc(sizeof(struct sbp2_command_info), GFP_ATOMIC);
        if (!command) {
            spin_unlock_irqrestore(&scsi_id->sbp2_command_orb_lock, flags);
            return(-ENOMEM);
        }
        memset(command, '\0', sizeof(struct sbp2_command_info));
        command->command_orb_dma =
            pci_map_single (hi->host->pdev, &command->command_orb,
                    sizeof(struct sbp2_command_orb),
                    PCI_DMA_BIDIRECTIONAL);
        SBP2_DMA_ALLOC("single command orb DMA");
        command->sge_dma =
            pci_map_single (hi->host->pdev, &command->scatter_gather_element,
                    sizeof(command->scatter_gather_element),
                    PCI_DMA_BIDIRECTIONAL);
        SBP2_DMA_ALLOC("scatter_gather_element");
        INIT_LIST_HEAD(&command->list);
        list_add_tail(&command->list, &scsi_id->sbp2_command_orb_completed);
    }
    spin_unlock_irqrestore(&scsi_id->sbp2_command_orb_lock, flags);
    return 0;
}

/*
 * This function is called to delete a pool of command orbs.
 */
static void sbp2util_remove_command_orb_pool(struct scsi_id_instance_data *scsi_id)
{
    struct hpsb_host *host = scsi_id->hi->host;
    struct list_head *lh, *next;
    struct sbp2_command_info *command;
    unsigned long flags;

    spin_lock_irqsave(&scsi_id->sbp2_command_orb_lock, flags);
    if (!list_empty(&scsi_id->sbp2_command_orb_completed)) {
        list_for_each_safe(lh, next, &scsi_id->sbp2_command_orb_completed) {
            command = list_entry(lh, struct sbp2_command_info, list);

            /* Release our generic DMA's */
            pci_unmap_single(host->pdev, command->command_orb_dma,
                     sizeof(struct sbp2_command_orb),
                     PCI_DMA_BIDIRECTIONAL);
            SBP2_DMA_FREE("single command orb DMA");
            pci_unmap_single(host->pdev, command->sge_dma,
                     sizeof(command->scatter_gather_element),
                     PCI_DMA_BIDIRECTIONAL);
            SBP2_DMA_FREE("scatter_gather_element");

            kfree(command);
        }
    }
    spin_unlock_irqrestore(&scsi_id->sbp2_command_orb_lock, flags);
    return;
}

/*
 * This function finds the sbp2_command for a given outstanding command
 * orb.Only looks at the inuse list.
 */
static struct sbp2_command_info *sbp2util_find_command_for_orb(
        struct scsi_id_instance_data *scsi_id, dma_addr_t orb)
{
    struct sbp2_command_info *command;
    unsigned long flags;

    spin_lock_irqsave(&scsi_id->sbp2_command_orb_lock, flags);
    if (!list_empty(&scsi_id->sbp2_command_orb_inuse)) {
        list_for_each_entry(command, &scsi_id->sbp2_command_orb_inuse, list) {
            if (command->command_orb_dma == orb) {
                spin_unlock_irqrestore(&scsi_id->sbp2_command_orb_lock, flags);
                return (command);
            }
        }
    }
    spin_unlock_irqrestore(&scsi_id->sbp2_command_orb_lock, flags);

    SBP2_ORB_DEBUG("could not match command orb %x", (unsigned int)orb);

    return(NULL);
}

/*
 * This function finds the sbp2_command for a given outstanding SCpnt.
 * Only looks at the inuse list.
 */
static struct sbp2_command_info *sbp2util_find_command_for_SCpnt(struct scsi_id_instance_data *scsi_id, void *SCpnt)
{
    struct sbp2_command_info *command;
    unsigned long flags;

    spin_lock_irqsave(&scsi_id->sbp2_command_orb_lock, flags);
    if (!list_empty(&scsi_id->sbp2_command_orb_inuse)) {
        list_for_each_entry(command, &scsi_id->sbp2_command_orb_inuse, list) {
            if (command->Current_SCpnt == SCpnt) {
                spin_unlock_irqrestore(&scsi_id->sbp2_command_orb_lock, flags);
                return (command);
            }
        }
    }
    spin_unlock_irqrestore(&scsi_id->sbp2_command_orb_lock, flags);
    return(NULL);
}

/*
 * This function allocates a command orb used to send a scsi command.
 */
static struct sbp2_command_info *sbp2util_allocate_command_orb(
        struct scsi_id_instance_data *scsi_id,
        Scsi_Cmnd *Current_SCpnt,
        void (*Current_done)(Scsi_Cmnd *))
{
    struct list_head *lh;
    struct sbp2_command_info *command = NULL;
    unsigned long flags;

    spin_lock_irqsave(&scsi_id->sbp2_command_orb_lock, flags);
    if (!list_empty(&scsi_id->sbp2_command_orb_completed)) {
        lh = scsi_id->sbp2_command_orb_completed.next;
        list_del(lh);
        command = list_entry(lh, struct sbp2_command_info, list);
        command->Current_done = Current_done;
        command->Current_SCpnt = Current_SCpnt;
        list_add_tail(&command->list, &scsi_id->sbp2_command_orb_inuse);
    } else {
        SBP2_ERR("sbp2util_allocate_command_orb - No orbs available!");
    }
    spin_unlock_irqrestore(&scsi_id->sbp2_command_orb_lock, flags);
    return (command);
}

/* Free our DMA's */
static void sbp2util_free_command_dma(struct sbp2_command_info *command)
{
    struct scsi_id_instance_data *scsi_id =
        (struct scsi_id_instance_data *)command->Current_SCpnt->device->host->hostdata[0];
    struct hpsb_host *host;

    if (!scsi_id) {
        printk(KERN_ERR "%s: scsi_id == NULL\n", __FUNCTION__);
        return;
    }

    host = scsi_id->ud->ne->host;

    if (command->cmd_dma) {
        if (command->dma_type == CMD_DMA_SINGLE) {
            pci_unmap_single(host->pdev, command->cmd_dma,
                     command->dma_size, command->dma_dir);
            SBP2_DMA_FREE("single bulk");
        } else if (command->dma_type == CMD_DMA_PAGE) {
            pci_unmap_page(host->pdev, command->cmd_dma,
                       command->dma_size, command->dma_dir);
            SBP2_DMA_FREE("single page");
        } /* XXX: Check for CMD_DMA_NONE bug */
        command->dma_type = CMD_DMA_NONE;
        command->cmd_dma = 0;
    }

    if (command->sge_buffer) {
        pci_unmap_sg(host->pdev, command->sge_buffer,
                 command->dma_size, command->dma_dir);
        SBP2_DMA_FREE("scatter list");
        command->sge_buffer = NULL;
    }
}

/*
 * This function moves a command to the completed orb list.
 */
static void sbp2util_mark_command_completed(struct scsi_id_instance_data *scsi_id, struct sbp2_command_info *command)
{
    unsigned long flags;

    spin_lock_irqsave(&scsi_id->sbp2_command_orb_lock, flags);
    list_del(&command->list);
    sbp2util_free_command_dma(command);
    list_add_tail(&command->list, &scsi_id->sbp2_command_orb_completed);
    spin_unlock_irqrestore(&scsi_id->sbp2_command_orb_lock, flags);
}


/**************************************
 * SBP-2 protocol related section
 **************************************/

/*
 * This function determines if we should convert scsi commands for a particular sbp2 device type
 */
int SBP2::command_conversion_device_type(UCHAR device_type)
{
    return (((device_type == TYPE_DISK) ||
         (device_type == TYPE_SDAD) ||
         (device_type == TYPE_ROM)) ? 1:0);
}

/*
 * This function queries the device for the maximum concurrent logins it
 * supports.
 */
int SBP2::query_logins(struct SCSI_ID_INSTANCE_DATA far *scsi_id)
{
    struct SBP2SCSI_HOST_INFO far *hi = scsi_id->hi;
    QUADLET data[2];
    int max_logins;
    int active_logins;

    SBP2_DEBUG("sbp2_query_logins");

    scsi_id->query_logins_orb->ulReserved1 = 0x0;
    scsi_id->query_logins_orb->ulRreserved2 = 0x0;

    scsi_id->query_logins_orb->ulQueryResponseLo = scsi_id->query_logins_response_dma;
    scsi_id->query_logins_orb->ulQueryResponseHi = ORB_SET_NODE_ID(hi->host->node_id);
    SBP2_DEBUG("sbp2_query_logins: query_response_hi/lo initialized");

    scsi_id->query_logins_orb->ulLunMisc = ORB_SET_FUNCTION(QUERY_LOGINS_REQUEST);
    scsi_id->query_logins_orb->ulLunMisc |= ORB_SET_NOTIFY(1);
    if (scsi_id->sbp2_device_type_and_lun != SBP2_DEVICE_TYPE_LUN_UNINITIALIZED) {
        scsi_id->query_logins_orb->ulLunMisc |= ORB_SET_LUN(scsi_id->sbp2_device_type_and_lun);
        SBP2_DEBUG("sbp2_query_logins: set lun to %d",
               ORB_SET_LUN(scsi_id->sbp2_device_type_and_lun));
    }
    SBP2_DEBUG("sbp2_query_logins: lun_misc initialized");

    scsi_id->query_logins_orb->ulReservedRespLength =
        ORB_SET_QUERY_LOGINS_RESP_LENGTH(sizeof(struct SBP2_QUERY_LOGINS_RESPONSE));
    SBP2_DEBUG("sbp2_query_logins: reserved_resp_length initialized");

    scsi_id->query_logins_orb->ulStatusFIFOLo = SBP2_STATUS_FIFO_ADDRESS_LO +
                            SBP2_STATUS_FIFO_ENTRY_TO_OFFSET(scsi_id->ud->id);
    scsi_id->query_logins_orb->ulStatusFIFOHi = (ORB_SET_NODE_ID(hi->host->node_id) |
                             SBP2_STATUS_FIFO_ADDRESS_HI);
    SBP2_DEBUG("sbp2_query_logins: status FIFO initialized");

    sbp2util_cpu_to_be32_buffer(scsi_id->query_logins_orb, sizeof(struct SBP2_QUERY_LOGINS_ORB));

    SBP2_DEBUG("sbp2_query_logins: orb byte-swapped");

    sbp2util_packet_dump(scsi_id->query_logins_orb, sizeof(struct SBP2_QUERY_LOGINS_ORB),
                 "sbp2 query logins orb", scsi_id->query_logins_orb_dma);

    memset(scsi_id->query_logins_response, 0, sizeof(struct SBP2_QUERY_LOGINS_RESPONSE));
    memset(&scsi_id->status_block, 0, sizeof(struct SBP2_STATUS_BLOCK));

    SBP2_DEBUG("sbp2_query_logins: query_logins_response/status FIFO memset");

    data[0] = ORB_SET_NODE_ID(hi->host->node_id);
    data[1] = scsi_id->query_logins_orb_dma;
    sbp2util_cpu_to_be32_buffer(data, 8);

//    atomic_set(&scsi_id->sbp2_login_complete, 0);
	pushf();
	cli();
	&scsi_id->sbp2_login_complete=0;
	popf();

    SBP2_DEBUG("sbp2_query_logins: prepared to write");
    hpsb_node_write(scsi_id->ne, scsi_id->sbp2_management_agent_addr, data, 8);
    SBP2_DEBUG("sbp2_query_logins: written");

    if (sbp2util_down_timeout(&scsi_id->sbp2_login_complete, 2*HZ)) {
        SBP2_INFO("Error querying logins to SBP-2 device - timed out");
        return(-EIO);
    }

    if (scsi_id->status_block.ORB_offset_lo != scsi_id->query_logins_orb_dma) {
        SBP2_INFO("Error querying logins to SBP-2 device - timed out");
        return(-EIO);
    }

    if (STATUS_GET_RESP(scsi_id->status_block.ORB_offset_hi_misc) ||
        STATUS_GET_DEAD_BIT(scsi_id->status_block.ORB_offset_hi_misc) ||
        STATUS_GET_SBP_STATUS(scsi_id->status_block.ORB_offset_hi_misc)) {

        SBP2_INFO("Error querying logins to SBP-2 device - timed out");
        return(-EIO);
    }

    sbp2util_cpu_to_be32_buffer(scsi_id->query_logins_response, sizeof(struct sbp2_query_logins_response));

    SBP2_DEBUG("length_max_logins = %x",
           (unsigned int)scsi_id->query_logins_response->length_max_logins);

    SBP2_DEBUG("Query logins to SBP-2 device successful");

    max_logins = RESPONSE_GET_MAX_LOGINS(scsi_id->query_logins_response->length_max_logins);
    SBP2_DEBUG("Maximum concurrent logins supported: %d", max_logins);

    active_logins = RESPONSE_GET_ACTIVE_LOGINS(scsi_id->query_logins_response->length_max_logins);
    SBP2_DEBUG("Number of active logins: %d", active_logins);

    if (active_logins >= max_logins) {
        return(-EIO);
    }

    return 0;
}

int SBP2_ModuleInit(void)
{
	int ret;

	SBP2_DEBUG("sbp2_module_init");

	ddprintf("INFO :sbp2: %s\n", version);

	/* Module load debug option to force one command at a time (serializing I/O) */
//	if (serialize_io) {
//		SBP2_ERR("Driver forced to serialize I/O (serialize_io = 1)");
//		scsi_driver_template.can_queue = 1;
//		scsi_driver_template.cmd_per_lun = 1;
//	}

	/* Set max sectors (module load option). Default is 255 sectors. */
//	scsi_driver_template.max_sectors = max_sectors;


	/* Register our high level driver with 1394 stack */
	fpGlobalStackOps->fpfnRegisterHighlevel(&sbp2_highlevel);

	ret = fpGlobalNodeMgrOps->fpfnRegisterProtocol(&sbp2_driver);
	if (ret) {
		SBP2_ERR("Failed to register protocol");
		fpGlobalStackOps->fpfnUnregisterHighlevel(&sbp2_highlevel);
		return ret;
	}

	return 0;
}
void SBP2_ModuleExit(void)
{
    SBP2_DEBUG("sbp2_module_exit");

    fpGlobalNodeMgrOps->fpfnUnregisterProtocol(&sbp2_driver);

    fpGlobalStackOps->fpfnUnregisterHighlevel(&sbp2_highlevel);
}

