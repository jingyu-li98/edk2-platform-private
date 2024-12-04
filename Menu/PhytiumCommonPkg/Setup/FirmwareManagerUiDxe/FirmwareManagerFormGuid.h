/** @file
The header file of Firmware manager form guid defines.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  FIRMWAREMANAGER_FORM_GUID_H_
#define  FIRMWAREMANAGER_FORM_GUID_H_

#define FIRMWARE_MANAGER_FORMSET_GUID\
  { \
  0xce101d62, 0x900d, 0x11ee, {0xb4, 0x6c, 0xf7, 0x9e, 0x7a, 0x18, 0x57, 0x9d} \
  }

#define  FIRMWARE_MANAGER_VARIABLE            L"FirmwareManagerSetup"
#define  FORM_FIRMWAREMANAGER_ID              0x3000

#define  FORM_FIRMWAREMANAGER_OPEN            0x3f00


#define  FORM_TEMP                            0x3100
#define  UPDATEBIOS_KEY                       0x3200

#define  LABEL_FORM_FIRMWAREMANAGER_START     0xff0c
#define  LABEL_FORM_FIRMWAREMANAGER_END       0xff0d
#define  LABEL_END                           0xfe12

#endif
