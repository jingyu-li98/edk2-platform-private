/** @file
Generic IPMI stack head file during PEI phase.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation. <BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef  PEI_IPMI_INIT_H_
#define  PEI_IPMI_INIT_H_

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/IpmiBaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PciLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/TimerLib.h>
#include <PiPei.h>
#include <Uefi.h>
#include "PeiIpmiBmcDef.h"

//
// IPMI Instance signature
//
#define SM_IPMI_BMC_SIGNATURE SIGNATURE_32 ('I', 'P', 'M', 'I')

#define INSTANCE_FROM_PEI_SM_IPMI_BMC_THIS(a) \
  CR ( \
  a, \
  PEI_IPMI_BMC_INSTANCE_DATA, \
  IpmiTransportPpi, \
  SM_IPMI_BMC_SIGNATURE \
  )

//
// Prototypes
//
#define BMC_KCS_TIMEOUT_PEI               5     // Single KSC request timeout
#define KCS_DELAY_UNIT_PEI                100   // Each KSC IO delay

typedef UINT32  BMC_STATUS;

#define BMC_OK        0
#define BMC_SOFTFAIL  1
#define BMC_HARDFAIL  2
#define BMC_UPDATE_IN_PROGRESS  3
#define BMC_NOTREADY  4

/**
  Updates the  Bmc Status.

  @param[in]   This         Pointer to IPMI protocol instance.
  @param[OUT]  BmcStatus    BMC status.

  @retval  EFI_SUCCESS            Return Successly.
  @retval  Other                  Failure.

**/
EFI_STATUS
PeiGetIpmiBmcStatus (
  IN  PEI_IPMI_TRANSPORT_PPI  *This,
  OUT BMC_STATUS              *BmcStatus
  );

/**
  Pei Phase Routine to send commands to BMC.

  @param[in]      This              A pointer to PEI_IPMI_TRANSPORT_PPI.
  @param[in]      NetFunction       Net Function of command to send.
  @param[in]      Lun               LUN of command to send.
  @param[in]      Command           IPMI command to send.
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
  IN  UINT8                   NetFunction,
  IN  UINT8                   Lun,
  IN  UINT8                   Command,
  IN  UINT8                   *RequestData,
  IN  UINT8                   RequestDataSize,
  OUT UINT8                   *ResponseData,
  IN OUT UINT8                *ResponseDataSize
  );

#endif

