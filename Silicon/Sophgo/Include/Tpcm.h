/** @file
  This file is used to implement TPCM communication.

  Copyright (c) 2025, SOPHGO Technologies Inc. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef _SOPHGO_TPCM_H_
#define _SOPHGO_TPCM_H_

#define SOPHGO_TPCM_SIGNATURE                      SIGNATURE_32 ('T', 'P', 'C', 'M')
#define SOPHGO_TPCM_FROM_SOPHGO_TPCM_PROTOCOL(a)   CR (a, SPI_MASTER, SophgoTpcmProtocol, SOPHGO_TPCM_SIGNATURE)

///
/// Global ID for the SOPHGO TPCM Protocol
///
#define SOPHGO_TPCM_PROTOCOL_GUID  \
  { 0x3593BEC8, 0x62C2, 0x417A, \
    { 0x8F, 0x1B, 0x7A, 0x8C, 0x1B, 0x24, 0xF7, 0x83 } }

extern EFI_GUID gSophgoTpcmProtocolGuid;

///
/// TPCM Image Verify Protocol GUID
///
#define TPCM_IMAGE_VERIFY_PROTOCOL_GUID \
  { 0xF89AB5CD, 0x2829, 0x422F, \
	  { 0xA5, 0xF3, 0x03, 0x28, 0xE0, 0x6C, 0xFC, 0xBB } }

extern EFI_GUID gTpcmImageVerifyProtocolGuid;

typedef struct _SOPHGO_TPCM_PROTOCOL       SOPHGO_TPCM_PROTOCOL;
typedef struct _TPCM_IMAGE_VERIFY_PROTOCOL TPCM_IMAGE_VERIFY_PROTOCOL;

#define TPCM_SPI_BUS_NUM             0
#define TPCM_SPI_CHIP_SELECT         0        // SPI_CS
#define TPCM_SPI_MODE                3        // CPHA=0, CPOL=0
#define TPCM_MAX_CLOCK               5000000  // 5MHz
#define TPCM_MAX_DATA_SIZE           4096

//
// TPCM command list
//
#define TPCM_CMD_GET_STS             0x0000100A
#define TPCM_CMD_GET_LICENSE_STS     0x00001015
#define TPCM_CMD_START_MEASURE       0x00001053
#define TPCM_CMD_SET_CTRL_STS        0x00001073
#define TPCM_CMD_GET_CTRL_STS        0x00001074

#define TPCM_TAG_INPUT               0x000000C1
#define TPCM_TAG_OUTPUT              0x000000C4
#define LICENSE_TYPE                 0x00010000

//
// Measure PCR (Platform Configuration Register)
//
#define TPCM_NO_PCR                  0
#define TPCM_PCR_BIOS                1
#define TPCM_PCR_BOOTLOADER          2
#define TPCM_PCR_KERNEL              3

#define TDD_CMD_CATEGORY_TPCM        3
#define TDD_CMD_CATEGORY_TPCM_ASYNC  10

#pragma pack(1)
typedef struct {
  UINT8  Head;            // 0xAC
  UINT8  Category;        // TDD_CMD_CATEGORY_TPCM=3
  UINT8  DataLength;      // valid: 1-250
  UINT8  Data[250];
  UINT8  Crc8;
  UINT8  NextPacketFlag;  // 0:Last 1:More
  UINT8  Tail;            // 0xAA
} TPCM_SPI_PACKET;
#pragma pack()

#pragma pack(1)
typedef struct {
  UINT32   Tag;
  UINT32   Length;
  UINT32   Command;
} TPCM_CMD_HEADER;
#pragma pack()

//
// Measure: Input cmd structure
//
#pragma pack(1)
typedef struct {
  UINT32   Tag;
  UINT32   Length;
  UINT32   Command;
  UINT32   PcrTag;
  UINT32   MeasurePhase;
  UINT8    Hash[32];
  UINT32   TargetNameLength;
  UINTN    TargetName;
} TPCM_MEASURE_REQ_HEADER;
#pragma pack()

//
// Measure: Output cmd structure
//
#pragma pack(1)
typedef struct {
  UINT32   Tag;
  UINT32   Length;
  UINT16   ControlNumber; // 0: allow exec; 1: forbid exec
  UINT16   Ret; // 0: success
} TPCM_MEASURE_RES;
#pragma pack()

//
// Get license: Input cmd structure
//
#pragma pack(1)
typedef struct {
  UINT32  Tag;
  UINT32  Length;
  UINT32  Command;
  UINT32  LicenseType;
} TPCM_GET_LICENSE_REQ_HEADER;
#pragma pack()

//
// Get license: Output cmd structure
//
#pragma pack(1)
typedef struct {
  UINT32  Tag;
  UINT32  Length;
  UINT32  Ret;
  UINT32  LicenseType;
  UINT32  RemainDays;
} TPCM_GET_LICENSE_RES;
#pragma pack()

//
// Get TPCM status: Input cmd structure
//
#pragma pack(1)
typedef struct {
  UINT32  Tag;
  UINT32  Length;
  UINT32  Command;
  UINT64  ReportTime;
} TPCM_GET_STS_REQ_HEADER;
#pragma pack()

//
// Get TPCM satus: Output cmd structure
//
#pragma pack(1)
typedef struct {
  UINT64 BeHostTime;
  UINT8  Res1[80];
  UINT32 Res2;
  UINT32 Res3;
  UINT32 Res4;
  UINT32 Res5;
  UINT32 Res6;
  UINT32 Res7;
  UINT32 Res8;
  UINT32 Res9;
  UINT32 Res10;
  UINT32 Res11;
  UINT32 Res12;
  UINT64 Res13;
  UINT32 Res14;
  UINT32 Res15;
  UINT32 BeTpcmType;
  UINT32 Res16;
  UINT32 Res17;
  UINT32 BeTpcmTotalFlash;
  UINT32 BeTpcmWhiltelistAvaiFlash;
  UINT32 Res18; // 0: trusted 1: not-tructed 2: non
  UINT32 Res19;
  UINT32 Res20;
  UINT32 Res21;
  UINT32 BeTpcmFirmwareVersion;
  UINT32 Res22;
  UINT32 Res23;
  UINT32 BeSmkGenerated;
  UINT32 Res24;
  UINT32 Res25;
  UINT32 Res26;
  UINT8  TpcmId[32];
} TPCM_STATUS;
#pragma pack()

#pragma pack(1)
typedef struct {
  UINT32       Tag;
  UINT32       Length;
  UINT32       Ret;
  TPCM_STATUS  TpcmInfo;
} TPCM_GET_STS_RES;
#pragma pack()

//
// Get TPCM en/disable status: Input cmd structure
//
#pragma pack(1)
typedef struct {
  UINT32  Tag;
  UINT32  Length;
  UINT32  Command;
  UINT8   TpcmId[32];
} TPCM_GET_CTRL_STS_REQ_HEADER;
#pragma pack()

//
// Get TPCM en/disable status: Output cmd structure
//
#pragma pack(1)
typedef struct {
  UINT32  Tag;
  UINT32  Length;
  UINT32  Ret;
  UINT32  Enabled; // 0: disabled; or enabled
} TPCM_GET_CTRL_STS_RES;
#pragma pack()

//
// En/Disable TPCM: Input cmd structure
//
#pragma pack(1)
typedef struct {
  UINT32  Tag;
  UINT32  Length;
  UINT32  Command;
  UINT8   TpcmId[32];
  UINT32  Enabled;
} TPCM_SET_CTRL_STS_REQ_HEADER;
#pragma pack()

//
// En/Disable TPCM: Output cmd structure
//
#pragma pack(1)
typedef struct {
  UINT32  Tag;
  UINT32  Length;
  UINT32  Ret;
} TPCM_SET_CTRL_STS_RES;
#pragma pack()

//
// Image Address Range
//
typedef struct {
  UINT64 Start;
  UINT64 Length;
} IMAGE_ADDR_RANGE;

typedef
EFI_STATUS
(EFIAPI *SG_TPCM_PROTOCOL_INIT) (
  VOID
  );

typedef
EFI_STATUS
(EFIAPI *SG_TPCM_PROTOCOL_MEASURE) (
  IN CONST UINT8  *Hash,
  IN UINTN        DataSize
  );

typedef
EFI_STATUS
(EFIAPI *SG_TPCM_PROTOCOL_GET_LICENSE) (
  IN OUT UINT32      *ShelfLife
  );

typedef
EFI_STATUS
(EFIAPI *SG_TPCM_PROTOCOL_GET_STATUS) (
  IN OUT TPCM_STATUS *TpcmInfo
  );

typedef
EFI_STATUS
(EFIAPI *SG_TPCM_PROTOCOL_GET_CONTROL_STATUS) (
  IN     TPCM_STATUS *TpcmInfo,
  IN OUT BOOLEAN     *IsEnabled
  );

typedef
EFI_STATUS
(EFIAPI *SG_TPCM_PROTOCOL_SET_CONTROL_STATUS) (
  IN TPCM_STATUS     *TpcmInfo,
  IN BOOLEAN         IsEnabled
  );

struct _SOPHGO_TPCM_PROTOCOL {
  SG_TPCM_PROTOCOL_INIT               InitTpcm;
  SG_TPCM_PROTOCOL_MEASURE            Measure;
  SG_TPCM_PROTOCOL_GET_LICENSE        GetTpcmLicense;
  SG_TPCM_PROTOCOL_GET_STATUS         GetTpcmStatus;
  SG_TPCM_PROTOCOL_GET_CONTROL_STATUS GetTpcmCtrlStatus;
  SG_TPCM_PROTOCOL_SET_CONTROL_STATUS SetTpcmCtrlStatus;
};

typedef
EFI_STATUS
(EFIAPI *TPCM_IMAGE_VERIFY_RAW) (
  IN  TPCM_IMAGE_VERIFY_PROTOCOL *This,
  IN  UINT32                     MeasureStage,
  IN  VOID                       *ImageInfo,
  IN  UINT32                     ImageInfoSize,
  IN  UINT32                     NumAddrRange,
  IN  IMAGE_ADDR_RANGE           Ranges[],
  OUT UINT32                     *MeasureResult,
  OUT UINT32                     *ControlResult
  );

typedef
BOOLEAN
(EFIAPI *TPCM_IMAGE_IS_VERIFY_ENABLED) (
  IN TPCM_IMAGE_VERIFY_PROTOCOL *This
  );

typedef struct _TPCM_IMAGE_VERIFY_PROTOCOL {
  TPCM_IMAGE_VERIFY_RAW        TpcmVerifyRaw;
  TPCM_IMAGE_IS_VERIFY_ENABLED IsTpcmEnabled;
} TPCM_IMAGE_VERIFY_PROTOCOL;

#endif // _SOPHGO_TPCM_H_
