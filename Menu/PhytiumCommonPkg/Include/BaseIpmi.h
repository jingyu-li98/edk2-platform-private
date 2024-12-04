/** @file
Main header file for all IPMI drivers.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef  BASE_IPMI_H_
#define  BASE_IPMI_H_

#include <IpmiPhyStandard.h>

#pragma pack(1)

//
// Define IPMI Version default 00h, in spec. 00h is reserved.
//
#define IPMI_VERSION_DEFAULT                   0x00

//
// System Interface Type
//
typedef enum {
  IpmiUnknow,
  IpmiKcs,
  IpmiSmic,
  IpmiBt,
  IpmiSsif,
  IpmiMaxInterfaceType
} IPMI_INTERFACE_TYPE;

//
// BMC Status
//
typedef enum {
  BmcStatusUnknow,
  BmcStatusOk,
  BmcStatusNotReady,
  BmcStatusNotFound,
  BmcStatusError
} IPMI_BMC_STATUS;

//
// Structure to store IPMI Network Function, LUN and command
//
typedef struct {
  UINT8 Lun   : 2;
  UINT8 NetFn : 6;
  UINT8 Cmd;
} IPMI_CMD_PACKET;

//
// Completion code
//
#define IPMI_COMPLETE_CODE_NO_ERROR               0x00
#define IPMI_RESPONSE_NETFN_BIT                   0x01

//
// LUN
//
#define IPMI_BMC_LUN               0x00

//
// Net Function Definition
//

#define IPMI_NETFN_APPLICATION     0x06
#define IPMI_NETFN_TRANSPORT       0x0C

//
// KCS Interface Control Codes
//
#define KCS_CC_GET_STATUS_ABORT   0x60
#define KCS_CC_WRITE_START        0x61
#define KCS_CC_WRITE_END          0x62
#define KCS_CC_READ               0x68

//
// KCS Interface Status Codes
//
#define KCS_SC_NO_ERROR             0x00
#define KCS_SC_ABORTED_BY_COMMAND   0x01
#define KCS_SC_ILLEGAL_CONTROL_CODE 0x02
#define KCS_SC_LENGTH_ERROR         0x06
#define KCS_SC_UNSPECIFIED_ERROR    0xFF

//
// KCS Interface Status Bits
//
#define KCS_IDLE_STATE      0x00
#define KCS_READ_STATE      0x01
#define KCS_WRITE_STATE     0x02
#define KCS_ERROR_STATE     0x03

#pragma pack()

#endif
