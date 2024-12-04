/** @file
  Head file for BDS Platform specific code

  Copyright (C) 2022, Phytium Technology Co., Ltd. All rights reserved.<BR>
  Copyright (C) 2015-2016, Red Hat, Inc.
  Copyright (c) 2004 - 2008, Intel Corporation. All rights reserved.<BR>
  Copyright (c) 2016, Linaro Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef PLATFORM_BM_H_
#define PLATFORM_BM_H_

#include <Guid/BootDiscoveryPolicy.h>
#include <Guid/EventGroup.h>
#include <Guid/NonDiscoverableDevice.h>
#include <Guid/TtyTerm.h>
#include <Guid/SerialPortLibVendor.h>

#include <IndustryStandard/Pci22.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BmcBaseLib.h>
#include <Library/BootLogoLib.h>
#include <Library/BmcConfigBootLib.h>
#include <Library/CapsuleLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/HobLib.h>
#include <Library/IpmiDeviceInfoToBmc.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PhytiumCommonGpioLib.h>
#include <Library/PhytiumPostCodeLib.h>
#include <Library/PhytiumPowerControlLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/X100Lib.h>

#include <Protocol/BootManagerPolicy.h>
#include <Protocol/DevicePath.h>
#include <Protocol/EsrtManagement.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/NonDiscoverableDevice.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PlatformBootManager.h>

#include "AdvancedConfigData.h"
/**
  Use SystemTable Conout to stop video based Simple Text Out consoles from
  going to the video device. Put up LogoFile on every video device that is a
  console.

  @param[in]  LogoFile   File name of logo to display on the center of the
                         screen.

  @retval EFI_SUCCESS     ConsoleControl has been flipped to graphics and logo
                          displayed.
  @retval EFI_UNSUPPORTED Logo not found
**/
EFI_STATUS
EnableQuietBoot (
  IN  EFI_GUID  *LogoFile
  );

/**
  Use SystemTable Conout to turn on video based Simple Text Out consoles. The
  Simple Text Out screens will now be synced up with all non video output
  devices

  @retval EFI_SUCCESS     UGA devices are back in text mode and synced up.
**/
EFI_STATUS
DisableQuietBoot (
  VOID
  );

#endif // PLATFORM_BM_H_
