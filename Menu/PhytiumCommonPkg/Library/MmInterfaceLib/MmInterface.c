/** @file
Define the implementation of phytium mm interface communication. Use
EFI_MM_COMMUNICATION2_PROTOCOL. If using the this interface, it is necessary
to ensure that the driver EFI_MM_COMMUNICATION2_PROTOCOL is loaded.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/MmSpec.h>
#include <Library/MmInterface.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/MmCommunication2.h>

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
  )
{
  EFI_STATUS                      Status;
  EFI_MM_COMMUNICATION2_PROTOCOL  *MmCommunication;

  Status = EFI_SUCCESS;
  //
  //Locate EFI_MM_COMMUNICATION2_PROTOCOL
  //
  Status = gBS->LocateProtocol (
                  &gEfiMmCommunication2ProtocolGuid,
                  NULL,
                  (VOID **) &MmCommunication
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Locate MMCommunication Failed!\n"));
    return Status;
  }

  DEBUG ((DEBUG_INFO, "PaCommBuffer : 0x%llx, VaCommBuffer :0x%llx\n", (UINT64)PaCommBuffer, (UINT64)VaCommBuffer));
  DEBUG ((DEBUG_INFO, "CommSize : %x\n", *CommSize));
  //
  //MM Communicate
  //
  Status = MmCommunication->Communicate (
                                MmCommunication,
                                PaCommBuffer,
                                VaCommBuffer,
                                CommSize
                                );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PaCommBuffer : 0x%llx, VaCommBuffer :0x%llx\n", (UINT64)PaCommBuffer, (UINT64)VaCommBuffer));
    DEBUG ((DEBUG_ERROR, "CommSize : %x\n", *CommSize));
    DEBUG ((DEBUG_ERROR, "Mm Communicate Failed Status : %r\n", Status));
    return Status;
  }

  return Status;
}

/**
  Get HEST table header and Error source count from MM. The content of protocol
  refers to the Phytium MM Interface SPEC. Main ID is 0x3, sub ID is 0x3.

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
  )
{
  EFI_STATUS                      Status;
  UINT8                           *CommBuffer;
  UINTN                           CommSize;
  UINT8                           *ContentBuffer;
  EFI_GUID                        Guid = RAS_HADNLER_UUID;
  MM_COMM_CONTENT_HEADER          MmHeader;
  UINT64                          BufferLength;

  Status = EFI_SUCCESS;
  BufferLength = PcdGet64 (PcdMmBufferSize);
  CommBuffer = AllocatePool (BufferLength);
  if (CommBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a(), Comm Buffer Install failed!\n", __FUNCTION__));
    goto ProcExit;
  }

  ZeroMem (&MmHeader, sizeof (MM_COMM_CONTENT_HEADER));
  MmHeader.MainId = RasFunction;
  MmHeader.SubId = RasHestHeaderGet;
  MmHeader.Length = sizeof (EFI_ACPI_6_4_HARDWARE_ERROR_SOURCE_TABLE_HEADER) + 4;
  ContentBuffer = AllocatePool (MmHeader.Length);
  if (ContentBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a(), Content Buffer Install failed!\n", __FUNCTION__));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }
  CommSize = MmHeader.Length + sizeof (MM_COMM_CONTENT_HEADER) + sizeof (EFI_GUID) + 8;
  ZeroMem (ContentBuffer, MmHeader.Length);
  UefiToMmCommFraming (Guid, &MmHeader, ContentBuffer, CommBuffer, &CommSize);
  Status = MmCommunicate (CommBuffer, CommBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Mm Communicate Failed Status : %r\n", Status));
    goto ProcExit;
  }
  UefiToMmCommParse (CommBuffer, &MmHeader, ContentBuffer);
  CopyMem (Code, ContentBuffer, 4);
  CopyMem ((VOID *) Header, ContentBuffer + 4, MmHeader.Length - 4);

ProcExit:
  if (CommBuffer != NULL) {
    FreePool (CommBuffer);
  }
  if (ContentBuffer != NULL) {
    FreePool (ContentBuffer);
  }

  return Status;
}

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
  )
{
  EFI_STATUS                      Status;
  UINT8                           *CommBuffer;
  UINTN                           CommSize;
  UINT8                           *ContentBuffer;
  EFI_GUID                        Guid = RAS_HADNLER_UUID;
  MM_COMM_CONTENT_HEADER          MmHeader;
  UINTN                           BufferLength;

  Status = EFI_SUCCESS;
  BufferLength = PcdGet64 (PcdMmBufferSize);
  CommBuffer = AllocatePool (BufferLength);
  if (CommBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a(), Comm Buffer Install failed!\n", __FUNCTION__));
    goto ProcExit;
  }

  ZeroMem (&MmHeader, sizeof (MM_COMM_CONTENT_HEADER));
  MmHeader.MainId = RasFunction;
  MmHeader.SubId = RasHestTableGet;
  MmHeader.Length = Header->Header.Length + 4;
  ContentBuffer = AllocatePool (MmHeader.Length);
  if (ContentBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a(), Content Buffer Install failed!\n", __FUNCTION__));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }
  CommSize = MmHeader.Length + sizeof (MM_COMM_CONTENT_HEADER) + sizeof (EFI_GUID) + 8;
  ZeroMem (ContentBuffer, MmHeader.Length);
  DEBUG ((DEBUG_INFO, "CommSize : %x\n", CommSize));
  UefiToMmCommFraming (Guid, &MmHeader, ContentBuffer, CommBuffer, &CommSize);
  Status = MmCommunicate (CommBuffer, CommBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Mm Communicate Failed Status : %r\n", Status));
    goto ProcExit;
  }
  UefiToMmCommParse (CommBuffer, &MmHeader, ContentBuffer);
  CopyMem (Code, ContentBuffer, 4);
  CopyMem ((VOID *) Table, ContentBuffer + 4, MmHeader.Length - 4);

ProcExit:
  if (CommBuffer != NULL) {
    FreePool (CommBuffer);
  }
  if (ContentBuffer != NULL) {
    FreePool (ContentBuffer);
  }

  return Status;
}

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
  )
{
  EFI_STATUS                      Status;
  UINT8                           *CommBuffer;
  UINTN                           CommSize;
  UINT8                           *ContentBuffer;
  EFI_GUID                        Guid = RAS_HADNLER_UUID;
  MM_COMM_CONTENT_HEADER          MmHeader;
  UINT64                          BufferLength;

  Status = EFI_SUCCESS;
  BufferLength = PcdGet64 (PcdMmBufferSize);
  CommBuffer = AllocatePool (BufferLength);
  if (CommBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a(), Comm Buffer Install failed!\n", __FUNCTION__));
    goto ProcExit;
  }
  ZeroMem (&MmHeader, sizeof (MM_COMM_CONTENT_HEADER));
  MmHeader.MainId = RasFunction;
  MmHeader.SubId = RasBertTableGet;
  MmHeader.Length = sizeof (EFI_ACPI_6_4_BOOT_ERROR_RECORD_TABLE_HEADER) + 4;
  ContentBuffer = AllocatePool (MmHeader.Length);
  if (ContentBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a(), Content Buffer Install failed!\n", __FUNCTION__));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }
  CommSize = MmHeader.Length + sizeof (MM_COMM_CONTENT_HEADER) + sizeof (EFI_GUID) + 8;
  ZeroMem (ContentBuffer, MmHeader.Length);
  UefiToMmCommFraming (Guid, &MmHeader, ContentBuffer, CommBuffer, &CommSize);
  Status = MmCommunicate (CommBuffer, CommBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Mm Communicate Failed Status : %r\n", Status));
    goto ProcExit;
  }
  UefiToMmCommParse (CommBuffer, &MmHeader, ContentBuffer);
  CopyMem (Code, ContentBuffer, 4);
  CopyMem ((VOID *) Header, ContentBuffer + 4, MmHeader.Length - 4);

ProcExit:
  if (CommBuffer != NULL) {
    FreePool (CommBuffer);
  }
  if (ContentBuffer != NULL) {
    FreePool (ContentBuffer);
  }

  return Status;
}

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
  )
{
  EFI_STATUS                      Status;
  UINT8                           *CommBuffer;
  UINTN                           CommSize;
  UINT8                           *ContentBuffer;
  EFI_GUID                        Guid = RAS_HADNLER_UUID;
  MM_COMM_CONTENT_HEADER          MmHeader;
  UINT64                          BufferLength;

  Status = EFI_SUCCESS;
  BufferLength = PcdGet64 (PcdMmBufferSize);
  CommBuffer = AllocatePool (BufferLength);
  if (CommBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a(), Comm Buffer Install failed!\n", __FUNCTION__));
    goto ProcExit;
  }
  ZeroMem (&MmHeader, sizeof (MM_COMM_CONTENT_HEADER));
  MmHeader.MainId = RasFunction;
  MmHeader.SubId = RasMemoryGet;
  //UINT64(Base) + UINT64(Size) + UINT32(Code)
  MmHeader.Length = 20;
  ContentBuffer = AllocatePool (MmHeader.Length);
  if (ContentBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a(), Content Buffer Install failed!\n", __FUNCTION__));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }
  CommSize = MmHeader.Length + sizeof (MM_COMM_CONTENT_HEADER) + sizeof (EFI_GUID) + 8;
  ZeroMem (ContentBuffer, MmHeader.Length);
  UefiToMmCommFraming (Guid, &MmHeader, ContentBuffer, CommBuffer, &CommSize);
  Status = MmCommunicate (CommBuffer, CommBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Mm Communicate Failed Status : %r\n", Status));
    goto ProcExit;
  }
  UefiToMmCommParse (CommBuffer, &MmHeader, ContentBuffer);
  CopyMem (Code, ContentBuffer, 4);
  CopyMem ((VOID *) Region, ContentBuffer + 4, MmHeader.Length - 4);

ProcExit:
  if (CommBuffer != NULL) {
    FreePool (CommBuffer);
  }
  if (ContentBuffer != NULL) {
    FreePool (ContentBuffer);
  }

  return Status;
}

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
  )
{
  EFI_STATUS                      Status;
  UINT8                           *CommBuffer;
  UINTN                           CommSize;
  UINT8                           *ContentBuffer;
  EFI_GUID                        Guid = RAS_HADNLER_UUID;
  MM_COMM_CONTENT_HEADER          MmHeader;
  UINT64                          BufferLength;

  Status = EFI_SUCCESS;
  BufferLength = PcdGet64 (PcdMmBufferSize);
  CommBuffer = AllocatePool (BufferLength);
  if (CommBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a(), Comm Buffer Install failed!\n", __FUNCTION__));
    goto ProcExit;
  }

  ZeroMem (&MmHeader, sizeof (MM_COMM_CONTENT_HEADER));
  MmHeader.MainId = RasFunction;
  MmHeader.SubId = RasEinjHeaderGet;
  MmHeader.Length = sizeof (GET_EINJ_HEADER_CONTENT_RESP);
  ContentBuffer = AllocatePool (MmHeader.Length);
  if (ContentBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a(), Content Buffer Install failed!\n", __FUNCTION__));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }
  CommSize = MmHeader.Length + sizeof (MM_COMM_CONTENT_HEADER) + sizeof (EFI_GUID) + 8;
  ZeroMem (ContentBuffer, MmHeader.Length);
  UefiToMmCommFraming (Guid, &MmHeader, ContentBuffer, CommBuffer, &CommSize);
  Status = MmCommunicate (CommBuffer, CommBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Mm Communicate Failed Status : %r\n", Status));
    goto ProcExit;
  }
  UefiToMmCommParse (CommBuffer, &MmHeader, ContentBuffer);
  CopyMem ((VOID *) Resp, ContentBuffer, MmHeader.Length);

ProcExit:
  if (CommBuffer != NULL) {
    FreePool (CommBuffer);
  }
  if (ContentBuffer != NULL) {
    FreePool (ContentBuffer);
  }

  return Status;
}

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
  )
{
  EFI_STATUS                      Status;
  UINT8                           *CommBuffer;
  UINTN                           CommSize;
  UINT8                           *ContentBuffer;
  EFI_GUID                        Guid = RAS_HADNLER_UUID;
  MM_COMM_CONTENT_HEADER          MmHeader;
  UINTN                           BufferLength;

  Status = EFI_SUCCESS;
  BufferLength = PcdGet64 (PcdMmBufferSize);
  CommBuffer = AllocatePool (BufferLength);
  if (CommBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a(), Comm Buffer Install failed!\n", __FUNCTION__));
    goto ProcExit;
  }

  ZeroMem (&MmHeader, sizeof (MM_COMM_CONTENT_HEADER));
  MmHeader.MainId = RasFunction;
  MmHeader.SubId = RasEinjTableGet;
  MmHeader.Length = Header->Header.Length + 4;
  ContentBuffer = AllocatePool (MmHeader.Length);
  if (ContentBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a(), Content Buffer Install failed!\n", __FUNCTION__));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }
  CommSize = MmHeader.Length + sizeof (MM_COMM_CONTENT_HEADER) + sizeof (EFI_GUID) + 8;
  ZeroMem (ContentBuffer, MmHeader.Length);
  DEBUG ((DEBUG_INFO, "CommSize : %x\n", CommSize));
  UefiToMmCommFraming (Guid, &MmHeader, ContentBuffer, CommBuffer, &CommSize);
  Status = MmCommunicate (CommBuffer, CommBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Mm Communicate Failed Status : %r\n", Status));
    goto ProcExit;
  }
  UefiToMmCommParse (CommBuffer, &MmHeader, ContentBuffer);
  CopyMem (Code, ContentBuffer, 4);
  CopyMem ((VOID *) Table, ContentBuffer + 4, MmHeader.Length - 4);

ProcExit:
  if (CommBuffer != NULL) {
    FreePool (CommBuffer);
  }
  if (ContentBuffer != NULL) {
    FreePool (ContentBuffer);
  }

  return Status;
}

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
  )
{
  EFI_STATUS                      Status;
  UINT8                           *CommBuffer;
  UINTN                           CommSize;
  UINT8                           *ContentBuffer;
  EFI_GUID                        Guid = RAS_HADNLER_UUID;
  MM_COMM_CONTENT_HEADER          MmHeader;
  UINTN                           BufferLength;

  Status = EFI_SUCCESS;
  BufferLength = PcdGet64 (PcdMmBufferSize);
  CommBuffer = AllocatePool (BufferLength);
  if (CommBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a(), Comm Buffer Install failed!\n", __FUNCTION__));
    goto ProcExit;
  }

  ZeroMem (&MmHeader, sizeof (MM_COMM_CONTENT_HEADER));
  MmHeader.MainId = RasFunction;
  MmHeader.SubId = RasTimeStampSync;
  MmHeader.Length =  8;
  ContentBuffer = AllocatePool (MmHeader.Length);
  if (ContentBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a(), Content Buffer Install failed!\n", __FUNCTION__));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }
  CommSize = MmHeader.Length + sizeof (MM_COMM_CONTENT_HEADER) + sizeof (EFI_GUID) + 8;
  ZeroMem (ContentBuffer, MmHeader.Length);
  CopyMem (ContentBuffer, (VOID *) &TimeStamp, 8);
  DEBUG ((DEBUG_INFO, "CommSize : %x\n", CommSize));
  UefiToMmCommFraming (Guid, &MmHeader, ContentBuffer, CommBuffer, &CommSize);
  Status = MmCommunicate (CommBuffer, CommBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Mm Communicate Failed Status : %r\n", Status));
    goto ProcExit;
  }
  UefiToMmCommParse (CommBuffer, &MmHeader, ContentBuffer);
  CopyMem (Code, ContentBuffer, 4);

ProcExit:
  if (CommBuffer != NULL) {
    FreePool (CommBuffer);
  }
  if (ContentBuffer != NULL) {
    FreePool (ContentBuffer);
  }

  return Status;
}

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
  )
{
  EFI_STATUS                      Status;
  UINT8                           *CommBuffer;
  UINTN                           CommSize;
  UINT8                           *ContentBuffer;
  EFI_GUID                        Guid = RAS_HADNLER_UUID;
  MM_COMM_CONTENT_HEADER          MmHeader;
  UINTN                           BufferLength;

  Status = EFI_SUCCESS;
  BufferLength = PcdGet64 (PcdMmBufferSize);
  CommBuffer = AllocatePool (BufferLength);
  if (CommBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a(), Comm Buffer Install failed!\n", __FUNCTION__));
    goto ProcExit;
  }

  ZeroMem (&MmHeader, sizeof (MM_COMM_CONTENT_HEADER));
  MmHeader.MainId = RasFunction;
  MmHeader.SubId = RasMemoryDeviceLocationSync;
  MmHeader.Length =  4 + Info->Count * sizeof (MEMORY_DEVICE_LOCATION);
  ContentBuffer = AllocatePool (MmHeader.Length);
  if (ContentBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a(), Content Buffer Install failed!\n", __FUNCTION__));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }
  CommSize = MmHeader.Length + sizeof (MM_COMM_CONTENT_HEADER) + sizeof (EFI_GUID) + 8;
  ZeroMem (ContentBuffer, MmHeader.Length);
  CopyMem (ContentBuffer, (VOID *) Info, MmHeader.Length);
  DEBUG ((DEBUG_INFO, "CommSize : %x\n", CommSize));
  UefiToMmCommFraming (Guid, &MmHeader, ContentBuffer, CommBuffer, &CommSize);
  Status = MmCommunicate (CommBuffer, CommBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Mm Communicate Failed Status : %r\n", Status));
    goto ProcExit;
  }
  UefiToMmCommParse (CommBuffer, &MmHeader, ContentBuffer);
  CopyMem (Code, ContentBuffer, 4);

ProcExit:
  if (CommBuffer != NULL) {
    FreePool (CommBuffer);
  }
  if (ContentBuffer != NULL) {
    FreePool (ContentBuffer);
  }

  return Status;
}

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
  )
{
  EFI_STATUS                      Status;
  UINT8                           *CommBuffer;
  UINTN                           CommSize;
  UINT8                           *ContentBuffer;
  EFI_GUID                        Guid = RAS_HADNLER_UUID;
  MM_COMM_CONTENT_HEADER          MmHeader;
  UINTN                           BufferLength;

  Status = EFI_SUCCESS;
  BufferLength = PcdGet64 (PcdMmBufferSize);
  CommBuffer = AllocatePool (BufferLength);
  if (CommBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a(), Comm Buffer Install failed!\n", __FUNCTION__));
    goto ProcExit;
  }

  ZeroMem (&MmHeader, sizeof (MM_COMM_CONTENT_HEADER));
  MmHeader.MainId = RasFunction;
  MmHeader.SubId = RasStrategySync;
  MmHeader.Length =  sizeof (RAS_STRATEGY);
  ContentBuffer = AllocatePool (MmHeader.Length);
  if (ContentBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a(), Content Buffer Install failed!\n", __FUNCTION__));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }
  CommSize = MmHeader.Length + sizeof (MM_COMM_CONTENT_HEADER) + sizeof (EFI_GUID) + 8;
  ZeroMem (ContentBuffer, MmHeader.Length);
  CopyMem (ContentBuffer, (VOID *) Info, MmHeader.Length);
  DEBUG ((DEBUG_INFO, "CommSize : %x\n", CommSize));
  UefiToMmCommFraming (Guid, &MmHeader, ContentBuffer, CommBuffer, &CommSize);
  Status = MmCommunicate (CommBuffer, CommBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Mm Communicate Failed Status : %r\n", Status));
    goto ProcExit;
  }
  UefiToMmCommParse (CommBuffer, &MmHeader, ContentBuffer);
  CopyMem (Code, ContentBuffer, 4);

ProcExit:
  if (CommBuffer != NULL) {
    FreePool (CommBuffer);
  }
  if (ContentBuffer != NULL) {
    FreePool (ContentBuffer);
  }

  return Status;
}

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
  )
{
  EFI_STATUS                      Status;
  UINT8                           *CommBuffer;
  UINTN                           CommSize;
  UINT8                           *ContentBuffer;
  EFI_GUID                        Guid = FIRMWARE_UPDATE_UUID;
  MM_COMM_CONTENT_HEADER          MmHeader;
  UINTN                           BufferLength;

  Status = EFI_SUCCESS;
  BufferLength = PcdGet64 (PcdMmBufferSize);
  CommBuffer = AllocatePool (BufferLength);
  if (CommBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a(), Comm Buffer Install failed!\n", __FUNCTION__));
    goto ProcExit;
  }

  ZeroMem (&MmHeader, sizeof (MM_COMM_CONTENT_HEADER));
  MmHeader.MainId = BiosUpdateCheck;
  MmHeader.SubId = BiosHeaderCheck;
  MmHeader.Length =  sizeof (BIOS_HEADER_CHECK_CONTENT_REQ);
  ContentBuffer = AllocatePool (MmHeader.Length);
  if (ContentBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a(), Content Buffer Install failed!\n", __FUNCTION__));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }
  CommSize = MmHeader.Length + sizeof (MM_COMM_CONTENT_HEADER) + sizeof (EFI_GUID) + 8;
  ZeroMem (ContentBuffer, MmHeader.Length);
  CopyMem (ContentBuffer, (VOID *) Header, MmHeader.Length);
  DEBUG ((DEBUG_INFO, "CommSize : %x\n", CommSize));
  UefiToMmCommFraming (Guid, &MmHeader, ContentBuffer, CommBuffer, &CommSize);
  Status = MmCommunicate (CommBuffer, CommBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Mm Communicate Failed Status : %r\n", Status));
    goto ProcExit;
  }
  UefiToMmCommParse (CommBuffer, &MmHeader, ContentBuffer);
  CopyMem (Code, ContentBuffer, 4);

ProcExit:
  if (CommBuffer != NULL) {
    FreePool (CommBuffer);
  }
  if (ContentBuffer != NULL) {
    FreePool (ContentBuffer);
  }

  return Status;
}

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
  )
{
  EFI_STATUS                      Status;
  UINT8                           *CommBuffer;
  UINTN                           CommSize;
  UINT8                           *ContentBuffer;
  EFI_GUID                        Guid = FIRMWARE_UPDATE_UUID;
  MM_COMM_CONTENT_HEADER          MmHeader;
  UINTN                           BufferLength;

  Status = EFI_SUCCESS;
  BufferLength = PcdGet64 (PcdMmBufferSize);
  CommBuffer = AllocatePool (BufferLength);
  if (CommBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a(), Comm Buffer Install failed!\n", __FUNCTION__));
    goto ProcExit;
  }

  ZeroMem (&MmHeader, sizeof (MM_COMM_CONTENT_HEADER));
  MmHeader.MainId = BiosUpdateCheck;
  MmHeader.SubId = BiosSectionCheck;
  MmHeader.Length =  Section->PacketLength + 8;
  ContentBuffer = AllocatePool (MmHeader.Length);
  if (ContentBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a(), Content Buffer Install failed!\n", __FUNCTION__));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }
  CommSize = MmHeader.Length + sizeof (MM_COMM_CONTENT_HEADER) + sizeof (EFI_GUID) + 8;
  ZeroMem (ContentBuffer, MmHeader.Length);
  CopyMem (ContentBuffer, (VOID *) Section, MmHeader.Length);
  DEBUG ((DEBUG_INFO, "CommSize : %x\n", CommSize));
  UefiToMmCommFraming (Guid, &MmHeader, ContentBuffer, CommBuffer, &CommSize);
  Status = MmCommunicate (CommBuffer, CommBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Mm Communicate Failed Status : %r\n", Status));
    goto ProcExit;
  }
  UefiToMmCommParse (CommBuffer, &MmHeader, ContentBuffer);
  CopyMem (Code, ContentBuffer, 4);

ProcExit:
  if (CommBuffer != NULL) {
    FreePool (CommBuffer);
  }
  if (ContentBuffer != NULL) {
    FreePool (ContentBuffer);
  }

  return Status;
}

/**
  Firmware check end. The content of protocol refers to the Phytium MM
  Interface SPEC. Main ID is 0x1, sub ID is 0x3.

  @param[out] Code      Complete code. 0 - success, other - failed.

  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
FirmwareCheckEnd (
  OUT UINT32                          *Code
  )
{
  EFI_STATUS                      Status;
  UINT8                           *CommBuffer;
  UINTN                           CommSize;
  UINT8                           *ContentBuffer;
  EFI_GUID                        Guid = FIRMWARE_UPDATE_UUID;
  MM_COMM_CONTENT_HEADER          MmHeader;
  UINTN                           BufferLength;

  Status = EFI_SUCCESS;
  BufferLength = PcdGet64 (PcdMmBufferSize);
  CommBuffer = AllocatePool (BufferLength);
  if (CommBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a(), Comm Buffer Install failed!\n", __FUNCTION__));
    goto ProcExit;
  }

  ZeroMem (&MmHeader, sizeof (MM_COMM_CONTENT_HEADER));
  MmHeader.MainId = BiosUpdateCheck;
  MmHeader.SubId = BiosSectionCheck;
  MmHeader.Length =  4;
  ContentBuffer = AllocatePool (MmHeader.Length);
  if (ContentBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a(), Content Buffer Install failed!\n", __FUNCTION__));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }
  CommSize = MmHeader.Length + sizeof (MM_COMM_CONTENT_HEADER) + sizeof (EFI_GUID) + 8;
  ZeroMem (ContentBuffer, MmHeader.Length);
  UefiToMmCommFraming (Guid, &MmHeader, ContentBuffer, CommBuffer, &CommSize);
  Status = MmCommunicate (CommBuffer, CommBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Mm Communicate Failed Status : %r\n", Status));
    goto ProcExit;
  }
  UefiToMmCommParse (CommBuffer, &MmHeader, ContentBuffer);
  CopyMem (Code, ContentBuffer, 4);

ProcExit:
  if (CommBuffer != NULL) {
    FreePool (CommBuffer);
  }
  if (ContentBuffer != NULL) {
    FreePool (ContentBuffer);
  }

  return Status;
}

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
  )
{
  EFI_STATUS                      Status;
  UINT8                           *CommBuffer;
  UINTN                           CommSize;
  UINT8                           *ContentBuffer;
  EFI_GUID                        Guid = FIRMWARE_UPDATE_UUID;
  MM_COMM_CONTENT_HEADER          MmHeader;
  UINTN                           BufferLength;

  Status = EFI_SUCCESS;
  BufferLength = PcdGet64 (PcdMmBufferSize);
  CommBuffer = AllocatePool (BufferLength);
  if (CommBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a(), Comm Buffer Install failed!\n", __FUNCTION__));
    goto ProcExit;
  }

  ZeroMem (&MmHeader, sizeof (MM_COMM_CONTENT_HEADER));
  MmHeader.MainId = BiosUpdateUpdate;
  MmHeader.SubId = BiosSectionUpdate;
  MmHeader.Length =  sizeof (BIOS_SECTION_UPDATE_CONTENT_REQ) + 16;
  ContentBuffer = AllocatePool (MmHeader.Length);
  if (ContentBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a(), Content Buffer Install failed!\n", __FUNCTION__));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }
  CommSize = MmHeader.Length + sizeof (MM_COMM_CONTENT_HEADER) + sizeof (EFI_GUID) + 8;
  ZeroMem (ContentBuffer, MmHeader.Length);
  CopyMem (ContentBuffer, (VOID *) Section, MmHeader.Length);
  UefiToMmCommFraming (Guid, &MmHeader, ContentBuffer, CommBuffer, &CommSize);
  Status = MmCommunicate (CommBuffer, CommBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Mm Communicate Failed Status : %r\n", Status));
    goto ProcExit;
  }
  UefiToMmCommParse (CommBuffer, &MmHeader, ContentBuffer);
  CopyMem (Code, ContentBuffer, 4);

ProcExit:
  if (CommBuffer != NULL) {
    FreePool (CommBuffer);
  }
  if (ContentBuffer != NULL) {
    FreePool (ContentBuffer);
  }

  return Status;
}

/**
  Firmware update end. The content of protocol refers to the Phytium MM
  Interface SPEC. Main ID is 0x2, sub ID is 0x2.

  @param[out] Code      Complete code. 0 - success, other - failed.

  @retval        EFI_OUT_OF_RESOURCES  Allocate communication buffer or content
                                       buffer failed.
  @retval        Other                 Communicate failed.
**/
EFI_STATUS
FirmwareUpdateEnd (
  OUT UINT32                          *Code
  )
{
  EFI_STATUS                      Status;
  UINT8                           *CommBuffer;
  UINTN                           CommSize;
  UINT8                           *ContentBuffer;
  EFI_GUID                        Guid = FIRMWARE_UPDATE_UUID;
  MM_COMM_CONTENT_HEADER          MmHeader;
  UINTN                           BufferLength;

  Status = EFI_SUCCESS;
  BufferLength = PcdGet64 (PcdMmBufferSize);
  CommBuffer = AllocatePool (BufferLength);
  if (CommBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a(), Comm Buffer Install failed!\n", __FUNCTION__));
    goto ProcExit;
  }

  ZeroMem (&MmHeader, sizeof (MM_COMM_CONTENT_HEADER));
  MmHeader.MainId = BiosUpdateUpdate;
  MmHeader.SubId = BiosCheckEnd;
  MmHeader.Length = 4;
  ContentBuffer = AllocatePool (MmHeader.Length);
  if (ContentBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a(), Content Buffer Install failed!\n", __FUNCTION__));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }
  CommSize = MmHeader.Length + sizeof (MM_COMM_CONTENT_HEADER) + sizeof (EFI_GUID) + 8;
  ZeroMem (ContentBuffer, MmHeader.Length);
  UefiToMmCommFraming (Guid, &MmHeader, ContentBuffer, CommBuffer, &CommSize);
  Status = MmCommunicate (CommBuffer, CommBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Mm Communicate Failed Status : %r\n", Status));
    goto ProcExit;
  }
  UefiToMmCommParse (CommBuffer, &MmHeader, ContentBuffer);
  CopyMem (Code, ContentBuffer, 4);

ProcExit:
  if (CommBuffer != NULL) {
    FreePool (CommBuffer);
  }
  if (ContentBuffer != NULL) {
    FreePool (ContentBuffer);
  }

  return Status;
}

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
  )
{
  EFI_STATUS                      Status;
  UINT8                           *CommBuffer;
  UINTN                           CommSize;
  UINT8                           *ContentBuffer;
  EFI_GUID                        Guid = MANAGEMENT_UUID;
  MM_COMM_CONTENT_HEADER          MmHeader;
  UINT64                          BufferLength;

  Status = EFI_SUCCESS;
  BufferLength = PcdGet64 (PcdMmBufferSize);
  CommBuffer = AllocatePool (BufferLength);
  if (CommBuffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a(), Comm Buffer Install failed!\n", __FUNCTION__));
    goto ProcExit;
  }

  ZeroMem (&MmHeader, sizeof (MM_COMM_CONTENT_HEADER));
  MmHeader.MainId = MmManagement;
  MmHeader.SubId = MmVersionInfoGet;
  MmHeader.Length = sizeof (GET_VERSION_INFO_CONTENT_RESP);
  ContentBuffer = AllocatePool (MmHeader.Length);
  if (ContentBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a(), Content Buffer Install failed!\n", __FUNCTION__));
    Status = EFI_OUT_OF_RESOURCES;
    goto ProcExit;
  }
  CommSize = MmHeader.Length + sizeof (MM_COMM_CONTENT_HEADER) + sizeof (EFI_GUID) + 8;
  ZeroMem (ContentBuffer, MmHeader.Length);
  UefiToMmCommFraming (Guid, &MmHeader, ContentBuffer, CommBuffer, &CommSize);
  Status = MmCommunicate (CommBuffer, CommBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Mm Communicate Failed Status : %r\n", Status));
    goto ProcExit;
  }
  UefiToMmCommParse (CommBuffer, &MmHeader, ContentBuffer);
  CopyMem (Resp, ContentBuffer, MmHeader.Length);

ProcExit:
  if (CommBuffer != NULL) {
    FreePool (CommBuffer);
  }
  if (ContentBuffer != NULL) {
    FreePool (ContentBuffer);
  }

  return Status;
}
