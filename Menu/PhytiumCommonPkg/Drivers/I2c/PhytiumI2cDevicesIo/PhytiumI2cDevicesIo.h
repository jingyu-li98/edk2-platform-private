/** @file
  Phytium I2c Device Driver.

  This driver provides the Transfer function to call the QueueRequest() of
  IoProtocol to read or write the i2c slave device.

  Copyright (C) 2016, Marvell International Ltd. All rights reserved.<BR>
  Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef PHYTIUM_I2C_DEVICES_IO_H_
#define PHYTIUM_I2C_DEVICES_IO_H_

#include <Uefi.h>

#define MAX_BUFFER_LENGTH 64

extern EFI_COMPONENT_NAME_PROTOCOL   gPhytiumI2cDevicesIoComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL  gPhytiumI2cDevicesIoComponentName2;

EFI_STATUS
EFIAPI
PhytiumI2cDevicesIoComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME2_PROTOCOL  *This,
  IN  CHAR8                         *Language,
  OUT CHAR16                        **DriverName
  );


EFI_STATUS
EFIAPI
PhytiumI2cDevicesIoComponentNameGetControllerName (
  IN   EFI_COMPONENT_NAME2_PROTOCOL  *This,
  IN   EFI_HANDLE                    ControllerHandle,
  IN   EFI_HANDLE                    ChildHandle    OPTIONAL,
  IN   CHAR8                         *Language,
  OUT  CHAR16                        **ControllerName
  );


EFI_STATUS
EFIAPI
PhytiumI2cDevicesIoSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath OPTIONAL
  );


EFI_STATUS
EFIAPI
PhytiumI2cDevicesIoTransfer (
  IN CONST PHYTIUM_I2C_DEVICES_IO_PROTOCOL  *This,
  IN UINT16                                 Address,
  IN UINT32                                 Length,
  IN UINT8                                  *Buffer,
  IN UINT8                                  Operation
  );


EFI_STATUS
EFIAPI
PhytiumI2cDevicesIoStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath OPTIONAL
  );


EFI_STATUS
EFIAPI
PhytiumI2cDevicesIoStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                    *ChildHandleBuffer OPTIONAL
  );


EFI_STATUS
EFIAPI
PhytiumI2cDevicesIoInitialise (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

#endif // PHYTIUM_I2C_DEVICES_IO_H_

