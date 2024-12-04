/**
  Define registers and structures related to phytium scmi communication.

  Copyright (C) 2022-2023, Phytium Technology Co, Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  _SCMI_H_
#define  _SCMI_H_

#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/ScmiLib.h>
#include <Library/TimerLib.h>

#define SMC_SCMI_ID  0xC300FFF0

#define SCMI_RESPONSE_TIMEOUT_INTERNAL    1000
#define SCMI_RESPONSE_TIMEOUT_CHECKTIMES  500

//
//SCMI mailbox flags
//
#define SCMI_FLAG_RESP_POLL       0
#define SCMI_FLAG_RESP_INT        1

#define SCMI_MSG_ID_SHIFT         0
#define SCMI_MSG_ID_WIDTH         8
#define SCMI_MSG_ID_MASK          ((1 << SCMI_MSG_ID_WIDTH) - 1)

#define SCMI_MSG_TYPE_SHIFT       8
#define SCMI_MSG_TYPE_WIDTH       2
#define SCMI_MSG_TYPE_MASK        ((1 << SCMI_MSG_TYPE_WIDTH) - 1)

#define SCMI_MSG_PROTO_ID_SHIFT   10
#define SCMI_MSG_PROTO_ID_WIDTH   8
#define SCMI_MSG_PROTO_ID_MASK    ((1 << SCMI_MSG_PROTO_ID_WIDTH) - 1)

#define SCMI_MSG_TOKEN_SHIFT      18
#define SCMI_MSG_TOKEN_WIDTH      10
#define SCMI_MSG_TOKEN_MASK       ((1 << SCMI_MSG_TOKEN_WIDTH) - 1)

#define SCMI_MSG_CREATE(PROTOCOL, MSG_ID, TOKEN)       \
  ((((PROTOCOL) & SCMI_MSG_PROTO_ID_MASK) << SCMI_MSG_PROTO_ID_SHIFT) |  \
  (((MSG_ID) & SCMI_MSG_ID_MASK) << SCMI_MSG_ID_SHIFT) |     \
  (((TOKEN) & SCMI_MSG_TOKEN_MASK) << SCMI_MSG_TOKEN_SHIFT))

#define SCMI_CH_STATUS_RES0_MASK  0xFFFFFFFE
#define SCMI_CH_STATUS_FREE_SHIFT 0
#define SCMI_CH_STATUS_FREE_WIDTH 1
#define SCMI_CH_STATUS_FREE_MASK  ((1 << SCMI_CH_STATUS_FREE_WIDTH) - 1)

//
//Helper macros to check and write the channel status
//
#define SCMI_IS_CHANNEL_FREE(STATUS)          \
  (!!(((STATUS) >> SCMI_CH_STATUS_FREE_SHIFT) & SCMI_CH_STATUS_FREE_MASK))

/*    assert(SCMI_IS_CHANNEL_FREE(status));     \*/
#define SCMI_MARK_CHANNEL_BUSY(status)  do {        \
    (status) &= ~(SCMI_CH_STATUS_FREE_MASK <<   \
        SCMI_CH_STATUS_FREE_SHIFT);   \
  } while (0)

typedef struct MAILBOX_MEM_E {
  UINT32 ResA;
  volatile UINT32 Status;
  UINT64 ResB;
  UINT32 Flags;
  volatile UINT32 Len;
  UINT32 MsgHeader;
  UINT32 Payload[];
} MAILBOX_MEM_E_T;

#endif
