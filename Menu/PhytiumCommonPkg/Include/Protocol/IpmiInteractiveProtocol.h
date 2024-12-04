/** @file
Ipmi protocol Header File.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef  IPMI_INTERACTIVE_PROTOCOL_H_
#define  IPMI_INTERACTIVE_PROTOCOL_H_

#include <BaseIpmi.h>
#include <Ppi/IpmiTransportPpi.h>

typedef struct _IPMI_PHY_PROTOCOL IPMI_PHY_PROTOCOL;

#define IPMI_PHY_PROTOCOL_GUID \
  { \
    0xb4e9d18a, 0xa590, 0x4174, 0x80, 0xa8, 0xda, 0x95, 0xbf, 0x39, 0x54, 0x47 \
  }

/**
  Return system interface type that BMC currently use.

  @retval IpmiKcs        If current system interface type is KCS.
  @retval IpmiSmic       If current system interface type is SMIC.
  @retval IpmiBt         If current system interface type is BT.
  @retval IpmiSsif       If current system interface type is SSIF.
  @retval IpmiUnknow     If current system interface type is Unknow.

*/
typedef
IPMI_INTERFACE_TYPE
(EFIAPI *IPMI_GET_INTERFACE_TYPE) (
  VOID
  );

/**
  Updates the  IPMI Version.

  @param[in]   This         Pointer to IPMI protocol instance.

  @retval  EFI_SUCCESS            Return Successly.
  @retval  Other                  Failure.

**/
typedef
UINT8
(EFIAPI *IPMI_VERSION) (
  IN  IPMI_PHY_PROTOCOL           *This
  );

/**
  Bmc Firmware version.

  @param[in]   This         Pointer to IPMI protocol instance.

  @retval  EFI_SUCCESS            Return Successly.
  @retval  Other                  Failure.

**/
typedef
UINT16
(EFIAPI *IPMI_BMC_FIRMWARE_VERSION) (
  IN  IPMI_PHY_PROTOCOL            *This
  );

/**
  Updates the  Bmc Status.

  @param[in]   This         Pointer to IPMI protocol instance.
  @param[OUT]  BmcStatus    BMC status.

  @retval  EFI_SUCCESS            Return Successly.
  @retval  Other                  Failure.

**/
typedef
EFI_STATUS
(EFIAPI *IPMI_GET_BMC_STATUS) (
  IN  IPMI_PHY_PROTOCOL            *This,
  OUT BMC_STATUS                   *BmcStatus
  );

/**
  This service enables submitting commands via Ipmi.

  @param[in]         This              This point for IPMI_PROTOCOL structure.
  @param[in]         NetFunction       Net function of the command.
  @param[in]         Command           IPMI Command.
  @param[in]         RequestData       Command Request Data.
  @param[in]         RequestDataSize   Size of Command Request Data.
  @param[out]        ResponseData      Command Response Data. The completion code is the first byte of response data.
  @param[in, out]    ResponseDataSize  Size of Command Response Data.

  @retval EFI_SUCCESS            The command byte stream was successfully submit to the device and a response was successfully received.
  @retval EFI_NOT_FOUND          The command was not successfully sent to the device or a response was not successfully received from the device.
  @retval EFI_NOT_READY          Ipmi Device is not ready for Ipmi command access.
  @retval EFI_DEVICE_ERROR       Ipmi Device hardware error.
  @retval EFI_TIMEOUT            The command time out.
  @retval EFI_UNSUPPORTED        The command was not successfully sent to the device.
  @retval EFI_OUT_OF_RESOURCES   The resource allcation is out of resource or data size error.
**/
typedef
EFI_STATUS
(EFIAPI *IPMI_PHY_SUBMIT_COMMAND) (
  IN  IPMI_PHY_PROTOCOL   *This,
  IN  IPMI_CMD_PACKET     SubmitCommand,
  IN  UINT8               *RequestData ,
  IN  UINT8               RequestDataSize,
  OUT UINT8               *ResponseData,
  IN OUT UINT8            *ResponseDataSize
);

//
// IPMI PHY COMMAND PROTOCOL
//
struct  _IPMI_PHY_PROTOCOL {
  IPMI_PHY_SUBMIT_COMMAND       IpmiPhySubmitCommand;
  IPMI_GET_INTERFACE_TYPE       IpmiInterfaceType;
  IPMI_VERSION                  IpmiVersion;
  IPMI_BMC_FIRMWARE_VERSION     BmcFirmwareVersion;
  IPMI_GET_BMC_STATUS           GetBmcStatus;
};

extern EFI_GUID gIpmiTransportProtocolGuid;
extern EFI_GUID gMmIpmiTransportProtocolGuid;

#endif
