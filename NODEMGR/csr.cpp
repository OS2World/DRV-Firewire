extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define  INCL_NOPMAPI
#define  INCL_DOSDEVICES
#define  INCL_DOSDEVIOCTL
#define  INCL_DOSERRORS
#define  INCL_NOXLATE_DOS16
#include <os2.h>
}
#include "firetype.h"
#include "fireerror.h"
#include "csr.hpp"
//#include "..\host.hpp"
#include "jiffies.h"
#include "ddprintf.h"
#include "globalmgr.hpp"
static struct csr1212_keyval far *node_cap = NULL;

struct csr1212_bus_ops csr_bus_ops=
{
    NULL, //bus_read
    allocate_addr_range,
    release_addr_range,
    NULL  //get max rom
};

struct HIGHLEVELOPS csr_highlevel =
{
    "standard registers",//name
    NULL, //pHighlevel
    add_host,
    remove_host,
    host_reset,
    NULL, //iso_receive
    NULL, //fcprequest
    NULL, // gethostinfo
    NULL  //destroyhostinfo
};

struct ADDROPS map_ops =
{
    read_maps,  //read
    NULL,       //write
    NULL,       //lock
    NULL        //lock64
};

struct ADDROPS fcp_ops =
{
    NULL,       //read
    write_fcp,  //write
    NULL,       //lock
    NULL        //lock64
};

struct ADDROPS reg_ops =
{
    read_regs,
    write_regs,
    lock_regs,
    lock64_regs
};

struct ADDROPS config_rom_ops =
{
    read_config_rom,       //read
    NULL,       //write
    NULL,       //lock
    NULL        //lock64
};
USHORT csr_crc16(ULONG far *data, int length)
{
    int check=0, i;
    int shift, sum, next=0;

    for (i = length; i; i--) {
        for (next = check, shift = 28; shift >= 0; shift -= 4 )
        {
            sum = ((next >> 12) ^ (be32_to_cpu(*data) >> shift)) & 0xf;
            next = (next << 4) ^ (sum << 12) ^ (sum << 5) ^ (sum);
        }
        check = next & 0xffff;
        data++;
    }

    return check;
}

void far __loadds host_reset(HOSTOPS far *host)
{
    host->pHost->pCSR->state &= 0x300;

    host->pHost->pCSR->bus_manager_id = 0x3f;
    host->pHost->pCSR->BandwidthAvailable = 4915;
    host->pHost->pCSR->channels_available_hi = 0xfffffffe;   /* pre-alloc ch 31 per 1394a-2000 */
    host->pHost->pCSR->channels_available_lo = ~0;
    host->pHost->pCSR->BroadcastChannel = 0x80000000 | 31;

    if (host->pHost->bIsIrm)
    {
        if (host->pHost->pHardDriver->fpfnHwCsrReg)
        {
            host->pHost->pHardDriver->fpfnHwCsrReg(2, 0xfffffffe, ~0);
        }
    }

    host->pHost->pCSR->NodeIds = ((ULONG)host->pHost->NodeId) << 16;

    if (!host->pHost->bIsRoot)
    {
        /* clear cmstr bit */
        host->pHost->pCSR->state &= ~0x100;
    }

    host->pHost->pCSR->pTopologyMap[1] =
        cpu_to_be32(be32_to_cpu(host->pHost->pCSR->pTopologyMap[1]) + 1);
    host->pHost->pCSR->pTopologyMap[2] = cpu_to_be32(((ULONG)host->pHost->usNodeCount) << 16
                                                | host->pHost->usSelfIdCount);
    host->pHost->pCSR->pTopologyMap[0] =
                cpu_to_be32(((ULONG)(host->pHost->usSelfIdCount + 2)) << 16
                            | csr_crc16(host->pHost->pCSR->pTopologyMap + 1,
                                        host->pHost->usSelfIdCount + 2));

    host->pHost->pCSR->pSpeedMap[1] =
                cpu_to_be32(be32_to_cpu(host->pHost->pCSR->pSpeedMap[1]) + 1);
    host->pHost->pCSR->pSpeedMap[0] = cpu_to_be32(0x3f1UL << 16
                                             | csr_crc16(host->pHost->pCSR->pSpeedMap+1,
                                                         0x3f1));
}

/*
 * HI == seconds (bits 0:2)
 * LO == fraction units of 1/8000 of a second, as per 1394 (bits 19:31)
 *
 * Convert to units and then to HZ, for comparison to jiffies.
 *
 * By default this will end up being 800 units, or 100ms (125usec per
 * unit).
 *
 * NOTE: The spec says 1/8000, but also says we can compute based on 1/8192
 * like CSR specifies. Should make our math less complex.
 */
void calculate_expire(CSR far *csr)
{
    unsigned long units;

    /* Take the seconds, and convert to units */
    units = ((unsigned long)(csr->split_timeout_hi & 0x07)) << 13;

    /* Add in the fractional units */
    units += (unsigned long)(csr->split_timeout_lo >> 19);

    /* Convert to jiffies */
    csr->expire = (unsigned long)(units * HZ) >> 13UL;

    /* Just to keep from rounding low */
    csr->expire++;

    ddprintf("VERBOSE: CSR: setting expire to %ld, HZ=%d\n", csr->expire, HZ);
}


void far __loadds add_host(HOSTOPS far *host)
{
    struct csr1212_keyval far *root;
    QUADLET bus_info[CSR_BUS_INFO_SIZE];

    fpGlobalStackOps->fpfnRegisterAddrSpace(&csr_highlevel, host, &reg_ops,
                CSR_REGISTER_BASE,
                CSR_REGISTER_BASE + CSR_CONFIG_ROM);
    fpGlobalStackOps->fpfnRegisterAddrSpace(&csr_highlevel, host, &config_rom_ops,
                CSR_REGISTER_BASE + CSR_CONFIG_ROM,
                CSR_REGISTER_BASE + CSR_CONFIG_ROM_END);
    if (FCP)
    {
        fpGlobalStackOps->fpfnRegisterAddrSpace(&csr_highlevel, host, &fcp_ops,
                    CSR_REGISTER_BASE + CSR_FCP_COMMAND,
                    CSR_REGISTER_BASE + CSR_FCP_END);
    }
    fpGlobalStackOps->fpfnRegisterAddrSpace(&csr_highlevel, host, &map_ops,
                CSR_REGISTER_BASE + CSR_TOPOLOGY_MAP,
                CSR_REGISTER_BASE + CSR_TOPOLOGY_MAP_END);
    fpGlobalStackOps->fpfnRegisterAddrSpace(&csr_highlevel, host, &map_ops,
                CSR_REGISTER_BASE + CSR_SPEED_MAP,
                CSR_REGISTER_BASE + CSR_SPEED_MAP_END);

//    host->pHost->pCSR->lock = SPIN_LOCK_UNLOCKED;

    host->pHost->pCSR->state                 = 0;
    host->pHost->pCSR->NodeIds              = 0;
    host->pHost->pCSR->split_timeout_hi      = 0;
    host->pHost->pCSR->split_timeout_lo      = 800UL << 19;
    calculate_expire(host->pHost->pCSR);
    host->pHost->pCSR->cycle_time            = 0;
    host->pHost->pCSR->bus_time              = 0;
    host->pHost->pCSR->bus_manager_id        = 0x3f;
    host->pHost->pCSR->BandwidthAvailable   = 4915;
    host->pHost->pCSR->channels_available_hi = 0xfffffffe;   /* pre-alloc ch 31 per 1394a-2000 */
    host->pHost->pCSR->channels_available_lo = ~0;
    host->pHost->pCSR->BroadcastChannel = 0x80000000 | 31;

    if (host->pHost->bIsIrm)
    {
        if (host->pHost->pHardDriver->fpfnHwCsrReg)
        {
            host->pHost->pHardDriver->fpfnHwCsrReg(2, 0xfffffffe, ~0);
        }
    }

    if (host->pHost->pCSR->MaxRec >= 9)
        host->pHost->pCSR->max_rom = 2;
    else if (host->pHost->pCSR->MaxRec >= 5)
            host->pHost->pCSR->max_rom = 1;
         else
            host->pHost->pCSR->max_rom = 0;

    host->pHost->pCSR->generation = 2;

    bus_info[1] = cpu_to_be32(0x31333934);
    bus_info[2] = cpu_to_be32((1UL << CSR_IRMC_SHIFT) |
                  (1UL << CSR_CMC_SHIFT) |
                  (1UL << CSR_ISC_SHIFT) |
                  (0UL << CSR_BMC_SHIFT) |
                  (0UL << CSR_PMC_SHIFT) |
                  (((ULONG)host->pHost->pCSR->CycClkAcc) << CSR_CYC_CLK_ACC_SHIFT) |
                  (((ULONG)host->pHost->pCSR->MaxRec) << CSR_MAX_REC_SHIFT) |
                  (((ULONG)host->pHost->pCSR->max_rom) << CSR_MAX_ROM_SHIFT) |
                  (((ULONG)host->pHost->pCSR->generation) << CSR_GENERATION_SHIFT) |
                  (ULONG)host->pHost->pCSR->LinkSpeed);

    bus_info[3] = cpu_to_be32(host->pHost->pCSR->GuidHi);
    bus_info[4] = cpu_to_be32(host->pHost->pCSR->GuidLo);

    /* The hardware copy of the bus info block will be set later when a
     * bus reset is issued. */

    csr1212_init_local_csr(host->pHost->pCSR->rom, bus_info, host->pHost->pCSR->max_rom);

    host->pHost->pCSR->rom->max_rom = host->pHost->pCSR->max_rom;

    root = host->pHost->pCSR->rom->root_kv;

    if(csr1212_attach_keyval_to_directory(root, node_cap) != CSR1212_SUCCESS) {
        ddprintf("ERROR: Failed to attach Node Capabilities to root directory\n");
    }

    host->pHost->bUpdateConfigRom=TRUE;
}

void far __loadds remove_host(HOSTOPS far *host)
{
    QUADLET bus_info[CSR_BUS_INFO_SIZE];

    bus_info[1] = cpu_to_be32(0x31333934);
    bus_info[2] = cpu_to_be32((0UL << CSR_IRMC_SHIFT) |
                  (0UL << CSR_CMC_SHIFT) |
                  (0UL << CSR_ISC_SHIFT) |
                  (0UL << CSR_BMC_SHIFT) |
                  (0UL << CSR_PMC_SHIFT) |
                  (((ULONG)host->pHost->pCSR->CycClkAcc) << CSR_CYC_CLK_ACC_SHIFT) |
                  (((ULONG)host->pHost->pCSR->MaxRec) << CSR_MAX_REC_SHIFT) |
                  (0UL << CSR_MAX_ROM_SHIFT) |
                  (0UL << CSR_GENERATION_SHIFT) |
                  (ULONG)host->pHost->pCSR->LinkSpeed);

    bus_info[3] = cpu_to_be32(host->pHost->pCSR->GuidHi);
    bus_info[4] = cpu_to_be32(host->pHost->pCSR->GuidLo);

    csr1212_detach_keyval_from_directory(host->pHost->pCSR->rom->root_kv, node_cap);

    csr1212_init_local_csr(host->pHost->pCSR->rom, bus_info, 0);
    host->pHost->bUpdateConfigRom = TRUE;
}



/* Read topology / speed maps and configuration ROM */
int far __loadds read_maps(HOSTOPS far *host, int nodeid, QUADLET far *buffer,
                     U64 addr, int length, USHORT fl)
{
//    unsigned long flags;
        QUADLET csraddr = (QUADLET)(addr - CSR_REGISTER_BASE);
        const char far *src;

//        spin_lock_irqsave(&host->pHost->pCSR->lock, flags);

    if (csraddr < CSR_SPEED_MAP) {
                src = ((char far *)host->pHost->pCSR->pTopologyMap) + csraddr
                        - CSR_TOPOLOGY_MAP;
        } else {
                src = ((char far *)host->pHost->pCSR->pSpeedMap) + csraddr - CSR_SPEED_MAP;
        }

        _fmemcpy(buffer, src, length);
//        spin_unlock_irqrestore(&host->pHost->pCSR->lock, flags);
        return RCODE_COMPLETE;
}


#define out if (--length == 0) break

int far __loadds read_regs(HOSTOPS far *host, int nodeid, QUADLET far *buf,
                     U64 addr, int length, USHORT flags)
{
        QUADLET csraddr = (QUADLET)(addr - CSR_REGISTER_BASE);
        QUADLET oldcycle;
        QUADLET ret;

        if ((csraddr | length) & 0x3)
                return RCODE_TYPE_ERROR;

        length /= 4;

        switch (csraddr) {
        case CSR_STATE_CLEAR:
                *(buf++) = cpu_to_be32(host->pHost->pCSR->state);
                out;
        case CSR_STATE_SET:
                *(buf++) = cpu_to_be32(host->pHost->pCSR->state);
                out;
        case CSR_NODE_IDS:
                *(buf++) = cpu_to_be32(host->pHost->pCSR->NodeIds);
                out;

        case CSR_RESET_START:
                return RCODE_TYPE_ERROR;

                /* address gap - handled by default below */

        case CSR_SPLIT_TIMEOUT_HI:
                *(buf++) = cpu_to_be32(host->pHost->pCSR->split_timeout_hi);
                out;
        case CSR_SPLIT_TIMEOUT_LO:
                *(buf++) = cpu_to_be32(host->pHost->pCSR->split_timeout_lo);
                out;

                /* address gap */
                return RCODE_ADDRESS_ERROR;

        case CSR_CYCLE_TIME:
                oldcycle = host->pHost->pCSR->cycle_time;
                host->pHost->pCSR->cycle_time =
                        host->pHost->pHardDriver->fpfnDevCtl(GET_CYCLE_COUNTER, 0);

                if (oldcycle > host->pHost->pCSR->cycle_time) {
                        /* cycle time wrapped around */
                        host->pHost->pCSR->bus_time += 1 << 7;
                }
                *(buf++) = cpu_to_be32(host->pHost->pCSR->cycle_time);
                out;
        case CSR_BUS_TIME:
                oldcycle = host->pHost->pCSR->cycle_time;
                host->pHost->pCSR->cycle_time =
                        host->pHost->pHardDriver->fpfnDevCtl(GET_CYCLE_COUNTER, 0);

                if (oldcycle > host->pHost->pCSR->cycle_time) {
                        /* cycle time wrapped around */
                        host->pHost->pCSR->bus_time += (1 << 7);
                }
                *(buf++) = cpu_to_be32(host->pHost->pCSR->bus_time
                                       | (host->pHost->pCSR->cycle_time >> 25));
                out;

                /* address gap */
                return RCODE_ADDRESS_ERROR;

        case CSR_BUSY_TIMEOUT:
                /* not yet implemented */
                return RCODE_ADDRESS_ERROR;

        case CSR_BUS_MANAGER_ID:
                if (host->pHost->pHardDriver->fpfnHwCsrReg)
                        ret = host->pHost->pHardDriver->fpfnHwCsrReg(0, 0, 0);
                else
                        ret = host->pHost->pCSR->bus_manager_id;

                *(buf++) = cpu_to_be32(ret);
                out;
        case CSR_BANDWIDTH_AVAILABLE:
                if (host->pHost->pHardDriver->fpfnHwCsrReg)
                        ret = host->pHost->pHardDriver->fpfnHwCsrReg(1, 0, 0);
                else
                        ret = host->pHost->pCSR->BandwidthAvailable;

                *(buf++) = cpu_to_be32(ret);
                out;
        case CSR_CHANNELS_AVAILABLE_HI:
                if (host->pHost->pHardDriver->fpfnHwCsrReg)
                        ret = host->pHost->pHardDriver->fpfnHwCsrReg(2, 0, 0);
                else
                        ret = host->pHost->pCSR->channels_available_hi;

                *(buf++) = cpu_to_be32(ret);
                out;
        case CSR_CHANNELS_AVAILABLE_LO:
                if (host->pHost->pHardDriver->fpfnHwCsrReg)
                        ret = host->pHost->pHardDriver->fpfnHwCsrReg(3, 0, 0);
                else
                        ret = host->pHost->pCSR->channels_available_lo;

                *(buf++) = cpu_to_be32(ret);
                out;

    case CSR_BROADCAST_CHANNEL:
        *(buf++) = cpu_to_be32(host->pHost->pCSR->BroadcastChannel);
        out;

                /* address gap to end - fall through to default */
        default:
                return RCODE_ADDRESS_ERROR;
        }

        return RCODE_COMPLETE;
}

int far __loadds write_regs(HOSTOPS far *host, int nodeid, int destid,
              QUADLET far *data, U64 addr, int length, USHORT flags)
{
        QUADLET csraddr = (QUADLET)(addr - CSR_REGISTER_BASE);

        if ((csraddr | length) & 0x3)
                return RCODE_TYPE_ERROR;

        length /= 4;

        switch (csraddr) {
        case CSR_STATE_CLEAR:
                /* FIXME FIXME FIXME */
                ddprintf("doh, someone wants to mess with state clear\n");
                out;
        case CSR_STATE_SET:
                ddprintf("doh, someone wants to mess with state set\n");
                out;

        case CSR_NODE_IDS:
                host->pHost->pCSR->NodeIds &= ((ULONG)NODE_MASK) << 16;
                host->pHost->pCSR->NodeIds |= be32_to_cpu(*(data++)) & (((ULONG)BUS_MASK) << 16);
                host->pHost->NodeId = host->pHost->pCSR->NodeIds >> 16;
                host->pHost->pHardDriver->fpfnDevCtl(SET_BUS_ID, host->pHost->NodeId >> 6);
                out;

        case CSR_RESET_START:
                /* FIXME - perform command reset */
                out;

                /* address gap */
                return RCODE_ADDRESS_ERROR;

        case CSR_SPLIT_TIMEOUT_HI:
                host->pHost->pCSR->split_timeout_hi =
                        be32_to_cpu(*(data++)) & 0x00000007;
        calculate_expire(host->pHost->pCSR);
                out;
        case CSR_SPLIT_TIMEOUT_LO:
                host->pHost->pCSR->split_timeout_lo =
                        be32_to_cpu(*(data++)) & 0xfff80000;
        calculate_expire(host->pHost->pCSR);
                out;

                /* address gap */
                return RCODE_ADDRESS_ERROR;

        case CSR_CYCLE_TIME:
                /* should only be set by cycle start packet, automatically */
                host->pHost->pCSR->cycle_time = be32_to_cpu(*data);
                host->pHost->pHardDriver->fpfnDevCtl(SET_CYCLE_COUNTER,
                                       be32_to_cpu(*(data++)));
                out;
        case CSR_BUS_TIME:
                host->pHost->pCSR->bus_time = be32_to_cpu(*(data++)) & 0xffffff80;
                out;

                /* address gap */
                return RCODE_ADDRESS_ERROR;

        case CSR_BUSY_TIMEOUT:
                /* not yet implemented */
                return RCODE_ADDRESS_ERROR;

        case CSR_BUS_MANAGER_ID:
        case CSR_BANDWIDTH_AVAILABLE:
        case CSR_CHANNELS_AVAILABLE_HI:
        case CSR_CHANNELS_AVAILABLE_LO:
                /* these are not writable, only lockable */
                return RCODE_TYPE_ERROR;

    case CSR_BROADCAST_CHANNEL:
        /* only the valid bit can be written */
        host->pHost->pCSR->BroadcastChannel = (host->pHost->pCSR->BroadcastChannel & ~0x40000000)
                        | (be32_to_cpu(*data) & 0x40000000);
        out;

                /* address gap to end - fall through */
        default:
                return RCODE_ADDRESS_ERROR;
        }

        return RCODE_COMPLETE;
}

#undef out


int far __loadds lock_regs(HOSTOPS far *host, int nodeid, QUADLET far *store,
                     U64 addr, QUADLET data, QUADLET arg, int extcode, USHORT fl)
{
        QUADLET csraddr = (QUADLET)(addr - CSR_REGISTER_BASE);
//        unsigned long flags;
        QUADLET far *regptr = NULL;

        if (csraddr & 0x3)
        return RCODE_TYPE_ERROR;

        if (csraddr < CSR_BUS_MANAGER_ID || csraddr > CSR_CHANNELS_AVAILABLE_LO
            || extcode != EXTCODE_COMPARE_SWAP)
                goto unsupported_lockreq;

        data = be32_to_cpu(data);
        arg = be32_to_cpu(arg);

    /* Is somebody releasing the BroadcastChannel on us? */
    if (csraddr == CSR_CHANNELS_AVAILABLE_HI && (data & 0x1)) {
        /* Note: this is may not be the right way to handle
         * the problem, so we should look into the proper way
         * eventually. */
        ddprintf("WARNING: Node [" NODE_BUS_FMT "] wants to release "
              "broadcast channel 31.  Ignoring.\n",
              NODE_BUS_ARGS(host->pHost, nodeid));

        data &= ~0x1;   /* keep broadcast channel allocated */
    }

        if (host->pHost->pHardDriver->fpfnHwCsrReg) {
                QUADLET old;

                old = host->pHost->pHardDriver->
                        fpfnHwCsrReg((csraddr - CSR_BUS_MANAGER_ID) >> 2,
                                   data, arg);

                *store = cpu_to_be32(old);
                return RCODE_COMPLETE;
        }

//        spin_lock_irqsave(&host->pHost->pCSR->lock, flags);

        switch (csraddr) {
        case CSR_BUS_MANAGER_ID:
                regptr = &host->pHost->pCSR->bus_manager_id;
        *store = cpu_to_be32(*regptr);
        if (*regptr == arg)
            *regptr = data;
                break;

        case CSR_BANDWIDTH_AVAILABLE:
        {
                QUADLET bandwidth;
                QUADLET oldQ;
                QUADLET newQ;

                regptr = &host->pHost->pCSR->BandwidthAvailable;
                oldQ = *regptr;

                /* bandwidth available algorithm adapted from IEEE 1394a-2000 spec */
                if (arg > 0x1fff) {
                        *store = cpu_to_be32(oldQ);  /* change nothing */
            break;
                }
                data &= 0x1fff;
                if (arg >= data) {
                        /* allocate bandwidth */
                        bandwidth = arg - data;
                        if (oldQ >= bandwidth) {
                                newQ = oldQ - bandwidth;
                                *store = cpu_to_be32(arg);
                                *regptr = newQ;
                        } else {
                                *store = cpu_to_be32(oldQ);
                        }
                } else {
                        /* deallocate bandwidth */
                        bandwidth = data - arg;
                        if (oldQ + bandwidth < 0x2000) {
                                newQ = oldQ + bandwidth;
                                *store = cpu_to_be32(arg);
                                *regptr = newQ;
                        } else {
                                *store = cpu_to_be32(oldQ);
                        }
                }
                break;
        }

        case CSR_CHANNELS_AVAILABLE_HI:
        {
                /* Lock algorithm for CHANNELS_AVAILABLE as recommended by 1394a-2000 */
                QUADLET affected_channels = arg ^ data;

                regptr = &host->pHost->pCSR->channels_available_hi;

                if ((arg & affected_channels) == (*regptr & affected_channels)) {
                        *regptr ^= affected_channels;
                        *store = cpu_to_be32(arg);
                } else {
                        *store = cpu_to_be32(*regptr);
                }

                break;
        }

        case CSR_CHANNELS_AVAILABLE_LO:
        {
                /* Lock algorithm for CHANNELS_AVAILABLE as recommended by 1394a-2000 */
                QUADLET affected_channels = arg ^ data;

                regptr = &host->pHost->pCSR->channels_available_lo;

                if ((arg & affected_channels) == (*regptr & affected_channels)) {
                        *regptr ^= affected_channels;
                        *store = cpu_to_be32(arg);
                } else {
                        *store = cpu_to_be32(*regptr);
                }
                break;
        }
        }

//        spin_unlock_irqrestore(&host->pHost->pCSR->lock, flags);

        return RCODE_COMPLETE;

 unsupported_lockreq:
        switch (csraddr) {
        case CSR_STATE_CLEAR:
        case CSR_STATE_SET:
        case CSR_RESET_START:
        case CSR_NODE_IDS:
        case CSR_SPLIT_TIMEOUT_HI:
        case CSR_SPLIT_TIMEOUT_LO:
        case CSR_CYCLE_TIME:
        case CSR_BUS_TIME:
    case CSR_BROADCAST_CHANNEL:
                return RCODE_TYPE_ERROR;

        case CSR_BUSY_TIMEOUT:
                /* not yet implemented - fall through */
        default:
                return RCODE_ADDRESS_ERROR;
        }
}

int far __loadds lock64_regs(HOSTOPS far *host, int nodeid, OCTLET far *store,
               U64 addr, OCTLET data, OCTLET arg, int extcode, USHORT fl)
{
    QUADLET csraddr = (QUADLET)(addr - CSR_REGISTER_BASE);
//    unsigned long flags;

    data = be64_to_cpu(data);
    arg = be64_to_cpu(arg);

    if (csraddr & 0x3)
        return RCODE_TYPE_ERROR;

    if (csraddr != CSR_CHANNELS_AVAILABLE
        || extcode != EXTCODE_COMPARE_SWAP)
        goto unsupported_lock64req;

    /* Is somebody releasing the BroadcastChannel on us? */
    if (csraddr == CSR_CHANNELS_AVAILABLE_HI && (data & 0x100000000ULL)) {
        /* Note: this is may not be the right way to handle
         * the problem, so we should look into the proper way
                 * eventually. */
        ddprintf("WARNING: Node [" NODE_BUS_FMT "] wants to release "
              "broadcast channel 31.  Ignoring.\n",
              NODE_BUS_ARGS(host->pHost, nodeid));

        data &= ~0x100000000ULL;    /* keep broadcast channel allocated */
    }

    if (host->pHost->pHardDriver->fpfnHwCsrReg) {
        QUADLET data_hi, data_lo;
        QUADLET arg_hi, arg_lo;
        QUADLET old_hi, old_lo;

        data_hi = (ULONG)(data >> 32);
        data_lo = (ULONG)(data & 0xFFFFFFFF);
        arg_hi = (ULONG)(arg >> 32);
        arg_lo = (ULONG)(arg & 0xFFFFFFFF);

        old_hi = host->pHost->pHardDriver->fpfnHwCsrReg((csraddr - CSR_BUS_MANAGER_ID) >> 2,
                                                  data_hi, arg_hi);

        old_lo = host->pHost->pHardDriver->fpfnHwCsrReg(((csraddr + 4) - CSR_BUS_MANAGER_ID) >> 2,
                                                  data_lo, arg_lo);

        *store = cpu_to_be64((((OCTLET)old_hi) << 32) | old_lo);
    } else {
        OCTLET old;
        OCTLET affected_channels = arg ^ data;

//        spin_lock_irqsave(&host->pHost->pCSR->lock, flags);

        old = (((OCTLET)host->pHost->pCSR->channels_available_hi) << 32) | host->pHost->pCSR->channels_available_lo;

        if ((arg & affected_channels) == (old & affected_channels)) {
            host->pHost->pCSR->channels_available_hi ^= (ULONG)(affected_channels >> 32);
            host->pHost->pCSR->channels_available_lo ^= (ULONG)(affected_channels & 0xffffffff);
            *store = cpu_to_be64(arg);
        } else {
            *store = cpu_to_be64(old);
        }

//        spin_unlock_irqrestore(&host->pHost->pCSR->lock, flags);
    }

    /* Is somebody erroneously releasing the BroadcastChannel on us? */
    if (host->pHost->pCSR->channels_available_hi & 0x1)
        host->pHost->pCSR->channels_available_hi &= ~0x1;

    return RCODE_COMPLETE;

 unsupported_lock64req:
    switch (csraddr) {
    case CSR_STATE_CLEAR:
    case CSR_STATE_SET:
    case CSR_RESET_START:
    case CSR_NODE_IDS:
    case CSR_SPLIT_TIMEOUT_HI:
    case CSR_SPLIT_TIMEOUT_LO:
    case CSR_CYCLE_TIME:
    case CSR_BUS_TIME:
    case CSR_BUS_MANAGER_ID:
    case CSR_BROADCAST_CHANNEL:
    case CSR_BUSY_TIMEOUT:
    case CSR_BANDWIDTH_AVAILABLE:
        return RCODE_TYPE_ERROR;

    default:
        return RCODE_ADDRESS_ERROR;
    }
}

int far __loadds write_fcp(HOSTOPS far *host, int nodeid, int dest,
             QUADLET far *data, U64 addr, int length, USHORT flags)
{
        QUADLET csraddr = (QUADLET)(addr - CSR_REGISTER_BASE);

        if (length > 512)
                return RCODE_TYPE_ERROR;

        switch (csraddr) {
        case CSR_FCP_COMMAND:
                fpGlobalStackOps->fpfnFcpRequest(host, nodeid, 0, (UCHAR far *)data, length);
                break;
        case CSR_FCP_RESPONSE:
                fpGlobalStackOps->fpfnFcpRequest(host, nodeid, 1, (UCHAR far *)data, length);
                break;
        default:
                return RCODE_TYPE_ERROR;
        }

        return RCODE_COMPLETE;
}

int far __loadds read_config_rom(HOSTOPS far *host, int nodeid, QUADLET far *buffer,
               U64 addr, int length, USHORT fl)
{
    QUADLET offset = (QUADLET)(addr - CSR1212_REGISTER_SPACE_BASE);

    if (csr1212_read(host->pHost->pCSR->rom, offset, buffer, length) == CSR1212_SUCCESS)
        return RCODE_COMPLETE;
    else
        return RCODE_ADDRESS_ERROR;
}

U64 far __loadds allocate_addr_range(U64 size, ULONG alignment, void far *__host)
{
    HOSTOPS far *host = (HOSTOPS far *)__host;

    return fpGlobalStackOps->fpfnAllocateAndRegisterAddrSpace(&csr_highlevel,
                            host,
                            &config_rom_ops,
                            size, alignment,
                            CSR1212_UNITS_SPACE_BASE,
                            CSR1212_UNITS_SPACE_END);
}

void far __loadds release_addr_range(U64 addr, void far *__host)
{
    HOSTOPS far *host = (HOSTOPS far *)__host;
    fpGlobalStackOps->fpfnUnRegisterAddrSpace(&csr_highlevel, host, addr);
}


int init_csr(void)
{
    node_cap = csr1212_new_immediate(CSR1212_KV_ID_NODE_CAPABILITIES, 0x0083c0);
    if (!node_cap) {
        ddprintf("ERROR: Failed to allocate memory for Node Capabilties ConfigROM entry!\n");
        return -FIRE_ERROR_NOMEMORY;
    }

    fpGlobalStackOps->fpfnRegisterHighlevel(&csr_highlevel);

    return 0;
}

void cleanup_csr(void)
{
    if (node_cap)
        csr1212_release_keyval(node_cap);
        fpGlobalStackOps->fpfnUnRegisterHighlevel(&csr_highlevel);
}
