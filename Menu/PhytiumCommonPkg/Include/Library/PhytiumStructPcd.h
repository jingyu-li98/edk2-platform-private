/** @file
  Header file for storing structure type PCD, this is where you can
  define the structures you want, such as I2C_BUS_INFO following.

  Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef PHYTIUM_STRUCT_PCD__LIB_H_
#define PHYTIUM_STRUCT_PCD__LIB_H_

#define PHYTIUM_I2C_BUS_MAX  64
#define PHYTIUM_SPI_BUS_MAX  64


typedef struct _I2C_BUS_INFO {
  UINT32  I2cControllerNumber;
  UINT64  I2cControllerAddress[PHYTIUM_I2C_BUS_MAX];
} I2C_BUS_INFO;


typedef struct _SPI_BUS_INFO {
  UINT32  SpiControllerNumber;
  UINT64  SpiControllerAddress[PHYTIUM_SPI_BUS_MAX];
} SPI_BUS_INFO;


#endif /* PHYTIUM_STRUCTUREPCD__LIB_H_ */
