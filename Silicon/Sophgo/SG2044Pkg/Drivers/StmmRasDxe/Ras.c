/** @file
Provides SOPHGO RAS functions.

Copyright (C) 2025, SOPHGO Technologies Inc. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Ras.h"

EFI_GUID  gRasEventNotificationTypeSEA = EFI_EVENT_NOTIFICATION_TYPE_DMAR_SEA;
EFI_GUID  gRasEventNotificationTypeSEI = EFI_EVENT_NOTIFICATION_TYPE_DMAR_SEI;
EFI_GUID  gRasErrorSectionPlatformMemoryGuid = EFI_ERROR_SECTION_PLATFORM_MEMORY_GUID;
EFI_GUID  gRasErrorSectionPlatformMemory2Guid = EFI_ERROR_SECTION_PLATFORM_MEMORY2_GUID;
EFI_GUID  gRasErrorSectionPlatformProcessorGuild = EFI_ERROR_SECTION_PROCESSOR_SPECIFIC_ARM_GUID;
EFI_GUID  gRasErrorSectionPcieGuid = EFI_ERROR_SECTION_PCIE_GUID;
EFI_GUID  gRasErrorSectionPciPcixBusGuid = EFI_ERROR_SECTION_PCI_PCIX_BUS_GUID;
EFI_GUID  gRasErrorSectionPciDeviceGuid = EFI_ERROR_SECTION_PCI_DEVICE_GUID;
EFI_GUID  gRasErrorSectionProcessorArmGuid = EFI_ERROR_SECTION_PROCESSOR_SPECIFIC_ARM_GUID;

EFI_NORFLASH_DRV_PROTOCOL   *mFlash = NULL;
RAS_TOTAL_COUNT             mRasTotalCount;

/** Init Apei Table List.

  @param [in,out]    List      A pointer to apei table list.

**/
VOID
ApeiTableInit (
  IN OUT APEI_TABLE_LIST  *List
  )
{
  if (List == NULL) {
    return;
  }

  ZeroMem (List, sizeof (APEI_TABLE_LIST));
}

/** Print apei table list information.

  @param [in]  List    A pointer to APEI_TABLE_LIST.

**/
VOID
DebugApeiTableList (
  IN APEI_TABLE_LIST  *List
  )
{
#ifdef  RAS_DEBUG
  UINT32  Index;

  if (List == NULL) {
    return;
  }

  DEBUG ((DEBUG_INFO, "BERT Tables: %d\n", List->BertNum));
  for (Index = 0; Index < List->BertNum; Index++) {
    DEBUG ((DEBUG_INFO, "  BERT[%d]:\n", Index));
    DEBUG ((DEBUG_INFO, "    Base: 0x%016lx\n", List->BertTable[Index].Base));
    DEBUG ((DEBUG_INFO, "    Size: 0x%08x\n", List->BertTable[Index].Size));
    DEBUG ((DEBUG_INFO, "    Source: 0x%08x\n", List->BertTable[Index].ErrorSourceNum));
    DEBUG ((DEBUG_INFO, "    Type: 0x%08x\n", List->BertTable[Index].Type));
  }

  DEBUG ((DEBUG_INFO, "HEST Tables: %d\n", List->HestNum));
  for (Index = 0; Index < List->HestNum; Index++) {
    DEBUG ((DEBUG_INFO, "  HEST[%d]:\n", Index));
    DEBUG ((DEBUG_INFO, "    Base: 0x%016lx\n", List->HestTable[Index].Base));
    DEBUG ((DEBUG_INFO, "    Size: 0x%08x\n", List->HestTable[Index].Size));
    DEBUG ((DEBUG_INFO, "    Source: 0x%08x\n", List->HestTable[Index].ErrorSourceNum));
    DEBUG ((DEBUG_INFO, "    Type: 0x%08x\n", List->HestTable[Index].Type));
    DEBUG ((DEBUG_INFO, "    AckReg: 0x%016lx\n", List->HestTable[Index].AckReg));
    DEBUG ((DEBUG_INFO, "    AckPreserve: 0x%016lx\n", List->HestTable[Index].AckPreserve));
    DEBUG ((DEBUG_INFO, "    AckWrite: 0x%016lx\n", List->HestTable[Index].AckWrite));
  }
#endif
}

/** Update apei table. The source buffer data comes from the content of the MM
    communicate protocol. For the protocol content, refer to Chapter 6.3.4 of the
    Phytium MM Interface Specification. This solution if V1. UEFI sends GHES to
    MM, which corresponds to the solution of protocol 03-01.

  @param [in]    Buffer    Source buffer.
  @param [in]    Length    Length of source buffer.
  @param [out]   List      A pointer to APEI_TABLE_LIST.

  @retval       Result    0 - success, 1 - failed.

**/
UINT32
UpdateApeiTable (
  IN  UINT8              *Buffer,
  IN  UINT32             Length,
  OUT APEI_TABLE_LIST    *List
  )
{
  APEI_SOURCE_TABLE  *Table;
  UINT32             Count;
  UINT32             Index;
  UINT32             Status;
  VOID               *Ptr;

  if ((Buffer == NULL) || (List == NULL)) {
    return 1;
  }

  Status = 0;
  Ptr = (VOID *)Buffer;
  CopyMem (&Count, Buffer, sizeof (UINT32));
  Table = (APEI_SOURCE_TABLE *)(Ptr + sizeof (UINT32));
  for (Index = 0; Index < Count; Index++, Table++) {
    if ((List->BertNum > 1) || (List->HestNum > 5)) {
      Status = 1;
      goto Exit;
    }
    if (Table->Type == TypeBert) {
      CopyMem (&List->BertTable[List->BertNum], Table, sizeof (APEI_SOURCE_TABLE));
      List->BertNum++;
    } else if (Table->Type == TypeHest) {
      CopyMem (&List->HestTable[List->HestNum], Table, sizeof (APEI_SOURCE_TABLE));
      List->HestNum++;
    }
  }
  DebugApeiTableList (List);

Exit:
  CopyMem (Buffer, &Status, sizeof (UINT32));
  return Status;
}

/**
  Update apei table. The source buffer data comes from the content of the MM
  communicate protocol. For the protocol content, refer to Chapter 6.3.4 of the
  Phytium MM Interface Specification. This solution if V2. UEFI get GHES from
  MM, which corresponds to the solution of protocol 03-04.

  @param [in]   Context        A pointer to HEST_CONTEXT.
  @param [out]  List           A pointer to APEI_TABLE_LIST.

  @retval      EFI_SUCCESS    Success.

**/
EFI_STATUS
UpdateApeiTableV2 (
  IN  HEST_CONTEXT       *Context,
  OUT APEI_TABLE_LIST    *List
  )
{
  UINT32                                                         Index;
  EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE *GhesV2;

  if ((Context == NULL) || (List == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0; Index < Context->HestHeader->ErrorSourceCount; Index++) {
    GhesV2 = (EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE *)
             ((UINT8 *)Context->HestHeader +
              sizeof (EFI_ACPI_6_5_HARDWARE_ERROR_SOURCE_TABLE_HEADER) +
              Index * sizeof (EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE));

    List->HestTable[Index].Base = MmioRead64 ((UINT64)GhesV2->ErrorStatusAddress.Address);
    List->HestTable[Index].Size = GhesV2->ErrorStatusBlockLength;
    List->HestTable[Index].ErrorSourceNum = Index + 1;
    List->HestTable[Index].Type = TypeHest;
    List->HestTable[Index].AckReg = (UINT64)GhesV2->ReadAckRegister.Address;
    List->HestTable[Index].AckPreserve = GhesV2->ReadAckPreserve;
    List->HestTable[Index].AckWrite = GhesV2->ReadAckWrite;
    List->HestNum++;
  }

  return EFI_SUCCESS;
}

/** Print hest table information.

  @param [in]  Context    A pointer to hest table.

**/
VOID
DebugHestTable (
  IN VOID  *Context
  )
{
#ifdef  RAS_DEBUG
  UINT8                                                           *Ptr;
  EFI_ACPI_6_5_HARDWARE_ERROR_SOURCE_TABLE_HEADER                *Header;
  UINT8                                                          Index;
  EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE *ErrorSource;

  if (Context == NULL) {
    return;
  }

  DEBUG ((DEBUG_INFO, "HEST Context: %p\n", Context));
  Ptr = Context;
  Header = (EFI_ACPI_6_5_HARDWARE_ERROR_SOURCE_TABLE_HEADER *)Context;

  DEBUG ((DEBUG_INFO, "HEST Header:\n"));
  DEBUG ((DEBUG_INFO, "  Signature: 0x%08x\n", Header->Header.Signature));
  DEBUG ((DEBUG_INFO, "  Length: 0x%08x\n", Header->Header.Length));
  DEBUG ((DEBUG_INFO, "  Revision: 0x%02x\n", Header->Header.Revision));
  DEBUG ((DEBUG_INFO, "  OemId: "));
  for (Index = 0; Index < 6; Index++) {
    DEBUG ((DEBUG_INFO, "%02x ", Header->Header.OemId[Index]));
  }
  DEBUG ((DEBUG_INFO, "\n"));
  DEBUG ((DEBUG_INFO, "  OemTableId: 0x%016lx\n", Header->Header.OemTableId));
  DEBUG ((DEBUG_INFO, "  OemRevision: 0x%08x\n", Header->Header.OemRevision));
  DEBUG ((DEBUG_INFO, "  CreatorId: 0x%08x\n", Header->Header.CreatorId));
  DEBUG ((DEBUG_INFO, "  CreatorRevision: 0x%08x\n", Header->Header.CreatorRevision));
  DEBUG ((DEBUG_INFO, "  ErrorSourceCount: %d\n", Header->ErrorSourceCount));

  Ptr = (UINT8 *)Context + sizeof (EFI_ACPI_6_5_HARDWARE_ERROR_SOURCE_TABLE_HEADER);
  for (Index = 0; Index < Header->ErrorSourceCount; Index++) {
    ErrorSource = (EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE *)
                 (Ptr + Index * sizeof (EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE));

    DEBUG ((DEBUG_INFO, "Error Source[%d]: %p\n", Index, ErrorSource));
    DEBUG ((DEBUG_INFO, "  Type: 0x%02x\n", ErrorSource->Type));
    DEBUG ((DEBUG_INFO, "  SourceId: 0x%04x\n", ErrorSource->SourceId));
    DEBUG ((DEBUG_INFO, "  RelatedSourceId: 0x%04x\n", ErrorSource->RelatedSourceId));
    DEBUG ((DEBUG_INFO, "  Flags: 0x%02x\n", ErrorSource->Flags));
    DEBUG ((DEBUG_INFO, "  Enabled: 0x%02x\n", ErrorSource->Enabled));
    DEBUG ((DEBUG_INFO, "  NumberOfRecordsToPreAllocate: 0x%08x\n",
            ErrorSource->NumberOfRecordsToPreAllocate));
    DEBUG ((DEBUG_INFO, "  MaxSectionsPerRecord: 0x%08x\n",
            ErrorSource->MaxSectionsPerRecord));
    DEBUG ((DEBUG_INFO, "  MaxRawDataLength: 0x%08x\n", ErrorSource->MaxRawDataLength));
    DEBUG ((DEBUG_INFO, "  ErrorStatusAddress:\n"));
    DEBUG ((DEBUG_INFO, "    SpaceId: 0x%02x\n",
            ErrorSource->ErrorStatusAddress.AddressSpaceId));
    DEBUG ((DEBUG_INFO, "    BitWidth: 0x%02x\n",
            ErrorSource->ErrorStatusAddress.RegisterBitWidth));
    DEBUG ((DEBUG_INFO, "    BitOffset: 0x%02x\n",
            ErrorSource->ErrorStatusAddress.RegisterBitOffset));
    DEBUG ((DEBUG_INFO, "    AccessSize: 0x%02x\n",
            ErrorSource->ErrorStatusAddress.AccessSize));
    DEBUG ((DEBUG_INFO, "    Address: 0x%016lx\n",
            ErrorSource->ErrorStatusAddress.Address));
    DEBUG ((DEBUG_INFO, "  NotificationStructure:\n"));
    DEBUG ((DEBUG_INFO, "    Type: 0x%02x\n",
            ErrorSource->NotificationStructure.Type));
    DEBUG ((DEBUG_INFO, "    PollInterval: 0x%08x\n",
            ErrorSource->NotificationStructure.PollInterval));
    DEBUG ((DEBUG_INFO, "    Vector: 0x%08x\n",
            ErrorSource->NotificationStructure.Vector));
    DEBUG ((DEBUG_INFO, "    SwitchToPollingThresholdValue: 0x%08x\n",
            ErrorSource->NotificationStructure.SwitchToPollingThresholdValue));
    DEBUG ((DEBUG_INFO, "    SwitchToPollingThresholdWindow: 0x%08x\n",
            ErrorSource->NotificationStructure.SwitchToPollingThresholdWindow));
    DEBUG ((DEBUG_INFO, "    ErrorThresholdValue: 0x%08x\n",
            ErrorSource->NotificationStructure.ErrorThresholdValue));
    DEBUG ((DEBUG_INFO, "    ErrorThresholdWindow: 0x%08x\n",
            ErrorSource->NotificationStructure.ErrorThresholdWindow));
    DEBUG ((DEBUG_INFO, "  ErrorStatusBlockLength: 0x%08x\n",
            ErrorSource->ErrorStatusBlockLength));
    DEBUG ((DEBUG_INFO, "  ReadAckRegister:\n"));
    DEBUG ((DEBUG_INFO, "    SpaceId: 0x%02x\n",
            ErrorSource->ReadAckRegister.AddressSpaceId));
    DEBUG ((DEBUG_INFO, "    BitWidth: 0x%02x\n",
            ErrorSource->ReadAckRegister.RegisterBitWidth));
    DEBUG ((DEBUG_INFO, "    BitOffset: 0x%02x\n",
            ErrorSource->ReadAckRegister.RegisterBitOffset));
    DEBUG ((DEBUG_INFO, "    AccessSize: 0x%02x\n",
            ErrorSource->ReadAckRegister.AccessSize));
    DEBUG ((DEBUG_INFO, "    Address: 0x%016lx\n",
            ErrorSource->ReadAckRegister.Address));
    DEBUG ((DEBUG_INFO, "  ReadAckPreserve: 0x%016lx\n",
            ErrorSource->ReadAckPreserve));
    DEBUG ((DEBUG_INFO, "  ReadAckWrite: 0x%016lx\n",
            ErrorSource->ReadAckWrite));
  }
#endif
}

/**
  HEST Table initialization. The steps are as follows:
  1.Create the HEST header.
  2.Fill Ghes information.
  3.Add GHES information into the HEST table.
  4.Parse GHES content(GHES Address and ACK Register) into APEI table.

  @retval    EFI_SUCCESS    Success.
  @retval    Other          Failed.

**/
EFI_STATUS
InitHestTable (
  VOID
  )
{
  EFI_STATUS                                                     Status;
  EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE GhesV2[MAX_GHES];
  UINT8                                                          Index;
  UINT8                                                          Checksum;

  Status = HestHeaderCreator (&mHestContext, HEST_TABLE_SIZE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Hset Header Creator Failed!\n"));
    return Status;
  }

  Status = GhesV2ContextForHest (GhesV2, MAX_GHES);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Ghes Context Failed!\n"));
    return Status;
  }

  for (Index = 0; Index < MAX_GHES; Index++) {
    DEBUG ((DEBUG_INFO, "Add Ghes[%d]:\n", Index));
    Status =  HestAddErrorSourceDescriptor (
                &mHestContext,
                &GhesV2[Index],
                sizeof (EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE)
                );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Hest Add Error Source Descriptor[%d] Failed!\n", Index));
      return Status;
    }
  }

  UpdateApeiTableV2 (&mHestContext, &mApeiList);
  DebugApeiTableList (&mApeiList);
  Checksum = CalculateCheckSum8 ((UINT8*) (mHestContext.HestHeader),
                             mHestContext.HestHeader->Header.Length);
  mHestContext.HestHeader->Header.Checksum = Checksum;
  DebugHestTable ((VOID *)mHestContext.HestHeader);

  return EFI_SUCCESS;
}

/** Print cper header.

  @param [in]  Header    A pointer ot EFI_COMMON_ERROR_RECORD_HEADER.

**/
VOID
PrintCperHeader (
  IN EFI_COMMON_ERROR_RECORD_HEADER  *Header
  )
{
#ifdef  RAS_DEBUG
  DEBUG ((DEBUG_INFO, "CPER HEADER Address : %p\n", (VOID *)Header));
  DEBUG ((DEBUG_INFO, "Signature Start : 0x%x\n", Header->SignatureStart));
  DEBUG ((DEBUG_INFO, "Revision : 0x%x\n", Header->Revision));
  DEBUG ((DEBUG_INFO, "Signature End : 0x%x\n", Header->SignatureEnd));
  DEBUG ((DEBUG_INFO, "Section Count : 0x%x\n", Header->SectionCount));
  DEBUG ((DEBUG_INFO, "Error Severity : 0x%x\n", Header->ErrorSeverity));
  DEBUG ((DEBUG_INFO, "Validation Bits : 0x%x\n", Header->ValidationBits));
  DEBUG ((DEBUG_INFO, "Record Length : 0x%x\n", Header->RecordLength));
  DEBUG ((DEBUG_INFO, "Time Stamp Century : %d\n", Header->TimeStamp.Century));
  DEBUG ((DEBUG_INFO, "Time Stamp Year : %d\n", Header->TimeStamp.Year));
  DEBUG ((DEBUG_INFO, "Time Stamp Month : %d\n", Header->TimeStamp.Month));
  DEBUG ((DEBUG_INFO, "Time Stamp Day : %d\n", Header->TimeStamp.Day));
  DEBUG ((DEBUG_INFO, "Time Stamp Flag : %d\n", Header->TimeStamp.Flag));
  DEBUG ((DEBUG_INFO, "Time Stamp Hours : %d\n", Header->TimeStamp.Hours));
  DEBUG ((DEBUG_INFO, "Time Stamp Minutes : %d\n", Header->TimeStamp.Minutes));
  DEBUG ((DEBUG_INFO, "Time Stamp Seconds : %d\n", Header->TimeStamp.Seconds));
  DEBUG ((DEBUG_INFO, "Platform ID : %g\n", &Header->PlatformID));
  DEBUG ((DEBUG_INFO, "Partition ID : %g\n", &Header->PartitionID));
  DEBUG ((DEBUG_INFO, "Creator ID : %g\n", &Header->CreatorID));
  DEBUG ((DEBUG_INFO, "Notification Type ID : %g\n", &Header->NotificationType));
  DEBUG ((DEBUG_INFO, "Record ID : 0x%llx\n", Header->RecordID));
  DEBUG ((DEBUG_INFO, "Flags : %x\n", Header->Flags));
  DEBUG ((DEBUG_INFO, "Persistence Info : 0x%llx\n", Header->PersistenceInfo));
#endif
}

/** Print section descriptor.

  @param [in]  Desc    A pointer to EFI_ERROR_SECTION_DESCRIPTORi.

**/
VOID
PrintSectionDescriptor (
  IN EFI_ERROR_SECTION_DESCRIPTOR *Desc
  )
{
#ifdef  RAS_DEBUG
  DEBUG ((DEBUG_INFO, "Section Descriptor Address : %p\n", Desc));
  DEBUG ((DEBUG_INFO, "Section Offset : 0x%x\n", Desc->SectionOffset));
  DEBUG ((DEBUG_INFO, "Section Length : 0x%x\n", Desc->SectionLength));
  DEBUG ((DEBUG_INFO, "Revision : 0x%x\n", Desc->Revision));
  DEBUG ((DEBUG_INFO, "Validation Bits : 0x%x\n", Desc->SecValidMask));
  DEBUG ((DEBUG_INFO, "Flags : 0x%x\n", Desc->SectionFlags));
  DEBUG ((DEBUG_INFO, "Section Type : %g\n", &Desc->SectionType));
  DEBUG ((DEBUG_INFO, "Fru ID : %g\n", &Desc->FruId));
  DEBUG ((DEBUG_INFO, "Section Severity : 0x%x\n", Desc->Severity));
  DEBUG ((DEBUG_INFO, "Fru String : %s\n", Desc->FruString));
#endif
}

/**
  Print ras strategy.

  @param[in]  Strategy    A pointer to RAS_STRATEGY.
**/
VOID
PrintRasStrategy (
  IN  RAS_STRATEGY    *Strategy
  )
{
#ifdef  RAS_DEBUG
  DEBUG ((DEBUG_INFO, "Strategy->BmcCperEnable : %x\n", Strategy->BmcCperEnable));
  DEBUG ((DEBUG_INFO, "Strategy->FatalErrorRecord : %x\n", Strategy->FatalErrorRecord));
  DEBUG ((DEBUG_INFO, "Strategy->MemoryIsoRecord : %x\n", Strategy->MemoryIsoRecord));
  DEBUG ((DEBUG_INFO, "Memory Fatal UCE Isolation Strategy :\n"));
  DEBUG ((DEBUG_INFO, "Strategy->MemFatalUceIsoStrategy.Enable : %x\n", Strategy->MemFatalUceIsoStrategy.Enable));
  DEBUG ((DEBUG_INFO, "Strategy->MemFatalUceIsoStrategy.ResetClean : %x\n", Strategy->MemFatalUceIsoStrategy.ResetClean));
  DEBUG ((DEBUG_INFO, "Strategy->MemFatalUceIsoStrategy.Method : %x\n", Strategy->MemFatalUceIsoStrategy.Method));
  DEBUG ((DEBUG_INFO, "Strategy->MemFatalUceIsoStrategy.Count : %d\n", Strategy->MemFatalUceIsoStrategy.Count));
  DEBUG ((DEBUG_INFO, "Memory Non-Fatal UCE Isolation Strategy :\n"));
  DEBUG ((DEBUG_INFO, "Strategy->MemNonFatalUceIsoStrategy.Enable : %x\n", Strategy->MemNonFatalUceIsoStrategy.Enable));
  DEBUG ((DEBUG_INFO, "Strategy->MemNonFatalUceIsoStrategy.ResetClean : %x\n", Strategy->MemNonFatalUceIsoStrategy.ResetClean));
  DEBUG ((DEBUG_INFO, "Strategy->MemNonFatalUceIsoStrategy.Method : %x\n", Strategy->MemNonFatalUceIsoStrategy.Method));
  DEBUG ((DEBUG_INFO, "Strategy->MemNonFatalUceIsoStrategy.Count : %d\n", Strategy->MemNonFatalUceIsoStrategy.Count));
  DEBUG ((DEBUG_INFO, "Memory CE Isolation Strategy :\n"));
  DEBUG ((DEBUG_INFO, "Strategy->MemCeIsoStrategy.Enable : %x\n", Strategy->MemCeIsoStrategy.Enable));
  DEBUG ((DEBUG_INFO, "Strategy->MemCeIsoStrategy.ResetClean : %x\n", Strategy->MemCeIsoStrategy.ResetClean));
  DEBUG ((DEBUG_INFO, "Strategy->MemCeIsoStrategy.Method : %x\n", Strategy->MemCeIsoStrategy.Method));
  DEBUG ((DEBUG_INFO, "Strategy->MemCeIsoStrategy.Count : %d\n", Strategy->MemCeIsoStrategy.Count));
  DEBUG ((DEBUG_INFO, "Memory CE Report Strategy :\n"));
  DEBUG ((DEBUG_INFO, "Strategy->MemCeReportStrategy.Enable : %x\n", Strategy->MemCeReportStrategy.Enable));
  DEBUG ((DEBUG_INFO, "Strategy->MemCeReportStrategy.Count : %d\n", Strategy->MemCeReportStrategy.Count));
  DEBUG ((DEBUG_INFO, "CPU CE Report Strategy :\n"));
  DEBUG ((DEBUG_INFO, "Strategy->CpuCeReportStrategy.Enable : %x\n", Strategy->CpuCeReportStrategy.Enable));
  DEBUG ((DEBUG_INFO, "Strategy->CpuCeReportStrategy.Count : %d\n", Strategy->CpuCeReportStrategy.Count));
  DEBUG ((DEBUG_INFO, "PCIe CE Report Strategy :\n"));
  DEBUG ((DEBUG_INFO, "Strategy->PcieCeReportStrategy.Enable : %x\n", Strategy->PcieCeReportStrategy.Enable));
  DEBUG ((DEBUG_INFO, "Strategy->PcieCeReportStrategy.Count : %d\n", Strategy->PcieCeReportStrategy.Count));
  DEBUG ((DEBUG_INFO, "Cpu Fatal UCE Isolation Strategy :\n"));
  DEBUG ((DEBUG_INFO, "Strategy->CpuFatalStrategy.Enable : %x\n", Strategy->CpuFatalStrategy.Enable));
  DEBUG ((DEBUG_INFO, "Strategy->CpuFatalStrategy.ResetClean : %x\n", Strategy->CpuFatalStrategy.ResetClean));
  DEBUG ((DEBUG_INFO, "Strategy->CpuFatalStrategy.Method : %x\n", Strategy->CpuFatalStrategy.Method));
  DEBUG ((DEBUG_INFO, "Strategy->CpuFatalStrategy.Record : %x\n", Strategy->CpuFatalStrategy.Record));
  DEBUG ((DEBUG_INFO, "Flash Write Protect Configuration:"));
  DEBUG ((DEBUG_INFO, "Strategy->FlashWpConfig.Enable : %d\n", Strategy->FlashWpConfig.Enable));
  DEBUG ((DEBUG_INFO, "Strategy->FlashWpConfig.BlockProtectBit : 0x%x\n", Strategy->FlashWpConfig.BlockProtectBit));
  DEBUG ((DEBUG_INFO, "Strategy->FlashWpConfig.WpDelay : %d\n", Strategy->FlashWpConfig.WpDelay));
  DEBUG ((DEBUG_INFO, "Strategy->MemAddrIsoStrategy.Enable : %d\n", Strategy->MemAddrIsoStrategy.Enable));
  DEBUG ((DEBUG_INFO, "Strategy->MemAddrIsoStrategy.ResetClean : %d\n", Strategy->MemAddrIsoStrategy.ResetClean));
  DEBUG ((DEBUG_INFO, "Strategy->MemAddrIsoStrategy.Type : %d\n", Strategy->MemAddrIsoStrategy.Type));
#endif
}

/**
  Check if ACK register status allows writing errors.

  @param[in] AckReg    ACK register address.
  @param[in] Preserve  ACK preserve value.
  @param[in] Write     ACK write value.

  @retval TRUE   Can write GHES.
  @retval FALSE  Cannot write GHES.
**/
BOOLEAN
CheckAckWrite (
  IN UINT64  AckReg,
  IN UINT64  Preserve,
  IN UINT64  Write
  )
{
  UINT64 Value;

  Value = MmioRead64 (AckReg);
  Value &= ~(Preserve);
  Value &= Write;

  return (Value != 0);
}

/**
  Clear ACK register.

  @param[in] AckReg    ACK register address.
  @param[in] Preserve  ACK preserve value.
  @param[in] Write     ACK write value.
**/
VOID
ClearAckWrite (
  IN UINT64  AckReg,
  IN UINT64  Preserve,
  IN UINT64  Write
  )
{
  UINT64 Value;

  Value = MmioRead64 (AckReg);
  Value &= Preserve;
  Value &= ~(Write);
  MmioWrite64 (AckReg, Value);
}

/**
  Generate BERT GHES to BERT memory address.

  @param[in] Type      Record type. 0 - no record, 1 - flash.
  @param[in] BertBase  A pointer to BERT region.
  @param[in] BertSize  BERT region size.
**/
VOID
GenerateBertGhes (
  IN UINT8   Type,
  IN VOID    *BertBase,
  IN UINT32  BertSize
  )
{
  UINT8                           *Record;
  UINT64                         Signature;
  UINT32                         Length;
  UINT8                          *Ptr;
  EFI_COMMON_ERROR_RECORD_HEADER *Header;
  EFI_ERROR_SECTION_DESCRIPTOR   *Descriptor;
  VOID                          *SectionData;
  BOOLEAN                       CorrectType;
  UINT32                        Index;
  UINT64                        Address;

  DEBUG ((DEBUG_INFO, "%a start\n", __func__));
