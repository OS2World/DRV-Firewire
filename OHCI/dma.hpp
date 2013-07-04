/*
** Module   :DMA.HPP
** Abstract :DMA classes
**
** Copyright (C) Alex Cherkaev
**
** Log: Fri  14/05/2004 Created
**
*/
#ifndef __DMA_HPP
#define __DMA_HPP

#include "memory.hpp"
#include "list.hpp"
#include "firetype.h"
#include "..\stack\packetlist.hpp"
class HOST;
typedef HOST far * PHOST;
class OHCIDRIVER;
typedef OHCIDRIVER* POHCIDRIVER;

#pragma pack(1)
// DMA command
typedef struct
{
    ULONG ulControl;
    ULONG ulAddress;
    ULONG ulBranchAddress;
    ULONG ulStatus;
} DMA_CMD;
//typedef DMACMD * PDMACMD;
// AT DMA prog
typedef struct
{
    DMA_CMD dmacmdBegin;
    ULONG  data[4];
    DMA_CMD dmacmdEnd;
} AT_DMA_PRG;
//typedef ATDMAPRG * PATDMAPRG;
typedef enum {DMA_CTX_ASYNC_REQ, DMA_CTX_ASYNC_RESP, DMA_CTX_ISO} CONTEXT_TYPE ;
// dma receive context
class DMA_RCV_CONTEXT
{
public:
    DMA_RCV_CONTEXT(POHCIDRIVER ohci, CONTEXT_TYPE type, int ctx, int num_desc,
          int buf_size, int split_buf_size, int context_base);
    ~DMA_RCV_CONTEXT(void);
    void Initialize(BOOL bGenerateIRQ);
    void StopContext(void);
    int PacketLength(int idx, QUADLET far *buf_ptr,
             int offset, unsigned char tcode);
    void InsertDMABuffer(int idx);
    void DMATasklet(void);


    POHCIDRIVER pDriver;//physical driver
    CONTEXT_TYPE context_type;//type of context
    int ctx; //context index
    USHORT num_desc; //?

    USHORT buf_size;
    USHORT split_buf_size;


    /* dma block descriptors */
//        struct dma_cmd **prg_cpu;
    DMA_CMD far ** prg_cpu;
    //array [num_desc] of virtual pointers to dma
//     dma_addr_t *prg_bus;
    ULONG * prg_bus;
    //array [num_desc] of physical pointers to dma
//    struct pci_pool *prg_pool;

    /* dma buffers */
    QUADLET far ** buf_cpu;
    //array [num_desc] of virtual pointers to buffers
//        quadlet_t **buf_cpu;
//        dma_addr_t *buf_bus;
    ULONG* buf_bus;
    //array [num_desc] of physical pointers to buffers

    USHORT buf_ind;
    USHORT buf_offset;
    QUADLET far *spb;
//    spinlock_t lock;
//    TASKLET task;
    USHORT ctrlClear;
    USHORT ctrlSet;
    USHORT cmdPtr;
    USHORT ctxtMatch;
private:
    PMEMORY aPrgMem;
    PMEMORY aBufMem;
};
class DMA_TRM_CONTEXT
{
public:
    DMA_TRM_CONTEXT(POHCIDRIVER pDriver, CONTEXT_TYPE type, int ctx, int num_desc,
          int context_base);
    ~DMA_TRM_CONTEXT(void);
    void Initialize(void);
    void Reset(void);
    void DMATasklet(void);

    POHCIDRIVER pDriver;
    CONTEXT_TYPE type;

    int ctx;
    unsigned int num_desc;

    /* dma block descriptors */
//    struct at_dma_prg **prg_cpu;
    AT_DMA_PRG far * *prg_cpu;
    ULONG *prg_bus;
//    struct pci_pool *prg_pool;

    unsigned int prg_ind;
    unsigned int sent_ind;
    int free_prgs;
    ULONG far *branchAddrPtr;

    /* list of packets inserted in the AT FIFO */
    PACKETLIST fifo_list;

    /* list of pending packets to be inserted in the AT FIFO */
    PACKETLIST pending_list;

//    spinlock_t lock;
//    struct tasklet_struct task;
    USHORT ctrlClear;
    USHORT ctrlSet;
    USHORT cmdPtr;
private:
    PMEMORY aPrgMem;
    PACKETLIST PacketList;

};


#endif  /*__DMA_HPP*/

