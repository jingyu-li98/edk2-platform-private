/** @file
A Library to support all BMC access via IPMI command during MM Phase.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/DebugLib.h>
#include <Library/IpmiBaseLib.h>
#include <Library/MmServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <PiDxe.h>
#include <Protocol/IpmiInteractiveProtocol.h>

IPMI_PHY_PROTOCOL        *mIpmiTransport = NULL;
VOID                     *mEfiIpmiProtocolNotifyReg = NULL;
EFI_EVENT                mEfiIpmiProtocolEvent;

/**
  Callback function for locating the IpmiTransport protocol.

  @param  Protocol     A pointer to EFI_GUID
  @param  Interface    A pointer to Interface
  @param  Handle       Handle

  @retval  EFI_SUCCESS:       Callback successfully.
  @retval  Other              Failure.

**/
EFI_STATUS
NotifyIpmiTransportCallback (
  IN CONST EFI_GUID                *Protocol,
  IN VOID                          *Interface,
  IN EFI_HANDLE                    Handle
  )
{
  EFI_STATUS  Status;
  Status = EFI_SUCCESS;
  if (mIpmiTransport == NULL) {
    Status = gMmst->MmLocateProtocol (
                      &gMmIpmiTransportProtocolGuid,
                      NULL,
                      (VOID **) &mIpmiTransport
                      );
  }

  return Status;
}

/**
  Routine to send commands to BMC.

  @retval  EFI_SUCCESS:       Always return success.
  @retval  Other              Failure.

**/
EFI_STATUS
InitializeIpmiBase (
  VOID
  )
{
  EFI_STATUS  Status;
  if (mIpmiTransport == NULL) {
    Status = gMmst->MmLocateProtocol (
                      &gMmIpmiTransportProtocolGuid,
                      NULL,
                      (VOID **) &mIpmiTransport
                      );
    if (EFI_ERROR (Status)) {
      Status = gMmst->MmRegisterProtocolNotify (
                        &gMmIpmiTransportProtocolGuid,
                        NotifyIpmiTransportCallback,
                        &mEfiIpmiProtocolNotifyReg
                        );
    }

    if (Status != EFI_SUCCESS) {
        return Status;
    }
  }

  return EFI_SUCCESS;
}

/**
  Routine to send commands to BMC.

  @param[in] NetFunction          Net function of the command.
  @param[in] Command              IPMI Command.
  @param[in] CommandData          Command Data.
  @param[in] CommandDataSize      Size of CommandData.
  @param[out] ResponseData        Response Data.
  @param[in,out] ResponseDataSize Response Data Size.

  @retval  EFI_NOT_AVAILABLE_YET  IpmiTransport Protocol is not installed yet.
  @retval  Other                  Failure.

**/
EFI_STATUS
IpmiSubmitCommand (
  IN UINT8        NetFunction,
  IN UINT8        Command,
  IN UINT8        *CommandData,
  IN UINT8        CommandDataSize,
  OUT UINT8       *ResponseData,
  IN OUT UINT8    *ResponseDataSize
  )
{
  EFI_STATUS       Status;
  IPMI_CMD_PACKET  Request;

  Request.NetFn    = NetFunction;
  Request.Cmd      = Command;
  Request.Lun      = IPMI_BMC_LUN;

  Status = gMmst->MmLocateProtocol (
                      &gMmIpmiTransportProtocolGuid,
                      NULL,
                      (VOID **) &mIpmiTransport
                      );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = mIpmiTransport->IpmiPhySubmitCommand (
                             mIpmiTransport,
                             Request,
                             CommandData,
                             CommandDataSize,
                             ResponseData,
                             ResponseDataSize
                             );
  return Status;
}

