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

#ifndef _INFORMATION_H_
#define _INFORMATION_H_

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
#include <Protocol/Smbios.h>
#include <IndustryStandard/SmBios.h>
#include <Library/HobLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/AtaPassThru.h>
#include <IndustryStandard/Pci.h>
#include <IndustryStandard/Atapi.h>
#include <IndustryStandard/Scsi.h>
#include <Protocol/PciIo.h>
#include <Protocol/IdeControllerInit.h>
#include <Protocol/ScsiPassThruExt.h>
#include <Library/PciLib.h>
#include <IndustryStandard/Nvme.h>
#include <Protocol/DevicePath.h>
#include <Protocol/NvmExpressPassthru.h>

#include "InformationNv.h"

extern UINT8  InformationVfrBin[];
extern UINT8  InformationStrings[];

#define SET_INFORMATION_PRIVATE_SIGNATURE SIGNATURE_32 ('S', 'I', 'p', 's')

typedef struct {
  UINT8   SerialNumber[21];
  UINT8   ModeName[21];
  UINT32  HardDiskTotalNum;
} SATADRIVE_HARD_DISK_INFO;

typedef struct {
  UINTN                            Signature;
  EFI_HANDLE                       DriverHandle;
  EFI_HII_HANDLE                   HiiHandle;
  SATADRIVE_HARD_DISK_INFO         Configuration;
  //
  // Consumed protocol
  //
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *HiiConfigRouting;
  //
  // Produced protocol
  //
  EFI_HII_CONFIG_ACCESS_PROTOCOL   ConfigAccess;
} INFORMATION_PRIVATE_DATA;

///
/// HII specific Vendor Device Path definition.
///
typedef struct {
  VENDOR_DEVICE_PATH             VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL       End;
} HII_VENDOR_DEVICE_PATH;

#endif
