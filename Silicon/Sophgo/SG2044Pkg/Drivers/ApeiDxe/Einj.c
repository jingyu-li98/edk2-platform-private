/** @file
  Error INJection (EINJ) table implementation.

  This file implements EINJ table initialization and error injection
  capabilities for SOPHGO platform according to ACPI 6.5 specification.

  Copyright (c) 2025, Sophgo Technologies Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/



#include "Einj.h"

//
// Global variables
//
EINJ_CONTEXT    mEinjContext;

//
// EINJ Table template
//
EINJ_TABLE mEinj = {
  {
    {
      EFI_ACPI_6_5_ERROR_INJECTION_TABLE_SIGNATURE,  // Signature
      sizeof (EFI_ACPI_6_5_ERROR_INJECTION_TABLE_HEADER),  // Length
      EFI_ACPI_6_5_ERROR_INJECTION_TABLE_REVISION,   // Revision
      0,                                             // Checksum
      {'S','O','P','H','G','O',' ',' '},             // OemId[6]
      ACPI_OEM_TABLE_ID,                             // OemTableId
      ACPI_OEM_REVISION,                             // OemRevision
      ACPI_CREATOR_ID,                               // CreatorId
      ACPI_CREATOR_REVISION                          // CreatorRevision
    },
    sizeof (EFI_ACPI_6_5_ERROR_INJECTION_TABLE_HEADER),
    0x0,
    {
      0x0,
      0x0,
      0x0
    },
    EINJ_ACTION_NO
  },
  {
    {
      // 0 EFI_ACPI_6_5_EINJ_BEGIN_INJECTION_OPERATION
      EFI_ACPI_6_5_EINJ_BEGIN_INJECTION_OPERATION,
      EFI_ACPI_6_5_EINJ_WRITE_REGISTER_VALUE,
      EFI_ACPI_6_5_EINJ_PRESERVE_REGISTER,
      0,
      {
        EFI_ACPI_6_5_SYSTEM_MEMORY,
        64,
        0,
        EFI_ACPI_6_5_QWORD,
        0
      },
      EINJ_BEGIN_OPERATION_VALUE,
      EINJ_WRITE_MASK
    },
    {
      // 1 EFI_ACPI_6_5_EINJ_GET_TRIGGER_ERROR_ACTION_TABLE
      EFI_ACPI_6_5_EINJ_GET_TRIGGER_ERROR_ACTION_TABLE,
      EFI_ACPI_6_5_EINJ_READ_REGISTER,
      0,
      0,
      {
        EFI_ACPI_6_5_SYSTEM_MEMORY,
        64,
        0,
        EFI_ACPI_6_5_QWORD,
        0
      },
      0,
      EINJ_READ_MASK
    },
    {
      // 2 EFI_ACPI_6_5_EINJ_SET_ERROR_TYPE
      EFI_ACPI_6_5_EINJ_SET_ERROR_TYPE,
      EFI_ACPI_6_5_EINJ_WRITE_REGISTER,
      EFI_ACPI_6_5_EINJ_PRESERVE_REGISTER,
      0,
      {
        EFI_ACPI_6_5_SYSTEM_MEMORY,
        64,
        0,
        EFI_ACPI_6_5_QWORD,
        0
      },
      0,
      EINJ_WRITE_MASK
    },
    {
      // 3 EFI_ACPI_6_5_EINJ_GET_ERROR_TYPE
      EFI_ACPI_6_5_EINJ_GET_ERROR_TYPE,
      EFI_ACPI_6_5_EINJ_READ_REGISTER,
      0,
      0,
      {
        EFI_ACPI_6_5_SYSTEM_MEMORY,
        64,
        0,
        EFI_ACPI_6_5_QWORD,
        0
      },
      0,
      0xFFFFFFFF
    },
    {
      // 4 EFI_ACPI_6_5_EINJ_END_OPERATION
      EFI_ACPI_6_5_EINJ_END_OPERATION,
      EFI_ACPI_6_5_EINJ_WRITE_REGISTER_VALUE,
      EFI_ACPI_6_5_EINJ_PRESERVE_REGISTER,
      0,
      {
        EFI_ACPI_6_5_SYSTEM_MEMORY,
        64,
        0,
        EFI_ACPI_6_5_QWORD,
        0
      },
      EINJ_END_OPERATION_VALUE,
      0xFFFFFFFF
    },
    {
      // 5 EFI_ACPI_6_5_EINJ_EXECUTE_OPERATION
      EFI_ACPI_6_5_EINJ_EXECUTE_OPERATION,
      EFI_ACPI_6_5_EINJ_WRITE_REGISTER_VALUE,
      EFI_ACPI_6_5_EINJ_PRESERVE_REGISTER,
      0,
      {
        EFI_ACPI_6_5_SYSTEM_MEMORY,
        32,
        0,
        EFI_ACPI_6_5_DWORD,
        FixedPcdGet64 (PcdRasEinjExecuteOperationAddress)
      },
      1,
      0xFFFFFFFF
    },
    {
      // 6 EFI_ACPI_6_5_EINJ_CHECK_BUSY_STATUS
      EFI_ACPI_6_5_EINJ_CHECK_BUSY_STATUS,
      EFI_ACPI_6_5_EINJ_READ_REGISTER_VALUE,
      0,
      0,
      {
        EFI_ACPI_6_5_SYSTEM_MEMORY,
        64,
        0,
        EFI_ACPI_6_5_QWORD,
        0
      },
      0x01,
      0x01
    },
    {
      // 7 EFI_ACPI_6_5_EINJ_GET_COMMAND_STATUS
      EFI_ACPI_6_5_EINJ_GET_COMMAND_STATUS,
      EFI_ACPI_6_5_EINJ_READ_REGISTER,
      0,
      0,
      {
        EFI_ACPI_6_5_SYSTEM_MEMORY,
        64,
        0,
        EFI_ACPI_6_5_QWORD,
        0
      },
      0,
      0x3
    },
    {
      // 8 EFI_ACPI_6_5_EINJ_SET_ERROR_TYPE_WITH_ADDRESS
      EFI_ACPI_6_5_EINJ_SET_ERROR_TYPE_WITH_ADDRESS,
      EFI_ACPI_6_5_EINJ_WRITE_REGISTER,
      EFI_ACPI_6_5_EINJ_PRESERVE_REGISTER,
      0,
      {
        EFI_ACPI_6_5_SYSTEM_MEMORY,
        64,
        0,
        EFI_ACPI_6_5_QWORD,
        0
      },
      0,
      0xFFFFFF
    },
    {
      // 9 EFI_ACPI_6_5_EINJ_GET_EXECUTE_OPERATION_TIMINGS
      EFI_ACPI_6_5_EINJ_GET_EXECUTE_OPERATION_TIMINGS,
      EFI_ACPI_6_5_EINJ_READ_REGISTER,
      EFI_ACPI_6_5_EINJ_PRESERVE_REGISTER,
      0,
      {
        EFI_ACPI_6_5_SYSTEM_MEMORY,
        64,
        0,
        EFI_ACPI_6_5_QWORD,
        0
      },
      0x200000,
      0xFFFFFF
    }
  }
};

//
// EINJ Trigger Error Action template
//
EINJ_TRIGGER_ERROR_ACTION mEinjTriggerErrorAction = {
  {
    sizeof (EFI_ACPI_6_5_EINJ_TRIGGER_ACTION_TABLE),
    0,
    sizeof (EINJ_TRIGGER_ERROR_ACTION),
    EINJ_TRIGGER_ERROR_ACTION_NO
  },
  {
    {
      EFI_ACPI_6_5_EINJ_TRIGGER_ERROR,
      EFI_ACPI_6_5_EINJ_WRITE_REGISTER_VALUE,
      0,
      0,
      {
        EFI_ACPI_6_5_SYSTEM_MEMORY,
        32,
        0,
        EFI_ACPI_6_5_DWORD,
        FixedPcdGet64 (PcdRasEinjTriggerErrorAddress)
      },
      0,
      0
    }
  }
};

/**
  Create EINJ table header and initialize data structures.

  This function:
  - Allocates and initializes EINJ data structure
  - Sets up register addresses for instruction entries
  - Configures trigger error action table

  @param[in,out]  Context   Pointer to EINJ context structure

  @retval EFI_SUCCESS       EINJ header created successfully
  @retval Others            Failed to create EINJ header
**/
EFI_STATUS
EinjHeaderCreator (
  IN OUT EINJ_CONTEXT  *Context
  )
{
  EINJ_DATA_STRUCTURE  *EinjData;

  EinjData = (EINJ_DATA_STRUCTURE *)(UINTN)(PcdGet64 (PcdRasMemoryBase) +
           PcdGet64 (PcdEinjRegionOffset));
  ZeroMem (EinjData, sizeof (EINJ_DATA_STRUCTURE));

  DEBUG ((DEBUG_INFO,
          "EINJ EinjData is at 0x%X, size = 0x%x\n",
          EinjData,
          sizeof (EINJ_DATA_STRUCTURE)));

  //
  // Initialize trigger error action table pointer and data
  //
  EinjData->TriggerErrorActionTablePtr = 
    (EINJ_TRIGGER_ERROR_ACTION*)(&(EinjData->TriggerErrorActionTable));
  CopyMem (
    EinjData->TriggerErrorActionTablePtr,
    &mEinjTriggerErrorAction,
    sizeof (EINJ_TRIGGER_ERROR_ACTION)
    );

  //
  // Initialize EINJ data fields
  //
  EinjData->OperationBegin = 0;
  EinjData->ErrorType = 0;
  EinjData->ErrorCapabilities = 0xFFF;
  EinjData->BusyStatus = 0;
  EinjData->CommandStatus = 0;

  //
  // Set up register addresses for instruction entries
  //
  mEinj.EinjInstructionEntry[0].RegisterRegion.Address =
    (UINT64)(&(EinjData->OperationBegin));
  mEinj.EinjInstructionEntry[1].RegisterRegion.Address =
    (UINT64)(&(EinjData->TriggerErrorActionTablePtr));
  mEinj.EinjInstructionEntry[2].RegisterRegion.Address =
    (UINT64)(&(EinjData->ErrorType));
  mEinj.EinjInstructionEntry[3].RegisterRegion.Address =
    (UINT64)(&(EinjData->ErrorCapabilities));
  mEinj.EinjInstructionEntry[4].RegisterRegion.Address =
    (UINT64)(&(EinjData->OperationBegin));
  mEinj.EinjInstructionEntry[6].RegisterRegion.Address =
    (UINT64)(&(EinjData->BusyStatus));
  mEinj.EinjInstructionEntry[7].RegisterRegion.Address =
    (UINT64)(&(EinjData->CommandStatus));
  mEinj.EinjInstructionEntry[8].RegisterRegion.Address =
    (UINT64)(&(EinjData->ErrorTypeWithAddress));
  mEinj.EinjInstructionEntry[9].RegisterRegion.Address =
    (UINT64)(&(EinjData->Timing));

  //
  // Set up vendor error type extension offset
  //
  EinjData->ErrorTypeWithAddress.VendorErrorTypeOffset =
    (UINT32)((UINTN)&(EinjData->VendorErrorTypeExtension) -
    (UINTN)&(EinjData->ErrorTypeWithAddress));

  //
  // Update context
  //
  Context->EinjData = EinjData;
  Context->EINJ = &mEinj;
  Context->ExecuteOperationEntry = &mEinj.EinjInstructionEntry[5];
  Context->GetErrorTypeEntry = &mEinj.EinjInstructionEntry[3];

  return EFI_SUCCESS;
}

/**
  Configure supported error injection types.

  This function configures which error types can be injected through EINJ.
  The error types are specified through a bit mask in BitsSupportedErrorType.

  @param[in]  Context                 Pointer to EINJ context
  @param[in]  BitsSupportedErrorType  Bit mask of supported error types

  @retval EFI_SUCCESS                 Error types configured successfully
**/
EFI_STATUS
EinjConfigErrorInjectCapability (
  IN EINJ_CONTEXT  *Context,
  IN UINT32        BitsSupportedErrorType
  )
{
  EFI_ACPI_6_5_EINJ_INJECTION_INSTRUCTION_ENTRY  *KeyEntry;
  UINT32                                         *EinjCapablity;

  KeyEntry = Context->GetErrorTypeEntry;
  EinjCapablity = (UINT32*)KeyEntry->RegisterRegion.Address;
  *EinjCapablity = BitsSupportedErrorType;
  KeyEntry->Value = BitsSupportedErrorType;

  return EFI_SUCCESS;
}

/**
  Initialize EINJ table.

  This function:
  - Creates EINJ header and initializes data structures
  - Configures supported error types
  - Calculates and sets table checksum

  @retval EFI_SUCCESS    EINJ table initialized successfully
  @retval Others         Initialization failed
**/
EFI_STATUS
InitEinjTable (
  VOID
  )
{
  UINT8       Checksum;
  EFI_STATUS  Status;
  UINT32      Type;

  ZeroMem (&mEinjContext, sizeof (EINJ_CONTEXT));

  //
  // Create EINJ header
  //
  Status = EinjHeaderCreator (&mEinjContext);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "EINJ header creation failed: %r\n", Status));
    return Status;
  }

  //
  // Configure supported error types
  //
  Type = EINJ_PROCESSOR_CORRECTABLE |
         EINJ_PROCESSOR_UNCORRECTABLE_NONFATAL |
         EINJ_PROCESSOR_UNCORRECTABLE_FATAL |
         EINJ_MEMORY_CORRECTABLE |
         EINJ_MEMORY_UNCORRECTABLE_NONFATAL |
         EINJ_MEMORY_UNCORRECTABLE_FATAL |
         EINJ_PCIE_CORRECTABLE |
         EINJ_PCIE_UNCORRECTABLE_NONFATAL |
         EINJ_PCIE_UNCORRECTABLE_FATAL |
         EINJ_PLATFORM_CORRECTABLE |
         EINJ_PLATFORM_UNCORRECTABLE_NONFATAL |
         EINJ_PLATFORM_UNCORRECTABLE_FATAL;

  DEBUG ((DEBUG_INFO, "EINJ supported error types: 0x%x\n", Type));

  Status = EinjConfigErrorInjectCapability (&mEinjContext, Type);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to configure error injection capabilities: %r\n", Status));
    return Status;
  }

  //
  // Update table length and calculate checksum
  //
  mEinjContext.EINJ->EinjTableHeader.Header.Length = sizeof (EINJ_TABLE);
  Checksum = CalculateCheckSum8 (
               (UINT8*)(mEinjContext.EINJ),
               mEinjContext.EINJ->EinjTableHeader.Header.Length
               );
  mEinjContext.EINJ->EinjTableHeader.Header.Checksum = Checksum;

  return EFI_SUCCESS;
}
