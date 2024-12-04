/** @file
Header file of functions about EINJ tables initialization.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef EINJ_H_
#define EINJ_H_

#include "Ghes.h"

#define EINJ_ACTION_NO             10
#define EINJ_BEGIN_OPERATION_VALUE 0xFFFF
#define EINJ_END_OPERATION_VALUE   0
#define EINJ_WRITE_MASK            0xFFFFFFFF
#define EINJ_READ_VALUE            0xFFFF
#define EINJ_READ_MASK             0xFFFFFFFF

#define EINJ_TRIGGER_ERROR_ACTION_NO                                    1

#define EFI_ACPI_EINJ_SET_ERROR_TYPE_WITH_ADDRESS                       0x08
#define EFI_ACPI_EINJ_GET_EXCUTE_OPERATION_TIMINGS                      0x09

//
// Error Type Definition
//
#define EINJ_PROCESSOR_CORRECTABLE                                      BIT0
#define EINJ_PROCESSOR_UNCORRECTABLE_NONFATAL                           BIT1
#define EINJ_PROCESSOR_UNCORRECTABLE_FATAL                              BIT2
#define EINJ_MEMORY_CORRECTABLE                                         BIT3
#define EINJ_MEMORY_UNCORRECTABLE_NONFATAL                              BIT4
#define EINJ_MEMORY_UNCORRECTABLE_FATAL                                 BIT5
#define EINJ_PCIE_CORRECTABLE                                           BIT6
#define EINJ_PCIE_UNCORRECTABLE_NONFATAL                                BIT7
#define EINJ_PCIE_UNCORRECTABLE_FATAL                                   BIT8
#define EINJ_PLATFORM_CORRECTABLE                                       BIT9
#define EINJ_PLATFORM_UNCORRECTABLE_NONFATAL                            BIT10
#define EINJ_PLATFORM_UNCORRECTABLE_FATAL                               BIT11
#define EINJ_VENDOR_DEFINED_ERROR_TYPE                                  BIT31

#define EINJ_PROCESSOR_APIC_VALID                                       BIT0
#define EINJ_MEMORY_ADDRESS_VALID                                       BIT1
#define EINJ_PCIE_SBDF_VALID                                            BIT2

//
// EINJ Table
//
typedef struct {
  EFI_ACPI_6_4_ERROR_INJECTION_TABLE_HEADER        EinjTableHeader;
  EFI_ACPI_6_4_EINJ_INJECTION_INSTRUCTION_ENTRY    EinjInstructionEntry[EINJ_ACTION_NO];
} EINJ_TABLE;

//
//Einj trigger action
//
typedef struct {
  EFI_ACPI_6_4_EINJ_TRIGGER_ACTION_TABLE           TriggerErrorHeader;
  EFI_ACPI_6_4_EINJ_INJECTION_INSTRUCTION_ENTRY    ErrorInstructionEntry[EINJ_TRIGGER_ERROR_ACTION_NO];
} EINJ_TRIGGER_ERROR_ACTION;

typedef struct {
  UINT32 Reserved: 8;
  UINT32 Function: 3;
  UINT32 Device: 5;
  UINT32 PrimaryOrDeviceBus: 8;
  UINT32 Segment: 8;
} EINJ_PCIE_SBDF;

typedef struct {
  UINT32         ErrorType;
  UINT32         VendorErrorTypeOffset;
  UINT32         Flags;
  UINT32         ApicId;
  UINT64         MemAddress;
  UINT64         MemAddressRange;
  EINJ_PCIE_SBDF PcieSBDF;
} EINJ_SET_ERROR_TYPE_WITH_ADDRESS;

typedef struct {
  UINT32 Length;
  UINT32 SBDF;
  UINT16 VendorId;
  UINT16 DeviceId;
  UINT8  RevId;
  UINT8  Reserved[3];
} EINJ_VENDOR_ERROR_TYPE;

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

// V2
typedef struct _EINJ_CONTEXT {
  EINJ_TABLE                                    *EINJ;
  EINJ_DATA_STRUCTURE                           *EinjData;
  EFI_ACPI_6_4_EINJ_INJECTION_INSTRUCTION_ENTRY *GetErrorTypeEntry;
  EFI_ACPI_6_4_EINJ_INJECTION_INSTRUCTION_ENTRY *ExecuteOperationEntry;
} EINJ_CONTEXT;

extern EINJ_CONTEXT    mEinjContext;

/**
  Einj table initialization.

  @retval    EFI_STATUS    Success.
  @ratval    Other         Failed.
**/
EFI_STATUS
InitEinjTable (
  VOID
  );

/**
  Config einj supported error type.

  @param[in]  Context                 A pointer to EINJ_CONTEXT.
  @param[in]  BitsSupportedErrorType  Supported error type, bit mask.
                                      Bit0  - Processor Correctable.
                                      Bit1  - Processor Uncorrectable fatal.
                                      Bit2  - Processor Incorrectable fatal.
                                      Bit3  - Memory Correctable.
                                      Bit4  - Memory Incorrectable non-fatal.
                                      Bit5  - Memory Incorrectable fatal.
                                      Bit6  - PCI Express Correctable.
                                      Bit7  - PCI Express Uncorrectable non-fatal.
                                      Bit8  - PCI Express Uncorrectable fatal.
                                      Bit9  - Platform Correctable.
                                      Bit10 - Platform Uncorrectable non-fatal.
                                      Bit11 - Platform Uncorrectable fatal.

  @retval     EFI_SUCCESS             Success.
**/
EFI_STATUS
EinjConfigErrorInjectCapability(
  IN EINJ_CONTEXT  *Context,
  IN UINT32        BitsSupportedErrorType
);

/**
  Creator einj table header.

  @param[in]  Context  A pointer to EINJ_CONTEXT.
**/
EFI_STATUS
EinjHeaderCreator (
  IN EINJ_CONTEXT  *Context
  );

#endif
