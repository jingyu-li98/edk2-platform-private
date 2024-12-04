/** @file
Phytium stmm ras fucntion header.

Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent.

**/
#ifndef  PHYTIUM_RAS_H_
#define  PHYTIUM_RAS_H_

#include "Bert.h"
#include "Ghes.h"
#include "Einj.h"

#include <IpmiCommandLib.h>
#include <Library/MmSpec.h>
#include <Library/SiliconLib.h>

#include <Protocol/SpiNorFlashProtocol.h>

#include <CommonRas.h>

#define  RAS_DEBUG

#define  HEST_REGION_SEA    4
#define  HEST_REGION_SEI    5

typedef enum    _PHYTIUM_STMM_HANDLER_STATUS PHYTIUM_STMM_HANDLER_STATUS;
typedef enum    _APEI_TYPE                   APEI_TYPE;
typedef enum    _PHYTIUM_RAS_MAIN_ID         PHYTIUM_RAS_MAIN_ID;
typedef enum    _GENERATE_GHES_ERROR         GENERATE_GHES_ERROR;
typedef enum    _HEST_HEADER_GET_ERROR       HEST_HEADER_GET_ERROR;
typedef enum    _HEST_TABLE_GET_ERROR        HEST_TABLE_GET_ERROR;
typedef struct  _APEI_SOURCE_TABLE           APEI_SOURCE_TABLE;
typedef struct  _APEI_TABLE_LIST             APEI_TABLE_LIST;
typedef struct  _GHES_REGISTER               GHES_REGISTER;

extern  APEI_TABLE_LIST             mApeiList;
extern  EFI_NORFLASH_DRV_PROTOCOL   *mFlash;
extern  RAS_TOTAL_COUNT             mRasTotalCount;

enum _PHYTIUM_STMM_HANDLER_STATUS {
  PHYTIUM_STMM_HANDLER_SUCCESS = 0,
  PHYTIUM_STMM_HANDLER_HEADER_ERROR = 1,
  PHYTIUM_STMM_HANDLER_LENGTH_ERROR = 2,
  PHYTIUM_STMM_HANDLER_FUNC_ID_UNSUPPORTED = 3,
};

enum  _APEI_TYPE {
  TypeBert = 1,
  TypeHest = 2,
};

#define  BERT_TABLE_MAX_NUM     1
struct _APEI_TABLE_LIST {
  UINT8             BertNum;
//#define  BERT_TABLE_MAX_NUM     1
  APEI_SOURCE_TABLE BertTable[BERT_TABLE_MAX_NUM];
  UINT8             HestNum;
  APEI_SOURCE_TABLE HestTable[MAX_GHES];
};

struct _GHES_REGISTER {
  UINT64  AckRegister;
  UINT64  ErrorStatusBlockAddress;
};

/**
  Init Apei Table List.

  @param[in, out]    List      A pointer to APEI_TABLE_LIST.

  @retval    Null.
**/
VOID
ApeiTableInit (
  IN OUT APEI_TABLE_LIST  *List
  );

/**
  HEST Table initialization. The steps are as follows:
  1.Create the HEST header.
  2.Fill Ghes information.
  3.Add GHES information into the HEST table.
  4.Parse GHES content(GHES Address and ACK Register) into APEI table.

  @retval    EFI_SUCCESS    Success.
  @retval    Other          Failed.

**/
EFI_STATUS
InitHestTable (
  VOID
  );

/**
  Generate GHES data and write it to the corresponding error status address
  described in the HEST table according to differt Severity. The source buffer
  data comes from the content of the MM communicate protocol. For the protocol
  content, refer to Chapter 6.3.4 of the Phytium MM Interface Specification.

  @param[in]    Buffer      The source buffer.
  @param[in]    Length      Length of the source buffer.
  @param[in]    List        The apei list to write error data.

  @retval       GenerateGhesSuccess           Success.
  @retval       GenerateGhesSignatureError    Signature of cper data is error.
  @retval       GenerateGhesTimeout           Write ghes data timeout. It means
                                              checking ack register timeout.
**/
VOID
GenerateHest (
  IN UINT64  Addr,
  IN UINT8   *Info,
  IN UINT32  Length
  );

/**
  Ras sub event handle. The source buffer data comes from the content of the MM
  communicate protocol. For the protocol content, refer to Chapter 6.3.4 of the
  Phytium MM Interface Specification.

  @param[in]      SubId      Sub event ID, refer to Chapter 6.3.4 of the Phytium
                             MM Interface Specification.
  @param[in,out]  Buffer     The content section in MM communicate buffer, refer
                             to Chapter 6.3.4 of the Phytium MM Interface
                             Specification.
  @param[in,out]  Length     Length of content.

  @retval    EFI_SUCCESS      Success.
  @retval    Other            Failed.
**/
UINT32
RasMainFunctionHandle (
  IN       UINT16  SubId,
  IN  OUT  UINT8   *Buffer,
  IN  OUT  UINT32  *Length
  );

#endif
