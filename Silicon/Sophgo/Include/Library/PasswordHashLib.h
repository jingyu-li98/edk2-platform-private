/** @file
  The header file of Password Hash Library.

  Copyright (c) 2025, Sophgo Technologies Ltd. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef __PASSWORD_HASH_LIB_H__
#define __PASSWORD_HASH_LIB_H__

#include <Uefi.h>

#define SHA256_DIGEST_SIZE 32

/**
  Initialize the Hash2 protocol for SHA256 calculation.

  @retval EFI_SUCCESS       Hash protocol initialized successfully
  @retval Others           Failed to initialize hash protocol
**/
EFI_STATUS
EFIAPI
PasswordHashInit (
  VOID
  );

/**
  Calculate hash value for the input password string.

  @param[in]   InputString     The input password string to be hashed
  @param[out]  HashValue      Buffer to store the calculated hash value

  @retval EFI_SUCCESS             Hash calculation succeeded
  @retval EFI_INVALID_PARAMETER   Invalid input parameters
  @retval EFI_OUT_OF_RESOURCES    Insufficient resources to perform hash
**/
EFI_STATUS
EFIAPI
PasswordHashCalculate (
  IN  CONST CHAR16  *InputString,
  OUT UINT8        *HashValue
  );

/**
  Compare two password hash values.

  @param[in]  Hash1     First hash value to compare
  @param[in]  Hash2     Second hash value to compare

  @retval TRUE     Hash values match
  @retval FALSE    Hash values do not match
**/
BOOLEAN
EFIAPI
PasswordHashCompare (
  IN CONST UINT8  *Hash1,
  IN CONST UINT8  *Hash2
  );

#endif // PASSWORD_HASH_LIB_H_ 