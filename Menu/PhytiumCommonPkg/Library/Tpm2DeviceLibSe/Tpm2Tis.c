/** @file
This library is Se interface TPM2 device lib.

Copyright (C) 2024, Phytium Technology Co Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/Tpm20.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/PcdLib.h>
#include <Library/ScmiLib.h>
#include <Library/TimerLib.h>
#include <Library/IoLib.h>

#define MAX_PACKAGE_SIZE      772
#define MAX_DATA_SIZE         (MAX_PACKAGE_SIZE -4)

#define SCMI_PROTOCOL_ID_TPM  0x81
#define TPM_SEND_COMMAND      0x21
#define TPM_REQUEST_RESPONSE  0x22
#define TPM_READ_RESPONSE     0x23

// Turn on debug message by enabling below define
/*#define TPM_DEBUG*/
#ifdef TPM_DEBUG
#define DBG(arg...) DEBUG((D_ERROR,## arg))
#else
#define DBG(arg...)
#endif

typedef struct {
  UINT16 DataOffset;
  UINT16 DataSize;
} DATA_DESCRIPTOR;

typedef struct {
  DATA_DESCRIPTOR DataDescriptor;
  UINT8 DataBlock[MAX_DATA_SIZE];
} TPM_SEND_PAYLOAD;

/**
  This service enables the sending of commands to the TPM2.

  @param[in]      SizeIn      Size of the TPM2 input.
  @param[in]      BufferIn    Pointer to the TPM2 input.

  @retval EFI_SUCCESS            The command byte stream was successfully sent to the device and a response was successfully received.
  @retval EFI_DEVICE_ERROR       The command was not successfully sent to the device or a response was not successfully received from the device.
**/
EFI_STATUS
EFIAPI
PackagingAndASend (
  IN UINT32      SizeIn,
  IN UINT8       *BufferIn
  )
{
  UINT16            DataOffset;
  UINT16            DataSize;
  UINT16            DataEnd;
  UINT32            Index;
  UINT32            Len;
  UINT32            ReturnValue;
  UINT32            PackageTotal;
  EFI_STATUS        Status;
  TPM_SEND_PAYLOAD  PayLoad;
  SCMI_ADDRESS_BASE Base;

  Base.MhuConfigBase = PcdGet64 (PcdMhuConfigBaseAddress);
  Base.MhuBase = PcdGet64 (PcdMhuBaseAddress);
  Base.ShareMemoryBase = PcdGet64 (PcdMhuShareMemoryBase);

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

  DataEnd = 0;
  PackageTotal = SizeIn / MAX_DATA_SIZE;
  PackageTotal += SizeIn % MAX_DATA_SIZE > 0 ? 1: 0;

  for (Index = 0; Index < PackageTotal; Index++) {
    DataOffset = Index * MAX_DATA_SIZE;
    if (SizeIn - Index * MAX_DATA_SIZE > MAX_DATA_SIZE) {
      DataSize = MAX_DATA_SIZE;
    } else {
      DataSize = SizeIn - Index * MAX_DATA_SIZE;
    }
    PayLoad.DataDescriptor.DataSize = DataSize;
    PayLoad.DataDescriptor.DataOffset = DataOffset;
    CopyMem(PayLoad.DataBlock, &BufferIn[Index * MAX_DATA_SIZE], DataSize);

    Status = ScmiCommandExecuteBytes (
               &Base,
               SCMI_PROTOCOL_ID_TPM,
               TPM_SEND_COMMAND,
               MAX_PACKAGE_SIZE,
               (UINT32 *)&PayLoad,
               &Len,
               &ReturnValue
               );

    if (EFI_ERROR(Status)) {
      return EFI_DEVICE_ERROR;
    }

    DataEnd += DataSize;
  }

  PayLoad.DataDescriptor.DataSize = 0;
  PayLoad.DataDescriptor.DataOffset = DataEnd;
  Status = ScmiCommandExecuteBytes (
             &Base,
             SCMI_PROTOCOL_ID_TPM,
             TPM_SEND_COMMAND,
             MAX_PACKAGE_SIZE,
             (UINT32 *)&PayLoad,
             &Len,
             &ReturnValue
             );

  return Status;
}

/**
  This service enables the receive response from the TPM2.

  @param[in,out]  SizeOut     Size of the TPM2 output.
  @param[in]      BufferOut   Pointer to the TPM2 output.

  @retval EFI_SUCCESS            The command byte stream was successfully sent to the device and a response was successfully received.
  @retval EFI_DEVICE_ERROR       The command was not successfully sent to the device or a response was not successfully received from the device.
**/
EFI_STATUS
EFIAPI
RecivePackages (
  IN OUT UINT32  *SizeOut,
  IN UINT8       *BufferOut
  )
{
  UINT32                Len;
  UINT32                Flag;
  UINT32                Index;
  UINT32                PackageTotal;
  UINT32                ResponseLen;
  UINT32                ReturnValues[MAX_DATA_SIZE + 1];
  UINT64                SeTpmStatusReg;
  EFI_STATUS            Status;
  DATA_DESCRIPTOR       DataDescriptor;
  SCMI_ADDRESS_BASE     Base;
  TPM2_RESPONSE_HEADER  *Respose;

  Base.MhuConfigBase = PcdGet64 (PcdMhuConfigBaseAddress);
  Base.MhuBase = PcdGet64 (PcdMhuBaseAddress);
  Base.ShareMemoryBase = PcdGet64 (PcdMhuShareMemoryBase);
  SeTpmStatusReg = PcdGet64(PcdSeTpmStatusReg);

  Flag = 0x0;
  while (0x1 != Flag) {
    Flag = MmioRead32 (SeTpmStatusReg);
  }
  ResponseLen = MmioRead32 (SeTpmStatusReg + 4);
  MmioWrite32 (SeTpmStatusReg, 0x0);
  MmioWrite32 (SeTpmStatusReg + 0x4, 0x0);

  PackageTotal = ResponseLen / MAX_DATA_SIZE;
  PackageTotal += ResponseLen % MAX_DATA_SIZE > 0 ? 1: 0;

  for (Index = 0; Index < PackageTotal; Index++) {
    DataDescriptor.DataOffset = Index * MAX_DATA_SIZE;
    if (ResponseLen - Index * MAX_DATA_SIZE > MAX_DATA_SIZE) {
      DataDescriptor.DataSize = MAX_DATA_SIZE;
    } else {
      DataDescriptor.DataSize = ResponseLen - Index * MAX_DATA_SIZE;
    }

    Status = ScmiCommandExecuteBytes (
               &Base,
               SCMI_PROTOCOL_ID_TPM,
               TPM_READ_RESPONSE,
               sizeof (DATA_DESCRIPTOR),
               (UINT32 *)&DataDescriptor,
               &Len,
               ReturnValues
               );

    CopyMem (&BufferOut[Index * MAX_DATA_SIZE], &ReturnValues[1], Len);
    if (EFI_ERROR(Status)) {
      return EFI_DEVICE_ERROR;
    }
  }

  DEBUG_CODE_BEGIN ();

  DBG ("TpmCommand ReceiveHeader - ");
  for (Index = 0; Index < sizeof (TPM2_RESPONSE_HEADER); Index++) {
    DBG ("%02x ", BufferOut[Index]);
  }
  DBG ("\n");

  DBG ("TpmCommand Receive - ");
  Respose = (TPM2_RESPONSE_HEADER *)BufferOut;
  *SizeOut = SwapBytes32(Respose->paramSize);

  for (Index = 0; Index < *SizeOut; Index++) {
    DBG ("%02x ", BufferOut[Index]);
  }
  DBG ("\n");

  DEBUG_CODE_END ();

  return EFI_SUCCESS;
}

/**
  This service requests use TPM2.

  @retval EFI_SUCCESS      Get the control of TPM2 chip.
  @retval EFI_NOT_FOUND    TPM2 not found.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
SeTpm2RequestUseTpm (
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
  @param[in]      BufferOut   Pointer to the TPM2 output.

  @retval EFI_SUCCESS            The command byte stream was successfully sent to the device and a response was successfully received.
  @retval EFI_DEVICE_ERROR       The command was not successfully sent to the device or a response was not successfully received from the device.
  @retval EFI_BUFFER_TOO_SMALL   The output parameter block is too small.
**/
EFI_STATUS
EFIAPI
SeTpm2SubmitCommand (
  IN UINT32      SizeIn,
  IN UINT8       *BufferIn,
  IN OUT UINT32  *SizeOut,
  IN UINT8       *BufferOut
  )
{
  EFI_STATUS  Status;

  Status = PackagingAndASend (SizeIn, BufferIn);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = RecivePackages (SizeOut, BufferOut);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
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
  return SeTpm2SubmitCommand (
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
  return SeTpm2RequestUseTpm ();
}

