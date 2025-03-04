/** @file
Provides Phytium stmm einj functions

Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <IndustryStandard/Acpi65.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>

#include "Einj.h"

EINJ_CONTEXT    mEinjContext;

EINJ_TABLE mEinj = {
  {
    ARM_ACPI_HEADER (
      EFI_ACPI_6_5_ERROR_INJECTION_TABLE_SIGNATURE,
      EFI_ACPI_6_5_ERROR_INJECTION_TABLE_HEADER,
      EFI_ACPI_6_5_ERROR_INJECTION_TABLE_REVISION
    ),
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
  Creator einj table header.

  @param[in]  Context  A pointer to EINJ_CONTEXT.
**/
EFI_STATUS
EinjHeaderCreator (
  IN OUT EINJ_CONTEXT  *Context
  )
{
  EINJ_DATA_STRUCTURE  *EinjData = NULL;

  EinjData = (EINJ_DATA_STRUCTURE *) (UINT64) (PcdGet64 (PcdRasMemoryBase) \
             + PcdGet64 (PcdEinjRegionOffset));
  ZeroMem (EinjData, sizeof (EINJ_DATA_STRUCTURE));

  DEBUG ((DEBUG_INFO,
          "EINJ EinjData is at 0x%X,size =0x%x\n",
          EinjData,
          sizeof (EINJ_DATA_STRUCTURE)
          ));
  EinjData->TriggerErrorActionTablePtr =
    (EINJ_TRIGGER_ERROR_ACTION*)(&(EinjData->TriggerErrorActionTable));
  CopyMem (
    EinjData->TriggerErrorActionTablePtr,
    &mEinjTriggerErrorAction,
    sizeof (EINJ_TRIGGER_ERROR_ACTION)
    );
  EinjData->OperationBegin = 0;
  EinjData->ErrorType = 0;
  EinjData->ErrorCapabilities = 0xFFF;
  EinjData->BusyStatus = 0;
  EinjData->CommandStatus = 0;
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
  EinjData->ErrorTypeWithAddress.VendorErrorTypeOffset =
    (UINT32)((UINTN)&(EinjData->VendorErrorTypeExtension) -
    (UINTN)&(EinjData->ErrorTypeWithAddress));
  Context->EinjData = EinjData;
  Context->EINJ = &mEinj;
  Context->ExecuteOperationEntry = &mEinj.EinjInstructionEntry[5];
  Context->GetErrorTypeEntry = &mEinj.EinjInstructionEntry[3];

  return EFI_SUCCESS;
}

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
  Einj table initialization.

  @retval    EFI_STATUS    Success.
  @ratval    Other         Failed.
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
  Status = EinjHeaderCreator (&mEinjContext);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Einj header creator failed : %r\n", Status));
    return Status;
  }

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
  DEBUG ((DEBUG_INFO, "Einj supported error type : %x\n", Type));
  Status = EinjConfigErrorInjectCapability (&mEinjContext, Type);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Einj config error inject capability failed : %r\n", Status));
    return Status;
  }

  mEinjContext.EINJ->EinjTableHeader.Header.Length = sizeof (EINJ_TABLE);
  Checksum = CalculateCheckSum8 (
               (UINT8*) (mEinjContext.EINJ),
               mEinjContext.EINJ->EinjTableHeader.Header.Length
               );
  mEinjContext.EINJ->EinjTableHeader.Header.Checksum = Checksum;

  return EFI_SUCCESS;
}
