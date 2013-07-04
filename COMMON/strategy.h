/*
** Module   :STRATEGY.H
** Abstract :Strategy funcs
**
** Copyright (C) Alexandr Cherkaev
** original code by Timur Tabi
**
** Log: Fri  30/04/2004	Created
**
*/

#ifndef __STRATEGY_H
#define __STRATEGY_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1);

typedef struct {                           /* template for request header */
  BYTE bLength;                            /* request packet length */
  BYTE bUnit;                              /* unit code for block DD only */
  BYTE bCommand;                           /* command code */
  USHORT usStatus;                         /* return status */
  ULONG dwReserved;                        /* reserved bytes */
  ULONG ulQlink;                           /* queue linkage */
  union {                                  /* command-specific data */
    struct {
      BYTE b;
      PFN ulDevHlp;                        /* dev help address */
      char __far *szArgs;                  /* argument pointer */
      BYTE bDrive;
    } init_in;
    struct {
      BYTE bUnits;
      USHORT usCodeEnd;                   // final code offset
      USHORT usDataEnd;                   // final data offset
      ULONG ul;
    } init_out;
    struct {
      BYTE bMedia;
      ULONG ulAddress;
      USHORT usCount;
      ULONG ulStartSector;
      USHORT usSysFileNum;
    } io;
    struct {
      BYTE bData;
    } peek;
    struct {
      BYTE bCategory;                     // category code
      BYTE bCode;                         // function code
      void __far *pvParm;                 // address of parameter buffer
      void __far *pvData;                 // address of data buffer
      USHORT usSysFileNum;                // system file number
      USHORT usPLength;                   // length of parameter buffer
      USHORT usDLength;                   // length of data buffer
    } ioctl;
    struct {
      USHORT usSysFileNum;                // system file number
    } open_close;
  } s;
} REQPACKET, __far *PREQPACKET;

#pragma pack();

/* Constants relating to the Strategy Routines
*/

#define RPDONE    0x0100         // return successful, must be set
#define RPBUSY    0x0200         // device is busy (or has no data to return)
#define RPDEV     0x4000         // user-defined error
#define RPERR     0x8000         // return error

// List of error codes, from chapter 8 of PDD reference
#define RPNOTREADY  0x0002
#define RPBADCMD    0x0003
#define RPGENFAIL   0x000c
#define RPDEVINUSE  0x0014
#define RPINITFAIL  0x0015

#ifdef __cplusplus
}
#endif

#endif  /*__STRATEGY_H*/

