/** @file
  X100 Library Header File.

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef  X100_LIB_H_
#define  X100_LIB_H_

#include <Library/UefiLib.h>
#include <Uefi.h>

BOOLEAN
EFIAPI
IsX100Existed (
  VOID
  );

EFI_STATUS
EFIAPI
GetDpChannel (
  VOID
  );

EFI_STATUS
ControlX100DevEnable (
  VOID
  );

EFI_STATUS
ModifyPciRootPortTag (
  VOID
  );

#endif
