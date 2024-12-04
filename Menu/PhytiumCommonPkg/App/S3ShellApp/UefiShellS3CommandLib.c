/** @file
  S3 module test application.

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "UefiShellS3CommandLib.h"

CONST CHAR16 ShellS3FileName[] = L"ShellS3Command";
EFI_HANDLE ShellS3HiiHandle = NULL;

STATIC CONST SHELL_PARAM_ITEM ParamList[] = {
  {L"help", TypeFlag},
  {NULL , TypeMax}
  };

/**
  Return the file name of the help text file if not using HII.

  @retval ShellI2cFileName The string pointer to the file name.
**/
STATIC
CONST CHAR16 *
EFIAPI
ShellCommandGetManFileNameUart (
  VOID
  )
{
  return ShellS3FileName;
}

/**
  Print the help information.
**/
STATIC
VOID
Usage (
  VOID
  )
{
  Print (L"S3 test app:\n"
         L"s3\n"
         );
}

/**
Handle the command of UartTestApp.

@param[in] Argc    The number of input parameters.

@param[in] Argv    The string of input parameters.

@retval EFI_SUCCESS   The command is executed successfully.

**/
SHELL_STATUS
EFIAPI
ShellCommandRunS3 (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  LIST_ENTRY    *CheckPackage;
  EFI_STATUS    Status;
  CHAR16        *ProblemParam;
  ARM_SMC_ARGS  ArmSmcArgs;
  UINT64        UefiStartAddress;

  Status = SHELL_SUCCESS;

  ShellInitialize ();
  ShellCommandLineParse (ParamList, &CheckPackage, &ProblemParam, TRUE);

  if (ShellCommandLineGetFlag (CheckPackage, L"help")) {
    Usage();
    return SHELL_SUCCESS;
  }

  UefiStartAddress = PcdGet64 (PcdFdBaseAddress);

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = 0xC400000E;
  ArmSmcArgs.Arg1 = UefiStartAddress;
  ArmSmcArgs.Arg2 = 0;
  ArmCallSmc (&ArmSmcArgs);

  S3SuspendEndEntry();

  return Status;
}

/**
  Constructor for the Shell Rtc Commands library.

  Install the handlers for Rtc UEFI Shell 2.0 profile commands.

  @param[in] ImageHandle    The image handle of the process.
  @param[in] SystemTable    The EFI System Table pointer.

  @retval EFI_SUCCESS        The shell command handlers were installed sucessfully.
  @retval EFI_UNSUPPORTED    The shell level required was not found.
**/
EFI_STATUS
EFIAPI
ShellS3CmdLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  DEBUG ((DEBUG_INFO, " CREAT SHELL COMMAND SUCCESS\n"));
  ShellS3HiiHandle = NULL;

  ShellS3HiiHandle = HiiAddPackages (
                           &gShellS3HiiGuid, gImageHandle,
                           UefiShellS3LibStrings, NULL
                           );

  if (ShellS3HiiHandle == NULL) {
    Print (L"Filed to add Hii package\n");
    return EFI_DEVICE_ERROR;
  }

  //
  // install our shell command handlers
  //
  ShellCommandRegisterCommandName (
    L"s3", ShellCommandRunS3, ShellCommandGetManFileNameUart, 0,
    L"s3", TRUE , ShellS3HiiHandle, STRING_TOKEN (STR_GET_HELP_S3) );

  return EFI_SUCCESS;

}

/**
  Destructor for the library.  free any resources.

  @param[in] ImageHandle    The image handle of the process.
  @param[in] SystemTable    The EFI System Table pointer.

  @retval EFI_SUCCESS   Always returned.
**/
EFI_STATUS
EFIAPI
ShellS3CmdLibDestructor (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  if (ShellS3HiiHandle != NULL) {
    HiiRemovePackages (ShellS3HiiHandle);
  }

  return EFI_SUCCESS;
}
