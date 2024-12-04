/** @file
IPMI Command Library Header File.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 2011 - 2021 Intel Corporation. <BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef  IPMI_BASE_LIB_H_
#define  IPMI_BASE_LIB_H_

//
// Prototype definitions for IPMI Library
//
/**
  Initialize the global varible with the pointer of IpmiTransport Protocol.

  @retval EFI_SUCCESS - Always return success

**/
EFI_STATUS
InitializeIpmiBase (
  VOID
  );

/**
  Routine to send commands to BMC.

  @param [in]  NetFunction          Net function of the command.
  @param [in]  Command              IPMI Command.
  @param [in]  CommandData          Command Data.
  @param [in]  CommandDataSize      Size of CommandData.
  @param [out]  ResponseData        Response Data.
  @param [in,out]  ResponseDataSize Response Data Size.

  @retval EFI_NOT_AVAILABLE_YET - IpmiTransport Protocol is not installed yet.

**/
EFI_STATUS
IpmiSubmitCommand (
  IN UINT8      NetFunction,
  IN UINT8      Command,
  IN UINT8      *CommandData,
  IN UINT8      CommandDataSize,
  OUT UINT8     *ResponseData,
  IN OUT UINT8  *ResponseDataSize
  );

#endif

