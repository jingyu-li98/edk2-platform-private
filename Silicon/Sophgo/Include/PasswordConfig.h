/** @file
  The header file of HII Config Access protocol implementation of
  password configuration module.

  Copyright (c) 2024, Sophgo Technologies Ltd. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  PASSWORD_CONFIG_H_
#define  PASSWORD_CONFIG_H_


/**
  Calculate SHA256 hash for the input string.

  @param[in]  InputString    Input string to calculate hash
  @param[out] HashValue     Calculated SHA256 hash value

  @retval EFI_SUCCESS       Hash calculation successful
  @retval EFI_INVALID_PARAMETER Invalid input parameters
  @retval EFI_DEVICE_ERROR  Hash calculation failed
**/
EFI_STATUS
CalculatePasswordHash (
  IN  CONST CHAR16  *InputString,
  OUT UINT8         *HashValue
  );
#endif
