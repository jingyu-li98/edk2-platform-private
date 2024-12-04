/** @file
Generic IPMI transport layer during PEI phase.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "IpmiKcsBmc.h"
#include "PeiGenericIpmi.h"

/**
  PEI Phase Routine to send commands to BMC.

  @param[in]      This              A pointer to PEI_IPMI_TRANSPORT_PPI.
  @param[in]      NetFunction       Net Function of command to send
  @param[in]      Lun               LUN of command to send
  @param[in]      Command           IPMI command to send
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
PeiIpmiCommandExecuteToBmc (
  IN  PEI_IPMI_TRANSPORT_PPI  *This,
  IN  UINT8                    NetFunction,
  IN  UINT8                    Lun,
  IN  UINT8                    Command,
  IN  UINT8                    *RequestData OPTIONAL,
  IN  UINT8                    RequestDataSize,
  OUT UINT8                    *ResponseData,
  IN OUT UINT8                 *ResponseDataSize
  )
{
  UINT8                   Context;
  UINT64                  KcsIoAddr;
  UINT64                  KcsTimePeriod;
  EFI_STATUS              Status;
  IPMI_CMD_PACKET         SubmitCmd;
  PEI_IPMI_BMC_INSTANCE_DATA  *IpmiInstance;

  SubmitCmd.NetFn = NetFunction;
  SubmitCmd.Cmd   = Command;
  SubmitCmd.Lun   = IPMI_BMC_LUN;
  Status = EFI_SUCCESS;

  IpmiInstance = INSTANCE_FROM_PEI_SM_IPMI_BMC_THIS (This);

  KcsIoAddr = IpmiInstance->IpmiIoBase;
  KcsTimePeriod = IpmiInstance->KcsTimeoutPeriod;

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

