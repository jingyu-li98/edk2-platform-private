/** @file
  Interface header file for the CRC library class.

  Copyright 2016 - 2018 Intel Corporation. <BR>
  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef CRC_LIB_H_
#define CRC_LIB_H_

#include <Uefi.h>

/**
  Calculate a 16-bit CRC.

  The algorithm used is MSB-first form of the ITU-T Recommendation V.41, which
  uses an initial value of 0x0000 and a polynomial of 0x1021. It is the same
  algorithm used by XMODEM.

  The output CRC location is not updated until the calculation is finished, so
  it is possible to pass a structure as the data, and the CRC field of the same
  structure as the output location for the calculated CRC. The CRC field should
  be set to zero before calling this function. Once the CRC field is updated by
  this function, running it again over the structure produces a CRC of zero.

  @param[in]  Data              A pointer to the target data.
  @param[in]  DataSize          The target data size.
  @param[out] CrcOut            A pointer to the return location of the CRC.

  @retval EFI_SUCCESS           The CRC was calculated successfully.
  @retval EFI_INVALID_PARAMETER A null pointer was provided.
**/
EFI_STATUS
CalculateCrc16 (
  IN  VOID    *Data,
  IN  UINTN   DataSize,
  OUT UINT16  *CrcOut
  );

/**
  Calculate CRC16 for target data.

  @param[in]  Data              The target data.
  @param[in]  DataSize          The target data size.
  @param[in]  Crc               Initial CRC.

  @return UINT16     The CRC16 value.

**/
UINT16
GenerateCrc16 (
  IN UINT8   *Data,
  IN UINTN   DataSize,
  IN UINT16  Crc
  );

#endif /* CRC_LIB_H_ */
