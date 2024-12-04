/** @file
Phytium advanced configuration header file, such as X100 configuration defines.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef ADVANCED_CONFIG_DATA_H_
#define ADVANCED_CONFIG_DATA_H_

#define VAR_ADVANCED_CONFIG_NAME                 L"AdvancedConfigVar"
#define VAR_X100_CONFIG                          L"X100ConfigVar"
#define VAR_X100_PCI_INFO                        L"X100PciInfoVar"
#define PLATFORM_SETUP_VARIABLE_FLAG             (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE)
#define VAR_X100_EDP_LIGHT                       L"X100EdpLightVar"

#pragma pack(1)

typedef struct _X100_SATA_PORT {
  UINT8  IsUsed;        //1 - used, 0 - not used
  UINT8  Enable;        //1 - enable, 0 - disable
} X100_SATA_PORT;

typedef struct _X100_DP_CONFIG {
  UINT8  IsUsed;
  UINT8  DownSpreadEnable;
} X100_DP_CONFIG;

typedef struct _X100_CONFIG {
  UINT8           UsbEnable;                   //0 - disable, 1 - enable
  UINT8           SataEnable;                  //0 - disable, 1 - enable
  X100_SATA_PORT  SataPort[4];
  UINT8           DisplayEnable;               //0 - disable, 1 - enable
  X100_DP_CONFIG  X100DpConfig[3];
// pcie control
  UINT8           PcieX2Dn4Enable;             //0 - disable, 1 - enable
  UINT8           PcieX2Dn5Enable;             //0 - disable, 1 - enable
  UINT8           PcieX1Dn6Enable;             //0 - disable, 1 - enable
  UINT8           PcieX1Dn7Enable;             //0 - disable, 1 - enable
  UINT8           PcieX1Dn8Enable;             //0 - disable, 1 - enable
  UINT8           PcieX1Dn9Enable;             //0 - disable, 1 - enable
} X100_CONFIG;

typedef struct _X100_DEV_PCI_INFO {
  UINT8         Enable;                      //0 - disable, 1 - enable
  UINT64        Seg;
  UINT64        Bus;
  UINT64        Dev;
  UINT64        Func;
} X100_DEV_PCI_INFO;

typedef struct {
  UINT8         X100EdpLight;
  //X100
  X100_CONFIG   X100Config;
  UINT8         UserPriv;
  UINT8         AutoReset;
} ADVANCED_CONFIG_DATA;

typedef struct X100_PCI_INFO {
  UINT8              X100IsExisted;          //0 - not existed, 1 - existed
  UINT8              DpChannel;
  X100_DEV_PCI_INFO  UsbSwitch;
  X100_DEV_PCI_INFO  SataSwitch;
  X100_DEV_PCI_INFO  DisplaySwitch;
} X100_PCI_INFO;

#pragma pack()

#endif
