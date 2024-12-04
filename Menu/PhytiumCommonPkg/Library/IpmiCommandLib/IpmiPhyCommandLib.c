/** @file
IPMI Command association.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright (c) 2018 - 2021, Intel Corporation. All rights reserved.<BR>

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
  Get BMC device ID information.IPMI General protocol:
  NetFunction Id is 0x06, command Id is 0x01.

  @param[out]   BmcContent  A pointer to BMC information.

  @retval      EFI_SUCCESS       Command completed successfully.
               EFI_DEVICE_ERROR  IPMI command failed.
               EFI_UNSUPPORTED   Command is not supported by BMC.
               EFI_BUFFER_TOO_SMALL   Response buffer is too small.
               EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.

**/
EFI_STATUS
EFIAPI
IpmiGetDeviceId (
  OUT  IPMI_BMC_INFO            *BmcContent
  )
{
  EFI_STATUS                    Status;
  UINT8                         DataSize;

  DataSize = sizeof (IPMI_BMC_INFO);
  Status = IpmiSubmitCommand (
             IPMI_NETFN_APP,
             IPMI_APP_GET_DEVICE_ID,
             NULL,
             0,
             (VOID *)BmcContent,
             &DataSize
             );
  return Status;
}

/**
  Get BMC Self Test information.IPMI General protocol:
  NetFunction Id is 0x06, command Id is 0x04.

  @param[out]  SelfTestResult  A pointer to Self Test Result.

  @retval      EFI_SUCCESS       Command completed successfully.
               EFI_DEVICE_ERROR  IPMI command failed.
               EFI_UNSUPPORTED   Command is not supported by BMC.
               EFI_BUFFER_TOO_SMALL   Response buffer is too small.
               EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.

**/
EFI_STATUS
EFIAPI
IpmiGetSelfTestResult (
  OUT IPMI_GET_SELF_TEST_RESULT   *SelfTestResult
  )
{
  EFI_STATUS                     Status;
  UINT8                          DataSize;

  DataSize = sizeof (IPMI_GET_SELF_TEST_RESULT);
  Status = IpmiSubmitCommand (
               IPMI_NETFN_APP,
               IPMI_APP_GET_SELFTEST_RESULTS,
               NULL,
               0,
               (VOID *)SelfTestResult,
               &DataSize
               );
  return Status;
}

/**
  Send CPU information. Phytium IPMI private protocol:
  NetFunction Id is 0x3a, command Id is 0x01.

  @param[in]    SetCpuInfo    A pointer to CPU information.
  @param[out]   CompleteCode  Completion codes defintions.
                              0x0:    Success.
                              other:  Failed.

  @retval      EFI_SUCCESS       Command completed successfully.
               EFI_DEVICE_ERROR  IPMI command failed.
               EFI_UNSUPPORTED   Command is not supported by BMC.
               EFI_BUFFER_TOO_SMALL   Response buffer is too small.
               EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.

**/
EFI_STATUS
EFIAPI
IpmiSendCpuInfo (
  IN  IPMI_SET_CPU_INFO   *SetCpuInfo,
  OUT  UINT8              *CompleteCode
  )
{
  EFI_STATUS                   Status;
  UINT8                        DataSize;

  DataSize = sizeof (UINT8);
  Status = IpmiSubmitCommand (
               IPMI_NETFN_PHY_APP,
               IPMI_PHY_SET_CPU_INFO,
               (VOID *)SetCpuInfo,
               sizeof (IPMI_SET_CPU_INFO),
               (VOID*)CompleteCode,
               &DataSize
               );
  return Status;
}

/**
  Send Spd information. Phytium IPMI private protocol:
  NetFunction Id is 0x3a, command Id is 0x03.

  @param[in]    SetSpdInfo    A pointer to Spd information.
  @param[out]   CompleteCode  Completion codes defintions.
                              0x0:    Success.
                              other:  Failed.

  @retval      EFI_SUCCESS       Command completed successfully.
               EFI_DEVICE_ERROR  IPMI command failed.
               EFI_UNSUPPORTED   Command is not supported by BMC.
               EFI_BUFFER_TOO_SMALL   Response buffer is too small.
               EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.

**/
EFI_STATUS
EFIAPI
IpmiSendSpdInfo (
  IN  IPMI_SET_SPD_INFO  *SetSpdInfo,
  OUT UINT8              *CompleteCode
  )
{
  EFI_STATUS                  Status;
  UINT8                       DataSize;

  DataSize = sizeof (UINT8);
  Status = IpmiSubmitCommand (
               IPMI_NETFN_PHY_APP,
               IPMI_PHY_SET_SPD_INFO,
               (VOID *)SetSpdInfo,
               sizeof (IPMI_SET_SPD_INFO),
               (VOID*)CompleteCode,
               &DataSize
               );
  return Status;
}

/**
  Send BIOS information. Phytium IPMI private protocol:
  NetFunction Id is 0x3a, command Id is 0x20.

  @param[in]    SetBiosInfo   A pointer to BIOS information.
  @param[out]   CompleteCode  Completion codes defintions.
                              0x0:    Success.
                              other:  Failed.

  @retval      EFI_SUCCESS       Command completed successfully.
               EFI_DEVICE_ERROR  IPMI command failed.
               EFI_UNSUPPORTED   Command is not supported by BMC.
               EFI_BUFFER_TOO_SMALL   Response buffer is too small.
               EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.

**/
EFI_STATUS
EFIAPI
IpmiSendBiosInfo (
  IN  IPMI_SET_BIOS_INFO    *SetBiosInfo,
  OUT  UINT8                *CompleteCode
  )
{
  EFI_STATUS                   Status;
  UINT8                        DataSize;

  DataSize = sizeof (UINT8);
  Status = IpmiSubmitCommand (
               IPMI_NETFN_PHY_APP,
               IPMI_PHY_SET_BIOS_INFO,
               (VOID *)SetBiosInfo,
               sizeof (IPMI_SET_BIOS_INFO),
               (VOID*)CompleteCode,
               &DataSize
               );
  return Status;
}

/**
  Send Pcie device information. Phytium IPMI private protocol:
  NetFunction Id is 0x3a, command Id is 0x09.

  @param[in]    SetPcieDeviceInfo  A pointer to PCIE device information.
  @param[out]   CompleteCode      Completion codes defintions.
                                  0x0:    Success.
                                  other:  Failed.

  @retval      EFI_SUCCESS       Command completed successfully.
               EFI_DEVICE_ERROR  IPMI command failed.
               EFI_UNSUPPORTED   Command is not supported by BMC.
               EFI_BUFFER_TOO_SMALL   Response buffer is too small.
               EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.

**/
EFI_STATUS
EFIAPI
IpmiSendPcieDeviceInfo (
  IN  IPMI_SET_PCIE_DEVICE_INFO  *SetPcieDeviceInfo,
  OUT UINT8                      *CompleteCode
  )
{
  EFI_STATUS                   Status;
  UINT8                        DataSize;

  DataSize = sizeof (UINT8);
  Status = IpmiSubmitCommand (
               IPMI_NETFN_PHY_APP,
               IPMI_PHY_SET_PCIE_DEVICE_INFO,
               (VOID *)SetPcieDeviceInfo,
               sizeof (IPMI_SET_PCIE_DEVICE_INFO),
               (VOID*)CompleteCode,
               &DataSize
               );
  return Status;
}

/**
  Send Pcie Net Card device information. Phytium IPMI private protocol:
  NetFunction Id is 0x3a, command Id is 0x09.

  @param[in]    SetPcieNetDeviceInfo  A pointer to PCIE Net device information.
  @param[out]   CompleteCode         Completion codes defintions.
                                     0x0:    Success.
                                     other:  Failed.

  @retval      EFI_SUCCESS       Command completed successfully.
               EFI_DEVICE_ERROR  IPMI command failed.
               EFI_UNSUPPORTED   Command is not supported by BMC.
               EFI_BUFFER_TOO_SMALL   Response buffer is too small.
               EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.

**/
EFI_STATUS
EFIAPI
IpmiSendPcieNetDeviceInfo (
  IN  IPMI_SET_PCIE_NET_DEVICE_INFO  *SetPcieNetDeviceInfo,
  OUT UINT8                          *CompleteCode
  )
{
  EFI_STATUS                   Status;
  UINT8                        DataSize;

  DataSize = sizeof (UINT8);
  Status = IpmiSubmitCommand (
               IPMI_NETFN_PHY_APP,
               IPMI_PHY_SET_PCIE_DEVICE_INFO,
               (VOID *)SetPcieNetDeviceInfo,
               sizeof (IPMI_SET_PCIE_NET_DEVICE_INFO),
               (VOID*)CompleteCode,
               &DataSize
               );
  return Status;
}

/**
  Send Hard disk information. Phytium IPMI private protocol:
  NetFunction Id is 0x3a, command Id is 0x16.

  @param[in]    SetHddDeviceInfo  A pointer to Hard disk device information.
  @param[out]   CompleteCode      Completion codes defintions.
                                  0x0:    Success.
                                  other:  Failed.

  @retval      EFI_SUCCESS       Command completed successfully.
               EFI_DEVICE_ERROR  IPMI command failed.
               EFI_UNSUPPORTED   Command is not supported by BMC.
               EFI_BUFFER_TOO_SMALL   Response buffer is too small.
               EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.

**/
EFI_STATUS
EFIAPI
IpmiSendHddDeviceInfo (
  IN  IPMI_SET_HDD_DEVICE_INFO  *SetHddDeviceInfo,
  OUT UINT8                     *CompleteCode
  )
{
  EFI_STATUS                   Status;
  UINT8                        DataSize;

  DataSize = sizeof (UINT8);
  Status = IpmiSubmitCommand (
               IPMI_NETFN_PHY_APP,
               IPMI_PHY_SET_HDD_DEVICE_INFO,
               (VOID *)SetHddDeviceInfo,
               sizeof (IPMI_SET_HDD_DEVICE_INFO),
               (VOID*)CompleteCode,
               &DataSize
               );
  return Status;
}

/**
  Reset Watchdog Timer information. IPMI Generic protocol:
  NetFunction Id is 0x06, command Id is 0x22.

  @param[out]   CompleteCode     Completion codes defintions.

  @retval      EFI_SUCCESS       Command completed successfully.
               EFI_DEVICE_ERROR  IPMI command failed.
               EFI_UNSUPPORTED   Command is not supported by BMC.
               EFI_BUFFER_TOO_SMALL   Response buffer is too small.
               EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.

**/
EFI_STATUS
EFIAPI
IpmiResetWatchdogTimer (
  OUT UINT8                   *CompleteCode
  )
{
  EFI_STATUS                   Status;
  UINT8                        DataSize;

  DataSize = sizeof (UINT8);
  Status = IpmiSubmitCommand (
             IPMI_NETFN_APP,
             IPMI_APP_RESET_WATCHDOG_TIMER,
             NULL,
             0,
             (VOID *)CompleteCode,
             &DataSize
             );
  return Status;
}

/**
  Set Watchdog Timer information. IPMI Generic protocol:
  NetFunction Id is 0x06, command Id is 0x24.

  @param[in]    SetWatchdogTimer   A pointer to Watchdog information.
  @param[out]   CompleteCode  Completion codes defintions.

  @retval      EFI_SUCCESS       Command completed successfully.
               EFI_DEVICE_ERROR  IPMI command failed.
               EFI_UNSUPPORTED   Command is not supported by BMC.
               EFI_BUFFER_TOO_SMALL   Response buffer is too small.
               EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.

**/
EFI_STATUS
EFIAPI
IpmiSetWatchdogTimer (
  IN  IPMI_SET_WATCHDOG_TIMER_REQUEST  *SetWatchdogTimer,
  OUT UINT8                            *CompleteCode
  )
{
  EFI_STATUS                   Status;
  UINT8                        DataSize;

  DataSize = sizeof (UINT8);
  Status = IpmiSubmitCommand (
             IPMI_NETFN_APP,
             IPMI_APP_SET_WATCHDOG_TIMER,
             (VOID *)SetWatchdogTimer,
             sizeof (IPMI_SET_WATCHDOG_TIMER_REQUEST),
             (VOID *)CompleteCode,
             &DataSize
             );
  return Status;
}

/**
  Get Watchdog Timer information. IPMI Generic protocol:
  NetFunction Id is 0x06, command Id is 0x25.

  @param[out]   GetWatchdogTimer  A pointer to BMC Watchdog information.

  @retval      EFI_SUCCESS       Command completed successfully.
               EFI_DEVICE_ERROR  IPMI command failed.
               EFI_UNSUPPORTED   Command is not supported by BMC.
               EFI_BUFFER_TOO_SMALL   Response buffer is too small.
               EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.

**/
EFI_STATUS
EFIAPI
IpmiGetWatchdogTimer (
  OUT IPMI_GET_WATCHDOG_TIMER_RESPONSE  *GetWatchdogTimer
  )
{
  EFI_STATUS                   Status;
  UINT8                        DataSize;

  DataSize = sizeof (IPMI_GET_WATCHDOG_TIMER_RESPONSE);
  Status = IpmiSubmitCommand (
             IPMI_NETFN_APP,
             IPMI_APP_GET_WATCHDOG_TIMER,
             NULL,
             0,
             (VOID *)GetWatchdogTimer,
             &DataSize
             );
  return Status;
}

/**
  Set System Boot Options information. IPMI Generic protocol:
  NetFunction Id is 0x00, command Id is 0x8.

  @param[in]    SetBootOptionsRequest      A pointer to an callee allocated buffer containing request data.
  @param[in]    SetBootOptionsRequestSize  Size of SetBootOptionsRequest.
  @param[out]   CompletionCode             Returned Completion Code from BMC.

  @retval      EFI_SUCCESS       Command completed successfully.
               EFI_DEVICE_ERROR  IPMI command failed.
               EFI_UNSUPPORTED   Command is not supported by BMC.
               EFI_BUFFER_TOO_SMALL   Response buffer is too small.
               EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.

**/
EFI_STATUS
EFIAPI
IpmiSetSystemBootOptions (
  IN  IPMI_SET_BOOT_OPTIONS_REQUEST  *SetBootOptionsRequest,
  IN  UINT32                         SetBootOptionsRequestSize,
  OUT UINT8                          *CompletionCode
  )
{
  EFI_STATUS                    Status;
  UINT8                         DataSize;

  if (SetBootOptionsRequest == NULL
      || CompletionCode == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  DataSize = sizeof (UINT8);
  Status = IpmiSubmitCommand (
             IPMI_NETFN_CHASSIS,
             IPMI_CHASSIS_SET_SYSTEM_BOOT_OPTIONS,
             (VOID *)SetBootOptionsRequest,
             SetBootOptionsRequestSize,
             (UINT8 *)CompletionCode,
             &DataSize
             );

  switch (*CompletionCode) {
  case IPMI_COMP_CODE_NORMAL:
    break;

  case IPMI_COMP_CODE_PARAM_UNSUPPORTED:
    Status = EFI_UNSUPPORTED;
    break;

  case IPMI_COMP_CODE_SET_IN_PROGRESS:
    Status = EFI_NOT_READY;
    break;

  case IPMI_COMP_CODE_READ_ONLY:
    Status = EFI_ACCESS_DENIED;
    break;

  default:
    Status = EFI_DEVICE_ERROR;
    break;
  }

  return Status;
}

/**
  Get System Boot Options information. IPMI Generic protocol:
  NetFunction Id is 0x00, command Id is 0x9.

  @param[in]        ParameterSelector           Boot Options parameter which want to get.
  @param[out]       GetBootOptionsResponse      A pointer to an callee allocated buffer to store response data.
  @param[in, out]   GetBootOptionsResponseSize  Size of GetBootOptionsResponse.

  @retval      EFI_SUCCESS       Command completed successfully.
               EFI_DEVICE_ERROR  IPMI command failed.
               EFI_UNSUPPORTED   Command is not supported by BMC.
               EFI_BUFFER_TOO_SMALL   Response buffer is too small.
               EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.

**/
EFI_STATUS
EFIAPI
IpmiGetSystemBootOptions (
  IN     UINT8                          ParameterSelector,
  OUT    IPMI_GET_BOOT_OPTIONS_RESPONSE *GetBootOptionsResponse,
  IN OUT UINT8                          *GetBootOptionsResponseSize
  )
{
  EFI_STATUS                    Status;
  IPMI_GET_BOOT_OPTIONS_REQUEST GetBootOptionsRequest;

  ZeroMem(&GetBootOptionsRequest, sizeof(IPMI_GET_BOOT_OPTIONS_REQUEST));
  if (GetBootOptionsResponse == NULL
      || GetBootOptionsResponseSize == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  GetBootOptionsRequest.ParameterSelector.Bits.ParameterSelector = ParameterSelector;
  GetBootOptionsRequest.ParameterSelector.Bits.Reserved = 0;
  GetBootOptionsRequest.SetSelector = 0;
  GetBootOptionsRequest.BlockSelector = 0;

  Status = IpmiSubmitCommand (
             IPMI_NETFN_CHASSIS,
             IPMI_CHASSIS_GET_SYSTEM_BOOT_OPTIONS,
             (VOID *)&GetBootOptionsRequest,
             sizeof (GetBootOptionsRequest),
             (VOID *)GetBootOptionsResponse,
             GetBootOptionsResponseSize
             );

  switch (GetBootOptionsResponse->CompletionCode) {
  case IPMI_COMP_CODE_NORMAL:
    break;

  case IPMI_COMP_CODE_PARAM_UNSUPPORTED:
    Status = EFI_UNSUPPORTED;
    break;

  default:
    Status = EFI_DEVICE_ERROR;
    break;
  }

  return Status;
}

/**
  Send CPER information. IPMI Generic protocol:
  NetFunction Id is 0x3a, command Id is 0x30.

  @param[in]    CperInforData   A pointer to Cper Information.
  @param[in]    CperDataSize   Cper Information Data Size.
  @param[out]   CperResponse   Cper Information Response.

  @retval      EFI_SUCCESS       Command completed successfully.
               EFI_DEVICE_ERROR  IPMI command failed.
               EFI_UNSUPPORTED   Command is not supported by BMC.
               EFI_BUFFER_TOO_SMALL   Response buffer is too small.
               EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.

**/
EFI_STATUS
EFIAPI
IpmiSendCperInfo (
  IN  UINT8            *CperInforData,
  IN  UINT16           CperDataSize,
  OUT UINT8            *CperResponse
)
{
  EFI_STATUS           Status;
  UINT8                DataSize;
  UINT8                Count;
  UINT8                Length;
  UINT8                PacketTicks;
  UINT8                RemainData;
  IPMI_SEND_CPER_INFO  SendCperInfo;

  Length = 0;
  Status = EFI_UNSUPPORTED;
  ZeroMem (&SendCperInfo, sizeof (SendCperInfo));

  DataSize = sizeof (*CperResponse);
  PacketTicks = CperDataSize / 250;
  RemainData = CperDataSize % 250;

  if (CperDataSize < 250) {
     //
     //Start package id is 0.
     //
     SendCperInfo.SsionId = 0;
     CopyMem (SendCperInfo.Data, CperInforData, CperDataSize);

     Length = CperDataSize + 1;
     Status = IpmiSubmitCommand (
                IPMI_NETFN_PHY_APP,
                IPMI_PHY_SET_CPER_INFO,
                (UINT8*)&SendCperInfo,
                Length,
                (UINT8*)CperResponse,
                &DataSize
                );
     if (EFI_ERROR (Status)) {
       return Status;
     }
  }

  for (Count = 0; Count < PacketTicks; Count++) {
     SendCperInfo.SsionId = Count;
     CopyMem (SendCperInfo.Data, (CperInforData + Count * 250), 250);
     Length = 250 + sizeof (SendCperInfo.SsionId);
     Status = IpmiSubmitCommand (
                IPMI_NETFN_PHY_APP,
                IPMI_PHY_SET_CPER_INFO,
                (UINT8*)&SendCperInfo,
                Length,
                (UINT8*)CperResponse,
                &DataSize
                );
     if (EFI_ERROR (Status)) {
       return Status;
     }
   }

   //
   //End package id is 0xff.
   //
  SendCperInfo.SsionId = 0xff;

  if (CperDataSize >= 250) {
    RemainData = RemainData + sizeof (SendCperInfo.SsionId);
    CopyMem (SendCperInfo.Data, CperInforData + Count * 250, RemainData);
  } else if (CperDataSize < 250 ) {
    RemainData = sizeof (SendCperInfo.SsionId);
    CopyMem (SendCperInfo.Data, CperInforData, RemainData);
  }

  Status = IpmiSubmitCommand (
             IPMI_NETFN_PHY_APP,
             IPMI_PHY_SET_CPER_INFO,
             (UINT8*)&SendCperInfo,
             RemainData,
             (UINT8*)CperResponse,
             &DataSize
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

