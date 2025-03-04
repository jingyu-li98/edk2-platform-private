/** @file
Header file of functions about BERT tables initialization.

Copyright (C) 2025, SOPHGO Technologies Inc. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  SOPHGO_BERT_H_
#define  SOPHGO_BERT_H_

#include "Ghes.h"

typedef struct _BERT_CONTEXT {
  EFI_ACPI_6_5_BOOT_ERROR_RECORD_TABLE_HEADER  *BertHeader;
  VOID                                         *Block;
  UINT32                                       BlockSize;
} BERT_CONTEXT;

extern BERT_CONTEXT  mBertContext;

/**
  Create BERT header.

  @param[in]  Context         A pointer to BERT_CONTEXT.
  @param[in]  ErrorBlockSize  Error block size.

  @retval     EFI_SUCCESS     Success.
  @retval     EFI_INVALID_PARAMETER  Context is NULL.
**/
EFI_STATUS
BertHeaderCreator (
  IN BERT_CONTEXT  *Context,
  IN UINT32        ErrorBlockSize
  );

/**
  Bert table initialization.

  @retval    EFI_SUCCESS     Success.
  @retval    EFI_DEVICE_ERROR  Failed to create BERT header.
**/
EFI_STATUS
InitBertTable (
  VOID
  );

#endif
