/** @file
  Describe generic hardware error source and generic error data handling.

  Copyright (C) 2025, SOPHGO Technologies Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include "Ras.h"

HEST_CONTEXT  mHestContext;
RAS_TIME      mRasTime;
SYNC_MEMORY_DEVICE_LOCATION  mMemDeviceLocation;
RAS_STRATEGY                 mRasStrategy;

/**
  Print EFI_TIME.

  @param[in]  Time  A pointer to EFI_TIME.
**/
VOID
PrintEfiTime (
  IN EFI_TIME  *Time
  )
{
#ifdef RAS_DEBUG
  DEBUG ((DEBUG_INFO, "EFI Time:\n"));
  DEBUG ((DEBUG_INFO, "Year: %d\n", Time->Year));
  DEBUG ((DEBUG_INFO, "Month: %d\n", Time->Month));
  DEBUG ((DEBUG_INFO, "Day: %d\n", Time->Day));
  DEBUG ((DEBUG_INFO, "Hour: %d\n", Time->Hour));
  DEBUG ((DEBUG_INFO, "Minute: %d\n", Time->Minute));
  DEBUG ((DEBUG_INFO, "Second: %d\n", Time->Second));
#endif
}

/**
  Binary conversion to BCD code.

  @param[in]  Value  Binary code to convert.

  @retval     BCD code.
**/
UINT8
BinToBcd (
  IN UINT8  Value
  )
{
  return (((Value / 10) << 4) | (Value % 10));
}

/**
  Error block content initialization. Specify the severity of this block.

  @param[in]  Block     A pointer to error block.
  @param[in]  Severity  Severity of this block.

  @retval     BlockHeader  A pointer to error block.
**/
EFI_ACPI_6_5_GENERIC_ERROR_STATUS_STRUCTURE *
ErrorBlockInitial (
  IN VOID    *Block,
  IN UINT32  Severity
  )
{
  EFI_ACPI_6_5_GENERIC_ERROR_STATUS_STRUCTURE  *BlockHeader;

  BlockHeader = Block;
  ZeroMem (&BlockHeader->BlockStatus, sizeof (EFI_ACPI_6_5_ERROR_BLOCK_STATUS));
  BlockHeader->RawDataOffset = 0;
  BlockHeader->RawDataLength = 0;
  BlockHeader->DataLength = 0;
  BlockHeader->ErrorSeverity = Severity;

  return BlockHeader;
}

/**
  Update the error block header content. If the current ErrorDataEntryCount is
  0, the error block is reset. If the current ErrorDataEntryCount it not 0, it
  means that the error data is the content of the same error block, and the
  error block header is not processed.

  @param[in,out]    ErrorBlock     A pointer to error block.
  @param[in]        Severity       Severity of this block.

  @retval    TRUE    Update successfully.
  @retval    FALSE   Update failed.

**/
BOOLEAN
ErrorBlockUpdateStatusStructure (
  IN OUT VOID *ErrorBlock,
  IN     UINT32  Severity
  )
{
  EFI_ACPI_6_5_GENERIC_ERROR_STATUS_STRUCTURE *BlockHeader;
  VOID                                        *EntriesBegin;

  if (ErrorBlock == NULL) {
    return FALSE;
  }

  BlockHeader =  ErrorBlock;
  EntriesBegin = ErrorBlock + sizeof (EFI_ACPI_6_5_GENERIC_ERROR_STATUS_STRUCTURE);
  if (BlockHeader->BlockStatus.ErrorDataEntryCount == 0) {
    SetMem (EntriesBegin, BlockHeader->DataLength, 0);
    BlockHeader->RawDataLength = 0;
    BlockHeader->RawDataOffset = 0;
    BlockHeader->DataLength = 0;
  }
  BlockHeader->ErrorSeverity = Severity;

  return TRUE;
}

/**
  Add new error data content to errro block. The data to add is from CPER.

  @param[in]    ErrorBlock              A pointer to error block.
  @param[in]    MaxBlockLength          The maximum length allowed for error data.
  @param[in]    Header                  A pointer to EFI_COMMON_ERROR_RECORD_HEADER.
  @param[in]    Descriptor              A pointer to EFI_ERROR_SECTION_DESCRIPTOR.
  @param[in]    GenericErrorData        Generic error data to add.
  @param[in]    SizeOfGenericErrorData  The length of generic error data.
  @param[in]    Correctable             1 - Correctable
                                        0 - Uncorrectable

  @retval       TRUE    Success.
  @retval       FALSE   Failed.

 **/
BOOLEAN
ErrorBlockAddErrorData (
  IN VOID                            *ErrorBlock,
  IN UINT32                          MaxBlockLength,
  IN EFI_COMMON_ERROR_RECORD_HEADER  *Header,
  IN EFI_ERROR_SECTION_DESCRIPTOR    *Descriptor,
  IN VOID                            *GenericErrorData,
  IN UINT32                          SizeOfGenericErrorData,
  IN BOOLEAN                         Correctable
  )
{
  EFI_ACPI_6_5_GENERIC_ERROR_DATA_ENTRY_STRUCTURE   *Entry;
  EFI_ACPI_6_5_GENERIC_ERROR_STATUS_STRUCTURE       *BlockHeader;
  EFI_ACPI_6_5_ERROR_BLOCK_STATUS                   *BlockStatus;
  UINT32                                            ExpectedNewDataLength;
  VOID                                              *GenericErrorDataFollowEntry;

  if (ErrorBlock == NULL || GenericErrorData == NULL || Header == NULL || Descriptor == NULL) {
    DEBUG ((DEBUG_ERROR, "[%a]:[%dL]Invalid Param \n", __func__, __LINE__));
    return FALSE;
  }

  BlockHeader = ErrorBlock;
  BlockStatus = &BlockHeader->BlockStatus;
  ErrorBlockUpdateStatusStructure (ErrorBlock,  Descriptor->Severity);
  ExpectedNewDataLength = BlockHeader->DataLength +
                          sizeof (EFI_ACPI_6_5_GENERIC_ERROR_DATA_ENTRY_STRUCTURE) +
                          SizeOfGenericErrorData;
  if (sizeof (EFI_ACPI_6_5_GENERIC_ERROR_STATUS_STRUCTURE) + ExpectedNewDataLength >
      MaxBlockLength) {
    DEBUG ((DEBUG_ERROR, "[%a]:[%dL]Out of BlockSize \n", __func__, __LINE__));
    return FALSE;
  }
  if (Correctable == 1) {
    if (BlockStatus->CorrectableErrorValid == 0) {
      BlockStatus->CorrectableErrorValid = 1;
    } else {
      BlockStatus->MultipleCorrectableErrors = 1;
    }
  } else {
    if (BlockStatus->UncorrectableErrorValid == 0) {
      BlockStatus->UncorrectableErrorValid = 1;
    } else {
      BlockStatus->MultipleUncorrectableErrors = 1;
    }
  }
  BlockStatus->ErrorDataEntryCount ++;

  //
  // Generate Error Data Entry
  //
  Entry = (EFI_ACPI_6_5_GENERIC_ERROR_DATA_ENTRY_STRUCTURE*)(ErrorBlock +
           sizeof (EFI_ACPI_6_5_GENERIC_ERROR_STATUS_STRUCTURE) +
           BlockHeader->DataLength);
  SetMem (Entry, sizeof (EFI_ACPI_6_5_GENERIC_ERROR_DATA_ENTRY_STRUCTURE), 0);
  CopyMem (&Entry->SectionType, &Descriptor->SectionType, sizeof (EFI_GUID));
  Entry->ErrorSeverity = Descriptor->Severity;
  Entry->Revision = EFI_ACPI_6_4_GENERIC_ERROR_DATA_ENTRY_REVISION;
  Entry->ValidationBits |= BIT2;
  Entry->Flags = Descriptor->SectionFlags;
  Entry->ErrorDataLength = SizeOfGenericErrorData;
  CopyMem (&Entry->FruId, &Descriptor->FruId, sizeof (EFI_GUID));
  CopyMem (&Entry->FruText, Descriptor->FruString, 20);
  CopyMem (Entry->Timestamp, &Header->TimeStamp, 8);
#ifdef RAS_DEBUG
  DEBUG ((DEBUG_INFO, "Ghes TimeStamp[0] : 0x%02x\n", Entry->Timestamp[0]));
  DEBUG ((DEBUG_INFO, "Ghes TimeStamp[1] : 0x%02x\n", Entry->Timestamp[1]));
  DEBUG ((DEBUG_INFO, "Ghes TimeStamp[2] : 0x%02x\n", Entry->Timestamp[2]));
  DEBUG ((DEBUG_INFO, "Ghes TimeStamp[3] : 0x%02x\n", Entry->Timestamp[3]));
  DEBUG ((DEBUG_INFO, "Ghes TimeStamp[4] : 0x%02x\n", Entry->Timestamp[4]));
  DEBUG ((DEBUG_INFO, "Ghes TimeStamp[5] : 0x%02x\n", Entry->Timestamp[5]));
  DEBUG ((DEBUG_INFO, "Ghes TimeStamp[6] : 0x%02x\n", Entry->Timestamp[6]));
  DEBUG ((DEBUG_INFO, "Ghes TimeStamp[7] : 0x%02x\n", Entry->Timestamp[7]));
#endif
  GenericErrorDataFollowEntry = (VOID*)Entry +
           sizeof (EFI_ACPI_6_4_GENERIC_ERROR_DATA_ENTRY_STRUCTURE);
  CopyMem (
       GenericErrorDataFollowEntry,
       GenericErrorData,
       SizeOfGenericErrorData
       );
  //
  // BlockHeader
  //
  BlockHeader->RawDataOffset = 0;
  BlockHeader->RawDataLength = 0;
  BlockHeader->DataLength = ExpectedNewDataLength;

  return TRUE;
}

/**
  Create HEST table header.

  @param[in, out]  Context               A pointer of HEST_CONTEXT.
  @param[in]       PreAllocatedHestSize  HEST table pre-allocate space size.

  @retval    EFI_SUCCESS             Success.
  @retval    EFI_BUFFER_TOO_SMALL    PreAllocatedHestSize is too small.

**/
EFI_STATUS
HestHeaderCreator (
  IN OUT HEST_CONTEXT  *Context,
  IN     UINT32        PreAllocatedHestSize
  )
{
  if (PreAllocatedHestSize < sizeof (EFI_ACPI_6_5_HARDWARE_ERROR_SOURCE_TABLE_HEADER)) {
    return EFI_BUFFER_TOO_SMALL;
  }

  Context->HestHeader = AllocatePool (PreAllocatedHestSize);
  if (Context->HestHeader == NULL) {
    DEBUG ((DEBUG_ERROR, "HestHeader allocate failed\n"));
  }

  ZeroMem (Context->HestHeader, PreAllocatedHestSize);
  *Context->HestHeader = (EFI_ACPI_6_5_HARDWARE_ERROR_SOURCE_TABLE_HEADER) {
   ARM_ACPI_HEADER (
      EFI_ACPI_6_5_HARDWARE_ERROR_SOURCE_TABLE_SIGNATURE,
      EFI_ACPI_6_5_HARDWARE_ERROR_SOURCE_TABLE_HEADER,
      EFI_ACPI_6_5_HARDWARE_ERROR_SOURCE_TABLE_REVISION
    ),
    0x0
  };

  Context->OccupiedMemorySize = PreAllocatedHestSize;

  return EFI_SUCCESS;
}

/**
  Ghes struct initialization.

  @param[in, out]  GhesV2         GhesV2 struct to init.
  @param[in]       BlockLength    Error status block length.

**/
VOID
GhesV2Initial (
  IN OUT EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE *GhesV2,
  IN     UINT32                                                         BlockLength
  )
{
  if (GhesV2 == NULL) {
    return;
  }

  *GhesV2 = (EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE) {
    .Type = EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_VERSION_2,
    .SourceId = 0,
    .RelatedSourceId = 0xFFFF,
    .Flags = 0,
    .Enabled = 1,
    .NumberOfRecordsToPreAllocate = 1,//ERROR BLOCK
    .MaxSectionsPerRecord = 1,// Num Entries(section)
    .MaxRawDataLength = BlockLength, // Max Size Of a Raw Data
    .ErrorStatusAddress = {
      .AddressSpaceId = EFI_ACPI_6_5_SYSTEM_MEMORY,
      .RegisterBitWidth = 64,
      .RegisterBitOffset = 0,
      .AccessSize = EFI_ACPI_6_5_QWORD,
      .Address = 0
    },
    .NotificationStructure = {
      .Type = EFI_ACPI_6_5_HARDWARE_ERROR_NOTIFICATION_GSIV,
      .Length = sizeof (EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE),
      .ConfigurationWriteEnable = {0, 0, 0, 0, 0, 0, 0} ,
      .PollInterval = 0,
      .Vector = 0,
      .SwitchToPollingThresholdValue = 0,
      .SwitchToPollingThresholdWindow = 0,
      .ErrorThresholdValue = 0,
      .ErrorThresholdWindow = 0
    },
    .ErrorStatusBlockLength = BlockLength,
    .ReadAckRegister = {
      .AddressSpaceId = EFI_ACPI_6_5_SYSTEM_MEMORY,
      .RegisterBitWidth = 64,
      .RegisterBitOffset = 0,
      .AccessSize = EFI_ACPI_6_5_QWORD,
      .Address = 0
    },
    .ReadAckPreserve = READ_ACK_PRESERVE,
    .ReadAckWrite = READ_ACK_WRITE
  };
  return;
}

/**
  Add notification struct for GHES.

  @param[in]    This      GHES struct.
  @param[in]    Type      Notification type, must be GSIV.
  @param[in]    Vertor    Interrupt vector.

 **/
VOID
GhesV2AddNotification (
  IN EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE  *This,
  IN UINT8                                                           Type,
  IN UINT32                                                          Vector
  )
{
  This->NotificationStructure = (EFI_ACPI_6_5_HARDWARE_ERROR_NOTIFICATION_STRUCTURE) {
    .Type = Type,
    .Length = sizeof (EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE),
    .ConfigurationWriteEnable = {
      .Type = 0,
      .PollInterval = 1,
      .SwitchToPollingThresholdValue = 1,
      .SwitchToPollingThresholdWindow = 1,
      .ErrorThresholdValue = 1,
      .ErrorThresholdWindow = 1
    },
    .PollInterval = 20,
    .Vector = Vector + 1,
    .SwitchToPollingThresholdValue = 0,
    .SwitchToPollingThresholdWindow = 0,
    .ErrorThresholdValue = 0,
    .ErrorThresholdWindow = 0
  };

  return;
}

/**
  Link GHES address and ack register to error block.

  @param[in]  GhesV2         GhesV2 struct to init.
  @param[in]  Register       A pointer to GHES_REGISTER.
  @param[in]  ErrorBlock     Error block to fill.

  @retval     EFI_SUCCESS           Success;
  @retval     EFI_INVALID_PARAMTER  Either of the input parameter is NULL.

**/
EFI_STATUS
GhesV2LinkErrorBlock (
  IN   EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE *GhesV2,
  IN   GHES_REGISTER                                                  *Register,
  OUT  VOID                                                           *ErrorBlock
  )
{
  if (ErrorBlock == NULL || Register == NULL || GhesV2 == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Register->ErrorStatusBlockAddress = (UINTN)ErrorBlock;
  GhesV2->ErrorStatusAddress.Address = (UINTN)&(Register->ErrorStatusBlockAddress);
  Register->AckRegister = READ_ACK_WRITE;
  GhesV2->ReadAckRegister.Address = (UINT64)&(Register->AckRegister);

  return EFI_SUCCESS;
}

/**
  Fill all GHES tables in the HEST table.

  @param[in]  GhesV2[MAX_GHES]    A point to GhesV2 tables.
  @param[in]  NumOfGhesV2         Number of GHES tables.

  @retval     EFI_SUCCESS            Success.
  @retval     EFI_INVALID_PARAMETER  GHES is NULL or NumOfGhesV2 is not equal to MAX_GHES.
**/
EFI_STATUS
GhesV2ContextForHest (
  IN EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE  GhesV2[MAX_GHES],
  IN UINT8                                                           NumOfGhesV2
  )
{
  UINT8           NumOfBlockPerGhes;
  UINT8           Iter;
  UINT32          BlockMemorySize;
  UINT32          ErrorSeverityArray[MAX_GHES] = {
                    PCIE_AER_ERROR_SEVERITY,    // PCIe AER
                    DDR_ECC_ERROR_SEVERITY      // DDR ECC
                  };
  VOID            *ErrorBlockHead;
  VOID            *ErrorBlock;
  VOID            *BlockMemory;
  GHES_REGISTER   *GhesRegisters;

  // Ensure the size is expected
  if ((GhesV2 == NULL) || (NumOfGhesV2 != MAX_GHES)) {
    return EFI_INVALID_PARAMETER;
  }

  NumOfBlockPerGhes = 1;
  Iter = 0;

  BlockMemorySize = MAX_GHES *
                    (sizeof (GHES_REGISTER) + NumOfBlockPerGhes * GENERIC_HARDWARE_ERROR_BLOCK_SIZE);
  BlockMemory = (VOID *)((UINT64)(PcdGet64 (PcdRasMemoryBase) + PcdGet64 (PcdHestRegionOffset)));
  DEBUG ((DEBUG_INFO, "Block Memory: %p\n", BlockMemory));
  ZeroMem (BlockMemory, BlockMemorySize);
  GhesRegisters = BlockMemory;
  ErrorBlockHead = BlockMemory + MAX_GHES * sizeof (GHES_REGISTER);
  ErrorBlock = ErrorBlockHead;

  // PCIe AER
  GhesV2Initial (&GhesV2[Iter], PCIE_AER_ERROR_BLOCK_SIZE);
  GhesV2[Iter].Type = EFI_ACPI_6_5_PCI_EXPRESS_ROOT_PORT_AER_STRUCTURE;
  GhesV2[Iter].SourceId = PCIE_AER_ERROR_SOURCE_ID;
  GhesV2[Iter].Flags = PCIE_AER_ERROR_FLAGS;
  GhesV2[Iter].Enabled = PCIE_AER_ERROR_ENABLED;
  GhesV2[Iter].NumberOfRecordsToPreAllocate = 1;
  GhesV2[Iter].MaxSectionsPerRecord = 1;
  GhesV2[Iter].Bus = PCIE_AER_ERROR_BUS;
  GhesV2[Iter].Device = PCIE_AER_ERROR_DEVICE;
  GhesV2[Iter].Function = PCIE_AER_ERROR_FUNCTION;
  GhesV2[Iter].DeviceControl = PCIE_AER_ERROR_DEVICE_CONTROL;
  GhesV2[Iter].UncorrectableErrorMask = PCIE_AER_ERROR_UNCORRECTABLE_MASK;
  GhesV2[Iter].UncorrectableErrorSeverity = PCIE_AER_ERROR_UNCORRECTABLE_SEVERITY;
  GhesV2[Iter].CorrectableErrorMask = PCIE_AER_ERROR_CORRECTABLE_MASK;
  GhesV2[Iter].AdvancedErrorCapabilitiesAndControl = PCIE_AER_ERROR_ADVANCED_CAP;
  GhesV2[Iter].RootErrorCommand = 0;  // No specific root error command

  GhesV2AddNotification (
    &GhesV2[Iter],
    EFI_ACPI_6_5_HARDWARE_ERROR_NOTIFICATION_PCI_EXPRESS,
    Iter
    );
  ErrorBlockInitial (ErrorBlock, ErrorSeverityArray[Iter]);
  GhesV2LinkErrorBlock (&GhesV2[Iter], &GhesRegisters[Iter], ErrorBlock);
  ErrorBlock += GhesV2[Iter].ErrorStatusBlockLength;

  // DDR ECC
  Iter++;
  GhesV2Initial (&GhesV2[Iter], DDR_ECC_ERROR_BLOCK_SIZE);
  GhesV2[Iter].Type = EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE;
  GhesV2[Iter].SourceId = DDR_ECC_ERROR_SOURCE_ID;
  GhesV2[Iter].RelatedSourceId = 0;  // No related source
  GhesV2[Iter].Flags = DDR_ECC_ERROR_FLAGS;
  GhesV2[Iter].Enabled = DDR_ECC_ERROR_ENABLED;
  GhesV2[Iter].NumberOfRecordsToPreAllocate = 1;
  GhesV2[Iter].MaxSectionsPerRecord = 1;
  GhesV2[Iter].MaxRawDataLength = DDR_ECC_ERROR_MAX_RAW_DATA_LENGTH;
  GhesV2[Iter].ErrorStatusBlockLength = DDR_ECC_ERROR_BLOCK_SIZE;

  GhesV2AddNotification (
    &GhesV2[Iter],
    EFI_ACPI_6_5_HARDWARE_ERROR_NOTIFICATION_MEMORY,
    Iter
    );
  ErrorBlockInitial (ErrorBlock, ErrorSeverityArray[Iter]);
  GhesV2LinkErrorBlock (&GhesV2[Iter], &GhesRegisters[Iter], ErrorBlock);
  ErrorBlock += GhesV2[Iter].ErrorStatusBlockLength;

  return EFI_SUCCESS;
}

/**
  Add error source descriptor to HEST table.

  @param[in,out]   Context                A pointer to HEST_CONTEXT.
  @param[in]       ErrorSourceDescriptor  A pointer to GHES struct.
  @param[in]       SizeOfDescriptor       Size of GHES.

  @retval          EFI_SUCCESS            Success.
  @retval          EFI_INVALID_PARAMETER  Context or ErrorSourceDescriptor is NULL.

 **/
EFI_STATUS
HestAddErrorSourceDescriptor (
  IN OUT HEST_CONTEXT  *Context,
  IN     VOID          *ErrorSourceDescriptor,
  IN     UINT32        SizeOfDescriptor
  )
{
  UINT16                                          *pSourceId;
  VOID                                            *Descriptor;
  EFI_ACPI_6_5_HARDWARE_ERROR_SOURCE_TABLE_HEADER *HestHeader;

  if ((Context == NULL) || (ErrorSourceDescriptor == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  HestHeader = Context->HestHeader;
  DEBUG ((DEBUG_INFO, "HestHeader->Header.Length : 0x%x\n", HestHeader->Header.Length));
  DEBUG ((DEBUG_INFO, "SizeOfDescriptor : 0x%x\n", SizeOfDescriptor));
  DEBUG ((DEBUG_INFO, "Context->OccupiedMemorySize : 0x%x\n", Context->OccupiedMemorySize));
  if (HestHeader->Header.Length + SizeOfDescriptor > Context->OccupiedMemorySize) {
    DEBUG ((DEBUG_ERROR, "[%a]:[%dL]: Hest Size Too small\n", __func__, __LINE__));
    return EFI_BUFFER_TOO_SMALL;
  }

  Descriptor = (UINT8*)HestHeader + HestHeader->Header.Length;
  CopyMem ((VOID*)Descriptor , ErrorSourceDescriptor, SizeOfDescriptor);
  pSourceId = Descriptor + sizeof (UINT16);
  *pSourceId = HestHeader->ErrorSourceCount;
  HestHeader->Header.Length += SizeOfDescriptor;
  HestHeader->ErrorSourceCount++;

  return EFI_SUCCESS;
}
