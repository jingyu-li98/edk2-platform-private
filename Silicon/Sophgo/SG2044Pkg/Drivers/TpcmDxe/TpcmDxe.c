/** @file
  This file is used to implement TPCM communication.

  Copyright (c) 2025, SOPHGO Technologies Inc. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Include/DwSpi.h>
#include <Include/Tpcm.h>
#include "TpcmDxe.h"

STATIC SOPHGO_SPI_PROTOCOL  *mSpiProtocol;
STATIC SPI_DEVICE           *mTpcmDevice;

  #define FAST_READ_DATA 0xb
  #define PAGE_PROGRAM 0x2
  #define WRITE_ENABLE 0x6

/**
  Computes and returns a 8-bit CRC for a data buffer.
**/
UINT8
CalculateCrc8 (
  IN UINT8  *Buffer,
  IN UINTN  Length
  )
{
  UINT8 Crc = 0x00;

  ASSERT (Buffer != NULL);
  ASSERT (Length <= (MAX_ADDRESS - ((UINTN)Buffer) + 1));

  while (Length--) {
    Crc ^= *Buffer++;
    for (UINTN Index = 0; Index < 8; Index ++) {
      Crc = (Crc & 0x80) ? ((Crc << 1) ^ 0x07) : (Crc << 1);
    }
  }

  return Crc;
}

/**
  Send and receive SPI packets to/from TPCM device.

  @param[in]  Command       The TPCM command to send.
  @param[in]  RequestData   Pointer to the request data buffer.
  @param[in]  RequestSize   Size of the request data buffer.
  @param[out] ResponseData  Pointer to the response data buffer.
  @param[in]  ResponseSize  Size of the response data buffer.

  @retval EFI_SUCCESS           The SPI transaction completed successfully.
  @retval EFI_INVALID_PARAMETER Invalid input parameters.
  @retval EFI_DEVICE_ERROR      The TPCM device returned an invalid response.
  @retval EFI_OUT_OF_RESOURCES  Failed to allocate memory.
**/
EFI_STATUS
TpcmSendReceiveSpiPacket (
  IN  UINT32  Command,
  IN  VOID    *RequestData,
  IN  UINTN   RequestSize,
  OUT VOID    *ResponseData,
  IN  UINTN   ResponseSize
  )
{
  EFI_STATUS      Status;
  TPCM_SPI_PACKET TxPacket;
  TPCM_SPI_PACKET RxPacket;
  SPI_TRANSFER    *SpiTransfer;
  UINT8           *TxBuffer = NULL;
  UINT8           *RxBuffer = NULL;
  UINT8           Crc8;
  UINTN           Offset, ChunkSize, PacketIndex;
  UINTN           MaxPackets;
  UINTN           MaxTotalDataSize;

  if (RequestData == NULL || ResponseData == NULL || RequestSize == 0 || ResponseSize == 0) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Invalid input parameters!\n",
      __func__
      ));
    return EFI_INVALID_PARAMETER;
  }

  MaxTotalDataSize = TPCM_MAX_DATA_SIZE / sizeof (TPCM_SPI_PACKET) * 250;
  if (RequestSize > MaxTotalDataSize) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Data size exceeds maximum limit of %d bytes!\n",
      __func__,
      MaxTotalDataSize
      ));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Allocate memory for SPI buffers
  //
  MaxPackets = (RequestSize + 249) / 250;
  TxBuffer = AllocateZeroPool (sizeof (TPCM_SPI_PACKET) * MaxPackets);
  RxBuffer = AllocateZeroPool (sizeof (TPCM_SPI_PACKET) * MaxPackets);
  if (TxBuffer == NULL || RxBuffer == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Cannot allocate Tx/Rx buffer\n",
      __func__
      ));
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  //
  // Fill SPI packets
  //
  Offset = 0;
  PacketIndex = 0;
  while (Offset < RequestSize) {
    ChunkSize = MIN (250, RequestSize - Offset);

    //
    // Fill Tx Packet
    //
    ZeroMem (&TxPacket, sizeof (TPCM_SPI_PACKET));
    TxPacket.Head          = 0xAC;
    TxPacket.Category      = TDD_CMD_CATEGORY_TPCM;
    TxPacket.DataLength    = (UINT8)ChunkSize;
    CopyMem (TxPacket.Data, (UINT8 *)RequestData + Offset, ChunkSize);
    TxPacket.Tail          = 0xAA;
    TxPacket.NextPacketFlag = (Offset + ChunkSize < RequestSize) ? 1 : 0;

    //
    // Calculate CRC8
    //
    Crc8 = CalculateCrc8 ((UINT8 *)&TxPacket, sizeof (TPCM_SPI_PACKET) - 2);
    TxPacket.Crc8 = Crc8;

    //
    // Copy TxPacket to TxBuffer
    //
    CopyMem (TxBuffer + PacketIndex * sizeof (TPCM_SPI_PACKET), &TxPacket, sizeof (TPCM_SPI_PACKET));

    //
    // Debug: Print Tx Packet
    //
    DEBUG ((DEBUG_INFO, "Tx Packet %d:\n", PacketIndex));
    DEBUG ((DEBUG_INFO, "  Head: 0x%02X\n", TxPacket.Head));
    DEBUG ((DEBUG_INFO, "  Category: 0x%02X\n", TxPacket.Category));
    DEBUG ((DEBUG_INFO, "  DataLength: %d\n", TxPacket.DataLength));
    DEBUG ((DEBUG_INFO, "  Data: "));
    for (UINTN i = 0; i < TxPacket.DataLength; i++) {
      DEBUG ((DEBUG_INFO, "%02X ", TxPacket.Data[i]));
    }
    DEBUG ((DEBUG_INFO, "\n"));
    DEBUG ((DEBUG_INFO, "  Tail: 0x%02X\n", TxPacket.Tail));
    DEBUG ((DEBUG_INFO, "  NextPacketFlag: %d\n", TxPacket.NextPacketFlag));
    DEBUG ((DEBUG_INFO, "  CRC8: 0x%02X\n", TxPacket.Crc8));
    DEBUG ((DEBUG_INFO, "\n"));

    Offset += ChunkSize;
    PacketIndex ++;
  }

  //
  // Initialize SPI_TRANSFER
  //
  SpiTransfer = AllocateZeroPool (sizeof (SPI_TRANSFER));
  if (SpiTransfer == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Cannot allocate SpiTransfer\n",
      __func__
      ));
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  SpiTransfer->TxBuf       = TxBuffer;
  SpiTransfer->RxBuf       = RxBuffer;
  SpiTransfer->Len         = PacketIndex * sizeof (TPCM_SPI_PACKET);
  SpiTransfer->SpeedHz     = TPCM_MAX_CLOCK;
  SpiTransfer->BitsPerWord = 8;

  //
  // Perform SPI transfer
  //
  Status = mSpiProtocol->SpiTransferOne (
                  mSpiProtocol,
                  mTpcmDevice,
                  SpiTransfer
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): SPI transfer failed (Status = %r)!\n",
      __func__,
      Status
      ));
    goto Exit;
  }

  //
  // Rebuild SPI response packets
  //
  Offset = 0;
  PacketIndex = 0;
  while (Offset < SpiTransfer->Len) {
    ZeroMem (&RxPacket, sizeof (TPCM_SPI_PACKET));
    CopyMem (&RxPacket, RxBuffer + PacketIndex * sizeof (TPCM_SPI_PACKET), sizeof (TPCM_SPI_PACKET));

    //
    // Debug: Print Rx Packet
    //
    DEBUG ((DEBUG_INFO, "Rx Packet %d:\n", PacketIndex));
    DEBUG ((DEBUG_INFO, "  Head: 0x%02X\n", RxPacket.Head));
    DEBUG ((DEBUG_INFO, "  Category: 0x%02X\n", RxPacket.Category));
    DEBUG ((DEBUG_INFO, "  DataLength: %d\n", RxPacket.DataLength));
    DEBUG ((DEBUG_INFO, "  Data: "));
    for (UINTN i = 0; i < RxPacket.DataLength; i++) {
      DEBUG ((DEBUG_INFO, "%02X ", RxPacket.Data[i]));
    }
    DEBUG ((DEBUG_INFO, "\n"));
    DEBUG ((DEBUG_INFO, "  Tail: 0x%02X\n", RxPacket.Tail));
    DEBUG ((DEBUG_INFO, "  NextPacketFlag: %d\n", RxPacket.NextPacketFlag));
    DEBUG ((DEBUG_INFO, "  CRC8: 0x%02X\n", RxPacket.Crc8));
    DEBUG ((DEBUG_INFO, "\n"));

    //
    // Verify packet format
    //
    if (RxPacket.Head != 0xAC || RxPacket.Tail != 0xAA) {
      DEBUG ((
        DEBUG_ERROR,
        "%a(): Rx packet format invalid! RxPacket.Head = 0x%x\t RxPacket.Tail = 0x%x!\n",
        __func__,
        RxPacket.Head,
        RxPacket.Tail
        ));
      Status = EFI_DEVICE_ERROR;
      goto Exit;
    }

    //
    // Verify CRC8
    //
    Crc8 = CalculateCrc8 ((UINT8 *)&RxPacket, sizeof (TPCM_SPI_PACKET) - 2);
    if (Crc8 != RxPacket.Crc8) {
      DEBUG ((
        DEBUG_ERROR,
        "%a(): CRC verify failed! RxPacket.Crc8 = 0x%x\t Crc8 = 0x%x!\n",
        __func__,
        RxPacket.Crc8,
        Crc8
        ));
      Status = EFI_DEVICE_ERROR;
      goto Exit;
    }

    //
    // Copy response data
    //
    if (Offset + RxPacket.DataLength > ResponseSize) {
      DEBUG ((
        DEBUG_ERROR,
        "%a(): Response buffer overflow!\n",
        __func__
        ));
      Status = EFI_BUFFER_TOO_SMALL;
      goto Exit;
    }
    CopyMem ((UINT8 *)ResponseData + Offset, RxPacket.Data, RxPacket.DataLength);

    //
    // Check for more packets
    //
    if (RxPacket.NextPacketFlag == 0) {
      break;
    }

    Offset += RxPacket.DataLength;
    PacketIndex++;
  }

  Status = EFI_SUCCESS;

Exit:
  if (TxBuffer) {
    FreePool (TxBuffer);
  }

  if (RxBuffer) {
    FreePool (RxBuffer);
  }

  if (SpiTransfer) {
    FreePool (SpiTransfer);
  }

  return Status;
}

/**
  Initialize the TPCM.

  This function initializes the TPCM by locating the SPI protocol,
  allocating memory for the TPCM device,
  and setting up the SPI slave device for communication with the TPCM.

  @retval EFI_SUCCESS           The TPCM initialization was successful.
  @retval EFI_DEVICE_ERROR      Failed to locate the SPI protocol or set up the SPI slave device.
  @retval EFI_OUT_OF_RESOURCES  Failed to allocate memory for the TPCM device.
  @retval EFI_NOT_FOUND         The SPI protocol is not available.
  @retval EFI_UNSUPPORTED       The SPI controller does not support the requested frequency or mode.

**/
EFI_STATUS
EFIAPI
TpcmInit (
  VOID
  )
{
  EFI_STATUS   Status;

  Status = EFI_SUCCESS;

  //
  // Check if already initialized
  //
  if (mSpiProtocol != NULL || mTpcmDevice != NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): TPCM already initialized!\n",
      __func__
      ));
    return EFI_ALREADY_STARTED;
  }

  //
  // Locate SPI protocol
  //
  Status = gBS->LocateProtocol (
                  &gSophgoSpiProtocolGuid,
                  NULL,
                  (VOID **)&mSpiProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Cannot locate Spi Master protocol (Status = %r)\n",
      __func__,
      Status
      ));
    return Status;
  }

  DEBUG ((DEBUG_INFO, "%a(): SPI protocol located successfully\n", __func__));

  mTpcmDevice = AllocateZeroPool (sizeof (SPI_DEVICE));
  if (mTpcmDevice == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Memory allocation for mTpcmDevice failed\n",
      __func__
      ));
    return EFI_OUT_OF_RESOURCES;
  }

  DEBUG ((DEBUG_INFO, "%a(): TPCM device memory allocated\n", __func__));

  Status = mSpiProtocol->SpiSetupDevice (
                          mSpiProtocol,
                          mTpcmDevice,
                          TPCM_SPI_BUS_NUM,
                          TPCM_SPI_CHIP_SELECT,
                          TPCM_SPI_MODE
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Cannot setup spi slave (Status = %r)\n",
      __func__,
      Status
      ));
    goto Exit;
  }

#if 0
  SPI_MEM_OP  *OpFlash;
  UINT8 *Buffer;
  OpFlash = AllocateZeroPool (sizeof (SPI_MEM_OP));

  DEBUG ((DEBUG_INFO, "%a(): write enable\n", __func__));
  gBS->SetMem (OpFlash, sizeof (SPI_MEM_OP), 0);

  OpFlash->Cmd.NBytes  = 1;
  OpFlash->Cmd.OpCode  = WRITE_ENABLE;

  OpFlash->SpeedHz     = 10 * 1000 * 1000;

  mSpiProtocol->SpiExecMemOp (mSpiProtocol, mTpcmDevice, OpFlash);

  DEBUG ((DEBUG_INFO, "%a(): write data\n", __func__));
  gBS->SetMem (OpFlash, sizeof (SPI_MEM_OP), 0);

  OpFlash->Cmd.NBytes  = 1;
  OpFlash->Cmd.OpCode  = PAGE_PROGRAM;

  OpFlash->Addr.NBytes = 3;
  OpFlash->Addr.Val    = 0x1200;

  OpFlash->Data.Dir    = SPI_MEM_DATA_OUT;
  OpFlash->Data.NBytes = 256;
  UINT8 *OutBuffer     = AllocateZeroPool (OpFlash->Data.NBytes);

  OpFlash->SpeedHz     = 10 * 1000 * 1000;

  for (UINT32 Loop = 0; Loop < OpFlash->Data.NBytes; Loop++)
    OutBuffer[Loop] = Loop & 0xff;
  OpFlash->Data.Buf.Out = OutBuffer;

  mSpiProtocol->SpiExecMemOp (mSpiProtocol, mTpcmDevice, OpFlash);

  DEBUG ((DEBUG_INFO, "%a(): fast read\n", __func__));

  gBS->SetMem (OpFlash, sizeof (SPI_MEM_OP), 0);

  OpFlash->Cmd.NBytes  = 1;
  OpFlash->Cmd.OpCode  = FAST_READ_DATA;

  OpFlash->Addr.NBytes = 3;
  OpFlash->Addr.Val    = 0x1200;

  OpFlash->Dummy.NBytes = 1;

  OpFlash->Data.Dir    = SPI_MEM_DATA_IN;
  OpFlash->Data.NBytes = 256;
  OpFlash->Data.Buf.In = AllocateZeroPool (OpFlash->Data.NBytes);

  OpFlash->SpeedHz     = 10 * 1000 * 1000;

  mSpiProtocol->SpiExecMemOp (mSpiProtocol, mTpcmDevice, OpFlash);

  Buffer = (UINT8 *)(OpFlash->Data.Buf.In);
  for (UINT32 Loop = 0; Loop < OpFlash->Data.NBytes; ++Loop) {
    if ((Loop != 0) && ((Loop % 0x20) == 0))
      DEBUG ((DEBUG_INFO, "\n"));
    DEBUG ((DEBUG_INFO, "%2x ", Buffer[Loop]));
  }
  DEBUG ((DEBUG_INFO, "\n"));
#endif
  return EFI_SUCCESS;

Exit:
  if (mTpcmDevice) {
    FreePool (mTpcmDevice);
  }

  return Status;
}

/**
  TPCM measurement

  @param[in]  Hash      Pointer to the hash data buffer
  @param[in]  DataSize  Size of the hash data

  @retval EFI_SUCCESS           Measurement completed successfully
  @retval EFI_INVALID_PARAMETER Invalid input parameters
  @retval EFI_DEVICE_ERROR     TPCM device returned an error
  @retval EFI_OUT_OF_RESOURCES Failed to allocate memory
**/
EFI_STATUS
EFIAPI
TpcmMeasure (
  IN CONST UINT8  *Hash,
  IN UINTN        DataSize
  )
{
  EFI_STATUS                  Status;
  TPCM_MEASURE_REQ_HEADER     *TpcmReq;
  TPCM_MEASURE_RES            *TpcmRes;

  if (Hash == NULL || DataSize == 0) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Invalid input parameters! Hash = %p, DataSize = %d\n",
      __func__,
      Hash,
      DataSize
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (mSpiProtocol == NULL || mTpcmDevice == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): TPCM not initialized!\n",
      __func__
      ));
    return EFI_NOT_STARTED;
  }

  TpcmReq = AllocateZeroPool (sizeof (TPCM_MEASURE_REQ_HEADER));
  TpcmRes = AllocateZeroPool (sizeof (TPCM_MEASURE_RES));
  if (TpcmReq == NULL || TpcmRes == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Failed to allocate memory for TpcmReq/TpcmRes!\n",
      __func__
      ));

    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  //
  // Initialize request
  //
  ZeroMem (TpcmReq, sizeof (TPCM_MEASURE_REQ_HEADER));
  TpcmReq->Tag         = TPCM_TAG_INPUT;
  TpcmReq->Length      = sizeof (TPCM_MEASURE_REQ_HEADER);
  TpcmReq->Command     = TPCM_CMD_START_MEASURE;
  CopyMem (TpcmReq->Hash, Hash, MIN (DataSize, sizeof (TpcmReq->Hash)));

  //
  // Debug: Print request data
  //
  DEBUG ((DEBUG_INFO, "%a(): Sending measure request:\n", __func__));
  DEBUG ((DEBUG_INFO, "  Tag: 0x%08X\n", TpcmReq->Tag));
  DEBUG ((DEBUG_INFO, "  Length: %d\n", TpcmReq->Length));
  DEBUG ((DEBUG_INFO, "  Command: 0x%08X\n", TpcmReq->Command));
  DEBUG ((DEBUG_INFO, "  Hash: "));
  for (UINTN i = 0; i < MIN (DataSize, sizeof (TpcmReq->Hash)); i++) {
    DEBUG ((DEBUG_INFO, "%02X ", TpcmReq->Hash[i]));
  }
  DEBUG ((DEBUG_INFO, "\n"));

  //
  // Send and receive SPI packet
  //
  Status = TpcmSendReceiveSpiPacket (
             TPCM_CMD_START_MEASURE,
             TpcmReq,
             sizeof (*TpcmReq),
             TpcmRes,
             sizeof (*TpcmRes)
             );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  //
  // Debug: Print response data
  //
  DEBUG ((DEBUG_INFO, "%a(): Received measure response:\n", __func__));
  DEBUG ((DEBUG_INFO, "  Tag: 0x%08X\n", TpcmRes->Tag));
  DEBUG ((DEBUG_INFO, "  Ret: 0x%08X\n", TpcmRes->Ret));

  if (TpcmRes->Tag != TPCM_TAG_OUTPUT) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Invalid response tag: 0x%08x\n",
      __func__,
      TpcmRes->Tag
      ));
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  if (TpcmRes->Ret != 0) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): TPCM command failed: 0x%08x\n",
      __func__,
      TpcmRes->Ret
      ));
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  Status = EFI_SUCCESS;

Exit:
  if (TpcmReq) {
    FreePool (TpcmReq);
  }

  if (TpcmRes) {
    FreePool (TpcmRes);
  }

  return Status;
}

/**
  Retrieve the remaining days of the TPCM license.

  This function sends a request to the TPCM device via SPI protocol to
  query the remaining days of the license.
  It validates the response and returns the result in the ShelfLife parameter.

  @param[in, out] ShelfLife  A pointer to a UINT32 variable that will store the remaining days of the TPCM license.

  @retval EFI_SUCCESS           The remaining license days were successfully retrieved.
  @retval EFI_INVALID_PARAMETER ShelfLife is NULL, or required protocols are not initialized.
  @retval EFI_OUT_OF_RESOURCES  Failed to allocate memory for the request or response.
  @retval EFI_DEVICE_ERROR      The TPCM device returned an invalid response or failed to execute the command.
  @retval EFI_NOT_FOUND         The SPI protocol or device is not available.
  @retval EFI_TIMEOUT           The SPI transfer operation timed out.
  @retval EFI_UNSUPPORTED       The function is not supported in the current environment.

**/
EFI_STATUS
EFIAPI
TpcmGetLicenseRemainingDays (
  IN OUT UINT32  *ShelfLife
  )
{
  EFI_STATUS                  Status;
  TPCM_GET_LICENSE_REQ_HEADER *TpcmReq;
  TPCM_GET_LICENSE_RES        *TpcmRes;

  if (ShelfLife == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Invalid input parameter! ShelfLife = NULL\n",
      __func__
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (mSpiProtocol == NULL || mTpcmDevice == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): TPCM not initialized!\n",
      __func__
      ));
    return EFI_NOT_STARTED;
  }

  TpcmReq = AllocateZeroPool (sizeof (TPCM_GET_LICENSE_REQ_HEADER));
  TpcmRes = AllocateZeroPool (sizeof (TPCM_GET_LICENSE_RES));
  if (TpcmReq == NULL || TpcmRes == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Failed to allocate memory for TpcmReq/TpcmRes!\n",
      __func__
      ));

    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  //
  // Initialize request
  //
  ZeroMem (TpcmReq, sizeof (TPCM_GET_LICENSE_REQ_HEADER));
  TpcmReq->Tag         = TPCM_TAG_INPUT;
  TpcmReq->Length      = sizeof (TPCM_GET_LICENSE_REQ_HEADER);
  TpcmReq->Command     = TPCM_CMD_GET_LICENSE_STS;
  TpcmReq->LicenseType = LICENSE_TYPE;

  //
  // Debug: Print request data
  //
  DEBUG ((DEBUG_INFO, "%a(): Sending license status request:\n", __func__));
  DEBUG ((DEBUG_INFO, "  Tag: 0x%08X\n", TpcmReq->Tag));
  DEBUG ((DEBUG_INFO, "  Length: %d\n", TpcmReq->Length));
  DEBUG ((DEBUG_INFO, "  Command: 0x%08X\n", TpcmReq->Command));
  DEBUG ((DEBUG_INFO, "  LicenseType: 0x%08X\n", TpcmReq->LicenseType));

  //
  // Send and receive SPI packet
  //
  Status = TpcmSendReceiveSpiPacket (
             TPCM_CMD_GET_LICENSE_STS,
             TpcmReq,
             sizeof (*TpcmReq),
             TpcmRes,
             sizeof (*TpcmRes)
             );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  //
  // Debug: Print response data
  //
  DEBUG ((DEBUG_INFO, "%a(): Received license status response:\n", __func__));
  DEBUG ((DEBUG_INFO, "  Tag: 0x%08X\n", TpcmRes->Tag));
  DEBUG ((DEBUG_INFO, "  Ret: 0x%08X\n", TpcmRes->Ret));
  DEBUG ((DEBUG_INFO, "  RemainDays: %d\n", TpcmRes->RemainDays));

  //
  // Validate response
  //
  if (TpcmRes->Tag != TPCM_TAG_OUTPUT || TpcmRes->Ret != 0) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Invalid response! Tag = 0x%08X, Ret = 0x%08X\n",
      __func__,
      TpcmRes->Tag,
      TpcmRes->Ret
      ));

    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  *ShelfLife = TpcmRes->RemainDays;

  DEBUG ((DEBUG_INFO, "%a(): Remaining days: %u\n", __func__, *ShelfLife));

  Status = EFI_SUCCESS;

Exit:
  if (TpcmReq) {
    FreePool (TpcmReq);
  }

  if (TpcmRes) {
    FreePool (TpcmRes);
  }

  return Status;
}

STATIC
BOOLEAN
IsLeapYear (
  IN UINT16 Year
  )
{
  return (Year % 4 == 0 && Year % 100 != 0) || (Year % 400 == 0);
}

STATIC
UINT8
GetDaysInMonth (
  IN UINT8  Month,
  IN UINT16 Year
  )
{
  if (Month < 1 || Month > 12) {
    return 0;
  }

  switch (Month) {
  case 1: case 3: case 5: case 7: case 8: case 10: case 12:
    return 31;
  case 4: case 6: case 9: case 11:
    return 30;
  case 2:
    return IsLeapYear (Year) ? 29 : 28;
  default:
    return 0;
  }
}

STATIC
UINT64
ConvertEfiTimeToSeconds (
  IN EFI_TIME *Time
  )
{
  if (Time == NULL || Time->Year < 1970 || Time->Month < 1 || Time->Month > 12
		  || Time->Day < 1 || Time->Day > 31) {
    return 0;
  }

  UINT64 TotalSeconds = 0;

  for (UINT16 Year = 1970; Year < Time->Year; Year++) {
    TotalSeconds += 365 * 24 * 3600;
    if (IsLeapYear (Year)) {
      TotalSeconds += 24 * 3600;
    }
  }

  for (UINT8 Month = 1; Month < Time->Month; Month++) {
    TotalSeconds += GetDaysInMonth (Month, Time->Year) * 24 * 3600;
  }

  TotalSeconds += (Time->Day - 1) * 24 * 3600;

  TotalSeconds += Time->Hour * 3600;
  TotalSeconds += Time->Minute * 60;
  TotalSeconds += Time->Second;

  return TotalSeconds;
}

/**
  Get the status of the TPCM.

  This function retrieves the current status of the TPCM by sending a request
  via SPI protocol and validating the response.
  It also logs the time of the request for auditing purposes.

  @param[in, out] TpcmInfo  A pointer to the TPCM_STATUS structure that will
                            be populated with the current status of the TPCM.

  @retval EFI_SUCCESS           The TPCM status was successfully retrieved.
  @retval EFI_INVALID_PARAMETER TpcmInfo is NULL, or required protocols are not initialized.
  @retval EFI_OUT_OF_RESOURCES  Failed to allocate memory for the request or response.
  @retval EFI_DEVICE_ERROR      The TPCM device returned an invalid response or failed to execute the command.
  @retval EFI_NOT_FOUND         The SPI protocol or device is not available.
  @retval EFI_TIMEOUT           The SPI transfer operation timed out.
  @retval EFI_UNSUPPORTED       The function is not supported in the current environment.

**/
EFI_STATUS
EFIAPI
TpcmGetStatus (
  IN OUT TPCM_STATUS *TpcmInfo
  )
{
  EFI_STATUS                    Status;
  TPCM_GET_STS_REQ_HEADER       *TpcmReq;
  TPCM_GET_STS_RES              *TpcmRes;
  EFI_TIME                      Time;

  if (TpcmInfo == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Invalid input parameter! TpcmInfo = NULL\n",
      __func__
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (mSpiProtocol == NULL || mTpcmDevice == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): TPCM not initialized!\n",
      __func__
      ));
    return EFI_NOT_STARTED;
  }

  //
  // Get current time
  //
  Status = gRT->GetTime (&Time, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Get Time failed(Status=%r)!\n",
      __func__,
      Status
      ));
    return Status;
  }

  TpcmReq = AllocateZeroPool (sizeof (TPCM_GET_STS_REQ_HEADER));
  TpcmRes = AllocateZeroPool (sizeof (TPCM_GET_STS_RES));
  if (TpcmReq == NULL || TpcmRes == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Failed to allocate memory for TpcmReq/TpcmRes!\n",
      __func__
      ));

    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  //
  // Initialize TPCM request
  //
  ZeroMem (TpcmReq, sizeof (TPCM_GET_STS_REQ_HEADER));
  TpcmReq->Tag         = TPCM_TAG_INPUT;
  TpcmReq->Length      = sizeof (TPCM_GET_STS_REQ_HEADER);
  TpcmReq->Command     = TPCM_CMD_GET_STS;
  TpcmReq->ReportTime  = ConvertEfiTimeToSeconds (&Time);

  //
  // Debug: Print request data
  //
  DEBUG ((DEBUG_INFO, "%a(): Sending status request:\n", __func__));
  DEBUG ((DEBUG_INFO, "  Tag: 0x%08X\n", TpcmReq->Tag));
  DEBUG ((DEBUG_INFO, "  Length: %d\n", TpcmReq->Length));
  DEBUG ((DEBUG_INFO, "  Command: 0x%08X\n", TpcmReq->Command));
  DEBUG ((DEBUG_INFO, "  ReportTime: %llu\n", TpcmReq->ReportTime));

  //
  // Send and receive SPI packet
  //
  Status = TpcmSendReceiveSpiPacket (
             TPCM_CMD_GET_STS,
             TpcmReq,
             sizeof (*TpcmReq),
             TpcmRes,
             sizeof (*TpcmRes)
             );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  //
  // Debug: Print response data
  //
  DEBUG ((DEBUG_INFO, "%a(): Received status response:\n", __func__));
  DEBUG ((DEBUG_INFO, "  Tag: 0x%08X\n", TpcmRes->Tag));
  DEBUG ((DEBUG_INFO, "  Ret: 0x%08X\n", TpcmRes->Ret));

  //
  // Check received message
  //
  if (TpcmRes->Tag != TPCM_TAG_OUTPUT) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Invalid response tag: 0x%08x\n",
      __func__,
      TpcmRes->Tag
      ));

    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  if (TpcmRes->Ret != 0) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): TPCM command failed: 0x%08x\n",
      __func__,
      TpcmRes->Ret
      ));

    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  ZeroMem (TpcmInfo, sizeof (TPCM_STATUS));
  CopyMem (TpcmInfo, &TpcmRes->TpcmInfo, sizeof (TPCM_STATUS));

  //
  // Debug: Print TPCM info
  //
  DEBUG ((DEBUG_INFO, "  TPCM Info:\n"));
  DEBUG ((DEBUG_INFO, "    TpcmId: "));
  for (UINTN i = 0; i < sizeof (TpcmInfo->TpcmId); i++) {
    DEBUG ((DEBUG_INFO, "%02X ", TpcmInfo->TpcmId[i]));
  }
  DEBUG ((DEBUG_INFO, "\n"));
  DEBUG ((DEBUG_INFO, "    Version: %d\n", TpcmInfo->Version));
  DEBUG ((DEBUG_INFO, "    Status: 0x%08X\n", TpcmInfo->Status));

  Status = EFI_SUCCESS;

Exit:
  if (TpcmReq) {
    FreePool (TpcmReq);
  }

  if (TpcmRes) {
    FreePool (TpcmRes);
  }

  return Status;
}

/**
  Get TPCM Control status.

  @param[in]      TpcmInfo    Pointer to the TPCM status information.
  @param[in out]  IsEnabled   Pointer to a boolean value to store the control
                              status.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_INVALID_PARAMETER One or more parameters are invalid.
  @retval EFI_DEVICE_ERROR      The SPI transfer or TPCM command failed.
**/
EFI_STATUS
EFIAPI
TpcmGetControlStatus (
  IN     TPCM_STATUS *TpcmInfo,
  IN OUT BOOLEAN     *IsEnabled
  )
{
  EFI_STATUS                    Status;
  TPCM_GET_CTRL_STS_REQ_HEADER  *TpcmReq;
  TPCM_GET_CTRL_STS_RES         *TpcmRes;

  if (TpcmInfo == NULL || IsEnabled == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Invalid input parameters! TpcmInfo = %p, IsEnabled = %p\n",
      __func__,
      TpcmInfo,
      IsEnabled
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (TpcmInfo->TpcmId == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Invalid TPCM ID!\n",
      __func__
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (mSpiProtocol == NULL || mTpcmDevice == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): TPCM not initialized!\n",
      __func__
      ));
    return EFI_NOT_STARTED;
  }

  TpcmReq = AllocateZeroPool (sizeof (TPCM_GET_CTRL_STS_REQ_HEADER));
  TpcmRes = AllocateZeroPool (sizeof (TPCM_GET_CTRL_STS_RES));
  if (TpcmReq == NULL || TpcmRes == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Failed to allocate memory for TpcmReq/TpcmRes!\n",
      __func__
      ));

    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  //
  // Initialize TPCM request
  //
  ZeroMem (TpcmReq, sizeof (TPCM_GET_CTRL_STS_REQ_HEADER));
  TpcmReq->Tag         = TPCM_TAG_INPUT;
  TpcmReq->Length      = sizeof (TPCM_GET_CTRL_STS_REQ_HEADER);
  TpcmReq->Command     = TPCM_CMD_GET_CTRL_STS;
  CopyMem (TpcmReq->TpcmId, TpcmInfo->TpcmId, sizeof (TpcmInfo->TpcmId));

  //
  // Debug: Print request data
  //
  DEBUG ((DEBUG_INFO, "%a(): Sending control status request:\n", __func__));
  DEBUG ((DEBUG_INFO, "  Tag: 0x%08X\n", TpcmReq->Tag));
  DEBUG ((DEBUG_INFO, "  Length: %d\n", TpcmReq->Length));
  DEBUG ((DEBUG_INFO, "  Command: 0x%08X\n", TpcmReq->Command));
  DEBUG ((DEBUG_INFO, "  TpcmId: "));
  for (UINTN i = 0; i < sizeof (TpcmReq->TpcmId); i++) {
    DEBUG ((DEBUG_INFO, "%02X ", TpcmReq->TpcmId[i]));
  }
  DEBUG ((DEBUG_INFO, "\n"));

  //
  // Send and receive SPI packet
  //
  Status = TpcmSendReceiveSpiPacket (
             TPCM_CMD_GET_CTRL_STS,
             TpcmReq,
             sizeof (*TpcmReq),
             TpcmRes,
             sizeof (*TpcmRes)
             );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  //
  // Debug: Print response data
  //
  DEBUG ((DEBUG_INFO, "%a(): Received control status response:\n", __func__));
  DEBUG ((DEBUG_INFO, "  Tag: 0x%08X\n", TpcmRes->Tag));
  DEBUG ((DEBUG_INFO, "  Ret: 0x%08X\n", TpcmRes->Ret));
  DEBUG ((DEBUG_INFO, "  Enabled: %d\n", TpcmRes->Enabled));

  //
  // Check received message
  //
  if (TpcmRes->Tag != TPCM_TAG_OUTPUT) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Invalid response tag: 0x%08x\n",
      __func__,
      TpcmRes->Tag
      ));

    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  if (TpcmRes->Ret != 0) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): TPCM command failed: 0x%08x\n",
      __func__,
      TpcmRes->Ret
      ));

    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  *IsEnabled = TpcmRes->Enabled ? TRUE : FALSE;

  DEBUG ((DEBUG_INFO, "TPCM is %s!\n", *IsEnabled ? L"enabled" : L"disabled"));

  Status = EFI_SUCCESS;

Exit:
  if (TpcmReq) {
    FreePool (TpcmReq);
  }

  if (TpcmRes) {
    FreePool (TpcmRes);
  }

  return Status;
}

/**
  Sets the control status of the TPCM (Trusted Platform Control Module).

  This function initializes a TPCM control status request, sends it via
  SPI protocol to the TPCM device, and validates the response to ensure
  the operation was successful.

  @param[in] TpcmInfo    Pointer to the TPCM status information, including
                         the TPCM ID.
  @param[in] IsEnabled   Boolean value indicating whether to enable or disable
                         the TPCM control status.

  @retval EFI_SUCCESS            The TPCM control status was successfully set.
  @retval EFI_INVALID_PARAMETER  One or more input parameters are invalid.
  @retval EFI_DEVICE_ERROR       The TPCM device returned an invalid response
                                 or failed to execute the command.

**/
EFI_STATUS
EFIAPI
TpcmSetControlStatus (
  IN TPCM_STATUS *TpcmInfo,
  IN BOOLEAN     IsEnabled
  )
{
  EFI_STATUS                    Status;
  TPCM_SET_CTRL_STS_REQ_HEADER  *TpcmReq;
  TPCM_SET_CTRL_STS_RES         *TpcmRes;

  if (TpcmInfo == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Invalid input parameter! TpcmInfo = NULL\n",
      __func__
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (TpcmInfo->TpcmId == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Invalid TPCM ID!\n",
      __func__
      ));
    return EFI_INVALID_PARAMETER;
  }

  if (mSpiProtocol == NULL || mTpcmDevice == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): TPCM not initialized!\n",
      __func__
      ));
    return EFI_NOT_STARTED;
  }

  TpcmReq = AllocateZeroPool (sizeof (TPCM_SET_CTRL_STS_REQ_HEADER));
  TpcmRes = AllocateZeroPool (sizeof (TPCM_SET_CTRL_STS_RES));
  if (TpcmReq == NULL || TpcmRes == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Failed to allocate memory for TpcmReq/TpcmRes!\n",
      __func__
      ));
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialize TPCM request
  //
  ZeroMem (TpcmReq, sizeof (TPCM_SET_CTRL_STS_REQ_HEADER));
  TpcmReq->Tag         = TPCM_TAG_INPUT;
  TpcmReq->Length      = sizeof (TPCM_SET_CTRL_STS_REQ_HEADER);
  TpcmReq->Command     = TPCM_CMD_SET_CTRL_STS;
  CopyMem (TpcmReq->TpcmId, TpcmInfo->TpcmId, sizeof (TpcmInfo->TpcmId));
  TpcmReq->Enabled     = IsEnabled;

  //
  // Debug: Print request data
  //
  DEBUG ((DEBUG_INFO, "%a(): Sending set control status request:\n", __func__));
  DEBUG ((DEBUG_INFO, "  Tag: 0x%08X\n", TpcmReq->Tag));
  DEBUG ((DEBUG_INFO, "  Length: %d\n", TpcmReq->Length));
  DEBUG ((DEBUG_INFO, "  Command: 0x%08X\n", TpcmReq->Command));
  DEBUG ((DEBUG_INFO, "  TpcmId: "));
  for (UINTN i = 0; i < sizeof (TpcmReq->TpcmId); i++) {
    DEBUG ((DEBUG_INFO, "%02X ", TpcmReq->TpcmId[i]));
  }
  DEBUG ((DEBUG_INFO, "\n"));
  DEBUG ((DEBUG_INFO, "  Enabled: %d\n", TpcmReq->Enabled));

  //
  // Send and receive SPI packet
  //
  Status = TpcmSendReceiveSpiPacket (
             TPCM_CMD_SET_CTRL_STS,
             TpcmReq,
             sizeof (*TpcmReq),
             TpcmRes,
             sizeof (*TpcmRes)
             );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  //
  // Debug: Print response data
  //
  DEBUG ((DEBUG_INFO, "%a(): Received set control status response:\n", __func__));
  DEBUG ((DEBUG_INFO, "  Tag: 0x%08X\n", TpcmRes->Tag));
  DEBUG ((DEBUG_INFO, "  Ret: 0x%08X\n", TpcmRes->Ret));

  //
  // Check received message
  //
  if (TpcmRes->Tag != TPCM_TAG_OUTPUT) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Invalid response tag: 0x%08x\n",
      __func__,
      TpcmRes->Tag
      ));
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  if (TpcmRes->Ret != 0) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): TPCM command failed: 0x%08x\n",
      __func__,
      TpcmRes->Ret
      ));
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  Status = EFI_SUCCESS;

Exit:
  if (TpcmReq) {
    FreePool (TpcmReq);
  }

  if (TpcmRes) {
    FreePool (TpcmRes);
  }

  return Status;
}

EFI_STATUS
EFIAPI
InitializeTpcmDxe (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS           Status;
  SOPHGO_TPCM_PROTOCOL *mTpcmProtocol;

  mTpcmProtocol = AllocateZeroPool (sizeof (SOPHGO_TPCM_PROTOCOL));
  if (mTpcmProtocol == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Failed to allocate memory for mTpcmProtocol!\n",
      __func__
      ));
    return EFI_OUT_OF_RESOURCES;
  }

  mTpcmProtocol->InitTpcm           = TpcmInit;
  mTpcmProtocol->Measure            = TpcmMeasure;
  mTpcmProtocol->GetTpcmLicense     = TpcmGetLicenseRemainingDays;
  mTpcmProtocol->GetTpcmStatus      = TpcmGetStatus;
  mTpcmProtocol->GetTpcmCtrlStatus  = TpcmGetControlStatus;
  mTpcmProtocol->SetTpcmCtrlStatus  = TpcmSetControlStatus;

  //
  // Install TPCM protocol
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gSophgoTpcmProtocolGuid,
                  mTpcmProtocol,
                  NULL
		  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Failed to Install mTpcmProtocol: %r\n",
      __func__,
      Status
      ));
    goto ErrorInstallProtocol;
  }

  return EFI_SUCCESS;

ErrorInstallProtocol:
  FreePool (mTpcmProtocol);

  return Status;
}
