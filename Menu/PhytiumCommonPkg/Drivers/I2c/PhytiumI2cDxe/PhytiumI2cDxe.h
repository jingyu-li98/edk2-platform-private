/** @file
  Phytium I2c Drivers.

  This driver implement Enumerate and  StartRequest function.

  Copyright (C) 2016, Marvell International Ltd. All rights reserved.<BR>
  Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef PHYTIUM_I2C_DXE_H_
#define PHYTIUM_I2C_DXE_H_

#include <Protocol/I2cMaster.h>
#include <Protocol/I2cEnumerate.h>
#include <Protocol/I2cBusConfigurationManagement.h>
#include <Protocol/DevicePath.h>
#include <Protocol/PhytiumI2c.h>

#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Uefi/UefiBaseType.h>
#include <Library/TimerLib.h>
#include <Library/I2cLib.h>
#include <Library/PhytiumStructPcd.h>


EFI_STATUS
EFIAPI
PhytiumI2cInitialise (
  IN EFI_HANDLE  ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
);


EFI_STATUS
EFIAPI
PhytiumI2cInitialiseController(
  IN EFI_HANDLE            ImageHandle,
  IN EFI_SYSTEM_TABLE      *SystemTable,
  IN EFI_PHYSICAL_ADDRESS  BaseAddress
  );


EFI_STATUS
EFIAPI
PhytiumI2cReset(
  IN CONST EFI_I2C_MASTER_PROTOCOL  *This
  );


EFI_STATUS
EFIAPI
PhytiumI2cStartRequest (
  IN CONST EFI_I2C_MASTER_PROTOCOL  *This,
  IN UINTN                          SlaveAddress,
  IN EFI_I2C_REQUEST_PACKET         *RequestPacket,
  IN EFI_EVENT                      Event     OPTIONAL,
  OUT EFI_STATUS                    *I2cStatus OPTIONAL
  );


EFI_STATUS
PhytiumI2cAllocDevice(
  IN UINT32                    SlaveAddress,
  IN UINT8                     Bus,
  IN OUT CONST EFI_I2C_DEVICE  **Device
  );


EFI_STATUS
EFIAPI
PhytiumI2cEnumerate(
  IN CONST EFI_I2C_ENUMERATE_PROTOCOL  *This,
  IN OUT CONST EFI_I2C_DEVICE          **Device
  );


EFI_STATUS
EFIAPI
PhytiumI2cEnableConf(
  IN CONST EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL  *This,
  IN UINTN                                                I2cBusConfiguration,
  IN EFI_EVENT                                            Event     OPTIONAL,
  IN EFI_STATUS                                           *I2cStatus OPTIONAL
  );

#endif /* PHYTIUM_I2C_DXE_H_ */
