/** @file
  This file is used to implement TPCM communication.

  Copyright (c) 2025, SOPHGO Technologies Inc. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef _SOPHGO_TPCM_H_
#define _SOPHGO_TPCM_H_

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#include <Include/DwSpi.h>
#include <Include/Tpcm.h>

#include <Library/DxeServicesTableLib.h>
#include <Library/UefiLib.h>

EFI_STATUS
EFIAPI
TpcmInit (
  VOID
  );

EFI_STATUS
EFIAPI
TpcmMeasure (
  IN CONST UINT8  *Hash,
  IN UINTN        DataSize
  )

EFI_STATUS
EFIAPI
TpcmGetLicenseRemainingDays (
  IN OUT UINT32      *ShelfLife
  );

EFI_STATUS
EFIAPI
TpcmGetStatus (
  IN OUT TPCM_STATUS *TpcmInfo
  );

EFI_STATUS
EFIAPI
TpcmGetControlStatus (
  IN     TPCM_STATUS *TpcmInfo,
  IN OUT BOOLEAN     *IsEnabled
  );

EFI_STATUS
EFIAPI
TpcmSetControlStatus (
  IN TPCM_STATUS     *TpcmInfo,
  IN BOOLEAN         IsEnabled
  );
#endif // _SOPHGO_TPCM_H_
