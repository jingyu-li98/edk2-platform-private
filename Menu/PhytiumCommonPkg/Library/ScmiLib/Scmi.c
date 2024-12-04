/**
  Realize sending and receiving SCMI messages at the communication layer.

  Copyright (C) 2022-2023, Phytium Technology Co, Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include "Scmi.h"

/**
  Send Scmi message. First set the channel to busy state, and then set the
  message header to MHU_SET register.

  @param[in]      Base    Mhu base address.
  @param[in,out]  MBox    A potinter of MAILBOX_MEM_E_T.

  @retval         EFI_SUCCESS.
**/
STATIC
EFI_STATUS
ScmiSendMessage (
  IN     UINT64           Base,
  IN OUT MAILBOX_MEM_E_T  *MBox
  )
{
  SCMI_MARK_CHANNEL_BUSY(MBox->Status);
  ArmDataMemoryBarrier();
  MmioWrite32 (Base + 0x8, MBox->MsgHeader);
  ArmDataMemoryBarrier();

  return EFI_SUCCESS;
}

/**
  Receiver scmi message result. The timeout is 500ms. Return the numbers of
  payloads.

  @param[in,out]  MBox    A potinter of MAILBOX_MEM_E_T.
  @param[out]     Len     Numbers of return payload.

  @retval         EFI_TIMEOUT.
                  EFI_SUCCESS.
**/
STATIC
EFI_STATUS
ScmiReceiveMessage (
  IN OUT MAILBOX_MEM_E_T  *MBox,
  OUT    UINT32           *Len
  )
{
  UINT32      TimeOut;

  TimeOut = SCMI_RESPONSE_TIMEOUT_CHECKTIMES;
  while (TimeOut--) {
    MicroSecondDelay (SCMI_RESPONSE_TIMEOUT_INTERNAL);
    if (SCMI_IS_CHANNEL_FREE(MBox->Status)) {
      break;
    }
    if (TimeOut == 0) {
      return EFI_TIMEOUT;
    }
  }
  ArmDataMemoryBarrier();
  DEBUG ((DEBUG_INFO, "MBox->Len : %d\n", MBox->Len));
  *Len = (MBox->Len - sizeof (MBox->MsgHeader)) / 4;

  return EFI_SUCCESS;
}

/**
  Receiver scmi message result wiht UINT8 type. The timeout is 500ms. Return the numbers of
  payloads.

  @param[in,out]  MBox    A potinter of MAILBOX_MEM_E_T.
  @param[out]     Len     Numbers of return payload.

  @retval         EFI_TIMEOUT.
                  EFI_SUCCESS.
**/
STATIC
EFI_STATUS
ScmiReceiveMessageBytes (
  IN OUT MAILBOX_MEM_E_T  *MBox,
  OUT    UINT32           *Len
  )
{
  UINT32      TimeOut;

  TimeOut = SCMI_RESPONSE_TIMEOUT_CHECKTIMES;
  while (TimeOut--) {
    MicroSecondDelay (SCMI_RESPONSE_TIMEOUT_INTERNAL);
    if (SCMI_IS_CHANNEL_FREE(MBox->Status)) {
      break;
    }
    if (TimeOut == 0) {
      return EFI_TIMEOUT;
    }
  }
  ArmDataMemoryBarrier();
  *Len = (MBox->Len - sizeof (MBox->MsgHeader));

  return EFI_SUCCESS;
}

/**
  Execute the SCMI command. Input protocol ID, message ID, the length and
  parameters of payload, return the length and results of payload. The
  parameter ReturnValues must not be NULL, and large enough.

  @param[in]   Base            A pointer to SCMI_ADDRESS_BASE.
  @param[in]   ProtocolId      Protocol ID.
  @param[in]   MessageId       Message ID.
  @param[in]   LenIn           Number of input payload parameters.
  @param[in]   ParIn           A pointer of input payload.
  @param[out]  LenOut          A pointer of output payload parameter number.
  @param[out]  ReturnValues    A pointer of output payload buffer.

  @retval      EFI_SUCCESS     Execute command successfully.
               EFI_TIMEOUT     Execute command timeout.
               EFI_INVALID_PARAMETER  ReturnValues or LenOut is NULL.
**/
EFI_STATUS
ScmiCommandExecute (
  IN   SCMI_ADDRESS_BASE  *Base,
  IN   UINT32             ProtocolId,
  IN   UINT32             MessageId,
  IN   UINT32             LenIn,
  IN   UINT32             *ParIn,
  OUT  UINT32             *LenOut,
  OUT  UINT32             *ReturnValues
  )
{
  MAILBOX_MEM_E_T  *MbxMem;
  UINT32           Token;
  EFI_STATUS       Status;
  UINT32           Index;

  DEBUG ((DEBUG_INFO,
          "Scmi Mhu Config Base : 0x%llx, Mhu Base : 0x%llx, Share Memory : 0x%llx\n",
          Base->MhuConfigBase,
          Base->MhuBase,
          Base->ShareMemoryBase
          ));

  if ((ReturnValues == NULL) || (LenOut == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  Status = EFI_SUCCESS;
  MmioWrite32 (Base->MhuConfigBase + 0xc, 0x1);
  //
  //Fill MailBox
  //
  Token = 0;
  MbxMem = (MAILBOX_MEM_E_T *) (UINT64) Base->ShareMemoryBase;
  CopyMem (MbxMem->Payload, ParIn, LenIn * 4);
  MbxMem->Len = sizeof (MbxMem->MsgHeader) + LenIn * 4;
  MbxMem->Flags = 0;
  MbxMem->MsgHeader = SCMI_MSG_CREATE(ProtocolId, MessageId, Token);
  DEBUG ((DEBUG_INFO,
          "Scmi Message Info :\nProtocol ID : %x\nMessage ID : %x\n",
          ProtocolId, MessageId
          ));
  for (Index = 0; Index < LenIn; Index++) {
    DEBUG ((DEBUG_INFO, "Payload[%d] : %x\n", Index, ParIn[Index]));
  }

  //
  //Send Message
  //
  ScmiSendMessage (Base->MhuBase, MbxMem);
  //
  //Receive Message
  //
  Status = ScmiReceiveMessage (MbxMem, LenOut);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Scmi receive message failed! Status : %r\n", Status));
    return Status;
  }

  //ReturnValues = MbxMem->Payload;
  CopyMem (ReturnValues, (VOID*)MbxMem->Payload, (*LenOut) * 4);
  DEBUG ((DEBUG_INFO, "Payload %p\n", MbxMem->Payload));
  DEBUG ((DEBUG_INFO, "Scmi Receive:\nLen : %d\n", *LenOut));
  for (Index = 0; Index < *LenOut; Index++) {
    DEBUG ((DEBUG_INFO, "Payload[%d] : %x\n",
                Index, MmioRead32 ((UINT64)(ReturnValues + Index))));
  }

  return EFI_SUCCESS;
}

/**
  Execute the SCMI command with UINT8 type. Input protocol ID, message ID, the length and
  parameters of payload, return the length and results of payload. The
  parameter ReturnValues must not be NULL, and large enough.

  @param[in]   Base            A pointer to SCMI_ADDRESS_BASE.
  @param[in]   ProtocolId      Protocol ID.
  @param[in]   MessageId       Message ID.
  @param[in]   LenIn           Number of input payload parameters.
  @param[in]   ParIn           A pointer of input payload.
  @param[out]  LenOut          A pointer of output payload parameter number.
  @param[out]  ReturnValues    A pointer of output payload buffer.

  @retval      EFI_SUCCESS     Execute command successfully.
               EFI_TIMEOUT     Execute command timeout.
               EFI_INVALID_PARAMETER  ReturnValues or LenOut is NULL.
**/
EFI_STATUS
ScmiCommandExecuteBytes (
  IN   SCMI_ADDRESS_BASE  *Base,
  IN   UINT32             ProtocolId,
  IN   UINT32             MessageId,
  IN   UINT32             LenIn,
  IN   UINT32             *ParIn,
  OUT  UINT32             *LenOut,
  OUT  UINT32             *ReturnValues
  )
{
  MAILBOX_MEM_E_T  *MbxMem;
  UINT32           Token;
  EFI_STATUS       Status;

  if ((ReturnValues == NULL) || (LenOut == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  Status = EFI_SUCCESS;
  MmioWrite32 (Base->MhuConfigBase + 0xc, 0x1);
  //
  //Fill MailBox
  //
  Token = 0;
  MbxMem = (MAILBOX_MEM_E_T *) (UINT64) Base->ShareMemoryBase;
  CopyMem (MbxMem->Payload, ParIn, LenIn);
  MbxMem->Len = sizeof (MbxMem->MsgHeader) + LenIn;
  MbxMem->Flags = 0;
  MbxMem->MsgHeader = SCMI_MSG_CREATE(ProtocolId, MessageId, Token);

  //
  //Send Message
  //
  ScmiSendMessage (Base->MhuBase, MbxMem);
  //
  //Receive Message
  //
  Status = ScmiReceiveMessageBytes (MbxMem, LenOut);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Scmi receive message failed! Status : %r\n", Status));
    return Status;
  }

  CopyMem (ReturnValues, (VOID*)MbxMem->Payload, *LenOut);

  return EFI_SUCCESS;
}
