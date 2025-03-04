/** @file
  TPCM Verification Protocol Definitions.

  Copyright (c) 2025, SOPHGO Technologies Inc. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef TPCM_VERIFICATION_H_
#define TPCM_VERIFICATION_H_

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/SecurityManagementLib.h>

//
// TPCM Status Codes
//
#define TPCM_SUCCESS 0
#define TPCM_FAILURE 1

//
// TPCM Measurement Stages
//
#define TPCM_MEASURE_STAGE_LOAD    0x01
#define TPCM_MEASURE_STAGE_EXECUTE 0x02

//
// TPCM Measurement Results
//
#define TPCM_MEASURE_SUCCESS       0x00
#define TPCM_MEASURE_FAILED        0x01

//
// TPCM Control Results
//
#define TPCM_CTRL_DISABLED         0x00
#define TPCM_CTRL_ENFORCED         0x01
#define TPCM_CTRL_BLOCK            0x02

EFI_STATUS
EFIAPI
TpcmVerifyImageRaw (
  IN  TPCM_IMAGE_VERIFY_PROTOCOL *This,
  IN  UINT32                     MeasureStage,
  IN  VOID                       *ImageInfo,
  IN  UINT32                     ImageInfoSize,
  IN  UINT32                     NumAddrRange,
  IN  IMAGE_ADDR_RANGE           Ranges[],
  OUT UINT32                     *MeasureResult,
  OUT UINT32                     *ControlResult
  );

BOOLEAN
EFIAPI
TpcmIsVerifyEnabled (
  IN TPCM_IMAGE_VERIFY_PROTOCOL *This
  );

#endif // TPCM_VERIFICATION_H_
