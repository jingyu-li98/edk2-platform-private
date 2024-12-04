/** @file
Define phytium mm interface protocol according PHYTIUM MM SPEC.

Copyright (C) 2024, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  MM_SPEC_H_
#define  MM_SPEC_H_

#include <IndustryStandard/Acpi64.h>
#include <CommonRas.h>
#include <Uefi.h>

#pragma pack(1)

//uuid
#define  FIRMWARE_UPDATE_UUID  \
  {0xd0cdc720, 0x92f3, 0x11ec, {0x9f, 0x99, 0x1b, 0x83, 0xe3, 0x6f, 0x23, 0x78}}
#define  RAS_HADNLER_UUID  \
  {0x9f4786f0, 0xcc14, 0x11ec, {0x8d, 0x37, 0x77, 0x77, 0x36, 0x6a, 0xef, 0x00}}
#define  MANAGEMENT_UUID  \
  {0x9e110f8c, 0x17d6, 0x11ed, {0xa7, 0x97, 0x3f, 0x8e, 0xc6, 0x1c, 0x18, 0x5f}}

//
//MM Spec Main ID List
//
typedef enum _MM_MAIN_ID {
  BiosUpdateCheck  = 1,
  BiosUpdateUpdate = 2,
  RasFunction      = 3,
  MmManagement     = 4,
} MM_MAIN_ID;

//
//MM Spec Sub ID of Main ID 01
//
typedef enum _BIOS_CHECK_SUB_ID {
  BiosHeaderCheck  = 1,
  BiosSectionCheck = 2,
  BiosCheckEnd     = 3,
} BIOS_CHECK_SUB_ID;

//
//MM Spec Sub ID of Main ID 02
//
typedef enum _BIOS_UPDATE_SUB_ID {
  BiosSectionUpdate = 1,
  BiosUpdateEnd     = 2,
} BIOS_UPDATE_SUB_ID;

//
//MM Spec Sub ID of Main ID 03
//
typedef enum _RAS_SUB_ID {
  RasUpdateApeiTable = 1,
  RasGenerateGhes    = 2,
  RasHestHeaderGet   = 3,
  RasHestTableGet    = 4,
  RasBertTableGet    = 5,
  RasMemoryGet       = 6,
  RasEinjHeaderGet   = 7,
  RasEinjTableGet    = 8,
  RasTimeStampSync   = 9,
  RasMemoryDeviceLocationSync  = 10,
  RasStrategySync = 11,
  RasCleanStrategyRecord = 12,
} RAS_SUB_ID;

//
//MM Spec Sub Id of Main ID 04
//
typedef enum _MM_MANAGEMENT_SUB_ID {
  MmVersionInfoGet  = 1,
} MM_MANAGEMENT_SUB_ID;

//
//Result type
//
typedef enum _STMM_HANDLER_STATUS {
  StmmHandlerSuccess           = 0,
  StmmHandlerHeaderError       = 1,
  StmmHandlerLengthError       = 2,
  StmmHandlerFuncIDUnsupported = 3,
} STMM_HANDLER_STATUS;

//
//event 01-01 firmware header check result
//
typedef enum _FIRMWARE_HEADER_CHECK_RESULT {
  HeaderCheckSuccess = 0,
  HeaderInvalid      = 1,
  HeaderErrorStep    = 2,
} FIRMWARE_HEADER_CHECK_RESULT;

//
//event 01-02 firmware sectional check result
//
typedef enum _FIRMWARE_SECTIONAL_CHECK_RESULT {
  SectionalCheckSuccess        = 0,
  SectionalCheckPacketNumError = 1,
  SectionalCheckErrorStep      = 2,
  SectionalCheckLengthError    = 3,
} FIRMWARE_SECTIONAL_CHECK_RESULT;

//
//event 01-03 check end result
//
typedef enum _FIRMWARE_CHECK_END_RESULT {
  CheckEndSuccess          = 0,
  CheckFirmwareLengthError = 1,
  CheckResultError         = 2,
  CheckErrorStep           = 3,
} FIRMWARE_CHECK_END_RESULT;

//
//event 02-01 firmware sectional update result
//
typedef enum _FIRMWARE_SECTIONAL_UPDATE_RESULT {
  SectionalUpdateSuccess         = 0,
  SectionalPacketNumError        = 1,
  SectionalUpdateAddressError    = 2,
  SectionalCheckFailed           = 3,
  SectionalUpdateFlashFailed     = 4,
  SectionalNoCheckEndMes         = 5,
  SectionalMesLenError           = 6,
} FIRMWARE_SECTIONAL_UPDATE_RESULT;

//
//event 02-02 firmware update end result
//
typedef enum _FIRMWARE_UPDATE_END_RESULT {
  FirmwareUpdateEndSuccess       = 0,
  FirmwareUpdateEndFilesizeError = 1,
  FirmwareUpdateEndErrorStep     = 2,
} FIRMWARE_UPDATE_END_RESULT;

//
//event 03-01 result
//
typedef enum _RAS_UPDATE_APEI_TABLE_RESULT {
  ResultSuccess    = 0,
  ExceedMaxNumber  = 1,
} RAS_UPDATE_APEI_TABLE_RESULT;

//
//event 03-02 result
//
typedef enum _RAS_GENERATE_GHES_ERROR {
  GenerateGhesSuccess        = 0,
  GenerateGhesSignatureError = 1,
  GenerateGhesTimeout        = 2,
  GenerateGhesNotNeedReport  = 3,
} RAS_GENERATE_GHES_ERROR;

//
//event 03-03 result
//
typedef enum _RAS_GET_HEST_HEADER_RESULT {
  GetHestHeaderSuccess  = 0,
  GetHestHeaderNotFound = 1,
} RAS_GET_HEST_HEADER_RESULT;

//
//event 03-04 result
//
typedef enum _RAS_GET_HEST_TABLE_RESULT {
  GetHestTableSuccess     = 0,
  GetHestTableNotFound    = 1,
  GetHestTableLengthError = 2,
} RAS_GET_HEST_TABLE_RESULT;

//
//event 03-05 result
//
typedef enum _RAS_GET_BERT_TABLE_RESULT {
  GetBertTableSuccess  = 0,
  GetBertTableNotFound = 1,
} RAS_GET_BERT_TABLE_RESULT;

//
//event 03-06 result
//
typedef enum _GET_RAS_MEMORY_RASULT {
  GetRasMemorySuccess = 0,
} GET_RAS_MEMORY_RASULT;

//
//event 03-07 result
//
typedef enum _RAS_GET_EINJ_HEADER_RESULT {
  GetEinjHeaderSuccess  = 0,
  GetEinjHeaderNotFound = 1,
} RAS_GET_EINJ_HEADER_RESULT;

//
//event 03-08 result
//
typedef enum _RAS_GET_EINJ_TABLE_RESULT {
  GetEinjTableSuccess     = 0,
  GetEinjTableNotFound    = 1,
  GetEinjTableLengthError = 2,
} RAS_GET_EINJ_TABLE_RESULT;

//
//event 03-09 result
//
typedef enum _RAS_TIMESTAMP_SYNC_RESULT {
  TimeStampSyncSuccess = 0,
} RAS_TIMESTAMP_SYNC_RESULT;

//
//event 03-0A result
//
typedef enum _RAS_MEMORY_DEVICE_LOCATION_SYNC_RESULT {
  MemoryDeviceLocationSyncSuccess = 0,
} RAS_MEMORY_DEVICE_LOCATION_SYNC_RESULT;

//
//event 03-0B result
//
typedef enum _RAS_SYNC_STRATEGY_RESULT {
  RasSyncStrategySuccess = 0,
} RAS_SYNC_STRATEGY_RESULT;

//
//MM Spec Sub ID of Main ID 04
//
typedef enum _MANAGE_SUB_ID {
  ManageVersionInfoGet = 1,
} MANAGE_SUB_ID;

//
//RAS Record Strategy
//
typedef enum _RAS_RECORD_TPYE {
  RasNoRecord = 0,
  RasFlashRecord = 1,
  RasBmcRecord = 2,
} RAS_RECORD_TPYE;

//
//Mm Conmucation Buffer Header
//
typedef struct _MM_COMM_CONTENT_HEADER {
  UINT16    MainId;
  UINT16    SubId;
  UINT32    Length;
} MM_COMM_CONTENT_HEADER;

//
//Main ID 01 Content Struct
//
//event 01-01 request
typedef struct _BIOS_HEADER_CHECK_CONTENT_REQ {
  UINT8    Vendor[16];
  UINT8    Product[32];
  UINT32   BuildTime;
  UINT32   FileSize;
  UINT8    Description[64];
  UINT32   TotalPackage;
  UINT8    Md5[16];
} BIOS_HEADER_CHECK_CONTENT_REQ;

//event 01-02 request
typedef struct _BIOS_SECTION_CHECK_CONTENT_REQ {
  UINT32   PacketNum;
  UINT32   PacketLength;
  UINT8    Data[1];
} BIOS_SECTION_CHECK_CONTENT_REQ;

//
//Main ID 02 Content Struct
//
//event 02-01 request
typedef struct _BIOS_SECTION_UPDATE_CONTENT_REQ {
  UINT32   PacketNum;
  UINT32   Size;
  UINT64   Address;
  UINT8    Data[1];
} BIOS_SECTION_UPDATE_CONTENT_REQ;

typedef struct _APEI_SOURCE_TABLE {
  UINT64    Base;
  UINT32    Size;
  UINT16    ErrorSourceNum;
  UINT16    Type;
  UINT64    AckReg;
  UINT64    AckPreserve;
  UINT64    AckWrite;
} APEI_SOURCE_TABLE;

//
//Main ID 03 Content Struct
//
//
//event 03-01 request
//
typedef struct _UPDATE_APEI_TABLE_CONTENT_REQ {
  UINT32             ErrorSourceCount;
  APEI_SOURCE_TABLE  ApeiTable[1];
} UPDATE_APEI_TABLE_CONTENT_REQ;

//
//event 03-01 response
//
typedef struct _UPDATE_APEI_TABLE_CONTENT_RESP {
  UINT32    Result;
  UINT32    Reserved;
} UPDATE_APEI_TABLE_CONTENT_RESP;

//
//event 03-03 response
//
typedef struct _GET_HEST_HEADER_CONTENT_RESP {
  UINT32                                           Result;
  EFI_ACPI_6_4_HARDWARE_ERROR_SOURCE_TABLE_HEADER  Header;
} GET_HEST_HEADER_CONTENT_RESP;

//
//event 03-05 response
//
typedef struct _GET_BERT_TABLE_CONTENT_RESP {
  UINT32                                       Result;
  EFI_ACPI_6_4_BOOT_ERROR_RECORD_TABLE_HEADER  Table;
} GET_BERT_TABLE_CONTENT_RESP;

typedef struct _RAS_RESERVED_MEMORY {
  UINT64    Address;
  UINT64    Size;
} RAS_RESERVED_MEMORY;

//
//event 03-06 response
//
typedef struct _GET_RAS_MEMORY_CONTENT_RESP {
  UINT32               Result;
  RAS_RESERVED_MEMORY  Memory;
} GET_RAS_MEMORY_CONTENT_RESP;

//
//event 03-07 response
//
typedef struct _GET_EINJ_HEADER_CONTENT_RESP {
  UINT32                                     Result;
  EFI_ACPI_6_4_ERROR_INJECTION_TABLE_HEADER  Table;
} GET_EINJ_HEADER_CONTENT_RESP;

//
//Memory Device location information
//
typedef struct _MEMORY_DEVICE_LOCATION {
  UINT16  SlotId;
  UINT16  SmbiosHandle;
} MEMORY_DEVICE_LOCATION;

#define  MAX_MEMORY_DEVICE  128

//
//event 03-10
//
typedef struct _SYNC_MEMORY_DEVICE_LOCATION {
  UINT32                  Count;
  MEMORY_DEVICE_LOCATION  Location[MAX_MEMORY_DEVICE];
} SYNC_MEMORY_DEVICE_LOCATION;

//
//Main ID 04 Content Struct
//
//
//event 04-01 response
//
typedef struct _GET_VERSION_INFO_CONTENT_RESP {
  UINT32  Result;
  UINT16  MainVersion;
  UINT16  SubVersion;
  UINT32  BuildTime;
  UINT8   Description[64];
} GET_VERSION_INFO_CONTENT_RESP;

/**
  Communication frame framing from UEFI to MM. Message must be large enough.

  @param[in]       Guid       MM event ID.
  @param[in]       Header     Event message header.
  @param[in]       Content    Event content.
  @param[in, out]  Message    Message after framing.
  @param[out]      OutLen     Length of message after framing.
**/
VOID
UefiToMmCommFraming (
  IN  EFI_GUID                Guid,
  IN  MM_COMM_CONTENT_HEADER  *Header,
  IN  UINT8                   *Content,
  OUT UINT8                   *Message,
  OUT UINTN                   *OutLen
  );

/**
  Communication frame parse from MM to UEFI. Content must be large enough.

  @param[in]       Message    Message to parse.
  @param[out]      Header     Content header after parse.
  @param[out]      Content    content after parse.
**/
VOID
UefiToMmCommParse (
  IN   UINT8                   *Message,
  OUT  MM_COMM_CONTENT_HEADER  *Header,
  OUT  UINT8                   *Content
  );

#pragma pack()

#endif
