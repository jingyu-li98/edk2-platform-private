/** @file
Generic MM Phase Send IPMI command to BMC.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation. <BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "IpmiKcsBmc.h"
#include "MmIpmiBmc.h"

/**
  Updates the  Bmc Status.

  @param[in]   This         Pointer to IPMI protocol instance.
  @param[out]  BmcStatus    BMC status.

  @retval  EFI_SUCCESS            Return Successly.
  @retval  Other                  Failure.

**/
EFI_STATUS
EFIAPI
MmIpmiGetBmcStatus (
  IN  IPMI_PHY_PROTOCOL       *This,
  OUT BMC_STATUS              *BmcStatus
  )
{
  IPMI_MM_ATTRIBUTE_DATA  *IpmiInstance;

  IpmiInstance = IPMI_MM_ATTRIBUTE_DATA_FROM_THIS (This);

  if ((IpmiInstance->BmcStatus == BMC_NOTREADY) ) {
    IpmiInstance->BmcStatus = BMC_SOFTFAIL;
  }

  *BmcStatus = IpmiInstance->BmcStatus;
  return EFI_SUCCESS;
}

/**
  MM Phase Routine to send commands to BMC.

  @param[in]      This              A pointer to IPMI_PHY_PROTOCOL.
  @param[in]      SubmitCmd         Include NetFn Lun and Cmd.
  @param[in]      RequestData       Command Data.
  @param[in]      RequestDataSize   Size of CommandData.
  @param[out]     ResponseData      Response Data.
  @param[out]     ResponseDataSize  Response Data Size.

  @retval  EFI_SUCCESS            Return Successly.
  @retval  EFI_NOT_AVAILABLE_YET  IpmiTransport Protocol is not installed yet.
  @retval  Other                  Failure.

**/
EFI_STATUS
EFIAPI
MmIpmiCommandExecuteToBmc (
  IN  IPMI_PHY_PROTOCOL  *This,
  IN  IPMI_CMD_PACKET    SubmitCmd,
  IN  UINT8              *RequestData OPTIONAL,
  IN  UINT8              RequestDataSize,
  OUT UINT8              *ResponseData,
  OUT UINT8              *ResponseDataSize
  )
{
  UINT8                   CompletionCode;
  EFI_STATUS              Status;
  UINT64                  KcsIoAddr;
  UINT64                  KcsTimePeriod;
  IPMI_MM_ATTRIBUTE_DATA     *IpmiInstance;

  CompletionCode = IPMI_COMPLETE_CODE_NO_ERROR;
  Status = EFI_SUCCESS;

  IpmiInstance = IPMI_MM_ATTRIBUTE_DATA_FROM_THIS (This);
  KcsIoAddr = IpmiInstance->IpmiBaseAddress;
  KcsTimePeriod = IpmiInstance->TotalTimeTicks;

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
             &CompletionCode,
             ResponseData,
             ResponseDataSize
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}


