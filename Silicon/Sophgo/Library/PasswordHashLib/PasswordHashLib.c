/** @file
  Implementation of Password Hash Library.

  Copyright (c) 2025, Sophgo Technologies Ltd. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Protocol/Hash2.h>

#define SHA256_DIGEST_SIZE 32
static EFI_HASH2_PROTOCOL  *mHash2Protocol = NULL;

/**
  Initialize the Hash2 protocol for SHA256 calculation.

  @retval EFI_SUCCESS       Hash protocol initialized successfully
  @retval Others           Failed to initialize hash protocol
**/
EFI_STATUS
EFIAPI
PasswordHashInit (
  VOID
  )
{
  EFI_STATUS  Status;

  if (mHash2Protocol != NULL) {
    return EFI_SUCCESS;
  }

  Status = gBS->LocateProtocol (
                  &gEfiHash2ProtocolGuid,
                  NULL,
                  (VOID **)&mHash2Protocol
                  );
  return Status;
}

/**
  Calculate SHA256 hash for the input string.

  @param[in]  InputString    Input string to calculate hash
  @param[out] HashValue     Calculated SHA256 hash value

  @retval EFI_SUCCESS       Hash calculation successful
  @retval EFI_INVALID_PARAMETER Invalid input parameters
  @retval EFI_DEVICE_ERROR  Hash calculation failed
**/
EFI_STATUS
EFIAPI
PasswordHashCalculate (
  IN  CONST CHAR16  *InputString,
  OUT UINT8         *HashValue
  )
{
  EFI_STATUS        Status;
  UINTN             InputSize;
  UINT8             *InputBuffer;
  EFI_HASH2_OUTPUT  HashOutput;

  if (InputString == NULL || HashValue == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = PasswordHashInit ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  InputSize = StrLen (InputString) * sizeof (CHAR16);
  InputBuffer = (UINT8 *)InputString;

  Status = mHash2Protocol->HashInit (mHash2Protocol, &gEfiHashAlgorithmSha256Guid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = mHash2Protocol->HashUpdate (
                            mHash2Protocol,
                            InputBuffer,
                            InputSize
                            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = mHash2Protocol->HashFinal (
                            mHash2Protocol,
                            &HashOutput
                            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CopyMem (HashValue, HashOutput.Sha256Hash, SHA256_DIGEST_SIZE);
  return EFI_SUCCESS;
}

/**
  Compare two password hashes.

  @param[in]  Hash1    First hash value to compare
  @param[in]  Hash2    Second hash value to compare

  @retval TRUE         Hashes match
  @retval FALSE        Hashes do not match
**/
BOOLEAN
EFIAPI
PasswordHashCompare (
  IN  CONST UINT8  *Hash1,
  IN  CONST UINT8  *Hash2
  )
{
  if (Hash1 == NULL || Hash2 == NULL) {
    return FALSE;
  }

  return (CompareMem (Hash1, Hash2, SHA256_DIGEST_SIZE) == 0);
} 