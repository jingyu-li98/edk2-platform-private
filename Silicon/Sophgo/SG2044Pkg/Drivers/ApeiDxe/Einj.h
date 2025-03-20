/** @file
  Header file for Error INJection (EINJ) table implementation.

  This file defines the structures and functions for EINJ table initialization
  and error injection capabilities configuration.

  Copyright (c) 2025, Sophgo Technologies Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef SOPHGO_EINJ_H_
#define SOPHGO_EINJ_H_

#include <Base.h>
#include <Uefi.h>
#include <IndustryStandard/Acpi65.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>

//
// ACPI Table Information
//
#define ACPI_OEM_TABLE_ID           SIGNATURE_64('S','G','2','0','4','4','0','0')
#define ACPI_OEM_REVISION           0x00000001
#define ACPI_CREATOR_ID             SIGNATURE_32('S','P','H','G')
#define ACPI_CREATOR_REVISION       0x00000001

#define EINJ_ACTION_NO                10
#define EINJ_BEGIN_OPERATION_VALUE    0xFFFF
#define EINJ_END_OPERATION_VALUE      0
#define EINJ_WRITE_MASK              0xFFFFFFFF
#define EINJ_READ_VALUE              0xFFFF
#define EINJ_READ_MASK               0xFFFFFFFF

#define EINJ_TRIGGER_ERROR_ACTION_NO  1

//
// EINJ Action Table Actions
//
#define EFI_ACPI_6_5_EINJ_SET_ERROR_TYPE_WITH_ADDRESS      0x08
#define EFI_ACPI_6_5_EINJ_GET_EXECUTE_OPERATION_TIMINGS    0x09

//
// Error Type Definition
//
#define EINJ_PROCESSOR_CORRECTABLE                BIT0
#define EINJ_PROCESSOR_UNCORRECTABLE_NONFATAL     BIT1
#define EINJ_PROCESSOR_UNCORRECTABLE_FATAL        BIT2
#define EINJ_MEMORY_CORRECTABLE                   BIT3
#define EINJ_MEMORY_UNCORRECTABLE_NONFATAL        BIT4
#define EINJ_MEMORY_UNCORRECTABLE_FATAL           BIT5
#define EINJ_PCIE_CORRECTABLE                     BIT6
#define EINJ_PCIE_UNCORRECTABLE_NONFATAL          BIT7
#define EINJ_PCIE_UNCORRECTABLE_FATAL             BIT8
#define EINJ_PLATFORM_CORRECTABLE                 BIT9
#define EINJ_PLATFORM_UNCORRECTABLE_NONFATAL      BIT10
#define EINJ_PLATFORM_UNCORRECTABLE_FATAL         BIT11
#define EINJ_VENDOR_DEFINED_ERROR_TYPE            BIT31

#define EINJ_PROCESSOR_APIC_VALID                 BIT0
#define EINJ_MEMORY_ADDRESS_VALID                 BIT1
#define EINJ_PCIE_SBDF_VALID                      BIT2

//
// EINJ Table
//
typedef struct {
  EFI_ACPI_6_5_ERROR_INJECTION_TABLE_HEADER        EinjTableHeader;
  EFI_ACPI_6_5_EINJ_INJECTION_INSTRUCTION_ENTRY    EinjInstructionEntry[EINJ_ACTION_NO];
} EINJ_TABLE;

//
// EINJ Trigger Action Table
//
typedef struct {
  EFI_ACPI_6_5_EINJ_TRIGGER_ACTION_TABLE          TriggerErrorHeader;
  EFI_ACPI_6_5_EINJ_INJECTION_INSTRUCTION_ENTRY   ErrorInstructionEntry[EINJ_TRIGGER_ERROR_ACTION_NO];
} EINJ_TRIGGER_ERROR_ACTION;

//
// PCIe SBDF Structure
//
typedef struct {
  UINT32 Reserved : 8;
  UINT32 Function : 3;
  UINT32 Device : 5;
  UINT32 PrimaryOrDeviceBus : 8;
  UINT32 Segment : 8;
} EINJ_PCIE_SBDF;

//
// Error Type With Address Structure
//
typedef struct {
  UINT32         ErrorType;
  UINT32         VendorErrorTypeOffset;
  UINT32         Flags;
  UINT32         ApicId;
  UINT64         MemAddress;
  UINT64         MemAddressRange;
  EINJ_PCIE_SBDF PcieSBDF;
} EINJ_SET_ERROR_TYPE_WITH_ADDRESS;

//
// Vendor Error Type Extension Structure
//
typedef struct {
  UINT32 Length;
  UINT32 SBDF;
  UINT16 VendorId;
  UINT16 DeviceId;
  UINT8  RevId;
  UINT8  Reserved[3];
} EINJ_VENDOR_ERROR_TYPE;

//
// EINJ Data Structure
//
typedef struct {
  UINT64                           OperationBegin;
  UINT64                           ErrorType;
  UINT64                           ErrorCapabilities;
  UINT64                           BusyStatus;
  UINT64                           CommandStatus;
  UINT64                           Timing;
  EINJ_TRIGGER_ERROR_ACTION        *TriggerErrorActionTablePtr;
  EINJ_SET_ERROR_TYPE_WITH_ADDRESS ErrorTypeWithAddress;
  EINJ_VENDOR_ERROR_TYPE           VendorErrorTypeExtension;
  EINJ_TRIGGER_ERROR_ACTION        TriggerErrorActionTable;
} EINJ_DATA_STRUCTURE;

//
// EINJ Context Structure
//
typedef struct {
  EINJ_TABLE                                     *EINJ;
  EINJ_DATA_STRUCTURE                            *EinjData;
  EFI_ACPI_6_5_EINJ_INJECTION_INSTRUCTION_ENTRY  *GetErrorTypeEntry;
  EFI_ACPI_6_5_EINJ_INJECTION_INSTRUCTION_ENTRY  *ExecuteOperationEntry;
} EINJ_CONTEXT;

extern EINJ_CONTEXT mEinjContext;

/**
  Initialize EINJ table.

  @retval EFI_SUCCESS  Successfully initialized the EINJ table.
  @retval Others       Failed to initialize the EINJ table.
**/
EFI_STATUS
InitEinjTable (
  VOID
  );

/**
  Configure EINJ supported error types.

  @param[in] Context               A pointer to EINJ_CONTEXT.
  @param[in] BitsSupportedErrorType Supported error type bit mask.
                                   Bit0  - Processor Correctable
                                   Bit1  - Processor Uncorrectable non-fatal
                                   Bit2  - Processor Uncorrectable fatal
                                   Bit3  - Memory Correctable
                                   Bit4  - Memory Uncorrectable non-fatal
                                   Bit5  - Memory Uncorrectable fatal
                                   Bit6  - PCIe Correctable
                                   Bit7  - PCIe Uncorrectable non-fatal
                                   Bit8  - PCIe Uncorrectable fatal
                                   Bit9  - Platform Correctable
                                   Bit10 - Platform Uncorrectable non-fatal
                                   Bit11 - Platform Uncorrectable fatal

  @retval EFI_SUCCESS  Successfully configured error injection capabilities.
**/
EFI_STATUS
EinjConfigErrorInjectCapability (
  IN EINJ_CONTEXT  *Context,
  IN UINT32        BitsSupportedErrorType
  );

/**
  Create EINJ table header.

  @param[in] Context  A pointer to EINJ_CONTEXT.

  @retval EFI_SUCCESS  Successfully created EINJ header.
  @retval Others       Failed to create EINJ header.
**/
EFI_STATUS
EinjHeaderCreator (
  IN OUT EINJ_CONTEXT  *Context
  );

#endif
