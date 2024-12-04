/** @file
This library is Optee interface TPM2 device lib.

Copyright (C) 2024, Phytium Technology Co Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/Tpm20.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/PcdLib.h>
#include <Library/TimerLib.h>
#include <Library/OpteeLib.h>


#define TPM_CMD_WRITE         0x1
#define TPM_CMD_READ          0x2

// Turn on debug message by enabling below define
//#define TPM_DEBUG
#ifdef TPM_DEBUG
#define DBG(arg...) DEBUG((D_ERROR,## arg))
#else
#define DBG(arg...)
#endif

/**
  This service requests use TPM2.

  @retval EFI_SUCCESS      Get the control of TPM2 chip.
  @retval EFI_NOT_FOUND    TPM2 not found.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
OpteeTpm2RequestUseTpm (
  VOID
  )
{
  return EFI_SUCCESS;
}


/**
  This service enables the sending of commands to the TPM2.

  @param[in]      SizeIn      Size of the TPM2 input.
  @param[in]      BufferIn    Pointer to the TPM2 input.
  @param[in,out]  SizeOut     Size of the TPM2 output.
  @param[in,out]  BufferOut   Pointer to the TPM2 output.

  @retval EFI_SUCCESS            The command byte stream was successfully sent to the device and a response was successfully received.
  @retval EFI_DEVICE_ERROR       The command was not successfully sent to the device or a response was not successfully received from the device.
  @retval EFI_BUFFER_TOO_SMALL   The output parameter block is too small.
**/
EFI_STATUS
EFIAPI
OpteeTpm2SubmitCommand (
  IN UINT32      SizeIn,
  IN UINT8       *BufferIn,
  IN OUT UINT32  *SizeOut,
  IN OUT UINT8   *BufferOut
  )
{
  UINT32                      Index;
  UINT32                      TpmOutSize;
  UINT16                      Data16;
  UINT32                      Data32;
  EFI_STATUS                  Status;
  OPTEE_INVOKE_FUNCTION_ARG   InvokeArg;
  OPTEE_OPEN_SESSION_ARG      TpmSessionArg;


  DEBUG_CODE_BEGIN ();
  UINTN  DebugSize;

  DBG ("TpmCommand Send - ");
  if (SizeIn > 0x100) {
    DebugSize = 0x40;
  } else {
    DebugSize = SizeIn;
  }

  for (Index = 0; Index < DebugSize; Index++) {
    DBG ("%02x ", BufferIn[Index]);
  }

  if (DebugSize != SizeIn) {
    DBG ("...... ");
    for (Index = SizeIn - 0x20; Index < SizeIn; Index++) {
      DBG ("%02x ", BufferIn[Index]);
    }
  }

  DBG ("\n");
  DEBUG_CODE_END ();

  ZeroMem (&TpmSessionArg, sizeof (OPTEE_OPEN_SESSION_ARG));
  CopyMem (&TpmSessionArg.Uuid, &gOpteeTpmGuid, sizeof (EFI_GUID));

  Status = OpteeInit ();
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "OpteeInit Failed\n"));
    return EFI_DEVICE_ERROR;
  }

  Status = OpteeOpenSession (&TpmSessionArg);
  if ((Status != EFI_SUCCESS) || (TpmSessionArg.Return != OPTEE_SUCCESS)) {
    DEBUG ((DEBUG_ERROR, "Can't Open Tpm Session\n"));
    return EFI_DEVICE_ERROR;
  }

  ZeroMem (&InvokeArg, sizeof (InvokeArg));
  InvokeArg.Session = TpmSessionArg.Session;
  InvokeArg.Function = TPM_CMD_WRITE;
  InvokeArg.Params[0].Attribute = OPTEE_MESSAGE_ATTRIBUTE_TYPE_MEMORY_INPUT;
  InvokeArg.Params[0].Union.Memory.Size = SizeIn;
  InvokeArg.Params[0].Union.Memory.BufferAddress = (UINT64)BufferIn;
  Status = OpteeInvokeFunction (&InvokeArg);
  if ((EFI_ERROR(Status) || (InvokeArg.Return != 0))) {
    DEBUG ((DEBUG_ERROR, "Tpm Send Command Fialed\n"));
    goto Exit;
  }

  ZeroMem (&InvokeArg, sizeof (InvokeArg));
  InvokeArg.Session = TpmSessionArg.Session;
  InvokeArg.Function = TPM_CMD_READ;
  InvokeArg.Params[0].Attribute = OPTEE_MESSAGE_ATTRIBUTE_TYPE_MEMORY_OUTPUT;
  InvokeArg.Params[0].Union.Memory.Size = 2048;
  InvokeArg.Params[0].Union.Memory.BufferAddress = (UINT64)BufferOut;
  Status = OpteeInvokeFunction (&InvokeArg);
  if ((EFI_ERROR(Status) || (InvokeArg.Return != 0))) {
    DEBUG ((DEBUG_ERROR, "Tpm Read Command Fialed\n"));
    goto Exit;
  }

  DEBUG_CODE_BEGIN ();
  DBG ("TpmCommand ReceiveHeader - ");
  for (Index = 0; Index < sizeof (TPM2_RESPONSE_HEADER); Index++) {
    DBG ("%02x ", BufferOut[Index]);
  }

  DBG ("\n");
  DEBUG_CODE_END ();
  //
  // Check the response data header (tag,parasize and returncode )
  //
  CopyMem (&Data16, BufferOut, sizeof (UINT16));
  // TPM2 should not use this RSP_COMMAND
  if (SwapBytes16 (Data16) == TPM_ST_RSP_COMMAND) {
    DEBUG ((DEBUG_ERROR, "TPM2: TPM_ST_RSP error - %x\n", TPM_ST_RSP_COMMAND));
    Status = EFI_UNSUPPORTED;
    goto Exit;
  }

  CopyMem (&Data32, (BufferOut + 2), sizeof (UINT32));
  TpmOutSize = SwapBytes32 (Data32);
  if (*SizeOut < TpmOutSize) {
    Status = EFI_BUFFER_TOO_SMALL;
    goto Exit;
  }
  *SizeOut = TpmOutSize;

  DEBUG_CODE_BEGIN ();
  for (Index = 0; Index < *SizeOut; Index++) {
    DBG ("%02x ", BufferOut[Index]);
  }

  DBG ("\n");
  DEBUG_CODE_END ();

  OpteeCloseSession(TpmSessionArg.Session);
  return EFI_SUCCESS;

Exit:
  OpteeCloseSession(TpmSessionArg.Session);
  return Status;
}

/**
  This service enables the sending of commands to the TPM2.

  @param[in]      InputParameterBlockSize  Size of the TPM2 input parameter block.
  @param[in]      InputParameterBlock      Pointer to the TPM2 input parameter block.
  @param[in,out]  OutputParameterBlockSize Size of the TPM2 output parameter block.
  @param[in]      OutputParameterBlock     Pointer to the TPM2 output parameter block.

  @retval EFI_SUCCESS            The command byte stream was successfully sent to the device and a response was successfully received.
  @retval EFI_DEVICE_ERROR       The command was not successfully sent to the device or a response was not successfully received from the device.
  @retval EFI_BUFFER_TOO_SMALL   The output parameter block is too small.
**/
EFI_STATUS
EFIAPI
Tpm2SubmitCommand (
  IN UINT32      InputParameterBlockSize,
  IN UINT8       *InputParameterBlock,
  IN OUT UINT32  *OutputParameterBlockSize,
  IN UINT8       *OutputParameterBlock
  )
{
  return OpteeTpm2SubmitCommand (
           InputParameterBlockSize,
           InputParameterBlock,
           OutputParameterBlockSize,
           OutputParameterBlock
           );

}

/**
  This service requests use TPM2.

  @retval EFI_SUCCESS      Get the control of TPM2 chip.
  @retval EFI_NOT_FOUND    TPM2 not found.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2RequestUseTpm (
  VOID
  )
{
  return OpteeTpm2RequestUseTpm ();
}

