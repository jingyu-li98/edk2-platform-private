/** @file
  Provides SOPHGO RAS functions.

  Copyright (c) 2025, Sophgo Technologies Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Apei.h"
#include "Bert.h"

APEI_TABLE_LIST  mApeiList;
GHES_REGISTER    *mGhesRegisters;

EFI_GUID  gRasEventNotificationTypeSEA = EFI_EVENT_NOTIFICATION_TYPE_DMAR_SEA;
EFI_GUID  gRasEventNotificationTypeSEI = EFI_EVENT_NOTIFICATION_TYPE_DMAR_SEI;
EFI_GUID  gRasErrorSectionPlatformMemoryGuid = EFI_ERROR_SECTION_PLATFORM_MEMORY_GUID;
EFI_GUID  gRasErrorSectionPlatformMemory2Guid = EFI_ERROR_SECTION_PLATFORM_MEMORY2_GUID;
EFI_GUID  gRasErrorSectionPlatformProcessorGuild = EFI_ERROR_SECTION_PROCESSOR_SPECIFIC_ARM_GUID;
EFI_GUID  gRasErrorSectionPcieGuid = EFI_ERROR_SECTION_PCIE_GUID;
EFI_GUID  gRasErrorSectionPciPcixBusGuid = EFI_ERROR_SECTION_PCI_PCIX_BUS_GUID;
EFI_GUID  gRasErrorSectionPciDeviceGuid = EFI_ERROR_SECTION_PCI_DEVICE_GUID;
EFI_GUID  gRasErrorSectionProcessorArmGuid = EFI_ERROR_SECTION_PROCESSOR_SPECIFIC_ARM_GUID;

/**
  Initialize APEI Table List.

  @param[in,out] List  A pointer to APEI_TABLE_LIST.
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

/**
  Initialize BERT table and integrate with APEI system.

  @retval EFI_SUCCESS           BERT table initialized successfully
  @retval EFI_OUT_OF_RESOURCES  Failed to allocate memory
  @retval Others                Other errors during initialization
**/
EFI_STATUS
InitBertTable (
  VOID
  )
{
  EFI_STATUS  Status;

  //
  // Initialize BERT table
  //
  Status = BertInitTable ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Failed to initialize BERT table - %r\n", __func__, Status));
    return Status;
  }

  //
  // Update APEI table with BERT information
  //
  mApeiList.BertTable[mApeiList.BertNum].Base = (UINT64)(UINTN)mBertContext.BertHeader;
  mApeiList.BertTable[mApeiList.BertNum].Size = sizeof (EFI_ACPI_6_5_BOOT_ERROR_RECORD_TABLE_HEADER);
  mApeiList.BertTable[mApeiList.BertNum].ErrorSourceNum = 0;
  mApeiList.BertTable[mApeiList.BertNum].Type = TypeBert;
  mApeiList.BertNum++;

  return EFI_SUCCESS;
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
