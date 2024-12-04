/** @file
Phytium advanced configuration header file, such as X100 configuration defines.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef BOOTMANAGER_CONFIG_DATA_H_
#define BOOTMANAGER_CONFIG_DATA_H_

#define VAR_BOOTMANAGER_CONFIG_NAME                 L"BootManagerConfigVar"
#define PLATFORM_SETUP_VARIABLE_FLAG             (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE)

//#pragma pack(1)

typedef struct {
  UINT8         UserPriv;
} BOOTMANAGER_CONFIGURATION_DATA;

//#pragma pack()

#endif
