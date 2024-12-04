/** @file
Generic MM IPMI Header File.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation. <BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef MM_IPMI_BMC_H_
#define MM_IPMI_BMC_H_

//
// Statements that include other files
//

#include <Guid/HobList.h>
#include <IndustryStandard/Ipmi.h>
#include <IpmiCommandLib.h>
#include <IpmiPhyStandard.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BmcBaseLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/IpmiBaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ReportStatusCodeLib.h>

#include <Library/MmServicesTableLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Uefi.h>
#include <Protocol/IpmiInteractiveProtocol.h>
#include "IpmiKcsBmc.h"

#define IPMI_MM_ATTRIBUTE_DATA_SIGNATURE       SIGNATURE_32 ('I','P','M','I')
#define IPMI_MM_ATTRIBUTE_DATA_FROM_THIS(a)    BASE_CR(a, IPMI_MM_ATTRIBUTE_DATA, mIpmiProtocol)

#define KCS_DELAY_UNIT_MM     100
#define BMC_KCS_TIMEOUT       5

typedef struct _IPMI_MM_ATTRIBUTE_DATA    IPMI_MM_ATTRIBUTE_DATA;

//
// MM IPMI Attribute data structure
//
struct _IPMI_MM_ATTRIBUTE_DATA {
  UINTN              Signature;
  IPMI_PHY_PROTOCOL  mIpmiProtocol;
  UINT64             IpmiBaseAddress;
  UINT8              IpmiVersion;
  BMC_STATUS         BmcStatus;
  UINT64             TotalTimeTicks;
  EFI_HANDLE         IpmiMmHandle;
};

/**
  Updates the  Bmc Status.

  @param[in]   This         Pointer to IPMI protocol instance.
  @param[OUT]  BmcStatus    BMC status.

  @retval  EFI_SUCCESS            Return Successly.
  @retval  Other                  Failure.

**/
EFI_STATUS
EFIAPI
MmIpmiGetBmcStatus (
  IN  IPMI_PHY_PROTOCOL         *This,
  OUT BMC_STATUS                *BmcStatus
  );

/**
  MM Phase Routine to send commands to BMC.

  @param[in]      This              A pointer to IPMI_PHY_PROTOCOL.
  @param[in]      SubmitCmd         Include NetFn Lun and Cmd.
  @param[in]      RequestData       Command Data.
  @param[in]      RequestDataSize   Size of CommandData.
  @param[in]      ResponseData      Response Data.
  @param[in,out]  ResponseDataSize  Response Data Size.

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
  IN OUT UINT8           *ResponseDataSize
  );

#endif

