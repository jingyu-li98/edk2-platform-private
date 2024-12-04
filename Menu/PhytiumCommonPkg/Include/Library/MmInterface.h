/** @file
Define the implementation of phytium mm interface communicate.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  MM_INTERFACE_H_
#define  MM_INTERFACE_H_

#include <IndustryStandard/Acpi64.h>
#include <Library/MmSpec.h>
/**
  MM communication.
  1.Locate EFI_MM_COMMUNICATION2_PROTOCOL.
  2.MM Communicate.

  @param[in, out]   PaCommBuffer  Communication buffer physical address.
  @param[in, out]   VaCommBuffer  Communication buffer virtual address.
  @param[in, out]   CommSize      Communication buffer size.

  @retval           EFI_SUCCESS   Communicate successfully.
  @retval           Other         Communicate failed.
**/
EFI_STATUS
MmCommunicate (
  IN OUT  VOID    *PaCommBuffer,
  IN OUT  VOID    *VaCommBuffer,
  IN OUT  UINTN   *CommSize OPTIONAL
  );

/**
  Get HEST table header and Error source count from MM. The content of protocol
  refers to the Phytium MM Interface SPEC.

  @param[out]    Code    Complete code. 0 - success, other - failed.
  @param[out]    Header  A pointer to EFI_ACPI_6_4_HARDWARE_ERROR_SOURCE_TABLE_HEADER.

  @retval        EFI_SUCCESS           Communicate successfully.
  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
GetHestHeaderFromMm (
  OUT  UINT32                                           *Code,
  OUT  EFI_ACPI_6_4_HARDWARE_ERROR_SOURCE_TABLE_HEADER  *Header
  );

/**
  Get HEST table from MM. The content of protocol refers to the Phytium MM
  Interface SPEC. Main ID is 0x3, sub ID is 0x4.

  @param[in]     Header  The header obtained through the protocol 03-04.
  @param[out]    Code    Complete code. 0 - success, other - failed.
  @param[out]    Table   A pointer to HEST table.

  @retval        EFI_SUCCESS           Communicate successfully.
  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
GetHestTableFromMm (
  IN   EFI_ACPI_6_4_HARDWARE_ERROR_SOURCE_TABLE_HEADER  *Header,
  OUT  UINT32                                           *Code,
  OUT  UINT8                                            *Table
  );

/**
  Get BERT table from MM. The content of protocol refers to the Phytium MM
  Interface SPEC. Main ID is 0x3, sub ID is 0x5.

  @param[out]    Code    Complete code. 0 - success, other - failed.
  @param[out]    Header  A pointer to EFI_ACPI_6_4_HARDWARE_ERROR_SOURCE_TABLE_HEADER.

  @retval        EFI_SUCCESS           Communicate successfully.
  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
GetBertTableFromMm (
  OUT  UINT32                                      *Code,
  OUT  EFI_ACPI_6_4_BOOT_ERROR_RECORD_TABLE_HEADER *Header
  );

/**
  Get RAS reserved region from MM. The content of protocol refers to the
  Phytium MM Interface SPEC. Main ID is 0x3, sub ID is 0x6.

  @param[out]    Code    Complete code. 0 - success, other - failed.
  @param[out]    Header  A pointer to EFI_ACPI_6_4_HARDWARE_ERROR_SOURCE_TABLE_HEADER.

  @retval        EFI_SUCCESS           Communicate successfully.
  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
GetRasRegionFromMm (
  OUT  UINT32               *Code,
  OUT  RAS_RESERVED_MEMORY  *Region
  );

/**
  Get EINJ table header and Error source count from MM. The content of protocol
  refers to the Phytium MM Interface SPEC. Main ID is 0x3, sub ID is 0x7.

  @param[out]    Code    Complete code. 0 - success, other - failed.
  @param[out]    Header  A pointer to EFI_ACPI_6_4_HARDWARE_ERROR_SOURCE_TABLE_HEADER.

  @retval        EFI_SUCCESS           Communicate successfully.
  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
GetEinjHeaderFromMm (
  OUT  GET_EINJ_HEADER_CONTENT_RESP    *Resp
  );

/**
  Get EINJ table from MM. The content of protocol refers to the Phytium MM
  Interface SPEC. Main ID is 0x3, sub ID is 0x8.

  @param[in]     Header  The header obtained through the protocol 03-07.
  @param[out]    Code    Complete code. 0 - success, other - failed.
  @param[out]    Table   A pointer to EINJ table.

  @retval        EFI_SUCCESS           Communicate successfully.
  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
GetEinjTableFromMm (
  IN   EFI_ACPI_6_4_ERROR_INJECTION_TABLE_HEADER  *Header,
  OUT  UINT32                                     *Code,
  OUT  UINT8                                      *Table
  );

/**
  Sync time to MM. The content of protocol refers to the Phytium MM
  Interface SPEC. Main ID is 0x3, sub ID is 0x9.

  @param[in]  TimeStamp    Time to sync, unit:ms.
  @param[out] Code         Complete code. 0 - success, other - failed.

  @retval        EFI_SUCCESS           Communicate successfully.
  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
SyncTimeStampToMm (
  IN  UINT64    TimeStamp,
  OUT UINT32    *Code
  );

/**
  Sync memory device location information to MM (smbios type 17 and dimm slot ID
  relation). The content of protocol refers to the Phytium MM Interface SPEC.
  Main ID is 0x3, sub ID is 0xA.

  @param[in]  Info    A pointer to SYNC_MEMORY_DEVICE_LOCATION.
  @param[out] Code    Complete code. 0 - success, other - failed.

  @retval        EFI_SUCCESS           Communicate successfully.
  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
SyncMemoryDeviceLocationToMm (
  IN  SYNC_MEMORY_DEVICE_LOCATION  *Info,
  OUT UINT32                       *Code
  );

/**
  Sync ras strategy to MM. The content of protocol refers to the Phytium MM
  Interface SPEC. Main ID is 0x3, sub ID is 0xA.

  @param[in]  Info    A pointer to RAS_STRATEGY.
  @param[out] Code    Complete code. 0 - success, other - failed.

  @retval        EFI_SUCCESS           Communicate successfully.
  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
SyncRasStrategyToMm (
  IN  RAS_STRATEGY  *Info,
  OUT UINT32        *Code
  );

/**
  Firmware header check. The content of protocol refers to the Phytium MM
  Interface SPEC. Main ID is 0x1, sub ID is 0x1.

  @param[in]  Header    A pointer to BIOS_HEADER_CHECK_CONTENT_REQ
  @param[out] Code      Complete code. 0 - success, other - failed.

  @retval        EFI_SUCCESS           Communicate successfully.
  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
FirmwareHeaderCheck (
  IN  BIOS_HEADER_CHECK_CONTENT_REQ *Header,
  OUT UINT32                        *Code
  );

/**
  Firmware section check. The content of protocol refers to the Phytium MM
  Interface SPEC. Main ID is 0x1, sub ID is 0x2.

  @param[in]  Header    A pointer to BIOS_SECTION_CHECK_CONTENT_REQ
  @param[out] Code      Complete code. 0 - success, other - failed.

  @retval        EFI_SUCCESS           Communicate successfully.
  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
FirmwareSectionCheck (
  IN  BIOS_SECTION_CHECK_CONTENT_REQ  *Section,
  OUT UINT32                          *Code
  );

/**
  Firmware check end. The content of protocol refers to the Phytium MM
  Interface SPEC. Main ID is 0x1, sub ID is 0x3.

  @param[out] Code      Complete code. 0 - success, other - failed.

  @retval        EFI_SUCCESS           Communicate successfully.
  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
FirmwareCheckEnd (
  OUT UINT32                          *Code
  );

/**
  Firmware section update. The content of protocol refers to the Phytium MM
  Interface SPEC. Main ID is 0x2, sub ID is 0x1.

  @param[in]  Section    A pointer to BIOS_SECTION_UPDATE_CONTENT_REQ
  @param[out] Code       Complete code. 0 - success, other - failed.

  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
FirmwareSectionUpdate (
  IN  BIOS_SECTION_UPDATE_CONTENT_REQ *Section,
  OUT UINT32                          *Code
  );

/**
  Firmware update end. The content of protocol refers to the Phytium MM
  Interface SPEC. Main ID is 0x2, sub ID is 0x2.

  @param[out] Code      Complete code. 0 - success, other - failed.

  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
FirmwareSectionEnd (
  OUT UINT32                          *Code
  );

/**
  Get MM version information from MM. The content of protocol
  refers to the Phytium MM Interface SPEC. Main ID is 0x4, sub ID is 0x1.

  @param[out]  Resp    A pointer to GET_VERSION_INFO_CONTENT_RESP

  @retval        EFI_SUCCESS           Communicate successfully.
  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
GetVersionInfoFromMm (
  OUT  GET_VERSION_INFO_CONTENT_RESP  *Resp
  );

#endif
