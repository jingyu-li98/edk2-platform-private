/** @file
Ras UI Information Header File.

Copyright (C) 2024, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef  RASUIDATA_H_
#define  RASUIDATA_H_

#include <CommonRas.h>

#define RAS_CONFIG_VAR L"RasConfigVar"

#pragma pack(1)

#define PLATFORM_SETUP_VARIABLE_FLAG             (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE)

#define RAS_FORMSET_GUID\
  {0x8a431142, 0x4aef, 0x11ee, {0xbc, 0xb9, 0x5f, 0xe9, 0x11, 0xcc, 0x32, 0xcb}}
#define VARSTORE_ID_RAS              0x6100

typedef struct _RAS_CONFIG {
  UINT8         UserPriv;
  UINT8         Reserved[3];
  RAS_STRATEGY  RasStrategy;
} RAS_CONFIG;

#pragma pack()

#endif
