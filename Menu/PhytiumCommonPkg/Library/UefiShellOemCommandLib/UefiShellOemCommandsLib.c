/** @file
The library of the OEM commands.

Copyright (C) 2022-2023, Phytium Technology Co., Ltd. All rights reserved<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include "BiosUpdate.h"
#include "UefiShellOemCommandsLib.h"

CONST CHAR16 gShellOemFileName[] = L"ShellOemCommands";
EFI_HANDLE gShellOemHiiHandle = NULL;

/**
  return the file name of the help text file if not using HII.

  @return The string pointer to the file name.
**/
CONST CHAR16*
EFIAPI
ShellCommandGetManFileNameOem (
  VOID
  )
{
  return (gShellOemFileName);
}

/**
  Constructor for the Shell Network1 Commands library.

  Install the handlers for Network1 UEFI Shell 2.0 profile commands.

  @param ImageHandle            The image handle of the process.
  @param SystemTable            The EFI System Table pointer.

  @retval EFI_SUCCESS           The shell command handlers were installed sucessfully.
  @retval EFI_UNSUPPORTED       The shell level required was not found.
**/
EFI_STATUS
EFIAPI
ShellOemCommandsLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  gShellOemHiiHandle = NULL;

  //
  // check our bit of the profiles mask
  //
  if ((PcdGet8(PcdShellProfileMask) & BIT3) == 0) {
    return (EFI_SUCCESS);
  }

  gShellOemHiiHandle = HiiAddPackages (
    &gShellOemHiiGuid,
    gImageHandle,
    UefiShellOemCommandsLibStrings,
    NULL
    );
  if (gShellOemHiiHandle == NULL) {
    return (EFI_DEVICE_ERROR);
  }
  //
  // install our shell command handlers
  //
  ShellCommandRegisterCommandName (
    L"Bootefi",
    ShellCommandRunBootefi,
    ShellCommandGetManFileNameOem,
    0,
    L"Bootefi",
    TRUE ,
    gShellOemHiiHandle,
    STRING_TOKEN(STR_GET_HELP_OEM_BOOTEFI)
    );

  ShellCommandRegisterCommandName (L"MemCp",
    ShellCommandRunMemCp,
    ShellCommandGetManFileNameOem,
    0,
    L"MemCp",
    TRUE ,
    gShellOemHiiHandle,
    STRING_TOKEN(STR_GET_HELP_OEM_MEMCP)
    );

  ShellCommandRegisterCommandName (L"updatebios",
    ShellCommandRunBiosUpdate,
    ShellCommandGetManFileNameOem,
    0,
    L"updatebios",
    TRUE ,
    gShellOemHiiHandle,
    STRING_TOKEN (STR_GET_HELP_BIOSUPDATE)
    );

  ShellCommandRegisterCommandName (L"Boot",
    ShellCommandRunBoot,
    ShellCommandGetManFileNameOem,
    0,
    L"Boot",
    TRUE ,
    gShellOemHiiHandle,
    STRING_TOKEN(STR_GET_HELP_BOOT)
    );

  return (EFI_SUCCESS);
}

/**
  Destructor for the library.  free any resources.

  @param ImageHandle            The image handle of the process.
  @param SystemTable            The EFI System Table pointer.
**/
EFI_STATUS
EFIAPI
ShellOemCommandsLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  if (gShellOemHiiHandle != NULL) {
    HiiRemovePackages(gShellOemHiiHandle);
  }
  return (EFI_SUCCESS);
}

