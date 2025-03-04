/** @file
  SOPHGO RAS function header.

  Copyright (C) 2025, SOPHGO Technologies Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef SOPHGO_RAS_H_
#define SOPHGO_RAS_H_

#include "Bert.h"
#include "Ghes.h"
#include "Einj.h"

#include <IpmiCommandLib.h>
#include <Library/MmSpec.h>
#include <Library/SiliconLib.h>

#include <Protocol/SpiNorFlashProtocol.h>

#include <CommonRas.h>

#define RAS_DEBUG

#define HEST_REGION_SEA  4  // System Error Architecture
#define HEST_REGION_SEI  5  // System Error Interface

//
// RAS Handler Status
//
typedef enum {
  PHYTIUM_STMM_HANDLER_SUCCESS = 0,
  PHYTIUM_STMM_HANDLER_HEADER_ERROR = 1,
  PHYTIUM_STMM_HANDLER_LENGTH_ERROR = 2,
  PHYTIUM_STMM_HANDLER_FUNC_ID_UNSUPPORTED = 3
} PHYTIUM_STMM_HANDLER_STATUS;

//
// APEI Type
//
typedef enum {
  TypeBert = 1,
  TypeHest = 2
} APEI_TYPE;

//
// BERT Table Limits
//
#define BERT_TABLE_MAX_NUM  1

//
// APEI Table List Structure
//
typedef struct {
  UINT8             BertNum;
  APEI_SOURCE_TABLE BertTable[BERT_TABLE_MAX_NUM];
  UINT8             HestNum;
  APEI_SOURCE_TABLE HestTable[MAX_GHES];
} APEI_TABLE_LIST;

//
// GHES Register Structure
//
typedef struct {
  UINT64  AckRegister;
  UINT64  ErrorStatusBlockAddress;
} GHES_REGISTER;

extern APEI_TABLE_LIST             mApeiList;
extern EFI_NORFLASH_DRV_PROTOCOL   *mFlash;
extern RAS_TOTAL_COUNT             mRasTotalCount;

/**
  Initialize APEI Table List.

  @param[in,out] List  A pointer to APEI_TABLE_LIST.
**/
VOID
ApeiTableInit (
  IN OUT APEI_TABLE_LIST  *List
  );

/**
  Initialize HEST Table.
  
  The steps are as follows:
  1. Create the HEST header
  2. Fill GHES information
  3. Add GHES information into the HEST table
  4. Parse GHES content (GHES Address and ACK Register) into APEI table

  @retval EFI_SUCCESS  Successfully initialized the HEST table.
  @retval Others       Failed to initialize the HEST table.
**/
EFI_STATUS
InitHestTable (
  VOID
  );

/**
  Generate GHES data and write it to the corresponding error status address
  described in the HEST table according to different severity levels.

  @param[in] Addr    The target address to write GHES data.
  @param[in] Info    The source buffer containing GHES data.
  @param[in] Length  Length of the source buffer.
**/
VOID
GenerateHest (
  IN UINT64  Addr,
  IN UINT8   *Info,
  IN UINT32  Length
  );

/**
  Handle RAS sub-events.

  The source buffer data comes from the content of the MM communicate protocol.
  For the protocol content, refer to Chapter 6.3.4 of the SOPHGO MM Interface
  Specification.

  @param[in]     SubId   Sub-event ID, refer to Chapter 6.3.4 of the SOPHGO MM
                        Interface Specification.
  @param[in,out] Buffer  The content section in MM communicate buffer.
  @param[in,out] Length  Length of content.

  @retval EFI_SUCCESS  Successfully handled the RAS sub-event.
  @retval Others       Failed to handle the RAS sub-event.
**/
UINT32
RasMainFunctionHandle (
  IN     UINT16  SubId,
  IN OUT UINT8   *Buffer,
  IN OUT UINT32  *Length
  );

#endif
