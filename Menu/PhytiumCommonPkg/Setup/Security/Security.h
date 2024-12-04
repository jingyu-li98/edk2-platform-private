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
#include <Library/UefiBootServicesTableLib.h>
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
#include <Library/MemoryAllocationLib.h>
#include <Library/HiiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Guid/HiiPlatformSetupFormset.h>
#include <Guid/HiiFormMapMethodGuid.h>
#include <Guid/DriverSampleHii.h>
#include <Guid/ZeroGuid.h>
#include <Protocol/HiiDatabase.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/HiiLib.h>
#include "SecurityNv.h"

extern UINT8  SecurityVfrBin[];
extern UINT8  SecurityPageStrings[];

#define SECURITY_PAGE_PRIVATE_SIGNATURE SIGNATURE_32 ('S', 'P', 'p', 's')

#define SECURITY_HII_DRIVER_LIST_SIZE               0x20
#define SECURITY_PAGE_KEY_DRIVER                    0x2000

typedef struct {
  EFI_STRING_ID   PromptId;
  EFI_STRING_ID   HelpId;
  EFI_STRING_ID   DevicePathId;
  EFI_GUID        FormSetGuid;
  BOOLEAN         EmptyLineAfter;
} SECURITY_HII_INSTANCE;

/**
  Rename the driver name if necessary.

  @param    DriverName          Input the driver name.
  @param    NewDriverName       Return the new driver name.
  @param    EmptyLineAfter      Whether need to insert empty line.

  @retval   New driver name if compared, else NULL.

**/
typedef
BOOLEAN
(EFIAPI *DRIVER_SPECIAL_HANDLER)(
  IN  CHAR16                   *DriverName,
  OUT CHAR16                   **NewName,
  OUT BOOLEAN                  *EmptyLineAfter
);

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
} SECURITY_PAGE_PRIVATE_DATA;

///
/// HII specific Vendor Device Path definition.
///
typedef struct {
  VENDOR_DEVICE_PATH             VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL       End;
} HII_VENDOR_DEVICE_PATH;

#endif
