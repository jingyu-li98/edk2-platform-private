/** @file
Support the definition of BMC setting Boot options.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright (c) 2017, Hisilicon Limited. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef  BMC_CONFIGBOOT_LIB_H_
#define  BMC_CONFIGBOOT_LIB_H_

#include <IndustryStandard/Ipmi.h>
#include <IpmiPhyStandard.h>
#include <Uefi.h>

#pragma pack (1)

//
// IPMI Boot Option CompletionCode.
//
#define IPMI_COMP_CODE_PARAM_UNSUPPORTED  0x80
#define IPMI_COMP_CODE_SET_IN_PROGRESS    0x81
#define IPMI_COMP_CODE_READ_ONLY          0x82
#define ForceShell                        0x4
#define ForcePxe                          0x1
#define ForcePrimaryRemoveableMedia       0xF
#define ForceDefaultHardDisk              0x2
#define ForceDefaultCD                    0x5
#define ForceBiosSetup                    0x6

//
// IPMI Boot Option param 5 (Boot Flags) info.
//
typedef struct {
  BOOLEAN IsBootFlagsValid;
  BOOLEAN IsPersistent;
  UINT8   DeviceSelector;
  UINT8   InstanceSelector;
} IPMI_BOOT_FLAGS_INFO;

/**
  Set Boot Info Acknowledge to notify BMC that the Boot Flags has been handled by UEFI.

  @retval EFI_SUCCESS                          The command byte stream was successfully submit to the device and a response was successfully received.
  @retval other                                Failed to write data to the device.

**/
EFI_STATUS
EFIAPI
IpmiSetBootInfoAck (
  VOID
  );

/**
  Retrieve Boot Info Acknowledge from BMC.

  @param[out] BootInitiatorAcknowledgeData    Pointer to returned buffer.

  @retval EFI_SUCCESS                         The command byte stream was successfully submit to the device and a response was successfully received.
  @retval EFI_INVALID_PARAMETER               BootInitiatorAcknowledgeData was NULL.
  @retval other                               Failed to write data to the device.

**/
EFI_STATUS
EFIAPI
IpmiGetBootInfoAck (
  OUT UINT8 *BootInitiatorAcknowledgeData
  );

/**
  Send command to clear BMC Boot Flags parameter.

  @retval EFI_SUCCESS                        The command byte stream was successfully submit to the device and a response was successfully received.
  @retval other                              Failed to write data to the device.

**/
EFI_STATUS
EFIAPI
IpmiClearBootFlags (
  VOID
  );

/**
  Retrieve Boot Flags parameter from BMC.

  @param[out] BootFlags                     Pointer to returned buffer.

  @retval EFI_SUCCESS                       The command byte stream was successfully submit to the device and a response was successfully received.
  @retval EFI_INVALID_PARAMETER             BootFlags was NULL.
  @retval other                             Failed to write data to the device.

**/
EFI_STATUS
EFIAPI
IpmiGetBootFlags (
  OUT IPMI_BOOT_FLAGS_INFO *BootFlags
  );

/**
  Looking for the backup of previous BootOrder,the restore it with respect to current BootOrder.

**/
VOID
EFIAPI
RestoreBootOrder (
  VOID
  );

/**
  Get IPMI device Boot Type.

**/
VOID
EFIAPI
IpmiBmcBootType (
  VOID
  );

#pragma pack()
#endif

