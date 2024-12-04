/** @file
Provides Phytium stmm bert functions

Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include "Bert.h"

BERT_CONTEXT  mBertContext;

/**
  Create BERT header.

  @param[in]  Context         A poniter to BERT_CONTEXT.
  @param[in]  ErrorBlockSize  Error block size.
**/
EFI_STATUS
BertHeaderCreator (
  IN BERT_CONTEXT  *Context,
  IN UINT32        ErrorBlockSize
  )
{
  if (Context == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Context->BertHeader = AllocateZeroPool (sizeof (EFI_ACPI_6_4_BOOT_ERROR_RECORD_TABLE_HEADER));
  Context->Block = (VOID *) ((UINT64) (PcdGet64 (PcdRasMemoryBase)  \
                   + PcdGet64 (PcdBertRegionOffset)));
  Context->BlockSize = ErrorBlockSize;
  *Context->BertHeader = (EFI_ACPI_6_4_BOOT_ERROR_RECORD_TABLE_HEADER) {
    ARM_ACPI_HEADER (
      EFI_ACPI_6_4_BOOT_ERROR_RECORD_TABLE_SIGNATURE,
      EFI_ACPI_6_4_BOOT_ERROR_RECORD_TABLE_HEADER,
      EFI_ACPI_6_4_BOOT_ERROR_RECORD_TABLE_REVISION
      ),
    Context->BlockSize,
    (UINT64)Context->Block
  };

  return EFI_SUCCESS;
}

/**
  Bert table initialization.

  @retval    EFI_STATUS    Success.
**/
EFI_STATUS
InitBertTable (
  VOID
  )
{
  UINT8       Checksum;
  EFI_STATUS  Status;

  ZeroMem (&mBertContext, sizeof (BERT_CONTEXT));
  Status = BertHeaderCreator (&mBertContext, FixedPcdGet64 (PcdBertRegionSize));
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Bert header creator failed : %r\n"));
    return Status;
  }

  mBertContext.BertHeader->Header.Length = sizeof (EFI_ACPI_6_4_BOOT_ERROR_RECORD_TABLE_HEADER);
  Checksum = CalculateCheckSum8 (
               (UINT8*) (mBertContext.BertHeader),
               mBertContext.BertHeader->Header.Length
               );
  mBertContext.BertHeader->Header.Checksum = Checksum;

  return EFI_SUCCESS;

}

