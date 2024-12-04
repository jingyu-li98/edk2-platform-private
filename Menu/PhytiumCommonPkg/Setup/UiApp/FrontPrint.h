/** @file
Some functions definition about print password input interface.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef FRONTPRINT_H_
#define FRONTPRINT_H_

/**
  Get string or password input from user.

  @param  Prompt            The prompt string shown on popup window.
  @param  StringPtr         Old user input and destination for use input string.

  @retval EFI_SUCCESS       If string input is read successfully
  @retval EFI_DEVICE_ERROR  If operation fails

**/
EFI_STATUS
ReadString (
  IN     CHAR16                      *Prompt,
  IN OUT CHAR16                      *StringPtr
  );

#endif
