/** @file
Generic IPMI transport layer common head file during PEI phase.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef  PEI_IPMI_COMMON_BMC_H_
#define  PEI_IPMI_COMMON_BMC_H_

#include <Ppi/IpmiTransportPpi.h>

#define MAX_SOFT_COUNT    10
#define COMP_CODE_NORMAL  0x00

//
// IPMI command completion codes to check for in the UpdateErrorStatus routine.
// These completion codes indicate a soft error and a running total of the occurrences
// of these errors is maintained.
//
#define COMP_CODE_NODE_BUSY               0xC0
#define COMP_CODE_TIMEOUT                 0xC3
#define COMP_CODE_OUT_OF_SPACE            0xC4
#define COMP_CODE_OUT_OF_RANGE            0xC9
#define COMP_CODE_CMD_RESP_NOT_PROVIDED   0xCE
#define COMP_CODE_FAIL_DUP_REQUEST        0xCF
#define COMP_CODE_SDR_REP_IN_UPDATE_MODE  0xD0
#define COMP_CODE_DEV_IN_FW_UPDATE_MODE   0xD1
#define COMP_CODE_BMC_INIT_IN_PROGRESS    0xD2
#define COMP_CODE_UNSPECIFIED             0xFF

#define COMPLETION_CODES \
  { \
    COMP_CODE_NODE_BUSY, COMP_CODE_TIMEOUT, COMP_CODE_OUT_OF_SPACE, COMP_CODE_OUT_OF_RANGE, \
    COMP_CODE_CMD_RESP_NOT_PROVIDED, COMP_CODE_FAIL_DUP_REQUEST, COMP_CODE_SDR_REP_IN_UPDATE_MODE, \
    COMP_CODE_DEV_IN_FW_UPDATE_MODE, COMP_CODE_BMC_INIT_IN_PROGRESS, COMP_CODE_UNSPECIFIED \
  }
//
// Ensure proper structure formats
//
#pragma pack(1)
//
// Pei Ipmi instance data
//
typedef struct {
  UINTN                  Signature;
  UINT64                 KcsTimeoutPeriod;
  BMC_STATUS             BmcStatus;
  UINT8                  SoftErrorCount;
  UINT64                 IpmiIoBase;
  PEI_IPMI_TRANSPORT_PPI IpmiTransportPpi;
  EFI_PEI_PPI_DESCRIPTOR PeiIpmiBmcDataDesc;
} PEI_IPMI_BMC_INSTANCE_DATA;

#pragma pack()


#endif
