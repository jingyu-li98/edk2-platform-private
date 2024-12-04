/** @file
  I2C operation interfaces.

  Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef I2C_LIB_H_
#define I2C_LIB_H_

typedef struct _I2C_INFO {
  UINT64  BusAddress;
  UINT32  Speed;
  UINT32  SlaveAddress;
} I2C_INFO;

/**
  Rtc i2c Read.

  @param[in]     Info    A pointer to I2C_INFO.
  @param[in]     Addr    Address to read from
  @param[in]     Alen    Address length.
  @param[in,out] Buffer  Buffer for read data
  @param[in]     Len     Number of bytes to be read

  @retval EFI_SUCCESS    Success.
  @retval EFI_TIMEOUT    Error happened,bus time out.

 */
EFI_STATUS
I2cRead (
  IN I2C_INFO  *Info,
  IN UINT64    Addr,
  IN UINT32    Alen,
  IN UINT8     *Buffer,
  IN UINT32    Len
  );

/**
  Rtc i2c write data.

  @param[in] Info     A pointer to I2C_INFO
  @param[in] Addr     Address to read from
  @param[in] Alen     Address length.
  @param[in] Buffer   Buffer for read data
  @param[in] Len      Number of bytes to be read

  @retval EFI_SUCCESS    Success.
  @retval EFI_TIMEOUT    Error happened,bus time out.

**/
EFI_STATUS
I2cWrite (
  IN I2C_INFO  *Info,
  IN UINT64    Addr,
  IN UINT32    Alen,
  IN UINT8     *Buffer,
  IN UINT32    Len
  );


/**
  I2c controller init.

  @param[in] Info     A pointer to I2C_INFO

  @retval     Null.
**/
VOID
I2cInit (
  IN I2C_INFO  *Info
  );


VOID
SpdSetpage (
  IN I2C_INFO  *Info,
  IN UINT32    PageNum
  );


/**
  Fix and set i2c speed to bus, standard, high or fast.

  @param[in]  Info  A pointer to I2C_INFO

  @retval EFI_SCEESSS    Success.
**/
EFI_STATUS
I2cSetBusSpeed (
  IN I2C_INFO  *Info
  );


#endif /* I2C_Lib_H_ */
