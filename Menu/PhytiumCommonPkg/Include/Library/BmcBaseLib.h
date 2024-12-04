/** @file
Define some base function about BMC.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef BMC_BASE_LIB_H_
#define BMC_BASE_LIB_H_

/**
  Set BMC Communicate GPIO.

  @retval  EFI_SUCCESS    BMC is in place and set successfully.
  @retval  EFI_NOT_FOUND  BMC is not in place.

**/
EFI_STATUS
EFIAPI
BmcGpioSet (
  IN UINT8 Die,
  IN UINT8 Index,
  IN UINT8 Port,
  IN UINT8 Level
  );

/**
  Check BMC in place or not.

  @retval  EFI_SUCCESS    BMC is in place.
  @retval  EFI_NOT_FOUND  BMC is not in place.

**/
EFI_STATUS
EFIAPI
CheckBmcInPlace (
  VOID
  );

/**
  Initialized the Kcs Interface.

**/
VOID
KcsInit (
  VOID
  );

#endif
