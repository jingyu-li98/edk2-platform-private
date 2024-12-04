/** @file
  The Header of protocol for I2c bus.

  Copyright (c) 2018, Marvell International Ltd. All rights reserved.<BR>
  Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef PHYTIUM_I2C_H_
#define PHYTIUM_I2C_H_

#include <Library/UefiLib.h>
#include <Protocol/I2cMaster.h>
#include <Protocol/I2cEnumerate.h>
#include <Protocol/I2cBusConfigurationManagement.h>
#include <Protocol/I2cIo.h>
#include <Protocol/I2cDevicesIo.h>

#define I2C_GUID \
  { \
  0x25d04920, 0x23c7, 0x11eb, { 0xb1, 0x2f, 0xb7, 0xa3, 0x3a, 0x75, 0x47, 0xbf } \
  }

/*
 * Helper macros for maintaining multiple I2C buses
 * and devices defined via EFI_I2C_DEVICE.
 */
#define I2C_DEVICE_ADDRESS(Index)      ((Index) & MAX_UINT16)
#define I2C_DEVICE_BUS(Index)          ((Index) >> 16)
#define I2C_DEVICE_INDEX(Bus, Address) (((Address) & MAX_UINT16) | (Bus) << 16)

#define I2C_MASTER_SIGNATURE              SIGNATURE_32 ('I', '2', 'C', 'M')
#define I2C_SLAVE_DEVICE_SIGNATURE        SIGNATURE_32 ('I', '2', 'C', 'D')

typedef
struct I2C_MASTER_CONTEXT {
  UINT32      Signature;
  EFI_HANDLE  Controller;
  EFI_LOCK    Lock;
  UINTN       TclkFrequency;
  UINTN       BaseAddress;
  INTN        Bus;
  EFI_I2C_MASTER_PROTOCOL I2cMaster;
  EFI_I2C_ENUMERATE_PROTOCOL I2cEnumerate;
  EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL I2cBusConf;
} I2C_MASTER_CONTEXT;

typedef struct {
  VENDOR_DEVICE_PATH            Vendor;
  EFI_DEVICE_PATH_PROTOCOL      End;
} I2C_DEVICE_PATH;

#define I2C_SC_FROM_MASTER(a) CR (a, I2C_MASTER_CONTEXT, I2cMaster, I2C_MASTER_SIGNATURE)
#define I2C_SC_FROM_ENUMERATE(a) CR (a, I2C_MASTER_CONTEXT, I2cEnumerate, I2C_MASTER_SIGNATURE)
#define I2C_SC_FROM_BUSCONF(a) CR (a, I2C_MASTER_CONTEXT, I2cBusConf, I2C_MASTER_SIGNATURE)

typedef struct {
  UINT32  Signature;
  EFI_HANDLE ControllerHandle;
  EFI_I2C_IO_PROTOCOL *I2cIo;
  PHYTIUM_I2C_DEVICES_IO_PROTOCOL I2cDeviceProtocol;
} I2C_SLAVE_DEVICE_CONTEXT;

#define I2CDEVICE_SC_FROM_IO(a) CR (a, I2C_SLAVE_DEVICE_CONTEXT, I2cIo, I2C_SLAVE_DEVICE_SIGNATURE)
#define I2CDEVICE_SC_FROM_I2CDEVICE(a) CR (a, I2C_SLAVE_DEVICE_CONTEXT, I2cDeviceProtocol, I2C_SLAVE_DEVICE_SIGNATURE)


#endif /* PHYTIUM_I2C_H_ */
