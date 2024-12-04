/** @file
This library abstract how to send/receive IPMI command.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation. <BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef  IPMI_COMMAND_LIB_H_
#define  IPMI_COMMAND_LIB_H_

#include <IndustryStandard/Ipmi.h>
#include <IpmiPhyStandard.h>
#include <Uefi.h>

//
//  Phytium OEM define Ipmi App and Command.
//
#define IPMI_NETFN_PHY_APP            0x3a

#define IPMI_PHY_SET_CPU_INFO         0x01
#define IPMI_PHY_SET_SPD_INFO         0x03
#define IPMI_PHY_SET_BIOS_INFO        0x20
#define IPMI_PHY_SET_PCIE_DEVICE_INFO 0x09
#define IPMI_PHY_SET_HDD_DEVICE_INFO  0x16
#define IPMI_PHY_SET_CPER_INFO        0x30

//
//  Some IPMI Spec define App and Command.
//
#define IPMI_NETFN_LAN_APP            0x0C
#define IPMI_SET_LAN_CONFIG_CMD       0x01
#define IPMI_GET_LAN_CONFIG_CMD       0x02

//
// IPMI Boot Option CompletionCode
//
#define IPMI_COMP_CODE_PARAM_UNSUPPORTED  0x80
#define IPMI_COMP_CODE_SET_IN_PROGRESS    0x81
#define IPMI_COMP_CODE_READ_ONLY          0x82

/**
  Get Bmc Id information.

  @param[out]  BmcInfo  A pointer to Bmc Information.

  @retval  EFI_SUCCESS            Return Successly.
  @retval  Other                  Failure.

**/
EFI_STATUS
EFIAPI
IpmiGetDeviceId (
  OUT IPMI_BMC_INFO   *DeviceId
  );

/**
  Get BMC Self Test Status.

  @param[out]  SelfTestResult  A pointer to Self Test Result.

  @retval  EFI_SUCCESS           Return Successly.
  @retval  Other                 Failure.

**/
EFI_STATUS
EFIAPI
IpmiGetSelfTestResult (
  OUT IPMI_GET_SELF_TEST_RESULT    *SelfTestResult
  );

/**
  Send CPU information.

  @param[in]    SetCpuInfo    A pointer to CPU information.
  @param[out]   CompleteCode  Completion codes defintions

  @retval  EFI_SUCCESS           Return Successly.
  @retval  Other                 Failure.

**/
EFI_STATUS
EFIAPI
IpmiSendCpuInfo (
  IN  IPMI_SET_CPU_INFO     *SetCpuInfo,
  OUT  UINT8                *CompleteCode
);

/**
  Send Spd information.

  @param[in]    SetSpdInfo    A pointer to Spd information.
  @param[out]   CompleteCode  Completion codes defintions.

  @retval  EFI_SUCCESS           Return Successly.
  @retval  Other                 Failure.

**/
EFI_STATUS
EFIAPI
IpmiSendSpdInfo (
  IN  IPMI_SET_SPD_INFO     *SetSpdInfo,
  OUT UINT8                 *CompleteCode
);

/**
  Send BIOS information.

  @param[in]    SetBiosInfo   A pointer to BIOS information.
  @param[out]   CompleteCode  Completion codes defintions.

  @retval  EFI_SUCCESS           Return Successly.
  @retval  Other                 Failure.

**/
EFI_STATUS
EFIAPI
IpmiSendBiosInfo (
  IN  IPMI_SET_BIOS_INFO    *SetBiosInfo,
  OUT  UINT8                *CompleteCode
);

/**
  Send Pcie device information.

  @param[in]    SetPcieDeviceInfo  A pointer to PCIE device information.
  @param[out]   CompleteCode      Completion codes defintions

  @retval  EFI_SUCCESS           Return Successly.
  @retval  Other                 Failure.

**/
EFI_STATUS
EFIAPI
IpmiSendPcieDeviceInfo (
  IN  IPMI_SET_PCIE_DEVICE_INFO  *SetPcieDeviceInfo,
  OUT UINT8                      *CompleteCode
);

/**
  Send Pcie Net Card device information.

  @param[in]   SetPcieNetDeviceInfo  A pointer to PCIE Net device information.
  @param[out]  CompleteCode         Completion codes defintions.

  @retval  EFI_SUCCESS           Return Successly.
  @retval  Other                 Failure.

**/
EFI_STATUS
EFIAPI
IpmiSendPcieNetDeviceInfo (
  IN  IPMI_SET_PCIE_NET_DEVICE_INFO  *SetPcieNetDeviceInfo,
  OUT UINT8                          *CompleteCode
);

/**
  Send Hard disk information.

  @param[in]    SetHddDeviceInfo  A pointer to Hard disk device information.
  @param[out]   CompleteCode      Completion codes defintions.

  @retval  EFI_SUCCESS           Return Successly.
  @retval  Other                 Failure.

**/
EFI_STATUS
EFIAPI
IpmiSendHddDeviceInfo (
  IN  IPMI_SET_HDD_DEVICE_INFO  *SetHddDeviceInfo,
  OUT UINT8                     *CompleteCode
);

/**
  Reset Watchdog Timer information.

  @param[out]   CompleteCode     Completion codes defintions.

  @retval  EFI_SUCCESS           Return Successly.
  @retval  Other                 Failure.

**/
EFI_STATUS
EFIAPI
IpmiResetWatchdogTimer (
  OUT UINT8                            *CompletionCode
  );

/**
  Set Watchdog Timer information.

  @param[in]   SetWatchdogTimer  A pointer to Watchdog information.
  @param[out]  CompleteCode      Completion codes defintions.

  @retval  EFI_SUCCESS             Return Successly.
  @retval  Other                 Failure.

**/
EFI_STATUS
EFIAPI
IpmiSetWatchdogTimer (
  IN  IPMI_SET_WATCHDOG_TIMER_REQUEST  *SetWatchdogTimer,
  OUT UINT8                            *CompletionCode
  );

/**
  Get Watchdog Timer information.

  @param[out]  GetWatchdogTimer  A pointer to BMC Watchdog information.

  @retval  EFI_SUCCESS           Return Successly.
  @retval  Other                 Failure.

**/
EFI_STATUS
EFIAPI
IpmiGetWatchdogTimer (
  OUT IPMI_GET_WATCHDOG_TIMER_RESPONSE *GetWatchdogTimer
  );

/**
  Set BMC System Boot Options information.
  NetFunction Id is 0x00, command Id is 0x8.

  @param[in]    SetBootOptionsRequest      A pointer to an callee allocated buffer containing request data.
  @param[in]    SetBootOptionsRequestSize  Size of SetBootOptionsRequest.
  @param[out]   CompletionCode             Returned Completion Code from BMC.

  @retval  EFI_SUCCESS           Return Successly.
  @retval  Other                 Failure.

**/
EFI_STATUS
EFIAPI
IpmiSetSystemBootOptions (
  IN  IPMI_SET_BOOT_OPTIONS_REQUEST  *SetBootOptionsRequest,
  IN  UINT32                         SetBootOptionsRequestSize,
  OUT UINT8                          *CompletionCode
  );

/**
  Get BMC System Boot Options information.

  @param[in]        ParameterSelector           Boot Options parameter which want to get.
  @param[out]       GetBootOptionsResponse      A pointer to an callee allocated buffer to store response data.
  @param[in, out]   GetBootOptionsResponseSize  Size of GetBootOptionsResponse.

  @retval  EFI_SUCCESS           Return Successly.
  @retval  Other                 Failure.

**/
EFI_STATUS
EFIAPI
IpmiGetSystemBootOptions (
  IN     UINT8                          ParameterSelector,
  OUT    IPMI_GET_BOOT_OPTIONS_RESPONSE *GetBootOptionsResponse,
  IN OUT UINT8                          *GetBootOptionsResponseSize
  );

/**
  Send CPER information to BMC.

  @param[in]    CperInforData   A pointer to CPER Data.
  @param[in]    CperDataSize    CPER Data Size.
  @param[out]   CperResponse    A pointer to CPER Response.

  @retval  EFI_SUCCESS           Return Successly.
  @retval  Other                 Failure.

**/
EFI_STATUS
EFIAPI
IpmiSendCperInfo (
  IN   UINT8                *CperInforData,
  IN   UINT16               CperDataSize,
  OUT  UINT8                *CperResponse
  );

#endif
