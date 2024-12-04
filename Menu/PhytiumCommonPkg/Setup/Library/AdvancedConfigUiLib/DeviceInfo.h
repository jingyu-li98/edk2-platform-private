/** @file
Header file of device information formset defines.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  DEVICE_INFO_H_
#define  DEVICE_INFO_H_

#include <Library/HiiLib.h>

#define _48_BIT_ADDRESS_FEATURE_SET_SUPPORTED   0x0400
#define ATAPI_DEVICE                            0x8000

#define  PCIE_CLASSCODE_VALID           0x0C
#define  PCIE_CLASSCODE_RESERVED        0xFE
#define  PCIE_CKASSCODE_MISC            0xFF

#define  TYPE_AHCI  0
#define  TYPE_IDE   1
#define  TYPE_NVME  2

typedef struct _HDD_INFO {
  CHAR8                       FirmwareVer[10];
  CHAR8                       ModuleName[42];
  CHAR8                       SerialNo[22];
  UINT64                      SizeGB;
  UINT32                      Type;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;
} HDD_INFO;

/**
  Get the pcie device information and put to formset.

  @param[in]  Handle       HII Handle.

  @retval     EFI_SUCCESS  Success.
  @retval     Other        Failed.
**/
EFI_STATUS
GetPcieDeviceInfo (
  IN EFI_HII_HANDLE  Handle
  );

/**
  Get the usb controller information and put to formset.

  @param[in]  Handle       HII Handle.

  @retval   EFI_SUCCESS  Success.
  @retval   Other        Failed.
**/
EFI_STATUS
GetUsbControllerInfo (
  IN EFI_HII_HANDLE  Handle
  );

/**
  Get the usb device information and put to formset.

  @param[in]  Handle       HII Handle.

  @retval   EFI_SUCCESS  Success.
  @retval   Other        Failed.
**/
EFI_STATUS
GetUsbDeviceInfo (
  IN EFI_HII_HANDLE  Handle
  );

/**
  Get hdd device list and put to formset.

  @param[in]    Handle         HII Handle.

  @retval   EFI_SUCCESS    Success.
  @retval   Other          Failed.
**/
EFI_STATUS
GetHddDeviceInfo (
  IN EFI_HII_HANDLE  Handle
  );

#endif
