/** @file
  The Platform Initialization.

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Library/PcdLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

/**
  Set PcdPlatformBootTimeOut if its configured.
**/
VOID
InitializeBootTimeout (
  VOID
  )
{
  EFI_STATUS      Status;
  UINT16          TimeOutTemp;
  UINTN           LenTemp;

  TimeOutTemp = 0;
  LenTemp = sizeof(UINT16);
  Status = gRT->GetVariable (
                  L"Timeout",
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &LenTemp,
                  &TimeOutTemp
                  );

  if (Status == EFI_SUCCESS) {
    PcdSet16S (PcdPlatformBootTimeOut, TimeOutTemp);
  }
}

/**
  The user entry point for the PlatformDxe module. The user code starts with
  this function.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS   The entry point is executed successfully.
  @retval other         Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
PlatformDxeInitialise (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  InitializeBootTimeout();

  return EFI_SUCCESS;
}
