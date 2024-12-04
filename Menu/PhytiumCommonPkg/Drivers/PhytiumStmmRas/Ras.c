/** @file
Provides Phytium stmm ras functions

Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Ras.h"

EFI_GUID  gRasEventNotificationTypeSEA = EFI_EVENT_NOTIFICATION_TYPE_DMAR_SEA;
EFI_GUID  gRasEventNotificationTypeSEI = EFI_EVENT_NOTIFICATION_TYPE_DMAR_SEI;
EFI_GUID  gRasErrorSectionPlatformMemoryGuid = EFI_ERROR_SECTION_PLATFORM_MEMORY_GUID;
EFI_GUID  gRasErrorSectionPlatformMemory2Guid = EFI_ERROR_SECTION_PLATFORM_MEMORY2_GUID;
EFI_GUID  gRasErrorSectionPlatformProcessorGuild = EFI_ERROR_SECTION_PROCESSOR_SPECIFIC_ARM_GUID;
EFI_GUID  gRasErrorSectiionPcieGuid = EFI_ERROR_SECTION_PCIE_GUID;
EFI_GUID  gRasErrorSectiionPciPcixBusGuid = EFI_ERROR_SECTION_PCI_PCIX_BUS_GUID;
EFI_GUID  gRasErrorSectiionPciDeviceGuid = EFI_ERROR_SECTION_PCI_DEVICE_GUID;
EFI_GUID  gRasErrorSectiionProcessorArmGuid = EFI_ERROR_SECTION_PROCESSOR_SPECIFIC_ARM_GUID;

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

  DEBUG ((DEBUG_INFO, "Bert Num : %d\n", List->BertNum));
  for (Index = 0; Index < List->BertNum; Index++) {
    DEBUG ((DEBUG_INFO, "Bert Base : 0x%llx\n", List->BertTable[Index].Base));
    DEBUG ((DEBUG_INFO, "Bert Size : 0x%x\n", List->BertTable[Index].Size));
    DEBUG ((DEBUG_INFO, "Bert Source : 0x%x\n", List->BertTable[Index].ErrorSourceNum));
    DEBUG ((DEBUG_INFO, "Bert Type : 0x%x\n", List->BertTable[Index].Type));
  }
  DEBUG ((DEBUG_INFO, "Hest Num : %d\n", List->HestNum));
  for (Index = 0; Index < List->HestNum; Index++) {
    DEBUG ((DEBUG_INFO, "Hest Base : 0x%llx\n", List->HestTable[Index].Base));
    DEBUG ((DEBUG_INFO, "Hest Size : 0x%x\n", List->HestTable[Index].Size));
    DEBUG ((DEBUG_INFO, "Hest Source : 0x%x\n", List->HestTable[Index].ErrorSourceNum));
    DEBUG ((DEBUG_INFO, "Hest Type : 0x%x\n", List->HestTable[Index].Type));
    DEBUG ((DEBUG_INFO, "Hest Ack Register : 0x%llx\n", List->HestTable[Index].AckReg));
    DEBUG ((DEBUG_INFO, "Hest Ack Preserve : 0x%x\n", List->HestTable[Index].AckPreserve));
    DEBUG ((DEBUG_INFO, "Hest AckWrite : 0x%x\n", List->HestTable[Index].AckWrite));

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

  Status = 0;
  Ptr = (VOID *)Buffer;
  CopyMem (&Count, Buffer, 4);
  Table = (APEI_SOURCE_TABLE *) (Ptr + 4);
  for (Index = 0; Index < Count; Index++, Table++) {
    if ((List->BertNum > 1) || (List->HestNum > 5)) {
      Status = 1;
      goto ProcExit;
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

ProcExit:
  CopyMem (Buffer, &Status, 4);

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
  UINT32                                                          Index;
  EFI_ACPI_6_4_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE  *GhesV2;

  for (Index = 0; Index < Context->HestHeader->ErrorSourceCount; Index++) {
    GhesV2 = (EFI_ACPI_6_4_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE *)
               ((UINT8*)Context->HestHeader
                + sizeof (EFI_ACPI_6_4_HARDWARE_ERROR_SOURCE_TABLE_HEADER)
                + Index * sizeof (EFI_ACPI_6_4_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE));
    List->HestTable[Index].Base =
        MmioRead64 ((UINT64)GhesV2->ErrorStatusAddress.Address);
    List->HestTable[Index].Size = GhesV2->ErrorStatusBlockLength;
    List->HestTable[Index].ErrorSourceNum = Index + 1;
    List->HestTable[Index].Type = TypeHest;
    List->HestTable[Index].AckReg = (UINT64)GhesV2->ReadAckRegister.Address;
    List->HestTable[Index].AckPreserve = (UINT64)GhesV2->ReadAckPreserve;
    List->HestTable[Index].AckWrite = (UINT64)GhesV2->ReadAckWrite;
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
  EFI_ACPI_6_4_HARDWARE_ERROR_SOURCE_TABLE_HEADER                 *Header;
  UINT8                                                           Index;
  EFI_ACPI_6_4_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE  *ErrorSource;

  DEBUG ((DEBUG_INFO, "Context : %p\n", Context));
  Ptr = Context;
  DEBUG ((DEBUG_INFO, "Ptr : %p\n", Ptr));
  Header = (EFI_ACPI_6_4_HARDWARE_ERROR_SOURCE_TABLE_HEADER *) Context;
  DEBUG ((DEBUG_INFO, "Hest Header :\n"));
  DEBUG ((DEBUG_INFO, "Header Signature : 0x%x\n", Header->Header.Signature));
  DEBUG ((DEBUG_INFO, "Length : 0x%x\n", Header->Header.Length));
  DEBUG ((DEBUG_INFO, "Revision : 0x%x\n", Header->Header.Revision));
  DEBUG ((DEBUG_INFO, "OemId : \n"));
  for (Index = 0; Index < 6; Index++) {
    DEBUG ((DEBUG_INFO, "%02x ", Header->Header.OemId[Index]));
  }
  DEBUG ((DEBUG_INFO, "\n"));
  DEBUG ((DEBUG_INFO, "Oem Table ID : 0x%x\n", Header->Header.OemTableId));
  DEBUG ((DEBUG_INFO, "Oem Revision : 0x%x\n", Header->Header.OemRevision));
  DEBUG ((DEBUG_INFO, "Creator ID : 0x%x\n", Header->Header.CreatorId));
  DEBUG ((DEBUG_INFO, "Creator Revision : 0x%x\n", Header->Header.CreatorRevision));
  DEBUG ((DEBUG_INFO, "Error Source Count : %d\n", Header->ErrorSourceCount));

  Ptr = (UINT8 *)Context + sizeof (EFI_ACPI_6_4_HARDWARE_ERROR_SOURCE_TABLE_HEADER);
  DEBUG ((DEBUG_INFO, "Ptr : %p\n", Ptr));
  for (Index = 0; Index < Header->ErrorSourceCount; Index++) {
    ErrorSource =
      (EFI_ACPI_6_4_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE *)
      (Ptr + Index * sizeof (EFI_ACPI_6_4_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE));
    DEBUG ((DEBUG_INFO, "Error Source [%d] : %p\n", Index, ErrorSource));
    DEBUG ((DEBUG_INFO, "Type : 0x%x\n", ErrorSource->Type));
    DEBUG ((DEBUG_INFO, "Source ID : 0x%x\n", ErrorSource->SourceId));
    DEBUG ((DEBUG_INFO, "Related Srouce ID : 0x%x\n", ErrorSource->RelatedSourceId));
    DEBUG ((DEBUG_INFO, "Flags : 0x%x\n", ErrorSource->Flags));
    DEBUG ((DEBUG_INFO, "Enabled : 0x%x\n", ErrorSource->Enabled));
    DEBUG ((DEBUG_INFO, "Number of Records to PreAllocate : 0x%x\n", ErrorSource->NumberOfRecordsToPreAllocate));
    DEBUG ((DEBUG_INFO, "Max Sections Per Record : 0x%x\n", ErrorSource->MaxSectionsPerRecord));
    DEBUG ((DEBUG_INFO, "Max Raw Data Length : 0x%x\n", ErrorSource->MaxRawDataLength));
    DEBUG ((DEBUG_INFO, "Error Status Address - Space ID : 0x%x\n", ErrorSource->ErrorStatusAddress.AddressSpaceId));
    DEBUG ((DEBUG_INFO, "Error Status Address - Bit Write : 0x%x\n", ErrorSource->ErrorStatusAddress.RegisterBitWidth));
    DEBUG ((DEBUG_INFO, "Error Status Address - Bit Offset : 0x%x\n", ErrorSource->ErrorStatusAddress.RegisterBitOffset));
    DEBUG ((DEBUG_INFO, "Error Status Address - Access Size : 0x%x\n", ErrorSource->ErrorStatusAddress.AccessSize));
    DEBUG ((DEBUG_INFO, "Error Status Address - Address : 0x%llx\n", ErrorSource->ErrorStatusAddress.Address));
    DEBUG ((DEBUG_INFO, "Notification - Type : 0x%x\n", ErrorSource->NotificationStructure.Type));
    DEBUG ((DEBUG_INFO, "Notification - Poll Interval : 0x%x\n", ErrorSource->NotificationStructure.PollInterval));
    DEBUG ((DEBUG_INFO,
            "Notification - Switch to Polling Threshold Value : 0x%x\n",
            ErrorSource->NotificationStructure.SwitchToPollingThresholdValue
            ));
    DEBUG ((DEBUG_INFO,
            "Notification - Switch to Polling Threshold Window : 0x%x\n",
            ErrorSource->NotificationStructure.SwitchToPollingThresholdWindow
            ));
    DEBUG ((DEBUG_INFO,
            "Notification - Error Threshold Value : 0x%x\n",
              ErrorSource->NotificationStructure.ErrorThresholdValue
              ));
    DEBUG ((DEBUG_INFO,
            "Notification - Error Threshold Window : 0x%x\n",
            ErrorSource->NotificationStructure.ErrorThresholdWindow
            ));
    DEBUG ((DEBUG_INFO, "Error Status Block Length : 0x%x\n", ErrorSource->ErrorStatusBlockLength));
    DEBUG ((DEBUG_INFO, "Read Ack Register - Space ID : 0x%x\n", ErrorSource->ReadAckRegister.AddressSpaceId));
    DEBUG ((DEBUG_INFO, "Read Ack Register - Bit Width : 0x%x\n", ErrorSource->ReadAckRegister.RegisterBitWidth));
    DEBUG ((DEBUG_INFO, "Read Ack Register - Bit Offset : 0x%x\n", ErrorSource->ReadAckRegister.RegisterBitOffset));
    DEBUG ((DEBUG_INFO, "Read Ack Register - Access Size : 0x%x\n", ErrorSource->ReadAckRegister.AccessSize));
    DEBUG ((DEBUG_INFO, "Read Ack Register - Address : 0x%llx\n", ErrorSource->ReadAckRegister.Address));
    DEBUG ((DEBUG_INFO, "Read Ack Preserve : 0x%llx\n", ErrorSource->ReadAckPreserve));
    DEBUG ((DEBUG_INFO, "Read Ack Write : 0x%llx\n", ErrorSource->ReadAckWrite));
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
  EFI_ACPI_6_4_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE GhesV2[MAX_GHES];
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
                sizeof (EFI_ACPI_6_4_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE)
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

/** Judge whether the ACK register status confirmation can write errors.

  @param [in]  AckReg    Ack register address.
  @param [in]  Preserve  Ack preserve value.
  @param [in]  Write     Ack write value.

  @retval     TRUE      Can write GHES.
  @retval     FALSE     Can't write GHES.

**/
BOOLEAN
CheckAckWrite (
  IN  UINT64    AckReg,
  IN  UINT64    Preserve,
  IN  UINT64    Write
  )
{
  UINT64 Value;

  Value = MmioRead64 (AckReg);
  Value &= ~(Preserve);
  Value &= Write;
  if (Value) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/** Clear ack rigister.

  @param [in]  AckReg    Ack register address.
  @param [in]  Preserve  Ack preserve value.
  @param [in]  Write     Ack write value.

**/
VOID
ClearAckWrite (
  IN  UINT64    AckReg,
  IN  UINT64    Preserve,
  IN  UINT64    Write
  )
{
  UINT64 Value;

  Value = MmioRead64 (AckReg);
  Value &= Preserve;
  Value &= ~(Write);
  MmioWrite64 (AckReg, Value);
}

/**
  Generate Bert Ghes to Bert memory address. BERT data is obtained from the
  corresponding path based on record type.

  @param[in]    Type        Record type. 0 - no record, 1 - flash.
  @param[in]    BertBase    A pointer to bert region.
  @param[in]    BertSize    Bert region size.
**/
VOID
GenerateBertGhes (
  IN UINT8  Type,
  IN VOID   *BertBase,
  IN UINT32 BertSize
  )
{
  UINT8                           *Record;
  UINT64                          Signarute;
  UINT32                          Length;
  UINT8                           *Ptr;
  EFI_COMMON_ERROR_RECORD_HEADER  *Header;
  EFI_ERROR_SECTION_DESCRIPTOR    *Descriptor;
  VOID                            *SectionData;
  BOOLEAN                         CorrectType;
  UINT32                          Index;
  UINT64                          Address;

  DEBUG ((DEBUG_INFO, "%a start\n", __FUNCTION__));
  switch (Type) {
  case RasNoRecord:
    DEBUG ((DEBUG_INFO, "Fatal Record no record\n"));
    return ;
    break;
  case RasFlashRecord:
    Address = FixedPcdGet64 (PcdRasFatalErrorFlashAddress);
    if (mFlash == NULL) {
      DEBUG ((DEBUG_ERROR, "mFlash == NULL\n"));
      return ;
    }
    if (mFlash->Read (Address, &Signarute, 8) != EFI_SUCCESS) {
      return ;
    }
    if (Signarute != FATAL_ERROR_RECORD_SIGNATURE) {
      DEBUG ((DEBUG_INFO, "%a(), Sinagure mismatch!\n", __FUNCTION__));
      return ;
    }
    Length = 0;
    if (mFlash->Read (Address + 8, &Length, 4) != EFI_SUCCESS) {
      return ;
    }
    Record = AllocatePool (Length);
    if (Record == NULL) {
      goto ProcExit ;
    }
    if (mFlash->Read (Address + 12, Record, Length) != EFI_SUCCESS) {
      goto ProcExit ;
    }
    break;
  default:
    DEBUG ((DEBUG_ERROR, "Fatal Record unsupported record type!\n"));
    return ;
    break;
  }
  Ptr = (VOID *) Record;
  Header = (EFI_COMMON_ERROR_RECORD_HEADER *) Ptr;
  PrintCperHeader (Header);
  if ((Header->SignatureStart != EFI_ERROR_RECORD_SIGNATURE_START)
      || (Header->SignatureEnd != EFI_ERROR_RECORD_SIGNATURE_END)) {
    DEBUG ((DEBUG_ERROR, "Head signature mismach!\n"));
    goto ProcExit;
  }

  for (Index = 0; Index < Header->SectionCount; Index++) {
    Descriptor = (EFI_ERROR_SECTION_DESCRIPTOR *)(Ptr +
                    sizeof (EFI_COMMON_ERROR_RECORD_HEADER) +
                    Index * sizeof (EFI_ERROR_SECTION_DESCRIPTOR));
    PrintSectionDescriptor (Descriptor);
    SectionData = (VOID *)(Ptr + Descriptor->SectionOffset);
    DEBUG ((DEBUG_INFO, "Section Address : %p\n", SectionData));
    DEBUG ((DEBUG_INFO, "Section Severity : %d\n", Descriptor->Severity));
    //Correctable
    switch (Header->ErrorSeverity & 0x3) {
    case EFI_GENERIC_ERROR_RECOVERABLE:
    case EFI_GENERIC_ERROR_FATAL:
      CorrectType = FALSE;
      break;
    case EFI_GENERIC_ERROR_CORRECTED:
    case EFI_GENERIC_ERROR_INFO:
      CorrectType = TRUE;
      break;
    default:
      break;
    }
    ErrorBlockAddErrorData (
      (VOID *)BertBase,
      BertSize,
      Header,
      Descriptor,
      SectionData,
      Descriptor->SectionLength,
      CorrectType
      );
  }

ProcExit:
  if (Record != NULL) {
    FreePool (Record);
  }
  DEBUG ((DEBUG_INFO, "%a end\n", __FUNCTION__));
}

/** Generate GHES data and write it to the corresponding error status address
    described in the HEST table according to differt Severity. The source buffer
    data comes from the content of the MM communicate protocol. For the protocol
    content, refer to Chapter 6.3.4 of the Phytium MM Interface Specification.
    All sections in each CPER message must have the same serverity level.

  @param [in]    Buffer      The source buffer.
  @param [in]    Length      Length of the source buffer.
  @param [in]    List        The apei list to write error data.

  @retval       GenerateGhesSuccess           Success.
  @retval       GenerateGhesSignatureError    Signature of cper data is error.
  @retval       GenerateGhesTimeout           Write ghes data timeout. It means
                                              checking ack register timeout.
**/
UINT32
GenerateHestGhes (
  IN   UINT8              *Buffer,
  IN   UINT32             Length,
  IN   APEI_TABLE_LIST    *List
  )
{
  UINT8                           *Ptr;
  EFI_COMMON_ERROR_RECORD_HEADER  *Header;
  EFI_ERROR_SECTION_DESCRIPTOR    *Descriptor;
  UINT32                          Status;
  UINT32                          Index;
  VOID                            *SectionData;
  UINT32                          Index1;
  BOOLEAN                         CorrectType;

  Status = GenerateGhesSuccess;
  Ptr = (VOID *)Buffer;
  Header = (EFI_COMMON_ERROR_RECORD_HEADER *) Ptr;
  PrintCperHeader (Header);
  if ((Header->SignatureStart != EFI_ERROR_RECORD_SIGNATURE_START)
      || (Header->SignatureEnd != EFI_ERROR_RECORD_SIGNATURE_END)) {
    DEBUG ((DEBUG_ERROR, "Head signature mismach!\n"));
    Status = GenerateGhesSignatureError;
    goto ProcExit;
  }
  //
  //sections
  //
  //SEA
  if (CompareGuid (&Header->NotificationType, &gRasEventNotificationTypeSEA)) {
    Index1 = HEST_REGION_SEA;
  } else if (CompareGuid (&Header->NotificationType, &gRasEventNotificationTypeSEI)) {
    Index1 = HEST_REGION_SEI;
  } else {    //Other
    Index1 = Header->ErrorSeverity;
  }
  DEBUG ((DEBUG_INFO, "Ghes to Region %d\n", Index1));
  if (CheckAckWrite (List->HestTable[Index1].AckReg,
                 List->HestTable[Index1].AckPreserve,
                 List->HestTable[Index1].AckWrite)) {
    DEBUG ((DEBUG_INFO, "Check Write Ack Success!\n"));
    for (Index = 0; Index < Header->SectionCount; Index++) {
      Descriptor = (EFI_ERROR_SECTION_DESCRIPTOR *)(Ptr +
                      sizeof (EFI_COMMON_ERROR_RECORD_HEADER) +
                      Index * sizeof (EFI_ERROR_SECTION_DESCRIPTOR));
      DEBUG ((DEBUG_INFO, "Setion[%d]:\n", Index));
      PrintSectionDescriptor (Descriptor);
      SectionData = (VOID *)(Ptr + Descriptor->SectionOffset);
      DEBUG ((DEBUG_INFO, "Section Address : %p\n", SectionData));
      DEBUG ((DEBUG_INFO, "Section Severity : %d\n", Descriptor->Severity));
      DEBUG ((DEBUG_INFO,
              "List->HestTable[%d].Base : 0x%llx\n",
              Index1, List->HestTable[Index1].Base
              ));
      //Correctable
      switch (Header->ErrorSeverity & 0x3) {
      case EFI_GENERIC_ERROR_RECOVERABLE:
      case EFI_GENERIC_ERROR_FATAL:
        CorrectType = FALSE;
        break;
      case EFI_GENERIC_ERROR_CORRECTED:
      case EFI_GENERIC_ERROR_INFO:
        CorrectType = TRUE;
        break;
      default:
        break;
      }
      DEBUG ((DEBUG_INFO, "Header Severity Type : %d\n", CorrectType));
      Status = ErrorBlockAddErrorData (
                 (VOID *)List->HestTable[Index1].Base,
                 List->HestTable[Index1].Size,
                 Header,
                 Descriptor,
                 SectionData,
                 Descriptor->SectionLength,
                 CorrectType
                 );
      if (Status == TRUE) {
        Status = GenerateGhesSuccess;
      } else {
        Status = GenerateGhesSignatureError;
        break;
      }
    }
    ClearAckWrite (List->HestTable[Index1].AckReg,
                 List->HestTable[Index1].AckPreserve,
                 List->HestTable[Index1].AckWrite);
  } else {
    Status = GenerateGhesTimeout;
  }

ProcExit:

  return Status;
}

/**
  Get HEST table header, corresponding to phytium MM protocol 03-03.

  @param [in,out]  Buffer    Content buffer.

  @retval    GetHestHeaderSuccess    Success.
  @retval    GetHestHeaderNotFound   HEST table if not found.

**/
UINT32
GetHestHeaderGet (
  IN OUT UINT8      *Buffer
  )
{
  if (mHestContext.HestHeader == NULL) {
    DEBUG ((DEBUG_ERROR, "Hest Context is not found!\n"));
    return GetHestHeaderNotFound;
  }

  CopyMem (Buffer + 4,
           (VOID*) mHestContext.HestHeader,
           sizeof (EFI_ACPI_6_4_HARDWARE_ERROR_SOURCE_TABLE_HEADER));

  return GetHestHeaderSuccess;
}

/**
  Get HEST table, corresponding to phytium MM protocol 03-04.

  @param[in,out]    Buffer    Content buffer.
  @param [in]       Length    Hest table length.

  @retval    GetHestTableSuccess       Success.
  @retval    GetHestTableNotFound      HEST table if not found.
  @retval    GetHestTableLengthError   HEST table header length mismatch.

**/
UINT32
GetHestErrorSourceStruct (
  IN OUT UINT8      *Buffer,
  IN     UINT32     Length
  )
{
  if (mHestContext.HestHeader == NULL) {
    DEBUG ((DEBUG_ERROR, "Hest Context is not found!\n"));
    return GetHestTableNotFound;
  }
  if (Length != mHestContext.HestHeader->Header.Length) {
    DEBUG ((DEBUG_ERROR, "Hest Header Length Mismatch!\n"));
    return GetHestTableLengthError;
  }

  CopyMem (Buffer + 4,
           (VOID*) mHestContext.HestHeader,
          mHestContext.HestHeader->Header.Length);

  return GetHestTableSuccess;
}

/**
  Get EINJ table header, corresponding to phytium MM protocol 03-07.

  @param [in,out]  Buffer    Content buffer.

  @retval    GetEinjHeaderSuccess    Success.
  @retval    GetEinjHeaderNotFound   EINJ table if not found.

**/
UINT32
GetEinjHeader (
  IN OUT UINT8      *Buffer
  )
{
  if (mEinjContext.EINJ == NULL) {
    DEBUG ((DEBUG_ERROR, "Einj Context is not found!\n"));
    return GetEinjHeaderNotFound;
  }

  CopyMem (Buffer + 4,
           (VOID*) &mEinjContext.EINJ->EinjTableHeader,
           sizeof (EFI_ACPI_6_4_ERROR_INJECTION_TABLE_HEADER)
           );

  return GetEinjHeaderSuccess;
}

/**
  Get EINJ table, corresponding to phytium MM protocol 03-08.

  @param [in,out]  Buffer    Content buffer.

  @retval    GetEinjTableSuccess       Success.
  @retval    GetEinjTableNotFound      EINJ table if not found.
  @retval    GetEinjTableLengthError   EINJ table Length mismatch.

**/
UINT32
GetEinjTable (
  IN OUT UINT8      *Buffer,
  IN     UINT32     Length
  )
{
  if (mEinjContext.EINJ == NULL) {
    DEBUG ((DEBUG_ERROR, "Einj Context is not found!\n"));
    return GetEinjTableNotFound;
  }
  if (Length != mEinjContext.EINJ->EinjTableHeader.Header.Length) {
    DEBUG ((DEBUG_ERROR, "Einj Header Length Mismatch!\n"));
    return GetEinjTableLengthError;
  }

  CopyMem (Buffer + 4,
           (VOID*) mEinjContext.EINJ,
          mEinjContext.EINJ->EinjTableHeader.Header.Length
          );

  return GetEinjTableSuccess;
}

/**
  Get BERT table, corresponding to phytium MM protocol 03-05.

  @param [in,out]  Buffer    Content buffer.

  @retval    GetBertTableSuccess       Success.
  @retval    GetBertTableNotFound      BERT table if not found.

**/
UINT32
GetBertTable (
  IN OUT UINT8      *Buffer,
  IN     UINT32     Length
  )
{
  if (mBertContext.BertHeader == NULL) {
    DEBUG ((DEBUG_ERROR, "BERT Context is not found!\n"));
    return GetBertTableNotFound;
  }

  CopyMem (Buffer + 4,
           (VOID*) mBertContext.BertHeader,
          mBertContext.BertHeader->Header.Length
          );

  return GetBertTableSuccess;
}

/**
  Parse time information from mm communication, corresponding to phytium MM
  protocol 03-09.

  @param[in, out]  Buffer    Buffer of mm communication content
  @param[out]      Time      A pointer to RAS_TIME.

  @retval    TimeStampSyncSuccess    Success.
**/
UINT32
ParseTimeSync (
  IN OUT UINT8      *Buffer,
  OUT    RAS_TIME   *Time
  )
{
  CopyMem (&Time->Utc, Buffer, 8);
  DEBUG ((DEBUG_INFO, "Time Sync Utc : %lld ms\n", Time->Utc));
  EpochToEfiTime ((UINTN)Time->Utc / 1000, &Time->Time);
  Time->Tick = MmioRead64 (FixedPcdGet64 (PcdTimerTickReadRegister));
  DEBUG ((DEBUG_INFO, "Time Tick : %lld", Time->Tick));
  PrintEfiTime (&Time->Time);

  return TimeStampSyncSuccess;
}

/**
  Syschronize memory devices location.

  @param[in]  Buffer    Buffer to parse.
  @param[in]  Length    Length of buffer.
  @param[out] Info      A pointer to SYNC_MEMORY_DEVICE_LOCATION.

  @retval  MemoryDeviceLocationSyncSuccess    Success.
**/
UINT32
SyncMemoryDeviceLocation (
  IN   UINT8                        *Buffer,
  IN   UINT32                       Length,
  OUT  SYNC_MEMORY_DEVICE_LOCATION  *Info
  )
{
  UINT32  Index;

  CopyMem (Info, Buffer, Length);
  for (Index = 0; Index < Info->Count; Index++) {
    DEBUG ((DEBUG_INFO,
            "Memory Device[%d]- Slot[%d]-Handle[0x%04x]\n",
            Info->Count,
            Info->Location[Index].SlotId,
            Info->Location[Index].SmbiosHandle
            ));
  }

  return MemoryDeviceLocationSyncSuccess;
}

/**
  Find memory device hanlde according to slot ID.

  @param[in]  SlotId         Slot ID to find.

  @retval     Handle         Memory device smbios handle.
**/
UINT16
FindMemorySlotHandle (
  IN UINT16  SlotId
  )
{
  UINT32  Index;
  UINT16  Handle;

  Handle = 0;
  DEBUG ((DEBUG_INFO, "FindMemorySlotHandle Slot : %d\n", SlotId));
  for (Index = 0; Index < mMemDeviceLocation.Count; Index++) {
    DEBUG ((DEBUG_INFO,
            "To find SlotId[%d] Handle[0x%04x]\n",
            mMemDeviceLocation.Location[Index].SlotId,
            mMemDeviceLocation.Location[Index].SmbiosHandle
            ));
    if (mMemDeviceLocation.Location[Index].SlotId == SlotId) {
      Handle = mMemDeviceLocation.Location[Index].SmbiosHandle;
      DEBUG ((DEBUG_INFO, "find Slot[%d] handle[0x%04x]!\n", SlotId, Handle));
      break;
    }
  }

  return Handle;
}

/**
  Find slot id according to memory device hanlde according.

  @param[in]  Handle         Memory device smbios handle to find.
  @retval     SlotId         Slot ID.
**/
UINT32
FindMemoryChannelID (
  IN  UINT16  Handle
  )
{
  UINT32  Index;
  UINT32  ChannelId;

  ChannelId = 0;
  DEBUG ((DEBUG_INFO, "FindMemorySlotHandle Handle : %04x\n", Handle));
  for (Index = 0; Index < mMemDeviceLocation.Count; Index++) {
    DEBUG ((DEBUG_INFO,
            "To find SlotId[%d] Handle[0x%04x]\n",
            mMemDeviceLocation.Location[Index].SlotId,
            mMemDeviceLocation.Location[Index].SmbiosHandle
            ));
    if (mMemDeviceLocation.Location[Index].SmbiosHandle == Handle) {
      ChannelId = mMemDeviceLocation.Location[Index].SlotId;
      DEBUG ((DEBUG_INFO, "find Slot[%d] handle[0x%04x]!\n", ChannelId, Handle));
      break;
    }
  }

  return ChannelId;
}

/**
  Update cper buffer from pbf. For example, timestamp.

  @param[in, out]  Buffer    CPER buffer from pbf.
  @param[in]       Length    Length of CPER buffer.

  @retval    EFI_SUCCESS    Success.
**/
EFI_STATUS
UpdateCperBuffer (
  IN  OUT  UINT8   *Buffer,
  IN       UINT32  Length
  )
{
  EFI_COMMON_ERROR_RECORD_HEADER  Header;
  RAS_TIME                        Time;
  EFI_ERROR_SECTION_DESCRIPTOR    Descriptor;
  EFI_PLATFORM_MEMORY_ERROR_DATA  MemorySection1;
  EFI_PLATFORM_MEMORY2_ERROR_DATA MemorySection2;

  //
  //Update Header
  //
  CopyMem ((VOID *) &Header, Buffer, sizeof (EFI_COMMON_ERROR_RECORD_HEADER));
  Header.ValidationBits |= BIT1;
  ZeroMem (&Time, sizeof (RAS_TIME));
  Time.Tick = MmioRead64 (FixedPcdGet64 (PcdTimerTickReadRegister));
  DEBUG ((DEBUG_INFO, "Time Tick : %lld\n", Time.Tick));
  Time.Utc = (Time.Tick - mRasTime.Tick) / (50 * 1000) + mRasTime.Utc;
  DEBUG ((DEBUG_INFO, "Time Utc Now : %lld\n", Time.Utc));
  EpochToEfiTime ((UINTN)Time.Utc / 1000, &Time.Time);
  Header.TimeStamp.Seconds = BinToBcd (Time.Time.Second);
  Header.TimeStamp.Minutes = BinToBcd (Time.Time.Minute);
  Header.TimeStamp.Hours = BinToBcd (Time.Time.Hour);
  Header.TimeStamp.Flag = 1;
  Header.TimeStamp.Day = BinToBcd (Time.Time.Day);
  Header.TimeStamp.Month = BinToBcd (Time.Time.Month);
  Header.TimeStamp.Year = BinToBcd (Time.Time.Year - 2000);
  Header.TimeStamp.Century = BinToBcd (20);
  CopyMem (Buffer, (VOID *) &Header, sizeof (EFI_COMMON_ERROR_RECORD_HEADER));
  //
  //Update Section Descriptor
  //
  ZeroMem (&Descriptor, sizeof (EFI_ERROR_SECTION_DESCRIPTOR));
  CopyMem (&Descriptor,
           (VOID *) (Buffer + sizeof (EFI_COMMON_ERROR_RECORD_HEADER)),
           sizeof (EFI_ERROR_SECTION_DESCRIPTOR)
           );
  PrintSectionDescriptor (&Descriptor);
  //
  //Update Section
  //
  //Memory Section 1
  if (CompareGuid (&Descriptor.SectionType, &gRasErrorSectionPlatformMemoryGuid)) {
    DEBUG ((DEBUG_INFO, "Update Memory1 Section!\n"));
    ZeroMem (&MemorySection1, sizeof (EFI_PLATFORM_MEMORY_ERROR_DATA));
    CopyMem (&MemorySection1,
             (VOID *) (Buffer + Descriptor.SectionOffset),
             sizeof (EFI_PLATFORM_MEMORY_ERROR_DATA)
             );
    //DDR Channel ID to card
    MemorySection1.Card = MemorySection1.ModuleHandle;
    MemorySection1.ValidFields |= EFI_PLATFORM_MEMORY_ERROR_MODULE_HANDLE_VALID;
    //Receive channel Id from PBF
    DEBUG ((DEBUG_INFO, "MemorySection1.ModuleHandle : 0x%x\n", MemorySection1.ModuleHandle));
    MemorySection1.ModuleHandle = FindMemorySlotHandle (MemorySection1.ModuleHandle);
    CopyMem ((VOID *) (Buffer + Descriptor.SectionOffset),
             &MemorySection1,
             sizeof (EFI_PLATFORM_MEMORY_ERROR_DATA)
             );
  } else if(CompareGuid (&Descriptor.SectionType, &gRasErrorSectionPlatformMemory2Guid)) {
    //Memory Section 2
    DEBUG ((DEBUG_INFO, "Update Memory2 Section!\n"));
    ZeroMem (&MemorySection2, sizeof (EFI_PLATFORM_MEMORY2_ERROR_DATA));
    CopyMem (&MemorySection2,
             (VOID *) (Buffer + Descriptor.SectionOffset),
             sizeof (EFI_PLATFORM_MEMORY2_ERROR_DATA)
             );
    //DDR Channel ID to card
    MemorySection2.Card = MemorySection2.ModuleHandle;
    MemorySection2.ValidFields |= EFI_PLATFORM_MEMORY2_MODULE_HANDLE_VALID;
    //Receive channel Id from PBF
    DEBUG ((DEBUG_INFO, "MemorySection2.ModuleHandle : 0x%x\n", MemorySection2.ModuleHandle));
    MemorySection2.ModuleHandle = FindMemorySlotHandle (MemorySection2.ModuleHandle);
    CopyMem ((VOID *) (Buffer + Descriptor.SectionOffset),
             &MemorySection2,
             sizeof (EFI_PLATFORM_MEMORY2_ERROR_DATA)
             );
  }

  return EFI_SUCCESS;
}

/**
  Update timestamp of CPER.

  @param[in]  Buffer    A pointer to CPER buffer.
  @param[in]  Length    CPER buffer length.
**/
VOID
UpdateTimeStamp (
  IN  OUT  UINT8   *Buffer,
  IN       UINT32  Length
  )
{
  EFI_COMMON_ERROR_RECORD_HEADER  Header;
  RAS_TIME                        Time;

  //
  //Update Header
  //
  CopyMem ((VOID *) &Header, Buffer, sizeof (EFI_COMMON_ERROR_RECORD_HEADER));
  Header.ValidationBits |= BIT1;
  ZeroMem (&Time, sizeof (RAS_TIME));
  Time.Tick = MmioRead64 (FixedPcdGet64 (PcdTimerTickReadRegister));
  DEBUG ((DEBUG_INFO, "Time Tick : %lld\n", Time.Tick));
  Time.Utc = (Time.Tick - mRasTime.Tick) / (50 * 1000) + mRasTime.Utc;
  DEBUG ((DEBUG_INFO, "Time Utc Now : %lld\n", Time.Utc));
  EpochToEfiTime ((UINTN)Time.Utc / 1000, &Time.Time);
  Header.TimeStamp.Seconds = BinToBcd (Time.Time.Second);
  Header.TimeStamp.Minutes = BinToBcd (Time.Time.Minute);
  Header.TimeStamp.Hours = BinToBcd (Time.Time.Hour);
  Header.TimeStamp.Flag = 1;
  Header.TimeStamp.Day = BinToBcd (Time.Time.Day);
  Header.TimeStamp.Month = BinToBcd (Time.Time.Month);
  Header.TimeStamp.Year = BinToBcd (Time.Time.Year - 2000);
  Header.TimeStamp.Century = BinToBcd (20);
  CopyMem (Buffer, (VOID *) &Header, sizeof (EFI_COMMON_ERROR_RECORD_HEADER));
}

/**
  Update memory cper. The card field means the DDR channel ID. The module handle
  field means the handle of smbios type 17 corresponding to the memory channel.

  @param[in]    Buffer    A pointer to CPER buffer.
  @param[in]    Length    CPER buffer length.
**/
VOID
UpdateMemoryCper (
  IN OUT UINT8   *Buffer,
  IN     UINT32  Length
  )
{
  EFI_ERROR_SECTION_DESCRIPTOR    Descriptor;
  EFI_PLATFORM_MEMORY_ERROR_DATA  MemorySection1;
  EFI_PLATFORM_MEMORY2_ERROR_DATA MemorySection2;
  //
  //Update Section Descriptor
  //
  ZeroMem (&Descriptor, sizeof (EFI_ERROR_SECTION_DESCRIPTOR));
  CopyMem (&Descriptor,
           (VOID *) (Buffer + sizeof (EFI_COMMON_ERROR_RECORD_HEADER)),
           sizeof (EFI_ERROR_SECTION_DESCRIPTOR)
           );
  PrintSectionDescriptor (&Descriptor);
  //
  //Update Section
  //
  //Memory Section 1
  if (CompareGuid (&Descriptor.SectionType, &gRasErrorSectionPlatformMemoryGuid)) {
    DEBUG ((DEBUG_INFO, "Update Memory1 Section!\n"));
    ZeroMem (&MemorySection1, sizeof (EFI_PLATFORM_MEMORY_ERROR_DATA));
    CopyMem (&MemorySection1,
             (VOID *) (Buffer + Descriptor.SectionOffset),
             sizeof (EFI_PLATFORM_MEMORY_ERROR_DATA)
             );
    MemorySection1.Card = MemorySection1.ModuleHandle;
    MemorySection1.ValidFields |= EFI_PLATFORM_MEMORY_ERROR_MODULE_HANDLE_VALID;
    //Receive channel Id from PBF
    DEBUG ((DEBUG_INFO, "MemorySection1.ModuleHandle : 0x%x\n", MemorySection1.ModuleHandle));
    MemorySection1.ModuleHandle = FindMemorySlotHandle (MemorySection1.ModuleHandle);
    CopyMem ((VOID *) (Buffer + Descriptor.SectionOffset),
             &MemorySection1,
             sizeof (EFI_PLATFORM_MEMORY_ERROR_DATA)
             );
  } else if(CompareGuid (&Descriptor.SectionType, &gRasErrorSectionPlatformMemory2Guid)) {
    //Memory Section 2
    DEBUG ((DEBUG_INFO, "Update Memory2 Section!\n"));
    ZeroMem (&MemorySection2, sizeof (EFI_PLATFORM_MEMORY2_ERROR_DATA));
    CopyMem (&MemorySection2,
             (VOID *) (Buffer + Descriptor.SectionOffset),
             sizeof (EFI_PLATFORM_MEMORY2_ERROR_DATA)
             );
    MemorySection2.Card = MemorySection2.ModuleHandle;
    MemorySection2.ValidFields |= EFI_PLATFORM_MEMORY2_MODULE_HANDLE_VALID;
    //Receive channel Id from PBF
    DEBUG ((DEBUG_INFO, "MemorySection2.ModuleHandle : 0x%x\n", MemorySection2.ModuleHandle));
    MemorySection2.ModuleHandle = FindMemorySlotHandle (MemorySection2.ModuleHandle);
    CopyMem ((VOID *) (Buffer + Descriptor.SectionOffset),
             &MemorySection2,
             sizeof (EFI_PLATFORM_MEMORY2_ERROR_DATA)
             );
  }
}

/**
  Handle fatal errors based on different record types.

  @param[in]    Buffer    A pointer to CPER buffer.
  @param[in]    Length    CPER data length.
  @param[in]    Type      Record type. 0 - no record, 1 - flash.
**/
VOID
FatalErrorHandle (
  IN  UINT8   *Buffer,
  IN  UINT32  Length,
  IN  UINT8   Type
  )
{
  EFI_COMMON_ERROR_RECORD_HEADER  *Header;
  UINT8                           *Record;
  UINT64                          Signature;
  UINT64                          Address;

  Signature = FATAL_ERROR_RECORD_SIGNATURE;
  Header = (EFI_COMMON_ERROR_RECORD_HEADER *) Buffer;

  if (Header->ErrorSeverity != EFI_GENERIC_ERROR_FATAL) {
    return ;
  }
  Record = AllocatePool (12 + Length);
  if (Record == NULL) {
    DEBUG ((DEBUG_ERROR, "alocate failed!\n"));
    return;
  }
  CopyMem (Record, &Signature, 8);
  CopyMem (Record + 8, &Length, 4);
  CopyMem (Record + 12, Buffer, Length);
  switch (Type) {
  case RasNoRecord:
    DEBUG ((DEBUG_INFO, "Fatal Record no record\n"));
    break;
  case RasFlashRecord:
    Address = FixedPcdGet64 (PcdRasFatalErrorFlashAddress);
    if (mFlash == NULL) {
      DEBUG ((DEBUG_ERROR, "mFlash == NULL\n"));
      return ;
    }
    if (mFlash->Erase (Address, SIZE_64KB) != EFI_SUCCESS) {
      DEBUG ((DEBUG_ERROR, "Flash erase failed!\n"));
      return;
    }
    if (mFlash->Write (Address, Record, 12 + Length) != EFI_SUCCESS) {
      DEBUG ((DEBUG_ERROR, "Flash write failed!\n"));
      return;
    }
    break;
  default:
    DEBUG ((DEBUG_ERROR, "Fatal Record unsupported record type!\n"));
    break;
  }

  if (Record != NULL) {
    FreePool (Record);
  }
}

/**
  Clear fatal error record according to different record types.

  @param[in]    Type      Record type. 0 - no record, 1 - flash.
**/
VOID
FatalErrorRecordClear (
  IN UINT8  Type
  )
{
  UINT64  Address;

  switch (Type) {
  case RasNoRecord:
    DEBUG ((DEBUG_INFO, "Fatal Record no record\n"));
    break;
  case RasFlashRecord:
    Address = FixedPcdGet64 (PcdRasFatalErrorFlashAddress);
    if (mFlash == NULL) {
      DEBUG ((DEBUG_ERROR, "mFlash == NULL\n"));
      return ;
    }
    if (mFlash->Erase (Address, SIZE_64KB) != EFI_SUCCESS) {
      DEBUG ((DEBUG_ERROR, "Flash erase failed!\n"));
      return;
    }
    break;
  default:
    DEBUG ((DEBUG_ERROR, "Fatal Record unsupported record type!\n"));
    break;
  }
}

/**
  Get memory isolation record from different record types.

  @param[in]    Type      Record type. 0 - no record, 1 - flash.
  @param[out]   Record    A pointer to record.

  @retval       EFI_SUCCESS    Get record successfully.
  @retval       EFI_NOT_FOUND  Record not found.
**/
EFI_STATUS
GetMemoryIsolationRecord (
  IN  UINT8           Type,
  OUT RAS_MEM_RECORD  *Record
  )
{
  EFI_STATUS  Status;
  UINT64      Address;

  Status = EFI_NOT_FOUND;
  switch (Type) {
  case RasNoRecord:
    DEBUG ((DEBUG_INFO, "Fatal Record no record\n"));
    Status = EFI_NOT_FOUND;
    break;
  case RasFlashRecord:
    Address = FixedPcdGet64 (PcdRasMemoryRecordFlashAddress);
    if (mFlash == NULL) {
      DEBUG ((DEBUG_ERROR, "mFlash == NULL\n"));
      Status = EFI_NOT_FOUND;
      goto ProcExit;
    }
    if (mFlash->Read (Address, Record, sizeof (RAS_MEM_RECORD)) != EFI_SUCCESS) {
      Status = EFI_NOT_FOUND;
      goto ProcExit;
    }
    Status = EFI_SUCCESS;
    break;
  default:
    DEBUG ((DEBUG_ERROR, "Fatal Record unsupported record type!\n"));
    Status = EFI_NOT_FOUND;
    break;
  }
ProcExit:
  return Status;
}

/**
  Record memory isolation data according to different record types.

  @param[in]    Type      Record type. 0 - no record, 1 - flash.
  @param[out]   Record    A pointer to record.

  @retval       EFI_SUCCESS       Record successfully.
  @retval       EFI_DEVICE_ERROR  Record failed.
  @retval       EFI_UNSUPPORTED   Record type unsupported.
**/
EFI_STATUS
SetMemoryIsolationRecord (
  IN  UINT8           Type,
  IN  RAS_MEM_RECORD  *Record
  )
{
  EFI_STATUS  Status;
  UINT64      Address;

  DEBUG ((DEBUG_INFO, "%a begin\n", __FUNCTION__));
  Status = EFI_DEVICE_ERROR;
  switch (Type) {
  case RasNoRecord:
    DEBUG ((DEBUG_INFO, "Memory Record no record\n"));
    Status = EFI_SUCCESS;
    break;
  case RasFlashRecord:
    Address = FixedPcdGet64 (PcdRasMemoryRecordFlashAddress);
    if (mFlash == NULL) {
      DEBUG ((DEBUG_ERROR, "mFlash == NULL\n"));
      Status = EFI_DEVICE_ERROR;
      goto ProcExit;
    }
    if (mFlash->Erase (Address, SIZE_64KB) != EFI_SUCCESS) {
      DEBUG ((DEBUG_ERROR, "Flash erase failed!\n"));
      Status = EFI_DEVICE_ERROR;
      goto ProcExit;
    }
    if (mFlash->Write (Address, Record, sizeof (RAS_MEM_RECORD)) != EFI_SUCCESS) {
      Status = EFI_DEVICE_ERROR;
      goto ProcExit;
    }
    Status = EFI_SUCCESS;
    break;
  default:
    DEBUG ((DEBUG_ERROR, "Memory Record unsupported record type!\n"));
    Status = EFI_UNSUPPORTED;
    break;
  }
ProcExit:
  DEBUG ((DEBUG_INFO, "%a end Status : %r\n", __FUNCTION__, Status));
  return Status;
}

/**
  Get memory channel id form memory error CPER.

  @param[in]    Buffer      A pointer to CPER buffer.
  @param[in]    Length      CPER length.

  @retval       ChannelId   Memory channel ID.
**/
UINT32
GetMemoryChannelIdFromCper (
  IN  UINT8          *Buffer,
  IN  UINT32         Length
  )
{
  EFI_ERROR_SECTION_DESCRIPTOR    Descriptor;
  EFI_PLATFORM_MEMORY_ERROR_DATA  MemorySection1;
  EFI_PLATFORM_MEMORY2_ERROR_DATA MemorySection2;
  UINT32                          ChannelId;

  ChannelId = 0;
  //
  //Update Section Descriptor
  //
  ZeroMem (&Descriptor, sizeof (EFI_ERROR_SECTION_DESCRIPTOR));
  CopyMem (&Descriptor,
           (VOID *) (Buffer + sizeof (EFI_COMMON_ERROR_RECORD_HEADER)),
           sizeof (EFI_ERROR_SECTION_DESCRIPTOR)
           );
  PrintSectionDescriptor (&Descriptor);
  //
  //Update Section
  //
  //Memory Section 1
  if (CompareGuid (&Descriptor.SectionType, &gRasErrorSectionPlatformMemoryGuid)) {
    ZeroMem (&MemorySection1, sizeof (EFI_PLATFORM_MEMORY_ERROR_DATA));
    CopyMem (&MemorySection1,
             (VOID *) (Buffer + Descriptor.SectionOffset),
             sizeof (EFI_PLATFORM_MEMORY_ERROR_DATA)
             );
    //Parse Channel ID
    DEBUG ((DEBUG_INFO, "MemorySection1.ModuleHandle : 0x%x\n", MemorySection1.ModuleHandle));
    ChannelId = FindMemoryChannelID (MemorySection1.ModuleHandle);
  } else if(CompareGuid (&Descriptor.SectionType, &gRasErrorSectionPlatformMemory2Guid)) {
    //Memory Section 2
    ZeroMem (&MemorySection2, sizeof (EFI_PLATFORM_MEMORY2_ERROR_DATA));
    CopyMem (&MemorySection2,
             (VOID *) (Buffer + Descriptor.SectionOffset),
             sizeof (EFI_PLATFORM_MEMORY2_ERROR_DATA)
             );
    //Parse Channel ID
    DEBUG ((DEBUG_INFO, "MemorySection2.ModuleHandle : 0x%x\n", MemorySection2.ModuleHandle));
    ChannelId = FindMemoryChannelID (MemorySection2.ModuleHandle);
  }

  DEBUG ((DEBUG_INFO, "%a Channel ID : %d\n", __FUNCTION__, ChannelId));
  return ChannelId;
}

#if 0
/**

**/
BOOLEAN
GetPprInfo (
  IN  UINT8          *Buffer,
  IN  UINT32         Length,
  OUT PPR_RECORD     *Ppr
  )
{
  BOOLEAN                         NeedPpr;
  EFI_ERROR_SECTION_DESCRIPTOR    Descriptor;
  EFI_PLATFORM_MEMORY_ERROR_DATA  MemorySection1;

  NeedPpr = FALSE;
  //
  //Update Section Descriptor
  //
  ZeroMem (&Descriptor, sizeof (EFI_ERROR_SECTION_DESCRIPTOR));
  CopyMem (&Descriptor,
           (VOID *) (Buffer + sizeof (EFI_COMMON_ERROR_RECORD_HEADER)),
           sizeof (EFI_ERROR_SECTION_DESCRIPTOR)
           );
  //Parse PPR Information
  //Memory Section 1
  if (CompareGuid (&Descriptor.SectionType, &gRasErrorSectionPlatformMemoryGuid)) {
    DEBUG ((DEBUG_INFO, "Update Memory1 Section!\n"));
    ZeroMem (&MemorySection1, sizeof (EFI_PLATFORM_MEMORY_ERROR_DATA));
    CopyMem (&MemorySection1,
             (VOID *) (Buffer + Descriptor.SectionOffset),
             sizeof (EFI_PLATFORM_MEMORY_ERROR_DATA)
             );
    if (MemorySection1.Device & BIT15) {
      //PPR
      Ppr->Signature = PPR_RECORD_SIGNATURE;
      Ppr->ChannelId = GetMemoryChannelIdFromCper (Buffer, Length);;
      Ppr->SubChannel = (UINT8) (MemorySection1.Device & (~BIT15));
      Ppr->RankNum = (UINT8) MemorySection1.RankNum;
      Ppr->BankGroup = (UINT8) ((MemorySection1.Bank >> 8) & 0xFF);
      Ppr->Bank = (UINT8) (MemorySection1.Bank & 0xFF);
      Ppr->Row = (UINT8) MemorySection1.Row;
      NeedPpr = TRUE;
    } else {
      //Not PPR
      ZeroMem (Ppr, sizeof (PPR_RECORD));
      NeedPpr = FALSE;
    }
  } else if(CompareGuid (&Descriptor.SectionType, &gRasErrorSectionPlatformMemory2Guid)) {
    //Memory Section 2
    //Not PPR
    ZeroMem (Ppr, sizeof (PPR_RECORD));
    NeedPpr = FALSE;
  }

  return NeedPpr;
}
#endif

/**
  Memory isolation handle according to different types.
  Step 1: Get memory isolation record according different types.
  Step 2: Compared to the isolation threshold, if the current count value is
          greater than the threshold, set the isolation state to 1.
  Step 3: Set memory isolation record according different types.

  @param[in]    Buffer        CPER buffer.
  @param[in]    Length        CPER buffer length.
  @param[in]    Strategy      A pointer to RAS_STRATEGY.
**/
VOID
MemoryIsolationHandle (
  IN  UINT8          *Buffer,
  IN  UINT32         Length,
  IN  RAS_STRATEGY   *Strategy
  )
{
  EFI_COMMON_ERROR_RECORD_HEADER  *Header;
  RAS_MEM_RECORD                  Record;
  UINT32                          ChannelId;
  EFI_STATUS                      Status;

  DEBUG ((DEBUG_INFO, "%a begin\n", __FUNCTION__));
  //
  //Isolate or not
  //
  Header = (EFI_COMMON_ERROR_RECORD_HEADER *) Buffer;
  switch (Header->ErrorSeverity) {
  case EFI_GENERIC_ERROR_FATAL:
    DEBUG ((DEBUG_INFO, "%a FATAL UCE\n", __FUNCTION__));
    if (!Strategy->MemFatalUceIsoStrategy.Enable) {
      goto ProcExit;
    }
    //
    //Get Memory Isolation Record
    //
    Status = GetMemoryIsolationRecord (Strategy->MemoryIsoRecord, &Record);
    DEBUG ((DEBUG_INFO, "Record.MemFatalUceRecord.Signature : %llx\n", Record.MemFatalUceRecord.Signature));
    if (EFI_ERROR (Status)) {
      goto ProcExit ;
    }
    if (Record.MemFatalUceRecord.Signature != MEMORY_FATAL_UCE_SIGNATURE) {
      goto ProcExit;
    }
    ChannelId = GetMemoryChannelIdFromCper (Buffer, Length);
    DEBUG ((DEBUG_INFO, "ChannelId : %d, ", ChannelId));
    Record.MemFatalUceRecord.ChannelRecord[ChannelId].Count++;
    DEBUG ((DEBUG_INFO, "Count : %d\n", Record.MemFatalUceRecord.ChannelRecord[ChannelId].Count));
    if (Record.MemFatalUceRecord.ChannelRecord[ChannelId].Count >= Strategy->MemFatalUceIsoStrategy.Count) {
      DEBUG ((DEBUG_INFO, "Memory Fatal Reach Isolation Threshold Channel[%d]\n", ChannelId));
      Record.MemFatalUceRecord.ChannelRecord[ChannelId].IsoState = 1;
    }
    //PPR record
    //GetPprInfo (Buffer, Length, &Record.PprRecord);
    break;
  case EFI_GENERIC_ERROR_RECOVERABLE:
    DEBUG ((DEBUG_INFO, "%a NON-FATAL UCE\n", __FUNCTION__));
    if (!Strategy->MemNonFatalUceIsoStrategy.Enable) {
      goto ProcExit;
    }
    //
    //Get Memory Isolation Record
    //
    Status = GetMemoryIsolationRecord (Strategy->MemoryIsoRecord, &Record);
    DEBUG ((DEBUG_INFO, "Record.MemNonFatalUceRecord.Signature : %llx\n", Record.MemNonFatalUceRecord.Signature));
    if (EFI_ERROR (Status)) {
      goto ProcExit ;
    }
    if (Record.MemNonFatalUceRecord.Signature != MEMORY_NON_FATAL_UCE_SIGNATURE) {
      goto ProcExit;
    }
    ChannelId = GetMemoryChannelIdFromCper (Buffer, Length);
    DEBUG ((DEBUG_INFO, "ChannelId : %d, ", ChannelId));
    Record.MemNonFatalUceRecord.ChannelRecord[ChannelId].Count++;
    DEBUG ((DEBUG_INFO, "Count : %d\n",Record.MemNonFatalUceRecord.ChannelRecord[ChannelId].Count));
    if (Record.MemNonFatalUceRecord.ChannelRecord[ChannelId].Count >= Strategy->MemNonFatalUceIsoStrategy.Count) {
      DEBUG ((DEBUG_INFO, "Memory Non-Fatal Reach Isolation Threshold Channel[%d]\n", ChannelId));
      Record.MemNonFatalUceRecord.ChannelRecord[ChannelId].IsoState = 1;
    }
    break;
  case EFI_GENERIC_ERROR_CORRECTED:
    DEBUG ((DEBUG_INFO, "%a CE\n", __FUNCTION__));
    if (!Strategy->MemCeIsoStrategy.Enable) {
      goto ProcExit;
    }
    Status = GetMemoryIsolationRecord (Strategy->MemoryIsoRecord, &Record);
    DEBUG ((DEBUG_INFO,"Record.MemCeRecord.Signature : %llx\n", Record.MemCeRecord.Signature));
    if (EFI_ERROR (Status)) {
      goto ProcExit ;
    }
    if (Record.MemCeRecord.Signature != MEMORY_CE_SIGNATURE) {
      goto ProcExit;
    }
    ChannelId = GetMemoryChannelIdFromCper (Buffer, Length);
    DEBUG ((DEBUG_INFO, "ChannelId : %d, ", ChannelId));
    Record.MemCeRecord.ChannelRecord[ChannelId].Count++;
    DEBUG ((DEBUG_INFO, "Count : %d\n",Record.MemCeRecord.ChannelRecord[ChannelId].Count));
    if (Record.MemCeRecord.ChannelRecord[ChannelId].Count >= Strategy->MemCeIsoStrategy.Count) {
      DEBUG ((DEBUG_INFO, "Memory CE Reach Isolation Threshold Channel[%d]\n", ChannelId));
      Record.MemCeRecord.ChannelRecord[ChannelId].IsoState = 1;
    }
    break;
  default:
    goto ProcExit;
    break;
  }
  //
  //Set Record
  //
  Status = SetMemoryIsolationRecord (Strategy->MemoryIsoRecord, &Record);

ProcExit:
  DEBUG ((DEBUG_INFO, "%a end\n", __FUNCTION__));
  return ;
}

/**
  Update memory isolation address record according to record type.

  @param[in]    Addr        Address to isolation.
  @param[in]    Rec         A pointer to RAS_MEM_RECORD.
**/
STATIC
VOID
UpdateMemAddrIsoRecord (
  IN     UINT64          Addr,
  IN OUT RAS_MEM_RECORD  *Rec
  )
{
  UINT32  Index;

  DEBUG ((DEBUG_INFO, "%a start!\n", __FUNCTION__));
  DEBUG ((DEBUG_INFO, "Check Address exist\n"));
  for (Index = 0; Index < Rec->MemAddrIsoRecord.Count; Index++) {
    DEBUG ((DEBUG_INFO, "Index[%d] : 0x%llx\n", Index, Rec->MemAddrIsoRecord.Addr[Index]));
    if (Rec->MemAddrIsoRecord.Addr[Index] == Addr) {
      DEBUG ((DEBUG_INFO, "Addr 0x%llx has exist.\n", Addr));
      goto ProcExit;
    }
  }
  //
  //create new address record
  //
  DEBUG ((DEBUG_INFO, "step 1-Rec->MemAddrIsoRecord.Count : %d\n", Rec->MemAddrIsoRecord.Count));
  DEBUG ((DEBUG_INFO, "step 1-Rec->MemAddrIsoRecord.Index : %d\n", Rec->MemAddrIsoRecord.Index));
  DEBUG ((DEBUG_INFO, "Addr : 0x%llx\n", Addr));
  Rec->MemAddrIsoRecord.Addr[Rec->MemAddrIsoRecord.Index] = Addr;
  if (Rec->MemAddrIsoRecord.Count >= MAX_ADDR_ISO_RECORD_COUNT) {
    Rec->MemAddrIsoRecord.Count = MAX_ADDR_ISO_RECORD_COUNT;
  } else {
    Rec->MemAddrIsoRecord.Count++;
  }
  Rec->MemAddrIsoRecord.Index++;
  if (Rec->MemAddrIsoRecord.Index >= MAX_ADDR_ISO_RECORD_COUNT) {
    Rec->MemAddrIsoRecord.Index = 0;
  }
  DEBUG ((DEBUG_INFO, "Step 2-Rec->MemAddrIsoRecord.Count : %d\n", Rec->MemAddrIsoRecord.Count));
  DEBUG ((DEBUG_INFO, "Step 2-Rec->MemAddrIsoRecord.Index : %d\n", Rec->MemAddrIsoRecord.Index));
  DEBUG ((DEBUG_INFO, "Memory Address Isolation List:\n"));
ProcExit:
  for (Index = 0; Index < Rec->MemAddrIsoRecord.Count; Index++) {
    DEBUG ((DEBUG_INFO, "Address[%d] : 0x%llx\n", Index, Rec->MemAddrIsoRecord.Addr[Index]));
  }
  DEBUG ((DEBUG_INFO, "%a End!\n", __FUNCTION__));
}

/**
  Memory address isolation handle according to different types.

  Step 1: Get align size.
  Step 2: Get memory isolation record according different types.
  Step 3: Update memory isolation address. If the address is existed, it will be
          ignored.
  Step 4: Set memory isolation record according different types.

  @param[in]    Buffer        CPER buffer.
  @param[in]    Length        CPER buffer length.
  @param[in]    Strategy      A pointer to RAS_STRATEGY.
**/
VOID
MemoryAddrIsolationHandle (
  IN  UINT8          *Buffer,
  IN  UINT32         Length,
  IN  RAS_STRATEGY   *Strategy
  )
{
  EFI_COMMON_ERROR_RECORD_HEADER  *Header;
  RAS_MEM_RECORD                  Record;
  EFI_STATUS                      Status;
  UINT64                          Addr;
  UINT64                          AlignSize;
  EFI_ERROR_SECTION_DESCRIPTOR    Descriptor;
  EFI_PLATFORM_MEMORY_ERROR_DATA  MemorySection1;
  EFI_PLATFORM_MEMORY2_ERROR_DATA MemorySection2;

  DEBUG ((DEBUG_INFO, "%a begin\n", __FUNCTION__));
  switch (Strategy->MemAddrIsoStrategy.Type) {
  case ADDR_ISO_512MB:
    AlignSize = 512 * SIZE_1MB;
    break;
  case ADDR_ISO_128MB:
    AlignSize = 128 * SIZE_1MB;
    break;
  default:
    DEBUG ((DEBUG_ERROR, "Isolation Type Unsupported : %d"));
    goto ProcExit;
  }
  DEBUG ((DEBUG_INFO, "Align Size = 0x%x\n", AlignSize));
  Header = (EFI_COMMON_ERROR_RECORD_HEADER *) Buffer;
  if ((Header->ErrorSeverity != EFI_GENERIC_ERROR_FATAL) &&
          (Header->ErrorSeverity != EFI_GENERIC_ERROR_RECOVERABLE)) {
    DEBUG ((DEBUG_ERROR, "Only handle Fatal-UCE and Non-Fatal-UCE"));
    goto ProcExit;
  }
  ZeroMem (&Descriptor, sizeof (EFI_ERROR_SECTION_DESCRIPTOR));
  CopyMem (&Descriptor,
           (VOID *) (Buffer + sizeof (EFI_COMMON_ERROR_RECORD_HEADER)),
           sizeof (EFI_ERROR_SECTION_DESCRIPTOR)
           );
  if (CompareGuid (&Descriptor.SectionType, &gRasErrorSectionPlatformMemoryGuid)) {
    ZeroMem (&MemorySection1, sizeof (EFI_PLATFORM_MEMORY_ERROR_DATA));
    CopyMem (&MemorySection1,
             (VOID *) (Buffer + Descriptor.SectionOffset),
             sizeof (EFI_PLATFORM_MEMORY_ERROR_DATA)
             );
    //Parse Address
    DEBUG ((DEBUG_INFO, "MemorySection1.Address : 0x%llx\n", MemorySection1.PhysicalAddress));
    Addr = MemorySection1.PhysicalAddress;
  } else if(CompareGuid (&Descriptor.SectionType, &gRasErrorSectionPlatformMemory2Guid)) {
    //Memory Section 2
    ZeroMem (&MemorySection2, sizeof (EFI_PLATFORM_MEMORY2_ERROR_DATA));
    CopyMem (&MemorySection2,
             (VOID *) (Buffer + Descriptor.SectionOffset),
             sizeof (EFI_PLATFORM_MEMORY2_ERROR_DATA)
             );
    //Parse Address
    DEBUG ((DEBUG_INFO, "MemorySection2.Address : 0x%llx\n", MemorySection2.PhysicalAddress));
    Addr = MemorySection2.PhysicalAddress;
  }
  Addr = Addr / AlignSize * AlignSize;
  DEBUG ((DEBUG_INFO, "Address after align : 0x%llx\n", Addr));
  //
  //Get Memory Isolation Record
  //
  Status = GetMemoryIsolationRecord (Strategy->MemoryIsoRecord, &Record);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "GetMemoryIsolationRecord : %r\n", Status));
    goto ProcExit;
  }
  DEBUG ((DEBUG_INFO, "Memory Address Isolation Record Signature : %llx\n", Record.MemAddrIsoRecord.Signature));
  if (Record.MemAddrIsoRecord.Signature != MEM_ADDR_ISO_RECORD_SIGNATURE) {
    DEBUG ((DEBUG_ERROR, "Signature Mismatch!\n"));
  }
  //
  //Update Memory Address Isolation Record
  //
  UpdateMemAddrIsoRecord (Addr, &Record);
  //
  //Update Memory record
  //
  Status = SetMemoryIsolationRecord (Strategy->MemoryIsoRecord, &Record);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SetMemoryIsolationRecord : %r\n", Status));
    goto ProcExit;
  }

ProcExit:
  DEBUG ((DEBUG_INFO, "%a end\n", __FUNCTION__));
}

/**
  CPU isolation handle according to different types.
  Step 1: Get CPU isolation record according different types.
  Step 2: Compared to the isolation threshold, if the current count value is
          greater than the threshold, set the isolation state to 1.
  Step 3: Set memory isolation record according different types.

  @param[in]    Buffer        CPER buffer.
  @param[in]    Length        CPER buffer length.
  @param[in]    Strategy      A pointer to RAS_STRATEGY.

**/
VOID
CpuIsolationHandle (
  IN  UINT8          *Buffer,
  IN  UINT32         Length,
  IN  RAS_STRATEGY   *Strategy
  )
{
  EFI_COMMON_ERROR_RECORD_HEADER  *Header;
  RAS_CPU_RECORD                  Record;
  UINT32                          Bit;
  UINT64                          Mpidr;
  UINT8                           *Descriptor;
  UINT8                           *Ptr;
  UINT64                          Address;

  Ptr = (VOID *)Buffer;
  Header = (EFI_COMMON_ERROR_RECORD_HEADER *) Buffer;
  Descriptor = (Ptr + sizeof (EFI_COMMON_ERROR_RECORD_HEADER) +
                 sizeof (EFI_ERROR_SECTION_DESCRIPTOR));
  CopyMem (&Mpidr, Descriptor + 16, 8);
  DEBUG ((DEBUG_INFO, "Mpidr : %llx\n", Mpidr));
  if (Header->ErrorSeverity == EFI_GENERIC_ERROR_FATAL) {
    DEBUG ((DEBUG_INFO, "Cpu Fatal Error Isolation Handle!\n"));
    if (Strategy->CpuFatalStrategy.Enable) {
      Bit = FindMapMask (Mpidr);
      DEBUG ((DEBUG_INFO,"Isolation Bit : %d\n", Bit));
      switch (Strategy->CpuFatalStrategy.Record) {
      case RasNoRecord:
        DEBUG ((DEBUG_INFO, "Fatal Record no record\n"));
        break;
      case RasFlashRecord:
        Address = FixedPcdGet64 (PcdRasCpuRecordFlashAddress);
        if (mFlash == NULL) {
          DEBUG ((DEBUG_ERROR, "mFlash == NULL\n"));
          return ;
        }
        //
        //MPIDR to Isolation Bit Map
        //
        if (mFlash->Read (Address, &Record, sizeof (RAS_CPU_RECORD)) != EFI_SUCCESS) {
          goto ProcExit;
        }
        //
        //Sinature mismatch
        //
        if (Record.Signature != CPU_FATAL_UCE_SIGNATURE) {
          ZeroMem (&Record, sizeof (RAS_CPU_RECORD));
          Record.Signature = CPU_FATAL_UCE_SIGNATURE;
        }
        Record.IsoMap[Bit/64] |= 0x1 << (Bit%64);
        DEBUG ((DEBUG_INFO, "Bit Map : %llx\n", Record.IsoMap[Bit/64]));
        if (mFlash->Erase (Address, SIZE_64KB) != EFI_SUCCESS) {
          DEBUG ((DEBUG_ERROR, "Flash erase failed!\n"));
          goto ProcExit;
        }
        if (mFlash->Write (Address, &Record, sizeof (RAS_CPU_RECORD)) != EFI_SUCCESS) {
          DEBUG ((DEBUG_ERROR, "Flash write failed!\n"));
          goto ProcExit;
        }
        break;
      default:
        DEBUG ((DEBUG_ERROR, "Fatal Record unsupported record type!\n"));
        break;
      }
    }
  }
ProcExit:
  return ;
}

/**
  CPER handle.
  Step 1: Update timestamp of CPER.
  Step 2: If CPER is memory type, update the memory information, otherwise go
          to step 3.
  Step 3: If CPER is CE(only memory now), perform reporting threshold.
  Step 4: If ipmi function is enabled, send CPER data to BMC.
  Step 5: Generate GHES data.
  Step 6: If CPER is fatal error, save the data for next BERT data.
  Step 7: If CPER is memory type, perform isolation processing.
  Step 8: If CPER is CPU type, perform isolation processing.

  @param[in]    Buffer        CPER data.
  @param[in]    Length        CPER data length.
  @param[in]    Strategy      A pointer to RAS_STRATEGY.

  @retval       0        Success.
  @retval       Other    Failed.
**/
UINT32
CperHandle (
  IN UINT8         *Buffer,
  IN UINT32        Length,
  IN RAS_STRATEGY  *Strategy
  )
{
  EFI_STATUS                      EfiStatus;
  EFI_COMMON_ERROR_RECORD_HEADER  Header;
  EFI_ERROR_SECTION_DESCRIPTOR    Descriptor;
  UINT32                          Status;
  UINT8                           Response;

  Status = GenerateGhesNotNeedReport;
  //
  //1.Update timestamp
  //
  UpdateTimeStamp (Buffer, Length);
  //
  //2.Cper handle according to section type
  //
  CopyMem ((VOID *) &Header, Buffer, sizeof (EFI_COMMON_ERROR_RECORD_HEADER));
  ZeroMem (&Descriptor, sizeof (EFI_ERROR_SECTION_DESCRIPTOR));
  CopyMem (&Descriptor,
           (VOID *) (Buffer + sizeof (EFI_COMMON_ERROR_RECORD_HEADER)),
           sizeof (EFI_ERROR_SECTION_DESCRIPTOR)
           );
  PrintSectionDescriptor (&Descriptor);
  if (CompareGuid (&Descriptor.SectionType, &gRasErrorSectionPlatformMemoryGuid)) {
    //Memory1
    DEBUG ((DEBUG_INFO, "Update Memory1 Section!\n"));
    UpdateMemoryCper (Buffer, Length);
    if (Header.ErrorSeverity == EFI_GENERIC_ERROR_CORRECTED) {
      //Ce threshold
      mRasTotalCount.MemCeCount++;
      if (Strategy->MemCeReportStrategy.Enable) {
        if (mRasTotalCount.MemCeCount < Strategy->MemCeReportStrategy.Count) {
          DEBUG ((DEBUG_INFO, "mRasTotalCount.MemCeCount : %d\n", mRasTotalCount.MemCeCount));
          goto FinishReport;
        } else {
          //clear
          DEBUG ((DEBUG_INFO, "Mem Ce Reach Threshold and clear!\n"));
          mRasTotalCount.MemCeCount = 0;
        }
      }
    }
  } else if (CompareGuid (&Descriptor.SectionType, &gRasErrorSectionPlatformMemory2Guid)) {
    //Memory2
    DEBUG ((DEBUG_INFO, "Update Memory2 Section!\n"));
    UpdateMemoryCper (Buffer, Length);
    if (Header.ErrorSeverity == EFI_GENERIC_ERROR_CORRECTED) {
      //Ce threshold
      mRasTotalCount.MemCeCount++;
      if (Strategy->MemCeReportStrategy.Enable) {
        if (mRasTotalCount.MemCeCount < Strategy->MemCeReportStrategy.Count) {
          DEBUG ((DEBUG_INFO, "mRasTotalCount.MemCeCount : %d\n", mRasTotalCount.MemCeCount));
          goto FinishReport;
        } else {
          //clear
          DEBUG ((DEBUG_INFO, "Mem Ce Reach Threshold and clear!\n"));
          mRasTotalCount.MemCeCount = 0;
        }
      }
    }
  } else if (CompareGuid (&Descriptor.SectionType, &gRasErrorSectiionPcieGuid)
          || CompareGuid (&Descriptor.SectionType, &gRasErrorSectiionPciPcixBusGuid)
          || CompareGuid (&Descriptor.SectionType, &gRasErrorSectiionPciDeviceGuid)) {
    //Pcie
    DEBUG ((DEBUG_INFO, "Pcie Section!\n"));
    if (Header.ErrorSeverity == EFI_GENERIC_ERROR_CORRECTED) {
      //Ce threshold
      mRasTotalCount.PcieCeCount++;
      if (Strategy->PcieCeReportStrategy.Enable) {
        if (mRasTotalCount.PcieCeCount < Strategy->PcieCeReportStrategy.Count) {
          DEBUG ((DEBUG_INFO, "mRasTotalCount.PcieCeCount : %d\n", mRasTotalCount.PcieCeCount));
          goto FinishReport;
        } else {
          //clear
          DEBUG ((DEBUG_INFO, "Pcie Ce Reach Threshold and clear!\n"));
          mRasTotalCount.PcieCeCount = 0;
        }
      }
    }
  } else if (CompareGuid (&Descriptor.SectionType, &gRasErrorSectiionProcessorArmGuid)) {
    //Pcie
    DEBUG ((DEBUG_INFO, "Processor Section!\n"));
    if (Header.ErrorSeverity == EFI_GENERIC_ERROR_CORRECTED) {
      //Ce threshold
      mRasTotalCount.CpuCeCount++;
      if (Strategy->CpuCeReportStrategy.Enable) {
        if (mRasTotalCount.CpuCeCount < Strategy->CpuCeReportStrategy.Count) {
          DEBUG ((DEBUG_INFO, "mRasTotalCount.CpuCeCount : %d\n", mRasTotalCount.CpuCeCount));
          goto FinishReport;
        } else {
          //clear
          DEBUG ((DEBUG_INFO, "Cpu Ce Reach Threshold and clear!\n"));
          mRasTotalCount.CpuCeCount = 0;
        }
      }
    }
  }
  //
  //Send to BMC
  //
  if (Strategy->BmcCperEnable) {
    EfiStatus = IpmiSendCperInfo (Buffer, Length, &Response);
    DEBUG ((DEBUG_INFO, "Ipmi Send CPER Status : %r, Reponse : %x\n", EfiStatus, Response));
  }
  Status = GenerateHestGhes (Buffer, Length, &mApeiList);
FinishReport:
  //
  //Check CPER rules
  //
  if (Status != GenerateGhesSignatureError) {
    //
    //3.Fatal Error
    //
    if (Header.ErrorSeverity == EFI_GENERIC_ERROR_FATAL) {
      FatalErrorHandle (Buffer, Length, Strategy->FatalErrorRecord);
    }
    //
    //4.Memory Isolation
    //
    if (CompareGuid (&Descriptor.SectionType, &gRasErrorSectionPlatformMemoryGuid) ||
          CompareGuid (&Descriptor.SectionType, &gRasErrorSectionPlatformMemory2Guid)) {
      MemoryIsolationHandle (Buffer, Length, Strategy);
    }

    //
    //5.Cpu Memory Isolation
    //
    if (CompareGuid (&Descriptor.SectionType, &gRasErrorSectionPlatformProcessorGuild)) {
      CpuIsolationHandle (Buffer, Length, Strategy);
    }
    //
    //6.Memory Address Isolation
    //
    if (Strategy->MemAddrIsoStrategy.Enable == 1) {
      MemoryAddrIsolationHandle (Buffer, Length, Strategy);
    }
  }

  return Status;
}

/**
  Ras record(memory and cpu) init according different record type.

  @param[in]    Strategy      A pointer to RAS_STRATEGY.
**/
VOID
RasRecordInit (
  IN  RAS_STRATEGY  *Strategy
  )
{
  RAS_MEM_RECORD  MemRecord;
  RAS_CPU_RECORD  CpuRecord;
  UINT64          Address1;
  UINT64          Address2;

  switch (Strategy->MemoryIsoRecord) {
  case RasNoRecord:
    DEBUG ((DEBUG_INFO, "no record\n"));
    goto ProcExit;
    break;
  case RasFlashRecord:
    Address1 = FixedPcdGet64 (PcdRasMemoryRecordFlashAddress);
    if (mFlash == NULL) {
      DEBUG ((DEBUG_ERROR, "mFlash == NULL\n"));
      goto ProcExit;
    }
    if (mFlash->Read (Address1, &MemRecord, sizeof (RAS_MEM_RECORD)) != EFI_SUCCESS) {
      goto ProcExit;
    }
    Address2 = FixedPcdGet64 (PcdRasCpuRecordFlashAddress);
    if (mFlash->Read (Address2, &CpuRecord, sizeof (RAS_CPU_RECORD)) != EFI_SUCCESS) {
      goto ProcExit;
    }
    break;
  default:
    DEBUG ((DEBUG_ERROR, "unsupported record type!\n"));
    goto ProcExit;
    break;
  }
  if (MemRecord.MemFatalUceRecord.Signature != MEMORY_FATAL_UCE_SIGNATURE) {
    ZeroMem (&MemRecord.MemFatalUceRecord, sizeof (RAS_MEM_TYPE_RECORD));
    MemRecord.MemFatalUceRecord.Signature = MEMORY_FATAL_UCE_SIGNATURE;
  }
  if (MemRecord.MemNonFatalUceRecord.Signature != MEMORY_NON_FATAL_UCE_SIGNATURE) {
    ZeroMem (&MemRecord.MemNonFatalUceRecord, sizeof (RAS_MEM_TYPE_RECORD));
    MemRecord.MemNonFatalUceRecord.Signature = MEMORY_NON_FATAL_UCE_SIGNATURE;
  }
  if (MemRecord.MemCeRecord.Signature != MEMORY_CE_SIGNATURE) {
    ZeroMem (&MemRecord.MemCeRecord, sizeof (RAS_MEM_TYPE_RECORD));
    MemRecord.MemCeRecord.Signature = MEMORY_CE_SIGNATURE;
  }
  if (MemRecord.MemAddrIsoRecord.Signature != MEM_ADDR_ISO_RECORD_SIGNATURE) {
    ZeroMem (&MemRecord.MemAddrIsoRecord, sizeof (RAS_MEM_ADDR_ISO_RECORD));
    MemRecord.MemAddrIsoRecord.Signature = MEM_ADDR_ISO_RECORD_SIGNATURE;
  }
  if (CpuRecord.Signature != CPU_FATAL_UCE_SIGNATURE) {
    ZeroMem (&CpuRecord, sizeof (RAS_CPU_RECORD));
    CpuRecord.Signature = CPU_FATAL_UCE_SIGNATURE;
  }
  //
  //clear last ppr record
  //
  ZeroMem (&MemRecord.PprRecord, sizeof (PPR_RECORD));
  //
  //Init Memory record
  //
  if (mFlash->Erase (Address1, SIZE_64KB) != EFI_SUCCESS) {
    goto ProcExit;
  }
  mFlash->Write (Address1, &MemRecord, sizeof (RAS_MEM_RECORD));
  //
  //Init Cpu record
  //
  if (mFlash->Erase (Address2, SIZE_64KB) != EFI_SUCCESS) {
    goto ProcExit;
  }
  mFlash->Write (Address2, &CpuRecord, sizeof (RAS_CPU_RECORD));

ProcExit:
  return ;
}

/**
  Ras sub event handle. The source buffer data comes from the content of the MM
  communicate protocol. For the protocol content, refer to Chapter 6.3.4 of the
  Phytium MM Interface Specification.

  @param [in]      SubId      Sub event ID, refer to Chapter 6.3.4 of the Phytium
                              MM Interface Specification.
  @param [in,out]  Buffer     The content section in MM communicate buffer, refer
                              to Chapter 6.3.4 of the Phytium MM Interface
                              Specification.
  @param [in,out]  Length     Length of content.

  @retval    EFI_SUCCESS      Success.
  @retval    Other            Failed.

**/
UINT32
RasMainFunctionHandle (
  IN       UINT16  SubId,
  IN  OUT  UINT8   *Buffer,
  IN  OUT  UINT32  *Length
  )
{
  UINT32                          Status;

  DEBUG ((DEBUG_INFO, "Check Step Sub ID : %02x\n", SubId));
  Status = -1;

  switch (SubId) {
  //
  //03-01
  //
  case RasUpdateApeiTable:
    ApeiTableInit (&mApeiList);
    Status = UpdateApeiTable (Buffer, *Length, &mApeiList);
    break;
  //
  //03-02
  //
  case RasGenerateGhes:
    DEBUG ((DEBUG_INFO, "CPER Data:\n"));
    Status = CperHandle (Buffer, *Length, &mRasStrategy);
    DEBUG ((DEBUG_INFO, "CPER Handle : %x\n", Status));
    break;
  //
  //03-03
  //
  case RasHestHeaderGet:
    DEBUG ((DEBUG_INFO, " RasHestHeaderGet!\n"));
    Status = GetHestHeaderGet (Buffer);
    break;
  //
  //03-04
  //
  case RasHestTableGet:
    DEBUG ((DEBUG_INFO, "RasHestErrorSourceGet!\n"));
    Status = GetHestErrorSourceStruct (Buffer, *Length - 4);
    DEBUG ((DEBUG_INFO, "Get Hest Error Source Status : %r\n", Status));
    if (Status == EFI_SUCCESS) {
      *Length = mHestContext.HestHeader->Header.Length + 4;
    }
    break;
  //
  //03-05
  //
  case RasBertTableGet:
    DEBUG ((DEBUG_INFO, "RasBertTableGet!\n"));
    Status = GetBertTable (Buffer, *Length - 4);
    if (Status == GetBertTableSuccess) {
      *Length = mBertContext.BertHeader->Header.Length + 4;
      DEBUG ((DEBUG_INFO,
              "mBertContext.BertHeader.Header.Length : 0x%x\n",
              mBertContext.BertHeader->Header.Length
              ));
      GenerateBertGhes (mRasStrategy.FatalErrorRecord, mBertContext.Block, mBertContext.BlockSize);
      FatalErrorRecordClear (mRasStrategy.FatalErrorRecord);
    }
    break;
  //
  //03-07
  //
  case RasEinjHeaderGet:
    DEBUG ((DEBUG_INFO, "RasEinjHeaderGet!\n"));
    Status = GetEinjHeader (Buffer);
    break;
  //
  //03-08
  //
  case RasEinjTableGet:
    DEBUG ((DEBUG_INFO, "RasEinjTableGet!\n"));
    Status = GetEinjTable (Buffer, *Length - 4);
    if (Status == EFI_SUCCESS) {
      *Length = mEinjContext.EINJ->EinjTableHeader.Header.Length + 4;
      DEBUG ((DEBUG_INFO,
              "mEinjContext.EINJ->EinjTableHeader.Header.Length : 0x%x\n",
              mEinjContext.EINJ->EinjTableHeader.Header.Length
             ));
    }
    break;
  //
  //03-09
  //
  case RasTimeStampSync:
    DEBUG ((DEBUG_INFO, "RasTimeStampSync!\n"));
    ZeroMem (&mRasTime, sizeof (RAS_TIME));
    Status = ParseTimeSync (Buffer, &mRasTime);
    break;
  //
  //case 03-0A
  //
  case RasMemoryDeviceLocationSync:
    DEBUG ((DEBUG_INFO, "RasMemoryDeviceLocationSync!\n"));
    ZeroMem (&mMemDeviceLocation, sizeof (SYNC_MEMORY_DEVICE_LOCATION));
    Status = SyncMemoryDeviceLocation (Buffer, *Length, &mMemDeviceLocation);
    break;
  //
  //case 03-0B
  //
  case RasStrategySync:
    CopyMem (&mRasStrategy, Buffer, *Length);
    RasRecordInit (&mRasStrategy);
    PrintRasStrategy (&mRasStrategy);
    if (mFlash != NULL) {
      mFlash->WriteProtectParConfig (
                mRasStrategy.FlashWpConfig.Enable,
                mRasStrategy.FlashWpConfig.BlockProtectBit,
                mRasStrategy.FlashWpConfig.WpDelay
                );
    }
    Status = RasSyncStrategySuccess;
    break;
  default:
    break;
  }
  CopyMem (Buffer, &Status, 4);

  return EFI_SUCCESS;
}
