/** @file
IPMI Command Interface NULL library.

Copyright (C) 2024, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <IndustryStandard/Ipmi.h>
#include <IpmiCommandLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IpmiBaseLib.h>
#include <Protocol/IpmiInteractiveProtocol.h>

/**
  Get BMC device ID information.

  @param[out]   BmcContent  A pointer to BMC information.

  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.

**/
EFI_STATUS
EFIAPI
IpmiGetDeviceId (
  OUT  IPMI_BMC_INFO            *BmcContent
  )
{
  //
  // Do nothing,just return EFI_UNSUPPORTED.
  //

    return EFI_UNSUPPORTED;
}

/**
  Get BMC Self Test information.

  @param[out]  SelfTestResult  A pointer to Self Test Result.

  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.

**/
EFI_STATUS
EFIAPI
IpmiGetSelfTestResult (
  OUT IPMI_GET_SELF_TEST_RESULT   *SelfTestResult
  )
{
  //
  // Do nothing,just return EFI_UNSUPPORTED.
  //

    return EFI_UNSUPPORTED;
}

/**
  Send CPU information.

  @param[in]    SetCpuInfo    A pointer to CPU information.
  @param[out]   CompleteCode  Completion codes defintions.

  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.

**/
EFI_STATUS
EFIAPI
IpmiSendCpuInfo (
  IN  IPMI_SET_CPU_INFO   *SetCpuInfo,
  OUT  UINT8              *CompleteCode
  )
{
  //
  // Do nothing,just return EFI_UNSUPPORTED.
  //

    return EFI_UNSUPPORTED;
}

/**
  Send Spd information.

  @param[in]    SetSpdInfo    A pointer to Spd information.
  @param[out]   CompleteCode  Completion codes defintions.

  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.

**/
EFI_STATUS
EFIAPI
IpmiSendSpdInfo (
  IN  IPMI_SET_SPD_INFO  *SetSpdInfo,
  OUT UINT8              *CompleteCode
  )
{
  //
  // Do nothing,just return EFI_UNSUPPORTED.
  //

    return EFI_UNSUPPORTED;
}

/**
  Send BIOS information.

  @param[in]    SetBiosInfo   A pointer to BIOS information.
  @param[out]   CompleteCode  Completion codes defintions.

  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.

**/
EFI_STATUS
EFIAPI
IpmiSendBiosInfo (
  IN  IPMI_SET_BIOS_INFO    *SetBiosInfo,
  OUT  UINT8                *CompleteCode
  )
{
  //
  // Do nothing,just return EFI_UNSUPPORTED.
  //

    return EFI_UNSUPPORTED;
}

/**
  Send Pcie device information.

  @param[in]    SetPcieDeviceInfo  A pointer to PCIE device information.
  @param[out]   CompleteCode      Completion codes defintions.

  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.

**/
EFI_STATUS
EFIAPI
IpmiSendPcieDeviceInfo (
  IN  IPMI_SET_PCIE_DEVICE_INFO  *SetPcieDeviceInfo,
  OUT UINT8                      *CompleteCode
  )
{
  //
  // Do nothing,just return EFI_UNSUPPORTED.
  //

    return EFI_UNSUPPORTED;
}

/**
  Send Pcie Net Card device information.

  @param[in]    SetPcieNetDeviceInfo  A pointer to PCIE Net device information.
  @param[out]   CompleteCode         Completion codes defintions.

  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.

**/
EFI_STATUS
EFIAPI
IpmiSendPcieNetDeviceInfo (
  IN  IPMI_SET_PCIE_NET_DEVICE_INFO  *SetPcieNetDeviceInfo,
  OUT UINT8                          *CompleteCode
  )
{
  //
  // Do nothing,just return EFI_UNSUPPORTED.
  //

    return EFI_UNSUPPORTED;
}

/**
  Send Hard disk information.

  @param[in]    SetHddDeviceInfo  A pointer to Hard disk device information.
  @param[out]   CompleteCode      Completion codes defintions.

  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.

**/
EFI_STATUS
EFIAPI
IpmiSendHddDeviceInfo (
  IN  IPMI_SET_HDD_DEVICE_INFO  *SetHddDeviceInfo,
  OUT UINT8                     *CompleteCode
  )
{
  //
  // Do nothing,just return EFI_UNSUPPORTED.
  //

    return EFI_UNSUPPORTED;
}

/**
  Reset Watchdog Timer information.

  @param[out]   CompleteCode     Completion codes defintions.

  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.

**/
EFI_STATUS
EFIAPI
IpmiResetWatchdogTimer (
  OUT UINT8                   *CompleteCode
  )
{
  //
  // Do nothing,just return EFI_UNSUPPORTED.
  //

    return EFI_UNSUPPORTED;
}

/**
  Set Watchdog Timer information.

  @param[in]    SetWatchdogTimer   A pointer to Watchdog information.
  @param[out]   CompleteCode  Completion codes defintions.

  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.

**/
EFI_STATUS
EFIAPI
IpmiSetWatchdogTimer (
  IN  IPMI_SET_WATCHDOG_TIMER_REQUEST  *SetWatchdogTimer,
  OUT UINT8                            *CompleteCode
  )
{
  //
  // Do nothing,just return EFI_UNSUPPORTED.
  //

    return EFI_UNSUPPORTED;
}

/**
  Get Watchdog Timer information.

  @param[out]   GetWatchdogTimer  A pointer to BMC Watchdog information.

  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.

**/
EFI_STATUS
EFIAPI
IpmiGetWatchdogTimer (
  OUT IPMI_GET_WATCHDOG_TIMER_RESPONSE  *GetWatchdogTimer
  )
{
  //
  // Do nothing,just return EFI_UNSUPPORTED.
  //

    return EFI_UNSUPPORTED;
}

/**
  Set System Boot Options information.

  @param[in]    SetBootOptionsRequest      A pointer to an callee allocated buffer containing request data.
  @param[in]    SetBootOptionsRequestSize  Size of SetBootOptionsRequest.
  @param[out]   CompletionCode             Returned Completion Code from BMC.

  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.

**/
EFI_STATUS
EFIAPI
IpmiSetSystemBootOptions (
  IN  IPMI_SET_BOOT_OPTIONS_REQUEST  *SetBootOptionsRequest,
  IN  UINT32                         SetBootOptionsRequestSize,
  OUT UINT8                          *CompletionCode
  )
{
  //
  // Do nothing,just return EFI_UNSUPPORTED.
  //

    return EFI_UNSUPPORTED;
}

/**
  Get System Boot Options information.

  @param[in]        ParameterSelector           Boot Options parameter which want to get.
  @param[out]       GetBootOptionsResponse      A pointer to an callee allocated buffer to store response data.
  @param[in, out]   GetBootOptionsResponseSize  Size of GetBootOptionsResponse.

  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.

**/
EFI_STATUS
EFIAPI
IpmiGetSystemBootOptions (
  IN     UINT8                          ParameterSelector,
  OUT    IPMI_GET_BOOT_OPTIONS_RESPONSE *GetBootOptionsResponse,
  IN OUT UINT8                          *GetBootOptionsResponseSize
  )
{
  //
  // Do nothing,just return EFI_UNSUPPORTED.
  //

    return EFI_UNSUPPORTED;
}

/**
  Send CPER information.

  @param[in]    CperInforData   A pointer to Cper Information.
  @param[in]    CperDataSize   Cper Information Data Size.
  @param[out]   CperResponse   Cper Information Response.

  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.

**/
EFI_STATUS
EFIAPI
IpmiSendCperInfo (
  IN  UINT8            *CperInforData,
  IN  UINT16           CperDataSize,
  OUT UINT8            *CperResponse
  )
{
  //
  // Do nothing,just return EFI_UNSUPPORTED.
  //

    return EFI_UNSUPPORTED;
}

