/** @file

  IPMI Manageability Protocol common file.

  Copyright (C) 2023 Advanced Micro Devices, Inc. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ManageabilityTransportLib.h>
#include <Library/ManageabilityTransportHelperLib.h>
#include <Library/ManageabilityTransportMctpLib.h>
#include <IndustryStandard/Mctp.h>
#include <IndustryStandard/Pldm.h>
#include <IndustryStandard/PldmSmbiosTransfer.h>
#include "PldmProtocolCommon.h"

extern CHAR16  *mTransportName;
extern UINT8   mPldmRequestInstanceId;

PLDM_MESSAGE_PACKET_MAPPING  PldmMessagePacketMappingTable[] = {
  { PLDM_TYPE_SMBIOS, PLDM_GET_SMBIOS_STRUCTURE_TABLE_METADATA_COMMAND_CODE, sizeof (PLDM_GET_SMBIOS_STRUCTURE_TABLE_METADATA_RESPONSE_FORMAT) },
  { PLDM_TYPE_SMBIOS, PLDM_SET_SMBIOS_STRUCTURE_TABLE_METADATA_COMMAND_CODE, sizeof (PLDM_SET_SMBIOS_STRUCTURE_TABLE_METADATA_RESPONSE_FORMAT) },
  { PLDM_TYPE_SMBIOS, PLDM_SET_SMBIOS_STRUCTURE_TABLE_COMMAND_CODE,          sizeof (PLDM_SET_SMBIOS_STRUCTURE_TABLE_REQUEST_FORMAT)           }
};

/**
  This function returns the expected full size of PLDM response message.

  @param[in]         PldmType        PLDM message type.
  @param[in]         PldmCommand     PLDM command of this PLDM type.

  @retval  Zero       No matched entry for this PldmType/PldmCommand.
  @retval  None-zero  Size of full packet is returned.
**/
UINT32
GetFullPacketResponseSize (
  IN UINT8  PldmType,
  IN UINT8  PldmCommand
  )
{
  INT16                        Index;
  PLDM_MESSAGE_PACKET_MAPPING  *ThisEntry;

  ThisEntry = PldmMessagePacketMappingTable;
  for (Index = 0; Index < (sizeof (PldmMessagePacketMappingTable)/ sizeof (PLDM_MESSAGE_PACKET_MAPPING)); Index++) {
    if ((PldmType == ThisEntry->PldmType) && (PldmCommand == ThisEntry->PldmCommand)) {
      return ThisEntry->ResponseSize;
    }

    ThisEntry++;
  }

  return 0;
}

/**
  This functions setup the final header/body/trailer packets for
  the acquired transport interface.

  @param[in]         TransportToken     The transport interface.
  @param[in]         PldmType           PLDM message type.
  @param[in]         PldmCommand        PLDM command of this PLDM type.
  @param[out]        PacketHeader       The pointer to receive header of request.
  @param[out]        PacketHeaderSize   Packet header size in bytes.
  @param[in, out]    PacketBody         The request body.
                                        When IN, it is the caller's request body.
                                        When OUT and NULL, the request body is not
                                        changed.
                                        Whee out and non-NULL, the request body is
                                        changed to comfort the transport interface.
  @param[in, out]    PacketBodySize     The request body size.
                                        When IN and non-zero, it is the new data
                                        length of request body.
                                        When IN and zero, the request body is unchanged.
  @param[out]        PacketTrailer      The pointer to receive trailer of request.
  @param[out]        PacketTrailerSize  Packet trailer size in bytes.

  @retval EFI_SUCCESS            Request packet is returned.
  @retval EFI_UNSUPPORTED        Request packet is not returned because
                                 the unsupported transport interface.
**/
EFI_STATUS
SetupPldmRequestTransportPacket (
  IN   MANAGEABILITY_TRANSPORT_TOKEN    *TransportToken,
  IN   UINT8                            PldmType,
  IN   UINT8                            PldmCommand,
  OUT  MANAGEABILITY_TRANSPORT_HEADER   *PacketHeader,
  OUT  UINT16                           *PacketHeaderSize,
  IN OUT UINT8                          **PacketBody,
  IN OUT UINT32                         *PacketBodySize,
  OUT  MANAGEABILITY_TRANSPORT_TRAILER  *PacketTrailer,
  OUT  UINT16                           *PacketTrailerSize
  )
{
  MANAGEABILITY_MCTP_TRANSPORT_HEADER  *MctpHeader;
  PLDM_REQUEST_HEADER                  *PldmRequestHeader;

  if ((PacketHeader == NULL) || (PacketHeaderSize == NULL) ||
      (PacketBody   == NULL) || (PacketBodySize == NULL) ||
      (PacketTrailer == NULL) || (PacketTrailerSize == NULL)
      )
  {
    DEBUG ((DEBUG_ERROR, "%a: One or more than one of the required parameters is NULL.\n", __func__));
    return EFI_INVALID_PARAMETER;
  }

  if (CompareGuid (&gManageabilityTransportMctpGuid, TransportToken->Transport->ManageabilityTransportSpecification)) {
    DEBUG ((DEBUG_MANAGEABILITY_INFO, "%a: Setup transport header for PLDM over MCTP.\n", __func__));

    // This is MCTP transport interface.
    MctpHeader = AllocateZeroPool (sizeof (MANAGEABILITY_MCTP_TRANSPORT_HEADER));
    if (MctpHeader == NULL) {
      DEBUG ((DEBUG_ERROR, "%a: Not enough memory for MANAGEABILITY_MCTP_TRANSPORT_HEADER.\n", __func__));
      return EFI_OUT_OF_RESOURCES;
    }

    MctpHeader->SourceEndpointId             = PcdGet8 (PcdMctpSourceEndpointId);
    MctpHeader->SourceEndpointId             = PcdGet8 (PcdMctpDestinationEndpointId);
    MctpHeader->MessageHeader.IntegrityCheck = FALSE;
    MctpHeader->MessageHeader.MessageType    = MCTP_MESSAGE_TYPE_PLDM;
    *PacketHeader                            = (MANAGEABILITY_TRANSPORT_HEADER *)MctpHeader;
    *PacketHeaderSize                        = sizeof (MANAGEABILITY_TRANSPORT_HEADER);
    *PacketTrailer                           = NULL;
    *PacketTrailerSize                       = 0;
  } else {
    DEBUG ((DEBUG_ERROR, "%a: No implementation of building up packet.\n", __func__));
    ASSERT (FALSE);
  }

  //
  // Create header for the final request message.
  //
  PldmRequestHeader = (PLDM_REQUEST_HEADER *)AllocateZeroPool (sizeof (PLDM_REQUEST_HEADER) + *PacketBodySize);
  if (PldmRequestHeader == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Not enough memory for final PLDM request message.\n", __func__));
    return EFI_OUT_OF_RESOURCES;
  }

  PldmRequestHeader->RequestBit          = PLDM_MESSAGE_HEADER_IS_REQUEST;
  PldmRequestHeader->HeaderVersion       = PLDM_MESSAGE_HEADER_VERSION;
  PldmRequestHeader->PldmType            = PldmType;
  PldmRequestHeader->PldmTypeCommandCode = PldmCommand;
  PldmRequestHeader->InstanceId          = mPldmRequestInstanceId;
  if ((*PacketBody != NULL) && (*PacketBodySize != 0)) {
    CopyMem (
      (VOID *)((UINT8 *)PldmRequestHeader + sizeof (PLDM_REQUEST_HEADER)),
      (VOID *)*PacketBody,
      *PacketBodySize
      );
  }

  *PacketBody     = (UINT8 *)PldmRequestHeader;
  *PacketBodySize = sizeof (PLDM_REQUEST_HEADER) + *PacketBodySize;
  return EFI_SUCCESS;
}

/**
  Common code to submit PLDM commands

  @param[in]         TransportToken    Transport token.
  @param[in]         PldmType          PLDM message type.
  @param[in]         PldmCommand       PLDM command of this PLDM type.
  @param[in]         RequestData       Command Request Data.
  @param[in]         RequestDataSize   Size of Command Request Data.
  @param[out]        ResponseData      Command Response Data. The completion code is the first byte of response data.
  @param[in, out]    ResponseDataSize  Size of Command Response Data.

  @retval EFI_SUCCESS            The command byte stream was successfully submit to the device and a response was successfully received.
  @retval EFI_NOT_FOUND          The command was not successfully sent to the device or a response was not successfully received from the device.
  @retval EFI_NOT_READY          Ipmi Device is not ready for Ipmi command access.
  @retval EFI_DEVICE_ERROR       Ipmi Device hardware error.
  @retval EFI_TIMEOUT            The command time out.
  @retval EFI_UNSUPPORTED        The command was not successfully sent to the device.
  @retval EFI_OUT_OF_RESOURCES   The resource allocation is out of resource or data size error.
**/
EFI_STATUS
CommonPldmSubmitCommand (
  IN     MANAGEABILITY_TRANSPORT_TOKEN  *TransportToken,
  IN     UINT8                          PldmType,
  IN     UINT8                          PldmCommand,
  IN     UINT8                          *RequestData OPTIONAL,
  IN     UINT32                         RequestDataSize,
  OUT    UINT8                          *ResponseData OPTIONAL,
  IN OUT UINT32                         *ResponseDataSize
  )
{
  EFI_STATUS                                 Status;
  UINT8                                      *ThisRequestData;
  UINT32                                     ThisRequestDataSize;
  MANAGEABILITY_TRANSFER_TOKEN               TransferToken;
  MANAGEABILITY_TRANSPORT_HEADER             PldmTransportHeader;
  MANAGEABILITY_TRANSPORT_TRAILER            PldmTransportTrailer;
  MANAGEABILITY_TRANSPORT_ADDITIONAL_STATUS  TransportAdditionalStatus;
  UINT8                                      *FullPacketResponseData;
  UINT32                                     FullPacketResponseDataSize;
  PLDM_RESPONSE_HEADER                       *ResponseHeader;
  UINT16                                     HeaderSize;
  UINT16                                     TrailerSize;

  if (TransportToken == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: No transport token for PLDM\n", __func__));
    return EFI_UNSUPPORTED;
  }

  Status = TransportToken->Transport->Function.Version1_0->TransportStatus (
                                                             TransportToken,
                                                             &TransportAdditionalStatus
                                                             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Transport %s for PLDM has problem - (%r)\n", __func__, mTransportName, Status));
    return Status;
  }

  ThisRequestData = RequestData;            // Save the original request data because the request data maybe modified
                                            // in SetupIpmiRequestTransportPacket() according to transport interface.
  ThisRequestDataSize = RequestDataSize;    // Save the original request data size because the request data size maybe modified
                                            //  in SetupIpmiRequestTransportPacket() according to transport interface.
  PldmTransportHeader  = NULL;
  PldmTransportTrailer = NULL;
  Status               = SetupPldmRequestTransportPacket (
                           TransportToken,
                           PldmType,
                           PldmCommand,
                           &PldmTransportHeader,
                           &HeaderSize,
                           &ThisRequestData,
                           &ThisRequestDataSize,
                           &PldmTransportTrailer,
                           &TrailerSize
                           );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Fail to build packets - (%r)\n", __func__, Status));
    return Status;
  }

  ZeroMem (&TransferToken, sizeof (MANAGEABILITY_TRANSFER_TOKEN));
  TransferToken.TransmitHeader      = PldmTransportHeader;
  TransferToken.TransmitHeaderSize  = HeaderSize;
  TransferToken.TransmitTrailer     = PldmTransportTrailer;
  TransferToken.TransmitTrailerSize = TrailerSize;

  // Transmit packet.
  if ((ThisRequestData == NULL) || (ThisRequestDataSize == 0)) {
    // Transmit parameter were not changed by SetupIpmiRequestTransportPacket().
    TransferToken.TransmitPackage.TransmitPayload    = RequestData;
    TransferToken.TransmitPackage.TransmitSizeInByte = ThisRequestDataSize;
  } else {
    // Transmit parameter were changed by SetupIpmiRequestTransportPacket().
    TransferToken.TransmitPackage.TransmitPayload    = ThisRequestData;
    TransferToken.TransmitPackage.TransmitSizeInByte = ThisRequestDataSize;
  }

  TransferToken.TransmitPackage.TransmitTimeoutInMillisecond = MANAGEABILITY_TRANSPORT_NO_TIMEOUT;

  // Set receive packet.
  FullPacketResponseDataSize = GetFullPacketResponseSize (PldmType, PldmCommand);
  if (FullPacketResponseDataSize == 0) {
    DEBUG ((DEBUG_ERROR, "  No mapping entry in PldmMessagePacketMappingTable for PLDM Type:%d Command %d\n", PldmType, PldmCommand));
    ASSERT (FALSE);
  }

  FullPacketResponseData = (UINT8 *)AllocateZeroPool (FullPacketResponseDataSize);
  if (FullPacketResponseData == NULL) {
    DEBUG ((DEBUG_ERROR, "  Not enough memory for FullPacketResponseDataSize.\n"));
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit2;
  }

  // Print out PLDM packet.
  DEBUG ((
    DEBUG_MANAGEABILITY_INFO,
    "%a: Send PLDM type: 0x%x, Command: 0x%x: Request size: 0x%x, Response size: 0x%x\n",
    __func__,
    PldmType,
    PldmCommand,
    TransferToken.TransmitPackage.TransmitSizeInByte,
    FullPacketResponseDataSize
    ));

  HelperManageabilityDebugPrint (
    (VOID *)TransferToken.TransmitPackage.TransmitPayload,
    TransferToken.TransmitPackage.TransmitSizeInByte,
    "PLDM full request payload.\n"
    );

  TransferToken.ReceivePackage.ReceiveBuffer                = FullPacketResponseData;
  TransferToken.ReceivePackage.ReceiveSizeInByte            = FullPacketResponseDataSize;
  TransferToken.ReceivePackage.TransmitTimeoutInMillisecond = MANAGEABILITY_TRANSPORT_NO_TIMEOUT;
  TransportToken->Transport->Function.Version1_0->TransportTransmitReceive (
                                                    TransportToken,
                                                    &TransferToken
                                                    );
  //
  // Check the response size.
  if (TransferToken.ReceivePackage.ReceiveSizeInByte < sizeof (PLDM_RESPONSE_HEADER)) {
    DEBUG ((
      DEBUG_MANAGEABILITY_INFO,
      "Invalid response header size of PLDM Type %d Command %d, Returned size: %d Expected size: %d\n",
      PldmType,
      PldmCommand,
      TransferToken.ReceivePackage.ReceiveSizeInByte,
      FullPacketResponseDataSize
      ));
    if (ResponseDataSize != NULL) {
      if (*ResponseDataSize > TransferToken.ReceivePackage.ReceiveSizeInByte) {
        *ResponseDataSize = TransferToken.ReceivePackage.ReceiveSizeInByte;
      }
    }

    if (ResponseData != NULL) {
      CopyMem ((VOID *)ResponseData, (VOID *)FullPacketResponseData, *ResponseDataSize);
    }

    goto ErrorExit;
  }

  //
  // Check the integrity of response. data.
  ResponseHeader = (PLDM_RESPONSE_HEADER *)FullPacketResponseData;
  if ((ResponseHeader->PldmHeader.DatagramBit != 0) ||
      (ResponseHeader->PldmHeader.RequestBit != 0) ||
      (ResponseHeader->PldmHeader.InstanceId != mPldmRequestInstanceId) ||
      (ResponseHeader->PldmHeader.PldmType != PldmType) ||
      (ResponseHeader->PldmHeader.PldmTypeCommandCode != PldmCommand))
  {
    DEBUG ((DEBUG_ERROR, "PLDM integrity check of response data is failed.\n"));
    DEBUG ((DEBUG_ERROR, "    Request bit  = %d (Expected value: 0)\n"));
    DEBUG ((DEBUG_ERROR, "    Datagram     = %d (Expected value: 0)\n"));
    DEBUG ((DEBUG_ERROR, "    Instance ID  = %d (Expected value: %d)\n", ResponseHeader->PldmHeader.InstanceId, mPldmRequestInstanceId));
    DEBUG ((DEBUG_ERROR, "    Pldm Type    = %d (Expected value: %d)\n", ResponseHeader->PldmHeader.PldmType, PldmType));
    DEBUG ((DEBUG_ERROR, "    Pldm Command = %d (Expected value: %d)\n", ResponseHeader->PldmHeader.PldmTypeCommandCode, PldmCommand));
    if (ResponseDataSize != NULL) {
      if (*ResponseDataSize > TransferToken.ReceivePackage.ReceiveSizeInByte) {
        *ResponseDataSize = TransferToken.ReceivePackage.ReceiveSizeInByte;
      }
    }

    if (ResponseData != NULL) {
      CopyMem ((VOID *)ResponseData, (VOID *)FullPacketResponseData, *ResponseDataSize);
    }

    goto ErrorExit;
  }

  //
  // Check the response size
  if (TransferToken.ReceivePackage.ReceiveSizeInByte != FullPacketResponseDataSize) {
    DEBUG ((
      DEBUG_ERROR,
      "The response size is incorrect: Response size %d (Expected %d), Completion code %d.\n",
      TransferToken.ReceivePackage.ReceiveSizeInByte,
      FullPacketResponseDataSize,
      ResponseHeader->PldmCompletionCode
      ));
    if (ResponseDataSize != NULL) {
      if (*ResponseDataSize > TransferToken.ReceivePackage.ReceiveSizeInByte) {
        *ResponseDataSize = TransferToken.ReceivePackage.ReceiveSizeInByte;
      }
    }

    if (ResponseData != NULL) {
      CopyMem ((VOID *)ResponseData, (VOID *)FullPacketResponseData, *ResponseDataSize);
    }

    goto ErrorExit;
  }

  if (*ResponseDataSize != (TransferToken.ReceivePackage.ReceiveSizeInByte - sizeof (PLDM_RESPONSE_HEADER))) {
    DEBUG ((DEBUG_ERROR, "  The size of response is not matched to RequestDataSize assigned by caller.\n"));
    DEBUG ((
      DEBUG_ERROR,
      "Caller expects %d, the response size minus PLDM_RESPONSE_HEADER size is %d, Completion Code %d.\n",
      *ResponseDataSize,
      TransferToken.ReceivePackage.ReceiveSizeInByte - sizeof (PLDM_RESPONSE_HEADER),
      ResponseHeader->PldmCompletionCode
      ));
    if (ResponseDataSize != NULL) {
      if (*ResponseDataSize > TransferToken.ReceivePackage.ReceiveSizeInByte) {
        *ResponseDataSize = TransferToken.ReceivePackage.ReceiveSizeInByte;
      }
    }

    if (ResponseData != NULL) {
      CopyMem ((VOID *)ResponseData, (VOID *)FullPacketResponseData, *ResponseDataSize);
    }

    goto ErrorExit;
  }

  // Print out PLDM full responses payload.
  HelperManageabilityDebugPrint ((VOID *)FullPacketResponseData, FullPacketResponseDataSize, "PLDM full response payload\n");

  // Copy response data (without header) to caller's buffer.
  if ((ResponseData != NULL) && (*ResponseDataSize != 0)) {
    *ResponseDataSize = FullPacketResponseDataSize - sizeof (PLDM_RESPONSE_HEADER);
    CopyMem (
      (VOID *)ResponseData,
      (VOID *)(FullPacketResponseData + sizeof (PLDM_RESPONSE_HEADER)),
      *ResponseDataSize
      );
  }

  // Return transfer status.
  //
ErrorExit:
  Status = TransferToken.TransferStatus;
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Failed to send PLDM command over %s\n", __func__, mTransportName));
  }

ErrorExit2:
  if (PldmTransportHeader != NULL) {
    FreePool ((VOID *)PldmTransportHeader);
  }

  if (PldmTransportTrailer != NULL) {
    FreePool ((VOID *)PldmTransportTrailer);
  }

  if (ThisRequestData != NULL) {
    FreePool ((VOID *)ThisRequestData);
  }

  if (FullPacketResponseData != NULL) {
    FreePool ((VOID *)FullPacketResponseData);
  }

  //
  // Update PLDM message instance ID.
  mPldmRequestInstanceId++;
  mPldmRequestInstanceId &= PLDM_MESSAGE_HEADER_INSTANCE_ID_MASK;
  return Status;
}
