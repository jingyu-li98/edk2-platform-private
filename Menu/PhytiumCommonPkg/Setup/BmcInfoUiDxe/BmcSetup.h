/** @file
The header file of bmc configuration data.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef    BMC_SETUP_H_
#define    BMC_SETUP_H_

//bmc variable guid
#define BMC_SETUP_VARIABLE_GUID  {0x8b17ce34, 0x99b1, 0x11ea, {0xb8, 0x35, 0x83, 0xf8, 0x44, 0xa3, 0x55, 0xee}}
//
// These are defined as the same with vfr file
//
#define BMC_INFO_FORMSET_GUID  {0x88d9d86c, 0x85f5, 0x11ea, {0x99, 0xed, 0x4b, 0x46, 0x73, 0xab, 0xbb, 0x7e}}


#define SETUP_BMC_CFG_GUID {0x8236697e, 0xf0f6, 0x405f, {0x99, 0x13, 0xac, 0xbc, 0x50, 0xaa, 0x45, 0xd1}}

#define PLATFORM_SETUP_VARIABLE_FLAG             (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE)

#define BMC_INFO_FORM_ID      0x8004
#define VARSTORE_BMC_ID       0x8100
#define MAX_STRING_LEN        200

#define KEY_IP                0x8801
#define KEY_GW                0x8802
#define KEY_SUBNETMASK        0x8803

#define KEY_POLICY            0x8804
#define KEY_MAIN              0x8881

#define KEY_PASSWORD_BASE     0x8900
#define KEY_PASSWORD_1        0x8901
#define KEY_PASSWORD_2        0x8902
#define KEY_PASSWORD_3        0x8903
#define KEY_PASSWORD_4        0x8904
#define KEY_PASSWORD_5        0x8905
#define KEY_PASSWORD_6        0x8906
#define KEY_PASSWORD_7        0x8907
#define KEY_PASSWORD_8        0x8908
#define KEY_PASSWORD_9        0x8909
#define KEY_PASSWORD_10       0x890a
#define KEY_PASSWORD_11       0x890b
#define KEY_PASSWORD_12       0x890c
#define KEY_PASSWORD_13       0x890d
#define KEY_PASSWORD_14       0x890e
#define KEY_PASSWORD_15       0x890f

#define POWER_UP              1
#define POWER_RESTORE         2
#define POWER_OFF             3
#define POWER_UNKNOWN         4

#define NO_ACTION             0x1
#define HARD_RESET            0x2
#define POWER_DONW            0x3
#define POWER_CYCLE           0x4

#define BMC_CONFIG_STORE      L"BmcSetup"

#define WATCHDOG_CONFIG_VAR   L"WatchdogConfig"

#define MIN_BMC_USER_LENGTH   6
#define MAX_BMC_USER_LENGTH   20

#define MAX_USER_NUMBER       0xf

typedef  struct {
  UINT8    NicIpSource;
  UINT8    Reserved1;
  UINT16   IpAddress[20];
  UINT16   SubnetMask[20];
  UINT16   GatewayIp[20];
  UINT16   DhcpName[64];
  UINT8    RestorePolicy;
  UINT8    WatchdogPost;
  UINT16   WatchdogTimeoutPost;
  UINT8    WatchdogTimeoutActionPost;
  UINT8    WatchdogOS;
  UINT16   WatchdogTimeoutOS;
  UINT8    WatchdogTimeoutActionOS;
  UINT8    UserTotalNum;
} BMC_CONFIG;

#define BMC_SETUP_DATA_VARSTORE \
  efivarstore BMC_CONFIG,  varid = VARSTORE_BMC_ID,  name  = BmcSetup,  guid  = BMC_INFO_FORMSET_GUID;


#endif
