/**
Header file of main formset defines.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef SETUP_FORMINIT_H_
#define SETUP_FORMINIT_H_

CHAR16 *
GetToken (
  IN EFI_STRING_ID   Token,
  IN EFI_HII_HANDLE  HiiHandle
  );


EFI_STATUS
GetDynamicInfo (
  IN EFI_HII_HANDLE  Handle
  );

/**
  Main Form Init.

  @param[in]  Handle    Hii handle.

  @retval     EFI_SUCCESS    Success.
  @retval     Other          Failed.
**/
EFI_STATUS
MainFormInit (
  IN EFI_HII_HANDLE    Handle
);

#endif
