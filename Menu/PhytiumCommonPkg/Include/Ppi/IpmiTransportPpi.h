/** @file
IPMI Ttransport PPI Header File.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 2014 - 2021 Intel Corporation. <BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef  IPMI_TRANSPORT_PPI_H_
#define  IPMI_TRANSPORT_PPI_H_

typedef struct _PEI_IPMI_TRANSPORT_PPI PEI_IPMI_TRANSPORT_PPI;

#define PEI_IPMI_TRANSPORT_PPI_GUID \
  { \
    0x5DF1B856, 0xDeB0, 0x4C84,  0x97, 0x83, 0x6A, 0xAF, 0x4B, 0x58, 0x58, 0x8A \
  }
//
// Common Defines
//
typedef UINT32  BMC_STATUS;

#define BMC_OK        0
#define BMC_SOFTFAIL  1
#define BMC_HARDFAIL  2
#define BMC_UPDATE_IN_PROGRESS  3
#define BMC_NOTREADY  4

//
//  IPMI Function Prototypes
//
typedef
EFI_STATUS
(EFIAPI *PEI_IPMI_SEND_COMMAND) (
  IN PEI_IPMI_TRANSPORT_PPI            *This,
  IN UINT8                             NetFunction,
  IN UINT8                             Lun,
  IN UINT8                             Command,
  IN UINT8                             *CommandData,
  IN UINT8                             CommandDataSize,
  OUT UINT8                            *ResponseData,
  OUT UINT8                            *ResponseDataSize
  );

/**
  Updates the  Bmc Status.

  @param[in]   This         Pointer to PEI_IPMI_TRANSPORT_PPI.
  @param[OUT]  BmcStatus    BMC status.

  @retval  EFI_SUCCESS            Return Successly.
  @retval  Other                  Failure.

**/
typedef
EFI_STATUS
(EFIAPI *PEI_IPMI_GET_CHANNEL_STATUS) (
  IN PEI_IPMI_TRANSPORT_PPI       *This,
  OUT BMC_STATUS                  *BmcStatus
  );

//
// IPMI TRANSPORT PPI
//
struct _PEI_IPMI_TRANSPORT_PPI {
  UINT64                      Revision;
  PEI_IPMI_SEND_COMMAND       IpmiSubmitCommand;
  PEI_IPMI_GET_CHANNEL_STATUS GetBmcStatus;
};

extern EFI_GUID gPeiIpmiTransportPpiGuid;

#endif
