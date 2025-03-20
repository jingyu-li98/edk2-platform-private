/** @file
  HEST (Hardware Error Source Table) implementation for SOPHGO SG2044 platform.

  This file implements HEST table creation and error source handling according to
  ACPI 6.5 specification, including:
  - PCIe Root Complex AER error sources
  - LPDDR5x inline ECC error source

  Copyright (c) 2025, Sophgo Technologies Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Hest.h"

PCIE_RC_CONFIG  mPcieRcConfig[PCIE_MAX_ROOT_COMPLEXES];
UINTN           mPcieRcCount = 0;
HEST_CONTEXT    mHestContext;

#define DDR_ECC_ERROR_SOURCE_ID            0x0001
#define DDR_ECC_ERROR_ACK_REGISTER         0x12345678  // 实际地址需要根据硬件定义
#define DDR_ECC_ERROR_ACK_PRESERVE         0xFFFFFFFF
#define DDR_ECC_ERROR_ACK_WRITE            0x00000001
#define GENERIC_HARDWARE_ERROR_BLOCK_SIZE  0x1000

/**
  Parse PCIe Root Complex configuration from conf.ini.

  This function reads PCIe configuration from conf.ini to support different PCIe topologies.
  Each RC section contains:
  - width: Number of lanes per port (4 or 8)
  - Memory and IO window configurations

  If no PCIe sections found in conf.ini, default to 5 ports.

  @retval EFI_SUCCESS          Configuration parsed successfully
  @retval EFI_OUT_OF_RESOURCES Too many RCs configured
  @retval EFI_INVALID_PARAMETER Invalid configuration parameters
**/
STATIC
EFI_STATUS
ParsePcieRcConfig (
  VOID
  )
{
  CHAR8       Value[64];
  UINT8       RcId;
  EFI_STATUS  Status;
  BOOLEAN     FoundPcieSections;

  //
  // Initialize RC count
  //
  mPcieRcCount = 0;
  FoundPcieSections = FALSE;

  //
  // Parse conf.ini
  //
  Status = IniConfIniParse (NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to parse conf.ini: %r\n", Status));
    return Status;
  }

  //
  // Try to parse each possible RC section
  //
  for (RcId = 0; RcId < PCIE_MAX_ROOT_COMPLEXES; RcId++) {
    CHAR8 Section[16];

    AsciiSPrint (Section, sizeof (Section), "pcie%d", RcId);

    //
    // Check if this RC exists in conf.ini
    //
    if (IniGetValueBySectionAndName (Section, "width", Value) == 0) {
      UINT8 Width;

      FoundPcieSections = TRUE;

      Width = (UINT8)AsciiStrDecimalToUintn (Value);
      if (Width != 4 && Width != 8) {
        DEBUG ((DEBUG_ERROR, "Invalid width %d for RC%d\n", Width, RcId));
        continue;
      }

      if (mPcieRcCount >= PCIE_MAX_ROOT_COMPLEXES) {
        DEBUG ((DEBUG_ERROR, "Too many RCs configured\n"));
        return EFI_OUT_OF_RESOURCES;
      }

      //
      // Configure this RC
      //
      mPcieRcConfig[mPcieRcCount].RcId = RcId;
      mPcieRcConfig[mPcieRcCount].Enabled = TRUE;
      mPcieRcConfig[mPcieRcCount].LanesPerPort = Width;
      mPcieRcConfig[mPcieRcCount].PortCount = 1;  // Each RC has 1 port

      DEBUG ((DEBUG_INFO,
              "RC%d: %d port, %d lanes/port\n",
              RcId,
              mPcieRcConfig[mPcieRcCount].PortCount,
              mPcieRcConfig[mPcieRcCount].LanesPerPort));

      mPcieRcCount++;
    }
  }

  //
  // If no PCIe sections found in conf.ini, use default configuration
  //
  if (!FoundPcieSections) {
    DEBUG ((DEBUG_INFO, "No PCIe sections found in conf.ini, using default 5 ports\n"));
    
    for (RcId = 0; RcId < 5; RcId++) {
      mPcieRcConfig[RcId].RcId = RcId;
      mPcieRcConfig[RcId].Enabled = TRUE;
      mPcieRcConfig[RcId].LanesPerPort = 8;  // Default to x8
      mPcieRcConfig[RcId].PortCount = 1;     // Each RC has 1 port

      DEBUG ((DEBUG_INFO,
              "Default RC%d: %d port, %d lanes/port\n",
              RcId,
              mPcieRcConfig[RcId].PortCount,
              mPcieRcConfig[RcId].LanesPerPort));
    }
    mPcieRcCount = 5;
  }

  return EFI_SUCCESS;
}

/**
  Initialize PCIe AER error source structure.

  @param[out] PcieAer      Pointer to PCIe AER structure to initialize
  @param[in]  RcConfig     Pointer to RC configuration
  @param[in]  ErrorBlock   Pointer to error status block
  @param[in]  BlockSize    Size of error status block

  @retval EFI_SUCCESS           Initialization successful
  @retval EFI_INVALID_PARAMETER Invalid parameter
**/
STATIC
EFI_STATUS
InitializePcieAerErrorSource (
  OUT EFI_ACPI_6_5_PCI_EXPRESS_ROOT_PORT_AER_STRUCTURE  *PcieAer,
  IN  PCIE_RC_CONFIG                                    *RcConfig,
  IN  VOID                                              *ErrorBlock,
  IN  UINTN                                             BlockSize
  )
{
  if (PcieAer == NULL || RcConfig == NULL || ErrorBlock == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Initialize base fields
  //
  PcieAer->Type = EFI_ACPI_6_5_PCI_EXPRESS_ROOT_PORT_AER;
  PcieAer->SourceId = RcConfig->RcId;
  ZeroMem (PcieAer->Reserved0, sizeof (PcieAer->Reserved0));
  PcieAer->Flags = EFI_ACPI_6_5_ERROR_SOURCE_FLAG_FIRMWARE_FIRST;
  PcieAer->Enabled = TRUE;

  //
  // Configure error record parameters
  //
  PcieAer->NumberOfRecordsToPreAllocate = 1;
  PcieAer->MaxSectionsPerRecord = 1;

  //
  // Set PCI location
  //
  PcieAer->Bus = 0;  // RC is always on bus 0
  PcieAer->Device = RcConfig->RcId;
  PcieAer->Function = 0;

  //
  // Configure device control and reserved fields
  //
  PcieAer->DeviceControl = 0;  // Use default device control settings
  ZeroMem (PcieAer->Reserved1, sizeof (PcieAer->Reserved1));

  //
  // Configure PCIe AER error masks
  //
  PcieAer->UncorrectableErrorMask = PCIE_AER_UNCORRECTABLE_MASK;
  PcieAer->UncorrectableErrorSeverity = PCIE_AER_UNCORRECTABLE_MASK;  // All uncorrectable errors are fatal
  PcieAer->CorrectableErrorMask = PCIE_AER_CORRECTABLE_MASK;

  //
  // Configure advanced error capabilities and control
  //
  PcieAer->AdvancedErrorCapabilitiesAndControl = 0;  // Use default AER settings
  PcieAer->RootErrorCommand = 0;  // Use default error command settings

  DEBUG ((DEBUG_INFO, "%a: PCIe AER source initialized for RC%d\n", __func__, RcConfig->RcId));
  return EFI_SUCCESS;
}

/**
  Initialize DDR ECC error source configuration.
  
  This function configures the Generic Hardware Error Source (GHES) for DDR ECC errors.
  It sets up notification type, polling interval, error status block parameters and
  error record configurations according to ACPI 6.5 specification.

  @param[out] GhesV2  Pointer to GHES v2 structure to be initialized
  
  @retval EFI_SUCCESS           DDR ECC error source initialized successfully
  @retval EFI_INVALID_PARAMETER GhesV2 is NULL
**/
STATIC
EFI_STATUS
InitializeDdrEccErrorSource (
  OUT EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE  *GhesV2
  )
{
  if (GhesV2 == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Configure basic GHES v2 fields
  //
  GhesV2->Type = EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_VERSION_2;
  GhesV2->SourceId = DDR_ECC_ERROR_SOURCE_ID;
  GhesV2->RelatedSourceId = 0xFFFF;  // No related source
  GhesV2->Flags = EFI_ACPI_6_5_ERROR_SOURCE_FLAG_FIRMWARE_FIRST;
  GhesV2->Enabled = TRUE;

  //
  // Configure error record parameters
  //
  GhesV2->NumberOfRecordsToPreAllocate = 1;
  GhesV2->MaxSectionsPerRecord = 1;
  GhesV2->MaxRawDataLength = sizeof (EFI_ERROR_SECTION_DESCRIPTOR) +
                            sizeof (EFI_MEMORY_ERROR_SECTION);

  //
  // Configure error status parameters
  //
  GhesV2->ErrorStatusAddress.AddressSpaceId = EFI_ACPI_6_5_SYSTEM_MEMORY;
  GhesV2->ErrorStatusAddress.RegisterBitWidth = 64;
  GhesV2->ErrorStatusAddress.RegisterBitOffset = 0;
  GhesV2->ErrorStatusAddress.AccessSize = EFI_ACPI_6_5_QWORD;
  GhesV2->ErrorStatusAddress.Address = (UINT64)(UINTN)AllocateZeroPool (
                                               GENERIC_HARDWARE_ERROR_BLOCK_SIZE
                                               );
  if (GhesV2->ErrorStatusAddress.Address == 0) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Configure notification
  //
  GhesV2->NotificationStructure.Type = EFI_ACPI_6_5_HARDWARE_ERROR_NOTIFICATION_POLLED;
  GhesV2->NotificationStructure.PollInterval = 1000;  // 1 second polling interval
  GhesV2->NotificationStructure.Vector = 0;
  GhesV2->NotificationStructure.SwitchToPollingThresholdValue = 0;
  GhesV2->NotificationStructure.SwitchToPollingThresholdWindow = 0;
  GhesV2->NotificationStructure.ErrorThresholdValue = 0;
  GhesV2->NotificationStructure.ErrorThresholdWindow = 0;

  //
  // Configure error status block
  //
  GhesV2->ErrorStatusBlockLength = GENERIC_HARDWARE_ERROR_BLOCK_SIZE;

  //
  // Configure read-ack register
  //
  GhesV2->ReadAckRegister.AddressSpaceId = EFI_ACPI_6_5_SYSTEM_MEMORY;
  GhesV2->ReadAckRegister.RegisterBitWidth = 64;
  GhesV2->ReadAckRegister.RegisterBitOffset = 0;
  GhesV2->ReadAckRegister.AccessSize = EFI_ACPI_6_5_QWORD;
  GhesV2->ReadAckRegister.Address = DDR_ECC_ERROR_ACK_REGISTER;
  GhesV2->ReadAckPreserve = DDR_ECC_ERROR_ACK_PRESERVE;
  GhesV2->ReadAckWrite = DDR_ECC_ERROR_ACK_WRITE;

  DEBUG ((DEBUG_INFO, "%a: DDR ECC error source initialized at 0x%lx\n",
          __func__, GhesV2->ErrorStatusAddress.Address));

  return EFI_SUCCESS;
}

/**
  Create GHES context for HEST table.

  This function creates Generic Hardware Error Source (GHES) structures for:
  1. PCIe Root Complex AER error sources
  2. LPDDR5x inline ECC error source

  @param[out] GhesV2       Array of GHES V2 structures
  @param[in]  NumOfGhesV2  Number of GHES structures to create

  @retval EFI_SUCCESS           GHES context created successfully
  @retval EFI_OUT_OF_RESOURCES  Failed to allocate memory
  @retval Others                Other errors during initialization
**/
EFI_STATUS
GhesV2ContextForHest (
  OUT EFI_ACPI_6_5_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE  GhesV2[],
  IN  UINT8                                                           NumOfGhesV2
  )
{
  EFI_STATUS      Status;
  UINT8           Index;
  VOID            *ErrorBlock;
  VOID            *CurrentBlock;
  GHES_REGISTER   *GhesRegisters;

  //
  // Parse PCIe configuration first
  //
  Status = ParsePcieRcConfig ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Allocate error blocks
  //
  ErrorBlock = AllocateZeroPool (NumOfGhesV2 * GENERIC_HARDWARE_ERROR_BLOCK_SIZE);
  if (ErrorBlock == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  GhesRegisters = AllocateZeroPool (NumOfGhesV2 * sizeof (GHES_REGISTER));
  if (GhesRegisters == NULL) {
    FreePool (ErrorBlock);
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Create PCIe AER error sources
  //
  CurrentBlock = ErrorBlock;
  for (Index = 0; Index < mPcieRcCount; Index++) {
    if (!mPcieRcConfig[Index].Enabled) {
      continue;
    }

    Status = InitializePcieAerErrorSource (
               (EFI_ACPI_6_5_PCI_EXPRESS_ROOT_PORT_AER_STRUCTURE *)&GhesV2[Index],
               &mPcieRcConfig[Index],
               CurrentBlock,
               PCIE_AER_ERROR_BLOCK_SIZE
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to initialize AER source for RC%d: %r\n",
              mPcieRcConfig[Index].RcId, Status));
      continue;
    }

    CurrentBlock = (UINT8 *)CurrentBlock + PCIE_AER_ERROR_BLOCK_SIZE;
  }
#if 1
  //
  // Create LPDDR5x ECC error source
  //
  Index = mPcieRcCount;
  Status = InitializeDdrEccErrorSource (
             &GhesV2[Index]
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to initialize DDR ECC error source: %r\n",
            Status));
  }
#endif
  return EFI_SUCCESS;
}

/**
  Creates and initializes HEST header.

  @param[in,out] Context    Pointer to HEST context
  @param[in]     TableSize  Size of HEST table

  @retval EFI_SUCCESS           HEST header created successfully
  @retval EFI_INVALID_PARAMETER Invalid Context pointer
  @retval EFI_OUT_OF_RESOURCES  Failed to allocate memory
**/
EFI_STATUS
HestHeaderCreator (
  IN OUT HEST_CONTEXT  *Context,
  IN     UINT32        TableSize
  )
{
  UINT8  AcpiOemId[6] = ACPI_OEM_ID;

  if (Context == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Invalid Context parameter\n", __func__));
    return EFI_INVALID_PARAMETER;
  }

  Context->HestHeader = AllocateZeroPool (TableSize);
  if (Context->HestHeader == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Failed to allocate HEST header\n", __func__));
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialize HEST header according to ACPI 6.5 specification
  //
  Context->HestHeader->Header.Signature = EFI_ACPI_6_5_HARDWARE_ERROR_SOURCE_TABLE_SIGNATURE;
  Context->HestHeader->Header.Length = sizeof (EFI_ACPI_6_5_HARDWARE_ERROR_SOURCE_TABLE_HEADER);
  Context->HestHeader->Header.Revision = EFI_ACPI_6_5_HARDWARE_ERROR_SOURCE_TABLE_REVISION;
  CopyMem (Context->HestHeader->Header.OemId, AcpiOemId, sizeof (AcpiOemId));
  Context->HestHeader->Header.OemTableId = ACPI_OEM_TABLE_ID;
  Context->HestHeader->Header.OemRevision = ACPI_OEM_REVISION;
  Context->HestHeader->Header.CreatorId = ACPI_CREATOR_ID;
  Context->HestHeader->Header.CreatorRevision = ACPI_CREATOR_REVISION;

  //
  // Initialize HEST specific fields
  //
  Context->HestHeader->ErrorSourceCount = 0;

  return EFI_SUCCESS;
}

/**
  Adds an error source descriptor to HEST.

  @param[in,out] Context          Pointer to HEST context
  @param[in]     ErrorSource     Pointer to error source descriptor
  @param[in]     ErrorSourceSize Size of error source descriptor

  @retval EFI_SUCCESS           Error source added successfully
  @retval EFI_INVALID_PARAMETER Invalid parameter
  @retval EFI_BUFFER_TOO_SMALL  Not enough space in HEST
**/
EFI_STATUS
HestAddErrorSourceDescriptor (
  IN OUT HEST_CONTEXT  *Context,
  IN     VOID          *ErrorSource,
  IN     UINT32        ErrorSourceSize
  )
{
  UINT8   *CurrentPtr;
  UINT32  NewLength;

  if (Context == NULL || ErrorSource == NULL || ErrorSourceSize == 0) {
    DEBUG ((DEBUG_ERROR, "%a: Invalid parameter\n", __func__));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Calculate new table length
  //
  NewLength = Context->HestHeader->Header.Length + ErrorSourceSize;
  if (NewLength > HEST_TABLE_SIZE) {
    DEBUG ((DEBUG_ERROR, "%a: HEST table size exceeded\n", __func__));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Add error source descriptor
  //
  CurrentPtr = (UINT8 *)Context->HestHeader + Context->HestHeader->Header.Length;
  CopyMem (CurrentPtr, ErrorSource, ErrorSourceSize);

  //
  // Update HEST header
  //
  Context->HestHeader->Header.Length = NewLength;
  Context->HestHeader->ErrorSourceCount++;

  //
  // Calculate and update checksum
  //
  Context->HestHeader->Header.Checksum = CalculateCheckSum8 (
                                          (UINT8 *)Context->HestHeader,
                                          Context->HestHeader->Header.Length
                                          );

  DEBUG ((
    DEBUG_INFO,
    "%a: Added error source %d, new length: %d\n",
    __func__,
    Context->HestHeader->ErrorSourceCount,
    NewLength
    ));

  return EFI_SUCCESS;
}
