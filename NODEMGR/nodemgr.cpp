/*
** Module   :NODEMGR.CPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  07/07/2004 Created
**
*/
extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define INCL_TYPES
#define  INCL_DOSDEVICES
#define  INCL_DOSDEVIOCTL
#define  INCL_NOXLATE_DOS16
#include <os2.h>

}
#include "globalmgr.hpp"
#include "nodemgr.hpp"
#include "ddprintf.h"
#include <string.h>
#include <devhelp.h>
#include <include.h>

HOSTINFO::HOSTINFO(PHOSTOPS pHostOps)
{
    HOSTINFO::pHostOps=pHostOps;
}

char *FindOuiName(int oui)
{
#ifdef CONFIG_IEEE1394_OUI_DB
    extern struct oui_list_struct {
        int oui;
        char *name;
    } oui_list[];
    int i;

    for (i = 0; oui_list[i].name; i++)
        if (oui_list[i].oui == oui)
            return oui_list[i].name;
#endif
    return NULL;
}


int far nodemgr_bus_read(struct csr1212_csr far *csr, U64 addr, USHORT length,
                            void far *buffer, void far *__ci)
{
    struct NODEMGR_CSR_INFO far *ci = (struct NODEMGR_CSR_INFO far *)__ci;
    int i, ret = 0;

    for (i = 0; i < 3; i++)
    {
        ret = ci->pHostOps->fpfnRead(ci->pHostOps,ci->nodeid, ci->generation, addr,
                (PQUADLET)buffer, length);
        if (!ret)
            break;

//        set_current_state(TASK_INTERRUPTIBLE);
        //        DevIODelay(300);
        DevHelp_ProcBlock((ULONG)&ret,300,WAIT_IS_INTERRUPTABLE);
//        if (schedule_timeout (HZ/3))
//            return -EINTR;
    }

    return ret;
}

int far nodemgr_get_max_rom(QUADLET far *bus_info_data, void far *__ci)
{
    return (CSR1212_BE32_TO_CPU(bus_info_data[2]) >> 8) & 0x3;
}

static struct csr1212_bus_ops nodemgr_csr_ops =
{
    nodemgr_bus_read,
    NULL,
    NULL,
    nodemgr_get_max_rom
};


void NODEMGR::Init(void)
{
//    class_register(&nodemgr_ne_class);
//    class_register(&nodemgr_ud_class);
    pStack->fpfnRegisterHighlevel(&GlobalNodeMgrOps);
}
void NODEMGR::Cleanup(void)
{
    pStack->fpfnUnRegisterHighlevel(&GlobalNodeMgrOps);

//    class_unregister(&nodemgr_ud_class);
//    class_unregister(&nodemgr_ne_class);
}
void NODEMGR::AddHost(HOSTOPS far *pHost)
{
//    struct host_info *hi;
    HOSTINFO * hi;

//    hi = hpsb_create_hostinfo(&nodemgr_highlevel, host, sizeof(*hi));
    hi=new HOSTINFO(pHost);

    if (!hi)
    {
        ddprintf("ERROR: NodeMgr: out of memory in add host\n");
        return;
    }

    hi->pHostOps = pHost;
//    init_completion(&hi->exited);
//    hi->exited.Init();
//        sema_init(&hi->reset_sem, 0);
//    hi->ResetSem.Init(0);
    hi->Kill=0; //thread running


//    sprintf(hi->daemon_name, "knodemgrd_%d", host->id);
    strcpy(hi->daemon_name, "nodemgrd_");
    hi->daemon_name[9]=(char)(((HOST far *)pHost->pHost)->usId+0x30);
    hi->daemon_name[10]=0;

//    hi->pid = kernel_thread(nodemgr_host_thread, hi, CLONE_KERNEL);
    DevHelp_AllocateCtxHook((NPFN)NodemgrHostCtxRoutine,&hi->pid);
    ddprintf("nodemgr hook allocated\n");
//    DevHelp_ArmCtxHook((ULONG)hi,hi->pid);

//    if (hi->pid < 0) {
//        HPSB_ERR ("NodeMgr: failed to start %s thread for %s",
//              hi->daemon_name, host->driver->name);
//        hpsb_destroy_hostinfo(&nodemgr_highlevel, host);
//        return;
//    }
    HostInfoList.AddTail(hi);
    return;
}

void NODEMGR::HostReset(HOSTOPS far *pHost)
{
    HOSTINFO far * hi;
    hi=HostInfoList.GetHostInfo(pHost);

    if (hi != NULL)
    {
        ddprintf("VERBOSE: NodeMgr: Processing host reset for %s\n", (char far *)hi->daemon_name);
//        ((HOSTINFO *)hi)->ResetSem.Up();
		pushf();
		cli();
		ResetCount++;
		popf();
        DevHelp_ArmCtxHook((ULONG)hi,hi->pid);
        ddprintf("nodemgr hook armed\n");
    }
    else ddprintf("ERROR: NodeMgr: could not process reset of unused host\n");

    return;
}

void NODEMGR::RemoveHost(HOSTOPS far *pHost)
{
    HOSTINFO far * hi;
    hi=HostInfoList.GetHostInfo(pHost);

    if (hi)
    {
//        if (hi->pid >= 0) {
//            kill_proc(hi->pid, SIGTERM, 1);
//        hi->Kill=1; //signal to thread to exit
//        ((HOSTINFO *)hi)->ResetSem.Up(); // run thread
//        ((HOSTINFO *)hi)->exited.Waitfor();// wait to thread finished
        DevHelp_FreeCtxHook(hi->pid);
//        RemoveHostDev();//remove host from nodelist
     }
     else
        ddprintf("ERROR: NodeMgr: host %s does not exist, cannot remove\n",(char far *)pHost->pHost->pHardDriver->name);

    return;
}
extern "C" void far NodemgrHostCtxRoutine(void *__hi)
#pragma aux  NodemgrHostCtxRoutine parm [ax] modify exact [];
{
    saveall();
    pushf();
    cli();
    if (bInNodeMgr)
    {
    	popf();
    	loadall();
    	return;
    }
    else
    {
    	bInNodeMgr=TRUE;
    	popf();
    }
    HOSTINFO *hi = (HOSTINFO *) __hi;
    int reset_cycles = 0;

    /* No userlevel access needed */
//    daemonize(hi->daemon_name);

    /* Setup our device-model entries */
//    nodemgr_create_host_dev_files(host);
//    pGlobalNodeMgr->CreateHostDev(hi->pHost);
    while (ResetCount>0)
    {
        /* Sit and wait for a signal to probe the nodes on the bus. This
         * happens when we get a bus reset. */
//        hi->ResetSem.DownWait(-1);
//        gNodemgrSerialize.DownWait(-1);
        // we are unblocked
//    while (!down_interruptible(&hi->reset_sem) &&
//           !down_interruptible(&nodemgr_serialize)) {
        unsigned int generation = 0;
        int i;

        /* Pause for 1/4 second in 1/16 second intervals,
         * to make sure things settle down. */
//        for (i = 0; i < 4 ; i++)
//        {
//            set_current_state(TASK_INTERRUPTIBLE);
//            if (hi->Kill==1) //exititng
//            {
//                gNodemgrSerialize.Up();
//                goto caught_signal;
//            }
//            DevHelp_ProcBlock((ULONG)&hi->Kill,250,WAIT_IS_INTERRUPTABLE);
//            if (schedule_timeout(HZ/16)) {
//                up(&nodemgr_serialize);
//                goto caught_signal;


            /* Now get the generation in which the node ID's we collect
             * are valid.  During the bus scan we will use this generation
             * for the read transactions, so that if another reset occurs
             * during the scan the transactions will fail instead of
             * returning bogus data. */
//            generation = get_hpsb_generation(host);
            generation=hi->pHostOps->fpfnGetGeneration(hi->pHostOps);

            /* If we get a reset before we are done waiting, then
             * start the the waiting over again */
//            while (!down_trylock(&hi->reset_sem))
//                i = 0;

//        }
        if (!pGlobalNodeMgr->CheckIrmCapability(hi->pHostOps, reset_cycles))
        {
            reset_cycles++;
//            gNodemgrSerialize.Up();
            continue;
        }

        /* Scan our nodes to get the bus options and create node
         * entries. This does not do the sysfs stuff, since that
         * would trigger hotplug callbacks and such, which is a
         * bad idea at this point. */
        pGlobalNodeMgr->NodeScan(hi, generation);
        if (!pGlobalNodeMgr->DoIrmDuties(hi, reset_cycles))
        {
            reset_cycles++;
//            gNodemgrSerialize.Up();
            continue;
        }

        reset_cycles = 0;

        /* This actually does the full probe, with sysfs
         * registration. */
        pGlobalNodeMgr->NodeProbe(hi, generation);

        /* Update some of our sysfs symlinks */
//        pGlobalNodeMgr->UpdateHostDevLinks(hi->pHost);

//        gNodemgrSerialize.Up();
        ddprintf("nodemgr hook finished\n");
        pushf();
        cli();
        ResetCount--;
        popf();
//        loadall();
//        return;
    }

//caught_signal:
//    ddprintf("VERBOSE: NodeMgr: Exiting thread\n");

//    hi->exited.Complete();
//    ddprintf("nodemgr hook finished2\n");
	pushf();
	cli();
	bInNodeMgr=FALSE;
	popf();
    loadall();
}
/* We need to ensure that if we are not the IRM, that the IRM node is capable of
 * everything we can do, otherwise issue a bus reset and try to become the IRM
 * ourselves. */
BOOL NODEMGR::CheckIrmCapability(HOSTOPS far *host, int cycles)
{
    QUADLET bc;
    int status;

    if (host->pHost->bIsIrm)
        return TRUE;

    status = host->fpfnRead(host,LOCAL_BUS | (host->pHost->IRMId),
               host->fpfnGetGeneration(host),
               (CSR_REGISTER_BASE | CSR_BROADCAST_CHANNEL),
               &bc, sizeof(QUADLET));

    if (status < 0 || !(be32_to_cpu(bc) & 0x80000000))
    {
        /* The current irm node does not have a valid BROADCAST_CHANNEL
         * register and we do, so reset the bus with force_root set */
        ddprintf("DEBUG: Current remote IRM is not 1394a-2000 compliant, resetting...\n");

        if (cycles >= 5) {
            /* Oh screw it! Just leave the bus as it is */
            ddprintf("DEBUG: Stopping reset loop for IRM sanity\n");
            return TRUE;
        }

        host->fpfnSendPhyConfig(host,NODEID_TO_NODE(host->pHost->NodeId), -1);
        host->fpfnResetBus(host,LONG_RESET_FORCE_ROOT);

        return FALSE;
    }

    return TRUE;
}
/* Because we are a 1394a-2000 compliant IRM, we need to inform all the other
 * nodes of the broadcast channel.  (Really we're only setting the validity
 * bit). Other IRM responsibilities go in here as well. */
BOOL NODEMGR::DoIrmDuties(HOSTINFO far *hi, int cycles)
{
    QUADLET bc;
    HOSTOPS far * host=hi->pHostOps;
    /* if irm_id == -1 then there is no IRM on this bus */
    if (!host->pHost->bIsIrm || host->pHost->IRMId == (NODEID)-1)
        return TRUE;

    host->pHost->pCSR->BroadcastChannel |= 0x40000000;  /* set validity bit */

    bc = cpu_to_be32(host->pHost->pCSR->BroadcastChannel);

    host->fpfnWrite(host,LOCAL_BUS | ALL_NODES, host->fpfnGetGeneration(host),
           (CSR_REGISTER_BASE | CSR_BROADCAST_CHANNEL),
           &bc, sizeof(QUADLET));

    /* If there is no bus manager then we should set the root node's
     * force_root bit to promote bus stability per the 1394
     * spec. (8.4.2.6) */
    if (host->pHost->BusMgrId == 0xffff && host->pHost->usNodeCount > 1)
    {
        USHORT root_node = host->pHost->usNodeCount - 1;
        struct NODE_ENTRY far *ne = ((HOSTINFO *)hi)->NodeEntryList.FindByNodeid(root_node | LOCAL_BUS);

        if (ne && ne->busopt.cmc)
            host->fpfnSendPhyConfig(host,root_node, -1);
        else
        {
            ddprintf("DEBUG: The root node is not cycle master capable; "
                   "selecting a new root node and resetting...\n");

            if (cycles >= 5)
            {
                /* Oh screw it! Just leave the bus as it is */
                ddprintf("DEBUG: Stopping reset loop for IRM sanity\n");
                return TRUE;
            }

            host->fpfnSendPhyConfig(host,NODEID_TO_NODE(host->pHost->NodeId), -1);
            host->fpfnResetBus(host,LONG_RESET_FORCE_ROOT);

            return FALSE;
        }
    }

    return TRUE;
}
void NODEMGR::NodeScan(struct HOSTINFO far *hi, int generation)
{
    int count;
    HOSTOPS far *host =hi->pHostOps;
    SELFID far *sid = (SELFID far *)host->pHost->pTopologyMap;
    NODEID nodeid = LOCAL_BUS;

    /* Scan each node on the bus */
    for (count = host->pHost->usSelfIdCount; count; count--, sid++)
    {
        if (sid->extended) continue;

        if (!sid->link_active)
        {
            nodeid++;
            continue;
        }
        NodeScanOne(hi, nodeid++, generation);
    }
}
void NODEMGR::NodeScanOne(HOSTINFO far *hi,
                  NODEID nodeid, int generation)
{
    HOSTOPS far *host = hi->pHostOps;
    struct NODE_ENTRY far *ne;
    OCTLET guid;
    struct csr1212_csr far *csr;
    struct NODEMGR_CSR_INFO far *ci;

    ci =(struct NODEMGR_CSR_INFO far *) malloc(sizeof(struct NODEMGR_CSR_INFO));
    if (!ci)
        return;

    ci->pHostOps = host;
    ci->nodeid = nodeid;
    ci->generation = generation;

    /* We need to detect when the ConfigROM's generation has changed,
     * so we only update the node's info when it needs to be.  */

    csr = csr1212_create_csr(&nodemgr_csr_ops, 5 * sizeof(QUADLET), ci);
    if (!csr || csr1212_parse_csr(csr) != CSR1212_SUCCESS)
    {
        ddprintf("ERROR: Error parsing configrom for node " NODE_BUS_FMT "\n",
             NODE_BUS_ARGS(host->pHost, nodeid));
        if (csr)
            csr1212_destroy_csr(csr);
        free(ci);
        return;
    }

    if (csr->bus_info_data[1] != IEEE1394_BUSID_MAGIC)
    {
        /* This isn't a 1394 device, but we let it slide. There
         * was a report of a device with broken firmware which
         * reported '2394' instead of '1394', which is obviously a
         * mistake. One would hope that a non-1394 device never
         * gets connected to Firewire bus. If someone does, we
         * shouldn't be held responsible, so we'll allow it with a
         * warning.  */
        ddprintf("WARNING: Node " NODE_BUS_FMT " has invalid busID magic [0x%lx]\n",
              NODE_BUS_ARGS(host->pHost, nodeid), csr->bus_info_data[1]);
    }

    guid = (((U64)be32_to_cpu(csr->bus_info_data[3])) << 32) | be32_to_cpu(csr->bus_info_data[4]);
    ne = ((HOSTINFO *)hi)->NodeEntryList.FindByGuid(guid);

    if (ne && ne->pHostOps != host && ne->bInLimbo)
    {
        /* Must have moved this device from one host to another */
        RemoveNe(hi, ne);
        ne = NULL;
    }

    if (!ne)
        CreateNode(guid, csr, hi, nodeid, generation);
    else
        UpdateNode(ne, csr, hi, nodeid, generation);

    return;
}

void NODEMGR::NodeProbe(HOSTINFO far *hi, int generation)
{
    HOST far *host = (HOST far *)hi->pHostOps->pHost;
    NODE_ENTRY far * TempNE;
    //struct class *class = &nodemgr_ne_class;
    //struct class_device *cdev;

    /* Do some processing of the nodes we've probed. This pulls them
     * into the sysfs layer if needed, and can result in processing of
     * unit-directories, or just updating the node and it's
     * unit-directories. */
//    down_read(&class->subsys.rwsem);
//    list_for_each_entry(cdev, &class->children, node)
//        nodemgr_probe_ne(hi, container_of(cdev, struct node_entry, class_dev), generation);
    TempNE=((HOSTINFO *)hi)->NodeEntryList.GetHead();
    while (TempNE!=NULL)
    {
        ProbeNE(hi,TempNE,generation);
        TempNE=((HOSTINFO *)hi)->NodeEntryList.GetNext();
    }
//        up_read(&class->subsys.rwsem);


    /* If we had a bus reset while we were scanning the bus, it is
     * possible that we did not probe all nodes.  In that case, we
     * skip the clean up for now, since we could remove nodes that
     * were still on the bus.  The bus reset increased hi->reset_sem,
     * so there's a bus scan pending which will do the clean up
     * eventually.
     *
     * Now let's tell the bus to rescan our devices. This may seem
     * like overhead, but the driver-model core will only scan a
     * device for a driver when either the device is added, or when a
     * new driver is added. A bus reset is a good reason to rescan
     * devices that were there before.  For example, an sbp2 device
     * may become available for login, if the host that held it was
     * just removed.  */

    if (generation == hi->pHostOps->fpfnGetGeneration(hi->pHostOps))
//        RescanBus();
//        bus_rescan_devices(&ieee1394_bus_type);

    return;
}
void NODEMGR::ProbeNE(HOSTINFO far *hi, NODE_ENTRY far *ne, int generation)
{
//    struct device *dev;

    if (ne->pHostOps != hi->pHostOps || ne->bInLimbo)
        return;

//    dev = get_device(&ne->device);
//    if (!dev)
//        return;

    /* If "needs_probe", then this is either a new or changed node we
     * rescan totally. If the generation matches for an existing node
     * (one that existed prior to the bus reset) we send update calls
     * down to the drivers. Otherwise, this is a dead node and we
     * suspend it. */
    if (ne->needs_probe)
        ProcessRootDirectory(hi, ne);
    else
        if (ne->generation == generation)
            UpdatePdrv(ne);
        else
            SuspendNE(ne);

//    put_device(dev);
}
void NODEMGR::SuspendNE(NODE_ENTRY far *ne)
{
//    struct class_device *cdev;
    UNIT_DIRECTORY far *ud;

    ddprintf("DEBUG:Node suspended: ID:BUS[" NODE_BUS_FMT "]  GUID[%0lx%0lx]\n",
           NODE_BUS_ARGS(ne->pHostOps->pHost, ne->NodeId), (ULONG)(ne->guid>>32),(ULONG)(ne->guid&0xffffffff));

    ne->bInLimbo=TRUE;
//    device_create_file(&ne->device, &dev_attr_ne_in_limbo);

//    down_write(&ne->device.bus->subsys.rwsem);
    ud=((NODE_ENTRY *)ne)->UnitDirsList.GetHead();
    while (ud!=NULL)
    {
//    list_for_each_entry(cdev, &nodemgr_ud_class.children, node) {
//        ud = container_of(cdev, struct unit_directory, class_dev);

//        if (ud->ne != ne)
            //continue;
        //if protocol driver has suspend func implemented?
//        if (ud->pProtocolDriver &&
//            (ud->pProtocolDriver->fpfnSuspend!=NULL ||
//              ud->pProtocolDriver->fpfnSuspend()))
//            device_release_driver(&ud->device);
//		ReleaseUD(ud);
        ud=((NODE_ENTRY *)ne)->UnitDirsList.GetNext();
    }
//    up_write(&ne->device.bus->subsys.rwsem);
}
void NODEMGR::UpdatePdrv(NODE_ENTRY far *ne)
{
    UNIT_DIRECTORY far *ud;
    PROTOCOL_DRIVER far *pdrv;
//    struct class *class = &nodemgr_ud_class;
//    struct class_device *cdev;

//    down_read(&class->subsys.rwsem);
//    list_for_each_entry(cdev, &class->children, node) {
//            ud = container_of(cdev, struct unit_directory, class_dev);
    ud=((NODE_ENTRY *)ne)->UnitDirsList.GetHead();
    while (ud!=NULL)
    {
        if (!ud->pProtocolDriver)
        {
        	ud=((NODE_ENTRY *)ne)->UnitDirsList.GetNext();
            continue;
        }

//        pdrv = container_of(ud->device.driver, struct hpsb_protocol_driver, driver);
        pdrv=ud->pProtocolDriver;

        if (pdrv->fpfnUpdate && pdrv->fpfnUpdate(ud))
        {
//            down_write(&ud->device.bus->subsys.rwsem);
//            device_release_driver(&ud->device);
//            up_write(&ud->device.bus->subsys.rwsem);
        }
        ud=((NODE_ENTRY *)ne)->UnitDirsList.GetNext();
    }
//    up_read(&class->subsys.rwsem);
}
/* This implementation currently only scans the config rom and its
 * immediate unit directories looking for software_id and
 * software_version entries, in order to get driver autoloading working. */

UNIT_DIRECTORY far *NODEMGR::ProcessUnitDirectory
    (HOSTINFO far *hi, NODE_ENTRY far *ne, struct csr1212_keyval far *ud_kv,
     unsigned int far *id, UNIT_DIRECTORY far *parent)
{
    UNIT_DIRECTORY far *ud;
    UNIT_DIRECTORY far *ud_temp = NULL;
    struct csr1212_dentry far *dentry;
    struct csr1212_keyval far *kv;
    UCHAR last_key_id = 0;

    ud =(UNIT_DIRECTORY *)malloc(sizeof(UNIT_DIRECTORY));
    if (!ud)
        goto unit_directory_error;

    _fmemset (ud, 0, sizeof(UNIT_DIRECTORY));

    ud->ne = ne;
    ud->bIgnoreDriver=ignore_drivers;
    ud->address = ud_kv->offset + CSR1212_CONFIG_ROM_SPACE_BASE;
    ud->ud_kv = ud_kv;
    ud->id = (*id)++;

    csr1212_for_each_dir_entry(ne->csr, kv, ud_kv, dentry) {
        switch (kv->key.id) {
        case CSR1212_KV_ID_VENDOR:
            if (kv->key.type == CSR1212_KV_TYPE_IMMEDIATE) {
                ud->vendor_id = kv->value.immediate;
                ud->flags |= UNIT_DIRECTORY_VENDOR_ID;

                if (ud->vendor_id)
                    ud->vendor_oui = FindOuiName(ud->vendor_id);
            }
            break;

        case CSR1212_KV_ID_MODEL:
            ud->model_id = kv->value.immediate;
            ud->flags |= UNIT_DIRECTORY_MODEL_ID;
            break;

        case CSR1212_KV_ID_SPECIFIER_ID:
            ud->specifier_id = kv->value.immediate;
            ud->flags |= UNIT_DIRECTORY_SPECIFIER_ID;
            break;

        case CSR1212_KV_ID_VERSION:
            ud->version = kv->value.immediate;
            ud->flags |= UNIT_DIRECTORY_VERSION;
            break;

        case CSR1212_KV_ID_DESCRIPTOR:
            if (kv->key.type == CSR1212_KV_TYPE_LEAF &&
                CSR1212_DESCRIPTOR_LEAF_TYPE(kv) == 0 &&
                CSR1212_DESCRIPTOR_LEAF_SPECIFIER_ID(kv) == 0 &&
                CSR1212_TEXTUAL_DESCRIPTOR_LEAF_WIDTH(kv) == 0 &&
                CSR1212_TEXTUAL_DESCRIPTOR_LEAF_CHAR_SET(kv) == 0 &&
                CSR1212_TEXTUAL_DESCRIPTOR_LEAF_LANGUAGE(kv) == 0) {
                switch (last_key_id) {
                case CSR1212_KV_ID_VENDOR:
                    ud->vendor_name_kv = kv;
                    csr1212_keep_keyval(kv);
                    break;

                case CSR1212_KV_ID_MODEL:
                    ud->model_name_kv = kv;
                    csr1212_keep_keyval(kv);
                    break;

                }
            } /* else if (kv->key.type == CSR1212_KV_TYPE_DIRECTORY) ... */
            break;

        case CSR1212_KV_ID_DEPENDENT_INFO:
            if (kv->key.type == CSR1212_KV_TYPE_DIRECTORY) {
                /* This should really be done in SBP2 as this is
                 * doing SBP2 specific parsing. */
                ud->flags |= UNIT_DIRECTORY_HAS_LUN_DIRECTORY;
                ud_temp = ProcessUnitDirectory(hi, ne, kv, id,
                                     parent);

                if (ud_temp == NULL)
                    break;

                /* inherit unspecified values */
                if ((ud->flags & UNIT_DIRECTORY_VENDOR_ID) &&
                    !(ud_temp->flags & UNIT_DIRECTORY_VENDOR_ID))
                {
                    ud_temp->flags |=  UNIT_DIRECTORY_VENDOR_ID;
                    ud_temp->vendor_id = ud->vendor_id;
                    ud_temp->vendor_oui = ud->vendor_oui;
                }
                if ((ud->flags & UNIT_DIRECTORY_MODEL_ID) &&
                    !(ud_temp->flags & UNIT_DIRECTORY_MODEL_ID))
                {
                    ud_temp->flags |=  UNIT_DIRECTORY_MODEL_ID;
                    ud_temp->model_id = ud->model_id;
                }
                if ((ud->flags & UNIT_DIRECTORY_SPECIFIER_ID) &&
                    !(ud_temp->flags & UNIT_DIRECTORY_SPECIFIER_ID))
                {
                    ud_temp->flags |=  UNIT_DIRECTORY_SPECIFIER_ID;
                    ud_temp->specifier_id = ud->specifier_id;
                }
                if ((ud->flags & UNIT_DIRECTORY_VERSION) &&
                    !(ud_temp->flags & UNIT_DIRECTORY_VERSION))
                {
                    ud_temp->flags |=  UNIT_DIRECTORY_VERSION;
                    ud_temp->version = ud->version;
                }
            }

            break;

        default:
            break;
        }
        last_key_id = kv->key.id;
    }

//  memcpy(&ud->device, &nodemgr_dev_template_ud,
//         sizeof(ud->device));

    if (parent) {
        ud->flags |= UNIT_DIRECTORY_LUN_DIRECTORY;
        ud->parent = parent;
    } else
        ud->parent = NULL;

//  snprintf(ud->device.bus_id, BUS_ID_SIZE, "%s-%u",
//       ne->device.bus_id, ud->id);

//  ud->class_dev.dev = &ud->device;
//  ud->class_dev.class = &nodemgr_ud_class;
//  snprintf(ud->class_dev.class_id, BUS_ID_SIZE, "%s-%u",
//       ne->device.bus_id, ud->id);

//  device_register(&ud->device);
//  class_device_register(&ud->class_dev);
//  get_device(&ud->device);

//  if (ud->vendor_oui)
//      device_create_file(&ud->device, &dev_attr_ud_vendor_oui);
//  nodemgr_create_ud_dev_files(ud);
    ((NODE_ENTRY *)ne)->UnitDirsList.AddTail(ud);
    return ud;

unit_directory_error:
    if (ud != NULL)
        free(ud);
    return NULL;
}

void NODEMGR::ProcessRootDirectory(HOSTINFO far *hi, NODE_ENTRY far *ne)
{
    unsigned int ud_id = 0;
    struct csr1212_dentry far *dentry;
    struct csr1212_keyval far *kv;
    UCHAR last_key_id = 0;

    ne->needs_probe = 0;

    csr1212_for_each_dir_entry(ne->csr, kv, ne->csr->root_kv, dentry)
    {
        switch (kv->key.id)
        {
        case CSR1212_KV_ID_VENDOR:
            ne->vendor_id = kv->value.immediate;

            if (ne->vendor_id)
                ne->vendor_oui = FindOuiName(ne->vendor_id);
            break;

        case CSR1212_KV_ID_NODE_CAPABILITIES:
            ne->capabilities = kv->value.immediate;
            break;

        case CSR1212_KV_ID_UNIT:
            ProcessUnitDirectory(hi, ne, kv, &ud_id, NULL);
            break;

        case CSR1212_KV_ID_DESCRIPTOR:
            if (last_key_id == CSR1212_KV_ID_VENDOR)
            {
                if (kv->key.type == CSR1212_KV_TYPE_LEAF &&
                    CSR1212_DESCRIPTOR_LEAF_TYPE(kv) == 0 &&
                    CSR1212_DESCRIPTOR_LEAF_SPECIFIER_ID(kv) == 0 &&
                    CSR1212_TEXTUAL_DESCRIPTOR_LEAF_WIDTH(kv) == 0 &&
                    CSR1212_TEXTUAL_DESCRIPTOR_LEAF_CHAR_SET(kv) == 0 &&
                    CSR1212_TEXTUAL_DESCRIPTOR_LEAF_LANGUAGE(kv) == 0)
                {
                    ne->vendor_name_kv = kv;
                    csr1212_keep_keyval(kv);
                }
            }
            break;
        }
        last_key_id = kv->key.id;
    }

//    if (ne->vendor_oui)
//        device_create_file(&ne->device, &dev_attr_ne_vendor_oui);
//    if (ne->vendor_name_kv)
//        device_create_file(&ne->device, &dev_attr_ne_vendor_name_kv);
}
NODE_ENTRY far * NODEMGR::CreateNode(OCTLET guid, struct csr1212_csr far *csr, HOSTINFO far *hi, NODEID nodeid, UINT generation)
{
    HOSTOPS far *hostop=hi->pHostOps;
    NODE_ENTRY *ne;

    ne =(NODE_ENTRY *)malloc(sizeof(NODE_ENTRY));
    if (!ne) return NULL;

    memset(ne, 0, sizeof(NODE_ENTRY));

    ne->tpool = &hostop->pHost->tpool[nodeid & NODE_MASK];

    ne->pHostOps = hostop;
    ne->NodeId = nodeid;
    ne->generation = generation;
    ne->needs_probe = 1;

    ne->guid = guid;
    ne->guid_vendor_id = (guid >> 40) & 0xffffff;
//    ne->guid_vendor_oui = nodemgr_find_oui_name(ne->guid_vendor_id);
    ne->csr = csr;

//    memcpy(&ne->device, &nodemgr_dev_template_ne,
//           sizeof(ne->device));
//    ne->device.parent = &host->device;
//    snprintf(ne->device.bus_id, BUS_ID_SIZE, "%016Lx",
//         (U64)(ne->guid));

//    ne->class_dev.dev = &ne->device;
//    ne->class_dev.class = &nodemgr_ne_class;
//    snprintf(ne->class_dev.class_id, BUS_ID_SIZE, "%016Lx",
//         (unsigned long long)(ne->guid));

//    device_register(&ne->device);
//    class_device_register(&ne->class_dev);
//    get_device(&ne->device);

//    if (ne->guid_vendor_oui)
//        device_create_file(&ne->device, &dev_attr_ne_guid_vendor_oui);
    //CreateRMNE(ne);

    UpdateBusOptions(ne);
    ((HOSTINFO *)hi)->NodeEntryList.AddTail(ne);
    ddprintf("DEBUG: %s added: ID:BUS[" NODE_BUS_FMT "] GUID[%0lx%0lx]\n",
           (char far *)((hostop->pHost->NodeId == nodeid) ? "Host" : "Node"),
           NODE_BUS_ARGS(hostop->pHost, nodeid), (ULONG)(guid>>32),(ULONG)(guid&0xffffffff));

    return ne;
}



/*
 * This function updates nodes that were present on the bus before the
 * reset and still are after the reset.  The nodeid and the config rom
 * may have changed, and the drivers managing this device must be
 * informed that this device just went through a bus reset, to allow
 * the to take whatever actions required.
 */
void NODEMGR::UpdateNode(NODE_ENTRY far *ne, struct csr1212_csr far *csr, HOSTINFO far *hi, NODEID nodeid, unsigned int generation)
{
    if (ne->NodeId != nodeid)
    {
        ddprintf("DEBUG: Node changed: " NODE_BUS_FMT " -> " NODE_BUS_FMT"\n",
               NODE_BUS_ARGS(ne->pHostOps->pHost, ne->NodeId),
               NODE_BUS_ARGS(ne->pHostOps->pHost, nodeid));
        ne->NodeId = nodeid;
    }

    if (ne->busopt.generation != ((be32_to_cpu(csr->bus_info_data[2]) >> 4) & 0xf))
    {
        free(ne->csr->pprivate);
        csr1212_destroy_csr(ne->csr);
        ne->csr = csr;

        /* If the node's configrom generation has changed, we
         * unregister all the unit directories. */
        RemoveUDs(ne);

        UpdateBusOptions(ne);

        /* Mark the node as new, so it gets re-probed */
        ne->needs_probe = 1;
    }

    if (ne->bInLimbo)
        ResumeNE(ne);

    /* Mark the node current */
    ne->generation = generation;
}

void NODEMGR::ResumeNE(NODE_ENTRY far *ne)
{
//    struct class_device *cdev;
    UNIT_DIRECTORY far *ud;

    ne->bInLimbo = FALSE;
//    device_remove_file(&ne->device, &dev_attr_ne_in_limbo);

//    down_read(&ne->device.bus->subsys.rwsem);
    ud=((NODE_ENTRY *)ne)->UnitDirsList.GetHead();
//    list_for_each_entry(cdev, &nodemgr_ud_class.children, node) {
//        ud = container_of(cdev, struct unit_directory, class_dev);
    while (ud!=NULL)
    {

//        if (ud->ne != ne)
//            /continue;

//        if (ud->device.driver && ud->device.driver->resume)
//            ud->device.driver->resume(&ud->device, 0);
        ud=((NODE_ENTRY *)ne)->UnitDirsList.GetNext();
    }
//    up_read(&ne->device.bus->subsys.rwsem);

    ddprintf("DEBUG: Node resumed: ID:BUS[" NODE_BUS_FMT "]  GUID[%lx%lx]\n",
           NODE_BUS_ARGS(ne->pHostOps->pHost, ne->NodeId), ne->guid>>32,ne->guid&0xffffffff);
}
void NODEMGR::UpdateBusOptions(NODE_ENTRY far *ne)
{
    static const USHORT mr[] = { 4, 64, 1024, 0};
    QUADLET busoptions = be32_to_cpu(ne->csr->bus_info_data[2]);

    ne->busopt.irmc         = (busoptions >> 31) & 1;
    ne->busopt.cmc          = (busoptions >> 30) & 1;
    ne->busopt.isc          = (busoptions >> 29) & 1;
    ne->busopt.bmc          = (busoptions >> 28) & 1;
    ne->busopt.pmc          = (busoptions >> 27) & 1;
    ne->busopt.cyc_clk_acc  = (busoptions >> 16) & 0xff;
    ne->busopt.max_rec      = 1UL << (((busoptions >> 12) & 0xf) + 1);
    ne->busopt.max_rom      = (busoptions >> 8) & 0x3;
    ne->busopt.generation   = (busoptions >> 4) & 0xf;
    ne->busopt.lnkspd       = busoptions & 0x7;

    ddprintf("VERBOSE: NodeMgr: raw=0x%0lx irmc=%d cmc=%d isc=%d bmc=%d pmc=%d "
             "cyc_clk_acc=%d max_rec=%d max_rom=%d gen=%d lspd=%d\n",
             busoptions, ne->busopt.irmc, ne->busopt.cmc,
             ne->busopt.isc, ne->busopt.bmc, ne->busopt.pmc,
             ne->busopt.cyc_clk_acc, ne->busopt.max_rec,
             mr[ne->busopt.max_rom],
             ne->busopt.generation, ne->busopt.lnkspd);
}

/* The following four convenience functions use a struct node_entry
 * for addressing a node on the bus.  They are intended for use by any
 * process context, not just the nodemgr thread, so we need to be a
 * little careful when reading out the node ID and generation.  The
 * thing that can go wrong is that we get the node ID, then a bus
 * reset occurs, and then we read the generation.  The node ID is
 * possibly invalid, but the generation is current, and we end up
 * sending a packet to a the wrong node.
 *
 * The solution is to make sure we read the generation first, so that
 * if a reset occurs in the process, we end up with a stale generation
 * and the transactions will fail instead of silently using wrong node
 * ID's.
 */

void NODEMGR::FillPacket(NODE_ENTRY far *ne, PACKET far *pkt)
{
    pkt->pHost = (HOST far *)ne->pHostOps->pHost;
    pkt->Generation = ne->generation;
//    barrier();
    pkt->NodeId = ne->NodeId;
}

int NODEMGR::Read(NODE_ENTRY far *ne, U64 addr,QUADLET far *buffer, int length)

{
    unsigned int generation = ne->generation;

//    barrier();
    return ne->pHostOps->fpfnRead(ne->pHostOps,ne->NodeId, generation,
             addr, buffer, length);
}

int NODEMGR::Write(NODE_ENTRY far *ne, U64 addr, QUADLET far *buffer, int length)
{
    unsigned int generation = ne->generation;

//    barrier();
    return ne->pHostOps->fpfnWrite(ne->pHostOps,ne->NodeId, generation,
              addr, buffer, length);
}

int NODEMGR::Lock(NODE_ENTRY far *ne, U64 addr, int extcode, QUADLET far *data, QUADLET arg)
{
    unsigned int generation = ne->generation;

//    barrier();
    return ne->pHostOps->fpfnLock(ne->pHostOps,ne->NodeId, generation,
             addr, extcode, data, arg);
}
void NODEMGR::RemoveNe(HOSTINFO far * hi, NODE_ENTRY far *ne)
{
//    struct device *dev = &ne->device;
//    dev = get_device(&ne->device);
//    if (!dev)
//        //return;

    ddprintf("DEBUG:Node removed: ID:BUS[" NODE_BUS_FMT "]  GUID[%016Lx]",
               NODE_BUS_ARGS(ne->pHostOps->pHost, ne->NodeId), ne->guid>>32,ne->guid&0xffffffff);

    RemoveUDs(ne);
    ReleaseNE(ne);
    ((HOSTINFO *)hi)->NodeEntryList.Remove(ne);

//    class_device_unregister(&ne->class_dev);
//    device_unregister(dev);

//    put_device(dev);
}
void NODEMGR::RemoveUDs(NODE_ENTRY far *ne)
{
    UNIT_DIRECTORY far *tempUD;

    tempUD=((NODE_ENTRY *)ne)->UnitDirsList.GetHead();
    while (tempUD!=NULL)
    {
        ReleaseUD(tempUD);
        ((NODE_ENTRY *)ne)->UnitDirsList.DeleteHead();
        tempUD=((NODE_ENTRY *)ne)->UnitDirsList.GetNext();
    }
}


/*struct NODE_ENTRY *NODEMGR::GuidGetEntry(U64 guid)
{
        struct NODE_ENTRY *ne;

    nodemgr_serialize.Down();
    ne=NodeEntryList.FindByGuid(guid);
    nodemgr_serialize.Up();

    return ne;
}

struct NODE_ENTRY *NODEMGR::NodeidGetEntry(HOST * host, NODEID nodeid)
{
    struct NODE_ENTRY *ne;

    nodemgr_serialize.Down();
    ne = NodeEntryList.FindByNodeId(host, nodeid);
    nodemgr_serialize.Up();

    return ne;
}

struct node_entry *NODEMGR::CheckNodeid(pHost nodeid_t nodeid)
{
    struct node_entry *ne;

    if (down_trylock(&nodemgr_serialize))
        return NULL;
    ne = find_entry_by_nodeid(nodeid);
    up(&nodemgr_serialize);

    return ne;
}
*/
void NODEMGR::ReleaseUD(UNIT_DIRECTORY far *ud)
{
    if (ud->vendor_name_kv)
        csr1212_release_keyval(ud->vendor_name_kv);
    if (ud->model_name_kv)
        csr1212_release_keyval(ud->model_name_kv);
    free(ud);
}
void NODEMGR::ReleaseNE(NODE_ENTRY far *ne)
{
    if (ne->vendor_name_kv)
        csr1212_release_keyval(ne->vendor_name_kv);

    free(ne);
}

