/** @file
  The Header of protocol for I2c devices.

  Copyright (C) 2016, Marvell International Ltd. All rights reserved.<BR>
  Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef I2C_DEVICES_IO_H_
#define I2C_DEVICES_IO_H_

#define PHYTIUM_I2C_DEVICES_IO_PROTOCOL_GUID { 0x665ec332, 0x17cb, 0x11eb, { 0x87, 0x06, 0x2f, 0xb0, 0xff, 0x02, 0x01, 0x6d }}

typedef struct _PHYTIUM_I2C_DEVICES_IO_PROTOCOL PHYTIUM_I2C_DEVICES_IO_PROTOCOL;

#define I2C_DEVICE_READ   0x1
#define I2C_DEVICE_WRITE  0x0

typedef
EFI_STATUS
(EFIAPI *EFI_PHYTIUM_I2C_DEVICES_IO_TRANSFER) (
  IN CONST PHYTIUM_I2C_DEVICES_IO_PROTOCOL  *This,
  IN UINT16                                 Address,
  IN UINT32                                 Length,
  IN UINT8                                  *Buffer,
  IN UINT8                                  Operation
  );

struct _PHYTIUM_I2C_DEVICES_IO_PROTOCOL {
  EFI_PHYTIUM_I2C_DEVICES_IO_TRANSFER  Transfer;
  UINT32                               Identifier;
};

extern EFI_GUID gPhytiumI2cDevicesIoProtocolGuid;

#endif /* I2C_DEVICES_IO_H_ */
