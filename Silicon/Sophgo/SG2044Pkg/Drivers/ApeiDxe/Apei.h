/** @file
  SOPHGO RAS function header.

  Copyright (C) 2025, Sophgo Technologies Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef APEI_H_
#define APEI_H_

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/IoLib.h>
#include <IndustryStandard/Acpi65.h>
#include <Guid/Cper.h>
#include "Hest.h"

#define MAX_BERT_TABLES  2
#define MAX_HEST_TABLES  5
#define MAX_GHES         42

//
// APEI Trusted Firmware Data Structure
//
typedef struct {
  UINT64    Base;           ///< Base address of trusted firmware data
  UINT32    Size;           ///< Size of trusted firmware data
  UINT32    Flags;          ///< Flags for trusted firmware data
  UINT32    Reserved;       ///< Reserved for future use
} APEI_TRUSTED_FIRMWARE_STRUCTURE;

typedef enum {
  TypeBert = 0,
  TypeHest,
  TypeMax
} APEI_TABLE_TYPE;

typedef struct {
  UINT64  Base;
  UINT32  Size;
  UINT32  ErrorSourceNum;
  UINT8   Type;
  UINT64  AckReg;
  UINT64  AckPreserve;
  UINT64  AckWrite;
} APEI_SOURCE_TABLE;

typedef struct {
  APEI_SOURCE_TABLE  BertTable[MAX_BERT_TABLES];
  APEI_SOURCE_TABLE  HestTable[MAX_HEST_TABLES];
  UINT32            BertNum;
  UINT32            HestNum;
} APEI_TABLE_LIST;

extern APEI_TABLE_LIST  mApeiList;
extern GHES_REGISTER    *mGhesRegisters;

/**
  Initialize APEI Table List.

  @param[in,out] List  A pointer to APEI_TABLE_LIST.
**/
VOID
ApeiTableInit (
  IN OUT APEI_TABLE_LIST  *List
  );

/**
  Initialize BERT Table.

  @retval EFI_SUCCESS  Successfully initialized the BERT table.
  @retval Others       Failed to initialize the BERT table.
**/
EFI_STATUS
InitBertTable (
  VOID
  );

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
  );

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
  );

#endif // APEI_H_
