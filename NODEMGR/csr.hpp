/*
** Module   :CSR.HPP
** Abstract :
**
** Copyright (C) Alex Cherkaev
**
** Log: Wed  02/06/2004 Created
**
*/
#ifndef __CSR_HPP
#define __CSR_HPP
#if __cplusplus
extern "C" {
#endif

#define FCP 1
//MODULE_PARM_DESC(fcp, "Map FCP registers (default = 1, disable = 0).");

#include "csr1212.h"
#include "idc.h"

#define CSR_REGISTER_BASE  0xfffff0000000ULL

/* register offsets relative to CSR_REGISTER_BASE */
#define CSR_STATE_CLEAR           0x0
#define CSR_STATE_SET             0x4
#define CSR_NODE_IDS              0x8
#define CSR_RESET_START           0xc
#define CSR_SPLIT_TIMEOUT_HI      0x18
#define CSR_SPLIT_TIMEOUT_LO      0x1c
#define CSR_CYCLE_TIME            0x200
#define CSR_BUS_TIME              0x204
#define CSR_BUSY_TIMEOUT          0x210
#define CSR_BUS_MANAGER_ID        0x21c
#define CSR_BANDWIDTH_AVAILABLE   0x220
#define CSR_CHANNELS_AVAILABLE    0x224
#define CSR_CHANNELS_AVAILABLE_HI 0x224
#define CSR_CHANNELS_AVAILABLE_LO 0x228
#define CSR_BROADCAST_CHANNEL     0x234
#define CSR_CONFIG_ROM            0x400
#define CSR_CONFIG_ROM_END        0x800
#define CSR_FCP_COMMAND           0xB00
#define CSR_FCP_RESPONSE          0xD00
#define CSR_FCP_END               0xF00
#define CSR_TOPOLOGY_MAP          0x1000
#define CSR_TOPOLOGY_MAP_END      0x1400
#define CSR_SPEED_MAP             0x2000
#define CSR_SPEED_MAP_END         0x3000

/* IEEE 1394 bus specific Configuration ROM Key IDs */
#define IEEE1394_KV_ID_POWER_REQUIREMENTS (0x30)

/* IEEE 1394 Bus Inforamation Block specifics */
#define CSR_BUS_INFO_SIZE (5 * sizeof(QUADLET))

#define CSR_IRMC_SHIFT 31
#define CSR_CMC_SHIFT  30
#define CSR_ISC_SHIFT  29
#define CSR_BMC_SHIFT  28
#define CSR_PMC_SHIFT  27
#define CSR_CYC_CLK_ACC_SHIFT 16
#define CSR_MAX_REC_SHIFT 12
#define CSR_MAX_ROM_SHIFT 8
#define CSR_GENERATION_SHIFT 4

void far add_host(HOSTOPS far *host);
void far remove_host(HOSTOPS far *host);
void far host_reset(HOSTOPS far *host);
int far read_maps(HOSTOPS far * host, int nodeid, QUADLET far *buffer,
  U64 addr, int length, USHORT fl);
int far write_fcp(HOSTOPS far * host, int nodeid, int dest,
  QUADLET far *data, U64 addr, int length, USHORT flags);
int far read_regs(HOSTOPS far *host, int nodeid, QUADLET far *buf,
  U64 addr, int length, USHORT flags);
int far write_regs(HOSTOPS far *host, int nodeid, int destid,
   QUADLET far *data, U64 addr, int length, USHORT flags);
int far lock_regs(HOSTOPS far *host, int nodeid, QUADLET far *store,
  U64 addr, QUADLET data, QUADLET arg, int extcode, USHORT fl);
int far lock64_regs(HOSTOPS far *host, int nodeid, OCTLET far * store,
    U64 addr, OCTLET data, OCTLET arg, int extcode, USHORT fl);
int far read_config_rom(HOSTOPS far *host, int nodeid, QUADLET far *buffer,
    U64 addr, int length, USHORT fl);
U64 far allocate_addr_range(U64 size, ULONG alignment, void far *__host);
void far release_addr_range(U64 addr, void far *__host);

#define CSR_SET_BUS_INFO_GENERATION(csr, gen)               \
    ((csr)->bus_info_data[2] =                  \
        cpu_to_be32((be32_to_cpu((csr)->bus_info_data[2]) & \
                 ~(0xf << CSR_GENERATION_SHIFT)) |          \
                (gen) << CSR_GENERATION_SHIFT))
struct CSR
{
    QUADLET state;
    QUADLET NodeIds;
    QUADLET split_timeout_hi, split_timeout_lo;
    ULONG   expire;   // Calculated from split_timeout
    QUADLET cycle_time;
    QUADLET bus_time;
    QUADLET bus_manager_id;
    QUADLET BandwidthAvailable;
    QUADLET channels_available_hi, channels_available_lo;
    QUADLET BroadcastChannel;

    /* Bus Info */
    QUADLET GuidHi, GuidLo;
    UCHAR CycClkAcc;
    UCHAR MaxRec;
    UCHAR max_rom;
    UCHAR generation;  /* Only use values between 0x2 and 0xf */
    UCHAR LinkSpeed;

    ULONG genTimestamp[16];

    struct csr1212_csr far *rom;

    QUADLET pTopologyMap[256];
    QUADLET pSpeedMap[1024];
};
typedef CSR far * PCSR;
extern struct csr1212_bus_ops csr_bus_ops;

int init_csr(void);
void cleanup_csr(void);
#if __cplusplus
}
#endif

#endif  /*__CSR_HPP*/

