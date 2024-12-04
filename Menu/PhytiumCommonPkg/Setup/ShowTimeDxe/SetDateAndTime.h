/** @file

Copyright (c) 2018 Loongson Technology Corporation Limited (www.loongson.cn).
All intellectual property rights(Copyright, Patent and Trademark) reserved.

Copyright (c) 2007 - 2017, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _SET_DATETIME_H_
#define _SET_DATETIME_H_

#include <Uefi.h>
#include <Protocol/HiiConfigRouting.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiString.h>
#include <Protocol/HiiPopup.h>
#include <Guid/MdeModuleHii.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HiiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Guid/HiiPlatformSetupFormset.h>
#include <Guid/HiiFormMapMethodGuid.h>
#include <Guid/DriverSampleHii.h>
#include <Guid/ZeroGuid.h>
#include "SetTimeNv.h"

extern UINT8  ShowTimeVfrBin[];
extern UINT8  SetDateAndTimeStrings[];

#define SET_DATETIME_PRIVATE_SIGNATURE SIGNATURE_32 ('S', 'D', 'p', 's')

typedef struct {
  UINTN                            Signature;
  EFI_HANDLE                       DriverHandle;
  EFI_HII_HANDLE                   HiiHandle;
  //
  // Consumed protocol
  //
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *HiiConfigRouting;
  //
  // Produced protocol
  //
  EFI_HII_CONFIG_ACCESS_PROTOCOL   ConfigAccess;
} DATE_TIME_PRIVATE_DATA;

#define SET_DATEANDTIME_PRIVATE_FROM_THIS(a)  CR (a, DATE_TIME_PRIVATE_DATA, ConfigAccess, SET_DATETIME_PRIVATE_SIGNATURE)
///
/// HII specific Vendor Device Path definition.
///
typedef struct {
  VENDOR_DEVICE_PATH             VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL       End;
} HII_VENDOR_DEVICE_PATH;

#endif
