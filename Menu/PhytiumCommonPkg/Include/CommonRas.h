/** @file
Ras common struct and define include.

Copyright (C) 2024, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef  COMMON_RAS_H_
#define  COMMON_RAS_H_

#define  MEMORY_FATAL_UCE_SIGNATURE        SIGNATURE_64 ('M', 'E', 'M', 'F', 'U', 'C', 'E', '1')
#define  MEMORY_NON_FATAL_UCE_SIGNATURE    SIGNATURE_64 ('M', 'E', 'M', 'N', 'F', 'U', 'C', 'E')
#define  MEMORY_CE_SIGNATURE               SIGNATURE_64 ('M', 'E', 'M', 'C', 'E', 'N', 'V', '1')
#define  CPU_FATAL_UCE_SIGNATURE           SIGNATURE_64 ('C', 'P', 'U', 'F', 'U', 'C', 'E', '1')
#define  FATAL_ERROR_RECORD_SIGNATURE      SIGNATURE_64 ('F', 'A', 'T', 'A', 'L', 'R', 'C', 'D')
#define  PPR_RECORD_SIGNATURE              SIGNATURE_64 ('P', 'P', 'R', 'R', 'E', 'C', 'O', 'R')
#define  MEM_ADDR_ISO_RECORD_SIGNATURE     SIGNATURE_64 ('M', 'E', 'M', 'A', 'D', 'D', 'R', 'I')

#define  DIMM_CHANNEL_MAX_COUNT            64
#define  CORE_MAP_MAX_COUNT                16

#define  ADDR_ISO_512MB                    0
#define  ADDR_ISO_128MB                    1

typedef struct _RAS_TOTAL_COUNT {
  UINT32  MemCeCount;
  UINT32  CpuCeCount;
  UINT32  PcieCeCount;
} RAS_TOTAL_COUNT;

typedef struct _MEM_CHANNEL_IOS_STATE {
  UINT16  ChannelID;
  UINT16  IsoState;         //1-isolation, 0-not isolation
  UINT32  Count;
} MEM_CHANNEL_IOS_STATE;

typedef struct _RAS_MEM_TYPE_RECORD {
  UINT64                 Signature;
  UINT32                 ResetClean;
  MEM_CHANNEL_IOS_STATE  ChannelRecord[DIMM_CHANNEL_MAX_COUNT];
} RAS_MEM_TYPE_RECORD;

typedef struct _PPR_RECORD {
  UINT64 Signature;
  UINT8  ChannelId;
  UINT8  SubChannel;
  UINT8  RankNum;
  UINT8  BankGroup;
  UINT8  Bank;
  UINT8  Row;
} PPR_RECORD;

typedef struct _RAS_MEM_ADDR_ISO_RECORD {
  UINT64  Signature;
  UINT32  Count;
  UINT32  Index;
#define  MAX_ADDR_ISO_RECORD_COUNT    128
  UINT64  Addr[MAX_ADDR_ISO_RECORD_COUNT];
} RAS_MEM_ADDR_ISO_RECORD;

typedef struct _RAS_MEM_RECORD {
  RAS_MEM_TYPE_RECORD      MemFatalUceRecord;
  RAS_MEM_TYPE_RECORD      MemNonFatalUceRecord;
  RAS_MEM_TYPE_RECORD      MemCeRecord;
  PPR_RECORD               PprRecord;
  RAS_MEM_ADDR_ISO_RECORD  MemAddrIsoRecord;
} RAS_MEM_RECORD;

typedef struct _RAS_CPU_RECORD {
  UINT64  Signature;
  UINT32  ResetClean;
  UINT64  IsoMap[CORE_MAP_MAX_COUNT];
} RAS_CPU_RECORD;

//
//Memory Error Isolation Strategy
//
typedef struct _MEM_ISO_INFOR_RECORD {
  UINT8  Enable;        //0-Disable, 1-Enable
  UINT8  ResetClean;    //0-not clean, 1-clean
  UINT8  Method;        //0-do nothing, 1-debug, 2-debug and isolation
  UINT8  Reserved;
  UINT32 Count;         //Threshold count
} MEM_ISO_INFOR_RECORD;

//
//Ce report Threshold strategy
//
typedef struct _CE_REPORT_STRATEGY {
  UINT32  Enable;        //0-Disable, 1-Enable
  UINT32  Count;         //Threshold count
} CE_REPORT_STRATEGY;

//
//Cpu Fatal Error Isolation Strategy
//
typedef struct _CPU_FATAL_ISO_STRATEGY {
  UINT8   Enable;        //0-Disable, 1-Enable
  UINT8   ResetClean;    //0-not clean, 1-clean
  UINT8   Method;        //0-do nothing, 1-debug, 2-debug and isolation
  UINT8   Record;        //0-no record, 1-flash, 2-bmc
} CPU_FATAL_ISO_STRATEGY;

//
//Qspi Flash write protect config
//
typedef struct _FLASH_WP_CONFIG {
  UINT8   Enable;
  UINT8   BlockProtectBit;
  UINT16  WpDelay;                        //us
} FLASH_WP_CONFIG;

//
//Memory Address Isolation Strategy
//
typedef struct _MEM_ADDR_ISO_STRATEGY {
  UINT8   Enable;              //0-disable, 1-enable
  UINT8   ResetClean;          //0-not clean, 1-clean
  UINT8   Type;                //0-512MB, 1-128MB
  UINT8   Reserved;
} MEM_ADDR_ISO_STRATEGY;

//
//RAS Strategy
//
typedef struct _RAS_STRATEGY {
  UINT8                   BmcCperEnable;
  UINT8                   FatalErrorRecord;         //0-no record, 1-flash, 2-bmc
  UINT8                   MemoryIsoRecord;          //0-no record, 1-flash, 2-bmc
  UINT8                   Reserved;
  MEM_ISO_INFOR_RECORD    MemFatalUceIsoStrategy;
  MEM_ISO_INFOR_RECORD    MemNonFatalUceIsoStrategy;
  MEM_ISO_INFOR_RECORD    MemCeIsoStrategy;
  CE_REPORT_STRATEGY      MemCeReportStrategy;
  CPU_FATAL_ISO_STRATEGY  CpuFatalStrategy;
  CE_REPORT_STRATEGY      CpuCeReportStrategy;
  CE_REPORT_STRATEGY      PcieCeReportStrategy;
  FLASH_WP_CONFIG         FlashWpConfig;
  MEM_ADDR_ISO_STRATEGY   MemAddrIsoStrategy;
} RAS_STRATEGY;

#endif
