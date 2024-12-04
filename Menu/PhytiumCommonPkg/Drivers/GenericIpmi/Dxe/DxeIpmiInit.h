/** @file
Ipmi Initlize Header File.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation. <BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef  DXE_IPMI_INIT_H_
#define  DXE_IPMI_INIT_H_

#include <Guid/HobList.h>
#include <IpmiPhyStandard.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/TimerLib.h>
#include <Protocol/IpmiInteractiveProtocol.h>

#define IPMI_ATTRIBUTE_DATA_SIGNATURE       SIGNATURE_32 ('I','P','M','I')
#define IPMI_ATTRIBUTE_DATA_FROM_THIS(a)    BASE_CR(a, IPMI_ATTRIBUTE_DATA, mIpmiProtocol)

#define  RETRY_TICKS       3
#define  KCS_DELAY_UNIT    100

typedef struct _IPMI_ATTRIBUTE_DATA  IPMI_ATTRIBUTE_DATA;

//
// Attribute data structure
//
struct _IPMI_ATTRIBUTE_DATA {
  UINTN              Signature;
  IPMI_PHY_PROTOCOL  mIpmiProtocol;
  EFI_HANDLE         ProtocolHandle;
  UINT64             IpmiBaseAddress;
  UINT64             IpmiBaseAddressOffset;
  UINT16             BmcFirmwareVersion;
  UINT8              IpmiVersion;
  IPMI_BMC_STATUS    BmcStatus;
  UINTN              TotalTimeTicks;
};

/**
  Fill Ipmi Attribute data content.

  @param[out]  IpmiInfo  A pointer to IPMI_ATTRIBUTE_DATA.

**/
VOID
InitialIpmiStructure (
  OUT IPMI_ATTRIBUTE_DATA  *IpmiInfo
  );

/**
  Fill Ipmi Attribute data content.

  @retval  IpmiKcs     If current system interface type is KCS.
  @retval  IpmiSmic    If current system interface type is SMIC.
  @retval  IpmiBt      If current system interface type is BT.
  @retval  IpmiSsif    If current system interface type is SSIF.
  @retval  IpmiUnknow  If current system interface type is Unknow.

**/
IPMI_INTERFACE_TYPE
EFIAPI
IpmiGetInterfaceType (
  VOID
  );

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
  );

#endif
