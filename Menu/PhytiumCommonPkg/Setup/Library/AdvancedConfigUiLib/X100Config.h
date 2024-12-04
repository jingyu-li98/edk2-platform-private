/** @file
Some defines of X100 configuration.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  X100_CONFIG_H_
#define  X100_CONFIG_H_

#include <Library/HiiLib.h>

VOID
CreateX100ConfigMenu (
  IN EFI_HII_HANDLE              HiiHandle,
  IN VOID                        *StartOpCodeHandle
  );

/**
  Get X100 firmware version and build time.

  @param[in]  Handle    Hii handle.

  @retval     EFI_SUCCESS    Success.
  @retval     Other          Failed.
**/
EFI_STATUS
GetX100BaseInfo (
  IN EFI_HII_HANDLE  Handle
  );

/**
  Check whether X100 exists.

  @retval   1     Existed.
  @retval   0     Not existed.
**/
UINT32
IsX100Existed (
  VOID
  );

#endif
