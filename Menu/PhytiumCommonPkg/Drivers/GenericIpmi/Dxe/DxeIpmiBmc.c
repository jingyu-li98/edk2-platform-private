/** @file
Send ipmi command and data to Bmc.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation. <BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <DxeIpmiInit.h>
#include <IpmiCommandLib.h>

#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/IpmiInteractiveProtocol.h>

#include "IpmiKcsBmc.h"

/**
  Routine to send commands to BMC.

  @param[in]      This              A pointer to IPMI_PHY_PROTOCOL.
  @param[in]      SubmitCmd         Include NetFn Lun and Cmd.
  @param[in]      RequestData       Command Data.
  @param[in]      RequestDataSize   Size of CommandData.
  @param[out]     ResponseData      Response Data.
  @param[in,out]  ResponseDataSize  Response Data Size.

  @retval  EFI_SUCCESS            Return Successly.
  @retval  EFI_NOT_AVAILABLE_YET  IpmiTransport Protocol is not installed yet.
  @retval  Other                  Failure.

**/
EFI_STATUS
EFIAPI
IpmiCommandExecuteToBmc (
  IN  IPMI_PHY_PROTOCOL  *This,
  IN  IPMI_CMD_PACKET    SubmitCmd,
  IN  UINT8              *RequestData OPTIONAL,
  IN  UINT8              RequestDataSize,
  OUT UINT8              *ResponseData,
  IN OUT UINT8           *ResponseDataSize
  )
{
  EFI_STATUS              Status;
  IPMI_ATTRIBUTE_DATA     *IpmiInfo;
  UINT8                   Context;
  UINT64                  KcsIoAddr;
  UINT64                  KcsTimePeriod;

  Status = EFI_SUCCESS;
  IpmiInfo = IPMI_ATTRIBUTE_DATA_FROM_THIS (This);
  KcsIoAddr = IpmiInfo->IpmiBaseAddress;
  KcsTimePeriod = IpmiInfo->TotalTimeTicks;

  Status = KcsSendToBmcMode (
             KcsIoAddr,
             KcsTimePeriod,
             &SubmitCmd,
             RequestData,
             RequestDataSize
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = KcsReceiveBmcMode (
             KcsIoAddr,
             KcsTimePeriod,
             &SubmitCmd,
             &Context,
             ResponseData,
             ResponseDataSize
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}
