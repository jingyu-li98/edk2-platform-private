/** @file
This Library can support all BMC access through IPMI commands.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation.<BR>


SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/DebugLib.h>
#include <Library/IpmiBaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <PiDxe.h>
#include <Protocol/IpmiInteractiveProtocol.h>

IPMI_PHY_PROTOCOL          *mIpmiProtocol = NULL;
VOID                       *mEfiIpmiProtocolNotifyReg;
EFI_EVENT                   mEfiIpmiProtocolEvent;

/**
  Callback function for locating the IpmiTransport protocol.

  @param [in]  Event    Event on which the callback is called.
  @param [in]  Context  The context of the Callback.

  @retval  EFI_SUCCESS  Return from EfiLocateProtocolInterface function.
  @retval  Other        Failure.

**/
EFI_STATUS
NotifyIpmiTransportCallback (
  IN  EFI_EVENT     Event,
  IN  VOID         *Context
  )
{
  EFI_STATUS    Status;

  Status = EFI_SUCCESS;
  if (mIpmiProtocol == NULL) {
    gBS->LocateProtocol (
           &gIpmiTransportProtocolGuid,
           NULL,
           (VOID **) &mIpmiProtocol
           );
  }

  return Status;
}

/**
  Initalize IpmiTransport Protocol.

  @retval  EFI_SUCCESS  Always return success.
  @retval  Other        Failure.

**/
EFI_STATUS
InitializeIpmiBase (
  VOID
  )
{
  EFI_STATUS  Status;
  if (mIpmiProtocol == NULL) {
    Status = gBS->CreateEvent (
                            EVT_NOTIFY_SIGNAL,
                            TPL_CALLBACK,
                            (EFI_EVENT_NOTIFY)NotifyIpmiTransportCallback,
                            NULL,
                            &mEfiIpmiProtocolEvent
                            );
    if (EFI_ERROR(Status)) {
       return Status;
     }

    Status = gBS->RegisterProtocolNotify (
                            &gIpmiTransportProtocolGuid,
                            mEfiIpmiProtocolEvent,
                            &mEfiIpmiProtocolNotifyReg
                            );
    if (EFI_ERROR(Status)) {
       return Status;
     }

    gBS->SignalEvent (mEfiIpmiProtocolEvent);
  }

  return EFI_SUCCESS;
}

/**
  Routine to send commands to BMC.

  @param[in]      NetFunction       Net function of the command.
  @param[in]      Command           IPMI Command.
  @param[in]      RequestData       Command Data.
  @param[in]      RequestDataSize   Size of CommandData.
  @param[out]     ResponseData      Response Data.
  @param[in,out]  ResponseDataSize  Response Data Size.

  @retval  EFI_SUCCESS    Return Successly.
  @retval  EFI_NOT_FOUND  IpmiTransport Protocol is not installed.
  @retval  Other          Failure.

**/
EFI_STATUS
EFIAPI
IpmiSubmitCommand (
  IN      UINT8   NetFunction,
  IN      UINT8   Command,
  IN      UINT8   *RequestData,
  IN      UINT8   RequestDataSize,
  OUT     UINT8   *ResponseData,
  IN OUT  UINT8   *ResponseDataSize
  )
{
  EFI_STATUS       Status;
  IPMI_CMD_PACKET  Request;
  Request.NetFn    = NetFunction;
  Request.Cmd      = Command;
  Request.Lun      = IPMI_BMC_LUN;

  Status = gBS->LocateProtocol (
                           &gIpmiTransportProtocolGuid,
                           NULL,
                          (VOID **) &mIpmiProtocol
                           );
  if (EFI_ERROR(Status)) {
     return Status;
  }

  Status = mIpmiProtocol->IpmiPhySubmitCommand (
                mIpmiProtocol,
                Request,
                RequestData,
                RequestDataSize,
                ResponseData,
                ResponseDataSize
                );
  return Status;
}
