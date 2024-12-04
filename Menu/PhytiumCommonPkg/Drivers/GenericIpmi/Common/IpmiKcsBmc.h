/** @file
KCS operation interfaces Header File.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation. <BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef  IPMI_KCS_BMC_H_
#define  IPMI_KCS_BMC_H_

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/TimerLib.h>
#include <Protocol/IpmiInteractiveProtocol.h>

#define KCS_WRITE_START       0x61
#define KCS_WRITE_END         0x62
#define KCS_READ              0x68
#define KCS_GET_STATUS        0x60
#define KCS_ABORT             0x60

typedef enum {
  KcsIdleState,
  KcsReadState,
  KcsWriteState,
  KcsErrorState
} KCS_STATE;

//
// KCS Interface Status Register
//
typedef struct {
  UINT8 Obf    : 1;
  UINT8 Ibf    : 1;
  UINT8 SmsAtn : 1;
  UINT8 BitCD  : 1;
  UINT8 Oem1   : 1;
  UINT8 Oem2   : 1;
  UINT8 BitS0  : 1;
  UINT8 BitS1  : 1;
} KCS_STATUS;


/**
  KCS System Interface Send Data Processing.

  @param[in]  KcsIoAddr        The data represents the IO address of the KCS Interface.
  @param[in]  Request          IPMI_CMD_PACKET sturcture, include NetFnLun and Cmd.
  @param[in]  RequestData      Command data buffer to be written to BMC.
  @param[in]  RequestDataSize  Command data length of command data buffer.

  @retval EFI_SUCCESS  Write Mode finish successfully.
  @retval EFI_ABORTED  KCS Interface is idle.
  @retval EFI_TIMEOUT  Output buffer is not full or iutput buffer is not empty
                       in a given time.
  @retval EFI_DEVICE_ERROR  KCS Interface cannot enter idle state.

**/
EFI_STATUS
KcsSendToBmcMode (
  IN  UINT64               KcsIoAddr,
  IN  UINT64               KcsTimePeriod,
  IN  IPMI_CMD_PACKET      *Request,
  IN  UINT8                *RequestData,
  IN  UINT8                RequestDataSize
  );

/**
  KCS System Interface Receive from BMC Data Processing.

  @param[in]   KcsIoAddr         The data represents the IO address of the KCS Interface.
  @param[out]  Response          IPMI_CMD_PACKET sturcture.
  @param[out]  Context           Cmd of this transaction.
  @param[out]  ResponseData      Data buffer to put the data read from BMC (from completion code).
  @param[out]  ResponseDataSize  Length of Data readed from BMC.

  @retval EFI_SUCCESS  Read Mode finish successfully.
  @retval EFI_ABORTED  KCS Interface is idle.
  @retval EFI_TIMEOUT  Output buffer is not full or iutput buffer is not empty
                       in a given time.
  @retval EFI_DEVICE_ERROR  KCS Interface cannot enter idle state.

**/
EFI_STATUS
KcsReceiveBmcMode (
  IN  UINT64               KcsIoAddr,
  IN  UINT64               KcsTimePeriod,
  OUT IPMI_CMD_PACKET      *Response,
  OUT UINT8                *Context,
  OUT UINT8                *ResponseData,
  OUT UINT8                *ResponseDataSize
  );

#endif
