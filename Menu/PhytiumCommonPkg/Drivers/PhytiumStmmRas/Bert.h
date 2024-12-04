/** @file
Header file of functions about BERT tables initialization.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  BERT_H_
#define  BERT_H_

#include "Ghes.h"

typedef struct _BERT_CONTEXT {
  EFI_ACPI_6_4_BOOT_ERROR_RECORD_TABLE_HEADER   *BertHeader;
  VOID                                          *Block;
  UINT32                                        BlockSize;
} BERT_CONTEXT;

extern BERT_CONTEXT  mBertContext;

/**
  Bert table initialization.

  @retval    EFI_STATUS    Success.
**/
EFI_STATUS
InitBertTable (
  VOID
  );

#endif
