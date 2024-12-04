/** @file
  Security Page String support

Copyright (c) 2004 - 2015, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#include "Security.h"

EFI_HII_HANDLE gStringPackHandle;

EFI_GUID mSecurityStringPackGuid = {
  0xba682829, 0x64e1, 0x164a, { 0xf9, 0x95, 0xc9, 0xe4, 0x20, 0xfa, 0x7c, 0xeb }
};

/**
  Initialize HII global accessor for string support.

**/
VOID
InitializeStringSupport (
  VOID
  )
{
  gStringPackHandle = HiiAddPackages (
                         &mSecurityStringPackGuid,
                         gImageHandle,
                         SecurityPageStrings,
                         NULL
                         );
  ASSERT (gStringPackHandle != NULL);
}

/**
  Remove the string package.

**/
VOID
UninitializeStringSupport (
  VOID
  )
{
  HiiRemovePackages (gStringPackHandle);
}

/**
  Get string by string id from HII Interface


  @param Id              String ID.

  @retval  CHAR16 *  String from ID.
  @retval  NULL      If error occurs.

**/
CHAR16 *
GetStringById (
  IN  EFI_STRING_ID   Id
  )
{
  return HiiGetString (gStringPackHandle, Id, NULL);
}

